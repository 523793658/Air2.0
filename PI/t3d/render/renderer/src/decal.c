#include "decal.h"

#include <pi_vector3.h>
#include <pi_spatial.h>
#include <rendersystem.h>
#include <entity.h>
#include <material.h>
#include <renderutil.h>

/**
 * DecaläÖÈ¾Æ÷
 */
typedef struct
{
	char *entity_list_name;
	char *decal_map_name;
	char *decal_matrix_name;
	char *decal_z_far_name;
	char *view_cam_name;
	char *decal_cam_name;
	PiBool is_deploy;
	uint decal_map_size;
	float z_far[4];
	PiRenderTarget *decal_rt;
	PiTexture *decal_map;
	PiRenderView *decal_map_view;
	PiRenderView *depth_view;
	PiMatrix4 decal_matrix;
	PiBool is_cleanly;
	PiBool need_draw;
} DecalRenderer;

static void _type_check(PiRenderer *renderer)
{
	PI_ASSERT((renderer)->type == ERT_DECAL, "Renderer type error!");
}

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	DecalRenderer *impl;
	impl = (DecalRenderer *)renderer->impl;

	PI_USE_PARAM(resources);

	if (!impl->is_deploy)
	{
		return FALSE;
	}

	impl->decal_rt = pi_rendertarget_new(TT_MRT, TRUE);
	impl->decal_map = pi_texture_2d_create(RF_ABGR16F, TU_COLOR, 1, 1, impl->decal_map_size, impl->decal_map_size, TRUE);

	impl->decal_map_view = pi_renderview_new_tex2d(RVT_COLOR, impl->decal_map, 0, 0, TRUE);
	impl->depth_view = pi_renderview_new(RVT_DEPTH_STENCIL, impl->decal_map_size, impl->decal_map_size, RF_D16, TRUE);

	pi_rendertarget_attach(impl->decal_rt, ATT_DEPTHSTENCIL, impl->depth_view);
	pi_rendertarget_attach(impl->decal_rt, ATT_COLOR0, impl->decal_map_view);
	pi_rendertarget_set_viewport(impl->decal_rt, 0, 0, impl->decal_map_size, impl->decal_map_size);

	return TRUE;
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	DecalRenderer *impl;
	PiColor background;
	PiVector *entity_list;
	PiCamera *decal_camera;
	PI_USE_PARAM(resources);
	PI_USE_PARAM(tpf);
	_type_check(renderer);
	impl = (DecalRenderer *)renderer->impl;
	pi_hash_lookup(resources, impl->entity_list_name, (void **)&entity_list);
	if (!impl->need_draw && impl->is_cleanly)
	{
		return;
	}
	pi_hash_lookup(resources, impl->decal_cam_name, (void **)&decal_camera);
	pi_rendersystem_set_target(impl->decal_rt);
	color_set(&background, 0.0f, 0.0f, 0.0f, 1.0f);
	pi_rendersystem_clearview(TBM_COLOR | TBM_DEPTH, &background, 1.0f, 0);
	impl->is_cleanly = TRUE;
	if (impl->need_draw)
	{
		pi_rendersystem_set_camera(decal_camera);
		pi_entity_draw_list(entity_list);
		impl->is_cleanly = FALSE;
	}
}

