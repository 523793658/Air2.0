#include "sky_box.h"
#include <pi_spatial.h>
#include <entity.h>
#include <rendertarget.h>
#include <rendersystem.h>

/**
 * 天空盒渲染器
 */
typedef struct
{
	PiMesh *mesh;
	PiRenderMesh *rmesh;
	PiEntity *entity;

	char *view_cam_name;
	char *output_name;
	SamplerState ss_src;

	PiTexture *env_map;
	PiMaterial *material;
	PiBool is_deploy;
	PiBool is_cylinder_mapping;

	/* 常量字符串 */
	char *CYLINDER_MAP;
	char *U_EnvMap;
} SkyRenderer;

static void _type_check(PiRenderer *renderer)
{
	PI_ASSERT((renderer)->type == ERT_SKY, "Renderer type error!");
}

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	SkyRenderer *impl = (SkyRenderer *)renderer->impl;

	if (!impl->is_deploy || impl->env_map == NULL)
	{
		return FALSE;
	}
	impl->CYLINDER_MAP = pi_conststr("CYLINDER_MAP");
	impl->U_EnvMap = pi_conststr("u_EnvMap");

	impl->mesh = pi_mesh_create_cube(NULL, NULL);
	impl->rmesh = pi_rendermesh_new(impl->mesh, TRUE);

	impl->material = pi_material_new(RS_SKY_VS, RS_SKY_FS);
	impl->entity = pi_entity_new();

	pi_renderstate_set_default_sampler(&impl->ss_src);
	pi_sampler_set_filter(&impl->ss_src, TFO_MIN_MAG_LINEAR);
	pi_sampler_set_addr_mode(&impl->ss_src, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);

	pi_material_set_depthwrite_enable(impl->material, FALSE);
	pi_material_set_cull_mode(impl->material, CM_FRONT);

	pi_entity_set_mesh(impl->entity, impl->rmesh);
	pi_entity_set_material(impl->entity, impl->material);

	pi_spatial_set_local_scaling(impl->entity->spatial, 750, 750, 750);
	pi_spatial_update(impl->entity->spatial);

	pi_sampler_set_texture(&impl->ss_src, impl->env_map);

	pi_material_set_def(impl->material, impl->CYLINDER_MAP, impl->is_cylinder_mapping);
	if (impl->is_cylinder_mapping)
	{
		pi_material_set_uniform(impl->material, impl->U_EnvMap, UT_SAMPLER_2D, 1, &impl->ss_src, TRUE);
	}
	else
	{
		pi_material_set_uniform(impl->material, impl->U_EnvMap, UT_SAMPLER_CUBE, 1, &impl->ss_src, TRUE);
	}

	return TRUE;
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	SkyRenderer *impl = (SkyRenderer *)renderer->impl;
	PiRenderTarget *target = NULL;
	PiCamera *camera = NULL;
	PI_USE_PARAM(tpf);
	_type_check(renderer);

	if (impl->env_map == NULL)
	{
		return;
	}

	pi_hash_lookup(resources, impl->output_name, (void **)&target);
	pi_hash_lookup(resources, impl->view_cam_name, (void **)&camera);
	pi_rendersystem_set_target(target);

	pi_rendersystem_set_camera(camera);

	pi_entity_draw(impl->entity);
}

static void _update(PiRenderer *renderer, float tpf, PiHash *resources)
{
	SkyRenderer *impl;
	PiCamera *camera = NULL;
	PiVector3 *cam_pos;
	PI_USE_PARAM(tpf);
	impl = (SkyRenderer *)renderer->impl;
	pi_hash_lookup(resources, impl->view_cam_name, (void **)&camera);

	cam_pos = pi_camera_get_location(camera);
	pi_spatial_set_local_translation(impl->entity->spatial, cam_pos->x, cam_pos->y, cam_pos->z);
	pi_spatial_update(impl->entity->spatial);
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	PI_USE_PARAM(renderer);
	PI_USE_PARAM(width);
	PI_USE_PARAM(height);
	//TODO:加入resize实现
}


PiRenderer* PI_API pi_sky_box_new_with_name(char* name)
{
	PiRenderer *renderer;
	SkyRenderer *impl = pi_new0(SkyRenderer, 1);

	renderer = pi_renderer_create(ERT_SKY, name, _init, _resize, _update, _draw, impl);
	return renderer;
}

PiRenderer *PI_API pi_sky_box_new()
{
	return pi_sky_box_new_with_name("sky");
}

void PI_API pi_sky_box_deploy(PiRenderer *renderer, char *view_cam_name, char *output_name)
{
	SkyRenderer *impl;
	_type_check(renderer);
	impl = (SkyRenderer *)renderer->impl;

	pi_free(impl->view_cam_name);
	pi_free(impl->output_name);

	impl->view_cam_name = pi_str_dup(view_cam_name);
	impl->output_name = pi_str_dup(output_name);

	impl->is_deploy = TRUE;
}

void PI_API pi_sky_box_free(PiRenderer *renderer)
{
	SkyRenderer *impl;
	_type_check(renderer);
	impl = (SkyRenderer *)renderer->impl;

	pi_mesh_free(impl->mesh);
	pi_rendermesh_free(impl->rmesh);
	pi_entity_free(impl->entity);
	pi_material_free(impl->material);
	pi_free(impl->view_cam_name);
	pi_free(impl->output_name);

	pi_free(renderer->impl);
	pi_renderer_destroy(renderer);
}

void PI_API pi_sky_box_set_env_map(PiRenderer *renderer, PiTexture *cube_map, PiBool is_cylinder)
{
	SkyRenderer *impl;
	_type_check(renderer);
	impl = (SkyRenderer *)renderer->impl;

	impl->env_map = cube_map;
	impl->is_cylinder_mapping = is_cylinder;

	if (impl->material != NULL)
	{
		pi_sampler_set_texture(&impl->ss_src, impl->env_map);
		pi_material_set_def(impl->material, impl->CYLINDER_MAP, is_cylinder);
		if (is_cylinder)
		{
			pi_material_set_uniform(impl->material, impl->U_EnvMap, UT_SAMPLER_2D, 1, &impl->ss_src, TRUE);
		}
		else
		{
			pi_material_set_uniform(impl->material, impl->U_EnvMap, UT_SAMPLER_CUBE, 1, &impl->ss_src, TRUE);
		}
	}
}