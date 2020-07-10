
#include "text_renderer.h"
#include "text.h"
#include "rendersystem.h"

/**
 * TextäÖÈ¾Æ÷
 */
typedef struct
{
	char *text_list_name;
	char *target_name;
	char *view_cam_name;

	PiBool is_deploy;
} TextRenderer;

static void _type_check(PiRenderer *renderer)
{
	PI_ASSERT(renderer->type == ERT_TEXT_RENDERER, "Renderer type error!");
}

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	TextRenderer *impl = (TextRenderer *)renderer->impl;

	PI_USE_PARAM(resources);

	if (!impl->is_deploy)
	{
		return FALSE;
	}

	return TRUE;
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	TextRenderer *impl;
	PiRenderTarget *target;
	PiCamera *view_camera;
	PiVector *text_list;
	PI_USE_PARAM(tpf);
	_type_check(renderer);
	impl = (TextRenderer *)renderer->impl;

	pi_hash_lookup(resources, impl->target_name, (void **)&target);
	pi_hash_lookup(resources, impl->view_cam_name, (void **)&view_camera);
	pi_hash_lookup(resources, impl->text_list_name, (void **)&text_list);

	pi_rendersystem_set_target(target);
	pi_rendersystem_set_camera(view_camera);

	{
		uint i, num;
		num = pi_vector_size(text_list);
		for (i = 0; i < num; i++)
		{
			PiText *text = pi_vector_get(text_list, i);
			PiEntity *entity = pi_text_entity_get(text);
			if (entity)
			{
				pi_entity_draw(entity);
			}
		}
	}
}

static void _update(PiRenderer *renderer, float tpf, PiHash *resources)
{
	PI_USE_PARAM(renderer);
	PI_USE_PARAM(tpf);
	PI_USE_PARAM(resources);
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	PI_USE_PARAM(renderer);
	PI_USE_PARAM(width);
	PI_USE_PARAM(height);
}

PiRenderer *PI_API pi_text_renderer_new()
{
	PiRenderer *renderer;
	TextRenderer *impl = pi_new0(TextRenderer, 1);

	renderer = pi_renderer_create(ERT_TEXT_RENDERER, "text", _init, _resize, _update, _draw, impl);
	return renderer;
}

void PI_API pi_text_renderer_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *text_list_name)
{
	TextRenderer *impl;
	_type_check(renderer);
	impl = (TextRenderer *)renderer->impl;
	pi_free(impl->target_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->text_list_name);
	impl->target_name = pi_str_dup(target_name);
	impl->view_cam_name = pi_str_dup(view_cam_name);
	impl->text_list_name = pi_str_dup(text_list_name);

	impl->is_deploy = TRUE;
}

void PI_API pi_text_renderer_delete(PiRenderer *renderer)
{
	TextRenderer *impl;
	_type_check(renderer);
	impl = (TextRenderer *)renderer->impl;
	pi_free(impl->target_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->text_list_name);
	pi_free(renderer->impl);

	pi_renderer_destroy(renderer);
}