static void _update(PiRenderer *renderer, float tpf, PiHash *resources)
{
	DecalRenderer *impl;
	PiCamera *view_camera;
	PiCamera *decal_camera;
	PiVector *entity_list;
	PI_USE_PARAM(tpf);
	impl = (DecalRenderer *)renderer->impl;
	pi_hash_lookup(resources, impl->entity_list_name, (void **)&entity_list);
	impl->need_draw = pi_vector_size(entity_list) > 0;
	if (impl->need_draw)
	{
		pi_hash_lookup(resources, impl->view_cam_name, (void **)&view_camera);
		pi_hash_lookup(resources, impl->decal_cam_name, (void **)&decal_camera);
		pi_mat4_copy(&impl->decal_matrix, pi_camera_get_view_matrix(view_camera));
		pi_mat4_inverse(&impl->decal_matrix, &impl->decal_matrix);
		pi_mat4_mul(&impl->decal_matrix, pi_camera_get_view_projection_matrix(decal_camera), &impl->decal_matrix);
		pi_hash_insert(resources, impl->decal_map_name, impl->decal_map);
		pi_hash_insert(resources, impl->decal_matrix_name, &impl->decal_matrix);
		pi_hash_insert(resources, impl->decal_z_far_name, &impl->z_far);
	}
	else{
		pi_hash_insert(resources, impl->decal_map_name, NULL);
	}
}

PiRenderer *PI_API pi_decal_new_with_name(char* name)
{
	PiRenderer *renderer;
	DecalRenderer *impl = pi_new0(DecalRenderer, 1);
	impl->decal_map_size = 512;
	impl->z_far[0] = 100;

	renderer = pi_renderer_create(ERT_DECAL, name, _init, NULL, _update, _draw, impl);
	return renderer;
}

PiRenderer *PI_API pi_decal_new()
{
	return pi_decal_new_with_name("decal");
}

void PI_API pi_decal_deploy(PiRenderer *renderer, char *decal_map_name, char *decal_matrix_name,  char *decal_z_far_name, char *view_cam_name, char *decal_cam_name, char *entity_list_name)
{
	DecalRenderer *impl;
	_type_check(renderer);
	impl = (DecalRenderer *)renderer->impl;

	pi_free(impl->decal_cam_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->entity_list_name);
	pi_free(impl->decal_map_name);
	pi_free(impl->decal_matrix_name);
	pi_free(impl->decal_z_far_name);

	impl->view_cam_name = pi_str_dup(view_cam_name);
	impl->decal_cam_name = pi_str_dup(decal_cam_name);
	impl->entity_list_name = pi_str_dup(entity_list_name);
	impl->decal_map_name = pi_str_dup(decal_map_name);
	impl->decal_matrix_name = pi_str_dup(decal_matrix_name);
	impl->decal_z_far_name = pi_str_dup(decal_z_far_name);

	impl->is_deploy = TRUE;
}

void PI_API pi_decal_free(PiRenderer *renderer)
{
	DecalRenderer *impl;
	_type_check(renderer);
	impl = (DecalRenderer *)renderer->impl;
	pi_rendertarget_free(impl->decal_rt);
	pi_texture_free(impl->decal_map);
	pi_renderview_free(impl->depth_view);
	pi_renderview_free(impl->decal_map_view);
	pi_free(impl->decal_cam_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->entity_list_name);
	pi_free(impl->decal_map_name);
	pi_free(impl->decal_matrix_name);
	pi_free(impl->decal_z_far_name);

	pi_free(renderer->impl);
	pi_renderer_destroy(renderer);
}

void PI_API pi_decal_update_camera(PiRenderer *renderer, PiCamera *view_cam, PiCamera *decal_cam)
{
	DecalRenderer *impl;
	_type_check(renderer);
	impl = (DecalRenderer *)renderer->impl;
	pi_camera_set_up(decal_cam, 0, 0, -1);
	pi_camera_set_direction(decal_cam, 0, -1, 0);
	renderutil_update_shadow_cam(view_cam, decal_cam, (float)impl->decal_map_size, (float)impl->decal_map_size, impl->z_far[0]);
}

void PI_API pi_decal_set_mapsize(PiRenderer *renderer, uint size)
{
	DecalRenderer *impl;
	_type_check(renderer);
	impl = (DecalRenderer *)renderer->impl;
	impl->decal_map_size = size;
}

void PI_API pi_decal_set_zfar(PiRenderer *renderer, float z_far)
{
	DecalRenderer *impl;
	_type_check(renderer);
	impl = (DecalRenderer *)renderer->impl;
	impl->z_far[0] = z_far;
}
