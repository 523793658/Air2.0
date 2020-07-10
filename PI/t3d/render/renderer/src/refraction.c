
#include "refraction.h"
#include <entity.h>
#include <rendertarget.h>
#include <rendersystem.h>

/**
 * ÕÛÉäÎïÌåäÖÈ¾Æ÷
 */
typedef struct
{
	SamplerState scene_color_sampler;

	char *target_name;
	char *view_cam_name;
	char *entity_list_name;
	char *scene_color_name;

	PiBool is_deploy;

	/* ³£Á¿×Ö·û´® */
	char *U_SceneColorTex;
} RefractionRenderer;

static void _type_check(PiRenderer *renderer)
{
	PI_ASSERT(renderer->type == ERT_REFRACTION, "Renderer type error!");
}

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	uint32 width, height;
	PiRenderTarget *target;
	RefractionRenderer *impl = (RefractionRenderer *)renderer->impl;

	PI_USE_PARAM(resources);

	if (!impl->is_deploy)
	{
		return FALSE;
	}

	impl->U_SceneColorTex = pi_conststr("u_SceneColorTex");

	pi_hash_lookup(resources, impl->target_name, (void **)&target);
	width = target->width;
	height = target->height;

	pi_renderstate_set_default_sampler(&impl->scene_color_sampler);
	pi_sampler_set_addr_mode(&impl->scene_color_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->scene_color_sampler, TFO_MIN_MAG_POINT);

	return TRUE;
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	RefractionRenderer *impl;
	PiRenderTarget *target;
	PiCamera *view_cam;
	PiVector *entity_list;

	PI_USE_PARAM(tpf);

	_type_check(renderer);

	impl = (RefractionRenderer *)renderer->impl;

	pi_hash_lookup(resources, impl->target_name, (void **)&target);
	pi_hash_lookup(resources, impl->view_cam_name, (void **)&view_cam);
	pi_hash_lookup(resources, impl->entity_list_name, (void **)&entity_list);

	pi_rendersystem_set_target(target);
	pi_rendersystem_set_camera(view_cam);
	pi_entity_draw_list(entity_list);
}

static void _update(PiRenderer *renderer, float tpf, PiHash *resources)
{
	RefractionRenderer *impl;
	PiTexture *scene_color_tex;
	PiVector *entity_list;
	uint n, i;

	PI_USE_PARAM(tpf);

	_type_check(renderer);

	impl = (RefractionRenderer *)renderer->impl;

	pi_hash_lookup(resources, impl->entity_list_name, (void **)&entity_list);

	pi_hash_lookup(resources, impl->scene_color_name, (void **)&scene_color_tex);
	pi_sampler_set_texture(&impl->scene_color_sampler, scene_color_tex);

	n = pi_vector_size(entity_list);

	for (i = 0; i < n; i++)
	{
		PiEntity *entity = (PiEntity *)pi_vector_get(entity_list, i);
		PiMaterial *material = entity->material;

		pi_material_set_uniform(material, impl->U_SceneColorTex, UT_SAMPLER_2D, 1, &impl->scene_color_sampler, FALSE);
	}
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	_type_check(renderer);
}

PiRenderer *PI_API pi_refraction_new_with_name(char* name)
{
	PiRenderer *renderer;
	RefractionRenderer *impl = pi_new0(RefractionRenderer, 1);

	renderer = pi_renderer_create(ERT_REFRACTION, name, _init, _resize, _update, _draw, impl);
	return renderer;
}

PiRenderer *PI_API pi_refraction_new()
{
	return pi_refraction_new_with_name("refraction");
}

void PI_API pi_refraction_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *entity_list_name, char *scene_color_name)
{
	RefractionRenderer *impl;

	_type_check(renderer);

	impl = (RefractionRenderer *)renderer->impl;

	pi_free(impl->target_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->entity_list_name);

	pi_free(impl->scene_color_name);

	impl->target_name = pi_str_dup(target_name);
	impl->view_cam_name = pi_str_dup(view_cam_name);
	impl->entity_list_name = pi_str_dup(entity_list_name);

	impl->scene_color_name = pi_str_dup(scene_color_name);

	impl->is_deploy = TRUE;
}

void PI_API pi_refraction_free(PiRenderer *renderer)
{
	RefractionRenderer *impl;
	_type_check(renderer);
	impl = (RefractionRenderer *)renderer->impl;

	pi_free(impl->target_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->entity_list_name);

	pi_free(impl->scene_color_name);

	pi_free(renderer->impl);
	pi_renderer_destroy(renderer);
}
