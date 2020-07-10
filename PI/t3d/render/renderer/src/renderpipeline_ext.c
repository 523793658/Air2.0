#include <renderpipeline_ext.h>
#include <renderinfo.h>
#include <rendersystem.h>

static PiBool PI_API _str_equal(const char* str1, const char* str2)
{
	return pi_str_equal(str1, str2, FALSE);
}

PiRenderPipelineExt* PI_API pi_renderpipeline_ext_new()
{
	PiRenderPipelineExt* pipeline = pi_new0(PiRenderPipelineExt, 1);
	pipeline->global_resources = pi_hash_new(0.75f, (PiHashFunc)pi_str_hash, (PiEqualFunc)_str_equal);
	pipeline->resources = pi_hash_new(0.75f, (PiHashFunc)pi_str_hash, (PiEqualFunc)_str_equal);
	pipeline->renderers = pi_vector_new();
	return pipeline;
}

void PI_API pi_renderpipeline_ext_free(PiRenderPipelineExt* pipeline)
{
	pi_hash_free(pipeline->global_resources);
	pi_hash_free(pipeline->resources);
	pi_vector_free(pipeline->renderers);
	pi_free(pipeline);
}

static PiSelectR	PI_API _select_func(void* user_data, void* value)
{
	PiKeyValue* kv = (PiKeyValue*)value;
	PiHash* hash = (PiHash*)user_data;
	pi_hash_insert(hash, kv->key, kv->value);
	return SELECT_NEXT;
}

static void _hash_insert_all(PiHash* dst, PiHash* src)
{
	pi_hash_foreach(src, _select_func, dst);
}

void PI_API pi_renderpipeline_ext_resize(PiRenderPipelineExt* pipeline, uint width, uint height)
{
	pipeline->width = width;
	pipeline->height = height;
	pipeline->is_resize = TRUE;
}

void PI_API pi_renderpipeline_ext_draw(PiRenderPipelineExt* pipeline, float tpf)
{
	sint i, size = pi_vector_size(pipeline->renderers);
	pi_hash_clear(pipeline->resources, FALSE);
	_hash_insert_all(pipeline->resources, pipeline->global_resources);

	for(i = 0; i < size; i++)
	{
		PiRenderer* renderer = (PiRenderer*)pi_vector_get(pipeline->renderers, i);
		pi_renderinfo_set_current_renderer_name(renderer->name);
		if(!renderer->is_init)
		{
			renderer->is_init = renderer->init_func(renderer, pipeline->resources);
			if (!renderer->is_init)
			{
				continue;
			}
		}

		if(pipeline->is_resize && renderer->system_resize)
		{
			renderer->resize_func(renderer, pipeline->width, pipeline->height);
		}

		if (renderer->is_enable)
		{
			renderer->update_func(renderer, tpf, pipeline->resources);
		}
	}
	pipeline->is_resize = FALSE;

	pi_rendersystem_begin_draw();

	//更新全局环境变量的设置

	for(i = 0; i < size; i++)
	{
		PiRenderer* renderer = (PiRenderer*)pi_vector_get(pipeline->renderers, i);

		if(renderer->is_init && renderer->is_enable)
		{
			pi_renderinfo_set_current_renderer_name(renderer->name);
			renderer->draw_func(renderer, tpf, pipeline->resources);
		}
	}

	pi_rendersystem_end_draw();

	pi_renderinfo_set_current_renderer_name(NULL);
}

void PI_API pi_renderpipeline_ext_add(PiRenderPipelineExt* pipeline, PiRenderer *renderer)
{
	pi_vector_push(pipeline->renderers, renderer);
}

static PiBool PI_API _renderer_remove(void *user_data, const void *data)
{
	return user_data == data;
}

void PI_API pi_renderpipeline_ext_remove(PiRenderPipelineExt* pipeline, PiRenderer *renderer)
{
	pi_vector_remove_if(pipeline->renderers, _renderer_remove, renderer);
}

void PI_API pi_renderpipeline_ext_add_global_resource(PiRenderPipelineExt* pipeline, char* name, void* resource)
{
	char* key = pi_str_dup(name);
	pi_hash_insert(pipeline->global_resources, key, resource);
}

void PI_API pi_renderpipeline_ext_remove_global_resource(PiRenderPipelineExt* pipeline, char* name)
{
	PiKeyValue kv;
	if(pi_hash_delete(pipeline->global_resources, name, &kv))
	{
		pi_free(kv.key);
	}
}