#include <default_renderer.h>

#include <pi_vector3.h>
#include <rendersystem.h>
#include <entity.h>
#include <material.h>

/**
 * Ä¬ÈÏEntityäÖÈ¾Æ÷
 */
typedef struct
{
	char *entity_list_name;
	char *target_name;
	char *view_cam_name;
	PiBool negtive;
	PiBool is_deploy;
} DefaultRenderer;

static void _type_check(PiRenderer *renderer)
{
	PI_ASSERT(renderer->type == ERT_DEFAULT, "Renderer type error!");
}

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	DefaultRenderer *impl = (DefaultRenderer *)renderer->impl;

	PI_USE_PARAM(resources);

	if (!impl->is_deploy)
	{
		return FALSE;
	}

	return TRUE;
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	DefaultRenderer *impl;
	PiRenderTarget *target;
	PiCamera *view_camera;
	PiVector *entity_list;
	PI_USE_PARAM(tpf);
	_type_check(renderer);
	impl = (DefaultRenderer *)renderer->impl;

	pi_hash_lookup(resources, impl->entity_list_name, (void **)&entity_list);
	pi_hash_lookup(resources, impl->view_cam_name, (void **)&view_camera);
	pi_hash_lookup(resources, impl->target_name, (void **)&target);

	pi_rendersystem_set_target(target);
	pi_rendersystem_set_camera(view_camera);
	if (impl->negtive)
	{
		pi_entity_draw_list_back(entity_list);
	}
	else
	{
		pi_entity_draw_list(entity_list);
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


PiRenderer* PI_API pi_default_renderer_new_with_name(char* name)
{
	PiRenderer *renderer;
	DefaultRenderer *impl = pi_new0(DefaultRenderer, 1);

	renderer = pi_renderer_create(ERT_DEFAULT, name, _init, _resize, _update, _draw, impl);
	return renderer;
}

PiRenderer *PI_API pi_default_renderer_new()
{
	return pi_default_renderer_new_with_name("default");
}

void PI_API pi_default_renderer_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *entity_list_name)
{
	DefaultRenderer *impl;
	_type_check(renderer);
	impl = (DefaultRenderer *)renderer->impl;
	pi_free(impl->target_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->entity_list_name);
	impl->target_name = pi_str_dup(target_name);
	impl->view_cam_name = pi_str_dup(view_cam_name);
	impl->entity_list_name = pi_str_dup(entity_list_name);

	impl->is_deploy = TRUE;
}

void PI_API pi_default_renderer_free(PiRenderer *renderer)
{
	DefaultRenderer *impl;
	_type_check(renderer);
	impl = (DefaultRenderer *)renderer->impl;
	pi_free(impl->target_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->entity_list_name);
	pi_free(renderer->impl);

	pi_renderer_destroy(renderer);
}

void PI_API pi_default_renderer_set_draw_order(PiRenderer* renderer, PiBool negtive)
{
	DefaultRenderer *impl;
	_type_check(renderer);
	impl = (DefaultRenderer *)renderer->impl;
	impl->negtive = negtive;
}
