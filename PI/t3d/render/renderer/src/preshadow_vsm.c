#include "preshadow_vsm.h"

#include <pi_vector3.h>
#include <pi_spatial.h>
#include <rendersystem.h>
#include <entity.h>
#include <material.h>
#include <renderutil.h>
#include <environment.h>

/**
 * VSMÒõÓ°Í¼äÖÈ¾Æ÷
 */
typedef struct
{
	PiVector3 light_dir;
	char* entity_list_name;
	char* vsm_data_name;
	char* view_cam_name;
	char* shadow_cam_name;
	char* env_name;
	uint shadow_map_size;
	PiRenderTarget* shadow_rt[2];
	PiTexture* shadow_map[2];
	PiRenderView* shadow_map_view[2];
	PiRenderView* depth_view[2];
	uint current_target_index;

	VSMShadowPipelineData vsm_data;

	uint blur_pass;
	PiMesh *blur_mesh;
	PiRenderMesh *blur_rmesh;
	PiEntity *blur_entity;
	PiMaterial *blur_material;
	PiCamera *blur_camera;
	SamplerState ss_blur;

	char *U_SrcTex;
	char *U_SrcSize;
	
	PiBool is_deploy;
} PreShadowRenderer;


static void _type_check(PiRenderer* renderer)
{
	PI_ASSERT((renderer)->type == ERT_PRE_SHADOW_VSM, "Renderer type error!");
}

static PiBool _init(PiRenderer* renderer, PiHash* resources)
{
	PreShadowRenderer* impl;
	uint size[2];

	PI_USE_PARAM(resources);

	impl = (PreShadowRenderer*)renderer->impl;

	if(!impl->is_deploy)
	{
		return FALSE;
	}

	impl->U_SrcTex = pi_conststr("u_SrcTex");
	impl->U_SrcSize = pi_conststr("u_SrcSize");

	impl->shadow_rt[0] = pi_rendertarget_new(TT_MRT, TRUE);
	impl->shadow_map[0] = pi_texture_2d_create(RF_GR32F, TU_COLOR, 1, 1, impl->shadow_map_size, impl->shadow_map_size, TRUE);
	impl->shadow_map_view[0] = pi_renderview_new_tex2d(RVT_COLOR, impl->shadow_map[0], 0, 0, TRUE);
	impl->depth_view[0] = pi_renderview_new(RVT_DEPTH_STENCIL, impl->shadow_map_size, impl->shadow_map_size, RF_D16, TRUE);
	pi_rendertarget_attach(impl->shadow_rt[0], ATT_DEPTHSTENCIL, impl->depth_view[0]);
	pi_rendertarget_attach(impl->shadow_rt[0], ATT_COLOR0, impl->shadow_map_view[0]);
	pi_rendertarget_set_viewport(impl->shadow_rt[0], 0, 0, impl->shadow_map_size, impl->shadow_map_size);

	impl->shadow_rt[1] = pi_rendertarget_new(TT_MRT, TRUE);
	impl->shadow_map[1] = pi_texture_2d_create(RF_GR32F, TU_COLOR, 1, 1, impl->shadow_map_size, impl->shadow_map_size, TRUE);
	impl->shadow_map_view[1] = pi_renderview_new_tex2d(RVT_COLOR, impl->shadow_map[1], 0, 0, TRUE);
	impl->depth_view[1] = pi_renderview_new(RVT_DEPTH_STENCIL, impl->shadow_map_size, impl->shadow_map_size, RF_D16, TRUE);
	pi_rendertarget_attach(impl->shadow_rt[1], ATT_DEPTHSTENCIL, impl->depth_view[1]);
	pi_rendertarget_attach(impl->shadow_rt[1], ATT_COLOR0, impl->shadow_map_view[1]);
	pi_rendertarget_set_viewport(impl->shadow_rt[1], 0, 0, impl->shadow_map_size, impl->shadow_map_size);

	impl->blur_mesh = pi_mesh_create_quad(NULL, NULL, 0.0f);
	impl->blur_rmesh = pi_rendermesh_new(impl->blur_mesh, TRUE);

	impl->blur_material = pi_material_new(RS_BOX_BLUR_VS, RS_BOX_BLUR_FS);
	
	impl->blur_entity = pi_entity_new();

	pi_renderstate_set_default_sampler(&impl->ss_blur);
	pi_sampler_set_filter(&impl->ss_blur, TFO_MIN_POINT_MAG_LINEAR);
	size[0] = impl->shadow_map_size;
	size[1] = impl->shadow_map_size;
	pi_material_set_uniform(impl->blur_material, impl->U_SrcSize, UT_IVEC2, 1, &size, TRUE);
	pi_material_set_depth_enable(impl->blur_material, FALSE);
	pi_material_set_depthwrite_enable(impl->blur_material, FALSE);
	
	pi_entity_set_mesh(impl->blur_entity, impl->blur_rmesh);
	pi_entity_set_material(impl->blur_entity, impl->blur_material);

	pi_spatial_set_local_scaling(impl->blur_entity->spatial, (float)impl->shadow_map_size, (float)impl->shadow_map_size, 1.0f);
	pi_spatial_update(impl->blur_entity->spatial);

	impl->blur_camera = pi_camera_new();
	pi_camera_set_location(impl->blur_camera, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->blur_camera, 0.0f, 0.0f, -1.0f);
	pi_camera_set_frustum(impl->blur_camera, -(float)impl->shadow_map_size / 2.0f, impl->shadow_map_size / 2.0f, -(float)impl->shadow_map_size / 2.0f, (float)impl->shadow_map_size / 2.0f, 0.0f, 2.0f, TRUE);

	return TRUE;
}

static void _draw(PiRenderer* renderer, float tpf, PiHash* resources)
{
	uint i;
	PreShadowRenderer* impl;
	PiColor background;
	PiVector* entity_list;
	PiCamera* shadow_camera;
	PI_USE_PARAM(tpf);
	_type_check(renderer);
	impl = (PreShadowRenderer*)renderer->impl;
	impl->current_target_index = 0;
	pi_rendersystem_set_target(impl->shadow_rt[impl->current_target_index]);
	color_set(&background, 1.0f, 1.0f, 1.0f, 1.0f);
	pi_rendersystem_clearview(TBM_COLOR | TBM_DEPTH, &background, 1.0f, 0);

	pi_hash_lookup(resources, impl->shadow_cam_name, (void**)&shadow_camera);
	pi_hash_lookup(resources, impl->entity_list_name, (void**)&entity_list);

	pi_rendersystem_set_camera(shadow_camera);
	pi_entity_draw_list(entity_list);

	pi_rendersystem_set_camera(impl->blur_camera);

	for(i = 0; i < impl->blur_pass; i++)
	{
		pi_sampler_set_texture(&impl->ss_blur, impl->shadow_map[impl->current_target_index]);
		pi_material_set_uniform(impl->blur_material, impl->U_SrcTex, UT_SAMPLER_2D, 1, &impl->ss_blur, TRUE);
		impl->current_target_index = !impl->current_target_index;
		pi_rendersystem_set_target(impl->shadow_rt[impl->current_target_index]);

		pi_entity_draw(impl->blur_entity);
	}
}

static void _update(PiRenderer* renderer, float tpf, PiHash* resources)
{
	PreShadowRenderer* impl;
	uint i;
	PiCamera* view_camera;
	PiCamera* shadow_camera;
	PiEnvironment *env;
	PI_USE_PARAM(tpf);
	impl = (PreShadowRenderer*)renderer->impl;
	impl->current_target_index = 0;

	pi_hash_lookup(resources, impl->view_cam_name, (void**)&view_camera);
	pi_hash_lookup(resources, impl->shadow_cam_name, (void**)&shadow_camera);
	pi_hash_lookup(resources, impl->env_name, (void**)&env);
	pi_vec3_set(&impl->light_dir, env->diffuse_dir.x, env->diffuse_dir.y, env->diffuse_dir.z);

	pi_mat4_copy(&impl->vsm_data.shadow_matrix, pi_camera_get_view_matrix(view_camera));
	pi_mat4_inverse(&impl->vsm_data.shadow_matrix, &impl->vsm_data.shadow_matrix);
	pi_mat4_mul(&impl->vsm_data.shadow_matrix, pi_camera_get_view_projection_matrix(shadow_camera), &impl->vsm_data.shadow_matrix);

	for(i = 0; i < impl->blur_pass; i++)
	{
		impl->current_target_index = !impl->current_target_index;
	}
	impl->vsm_data.shadow_map = impl->shadow_map[impl->current_target_index];
	pi_hash_insert(resources, impl->vsm_data_name, &impl->vsm_data);
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	PI_USE_PARAM(renderer);
	PI_USE_PARAM(width);
	PI_USE_PARAM(height);
}

PiRenderer* PI_API pi_preshadow_vsm_new()
{
	PiRenderer* renderer;
	PreShadowRenderer* impl = pi_new0(PreShadowRenderer, 1);

	impl->shadow_map_size = 1024;
	impl->vsm_data.shadow_z_far = 25;
	impl->blur_pass = 1;

	renderer = pi_renderer_create(ERT_PRE_SHADOW_VSM, "pre shadow vsm", _init, _resize, _update, _draw, impl);
	return renderer;
}

void PI_API pi_preshadow_vsm_deploy(PiRenderer* renderer, char* shadow_data_name, char* view_cam_name, char* shadow_cam_name, char* entity_list_name, char *env_name)
{
	PreShadowRenderer* impl;
	_type_check(renderer);
	impl = (PreShadowRenderer*)renderer->impl;

	pi_free(impl->vsm_data_name);
	pi_free(impl->shadow_cam_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->entity_list_name);
	pi_free(impl->env_name);
	impl->shadow_cam_name = pi_str_dup(shadow_cam_name);
	impl->view_cam_name = pi_str_dup(view_cam_name);
	impl->entity_list_name = pi_str_dup(entity_list_name);
	impl->vsm_data_name = pi_str_dup(shadow_data_name);
	impl->env_name = pi_str_dup(env_name);


	impl->is_deploy = TRUE;
}

void PI_API pi_preshadow_vsm_free(PiRenderer* renderer)
{
	PreShadowRenderer* impl;
	_type_check(renderer);
	impl = (PreShadowRenderer*)renderer->impl;

	pi_free(impl->vsm_data_name);
	pi_free(impl->shadow_cam_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->entity_list_name);
	pi_rendertarget_free(impl->shadow_rt[0]);
	pi_rendertarget_free(impl->shadow_rt[1]);
	pi_texture_free(impl->shadow_map[0]);
	pi_texture_free(impl->shadow_map[1]);
	pi_renderview_free(impl->shadow_map_view[0]);
	pi_renderview_free(impl->shadow_map_view[1]);
	pi_renderview_free(impl->depth_view[0]);
	pi_renderview_free(impl->depth_view[1]);

	pi_mesh_free(impl->blur_mesh);
	pi_rendermesh_free(impl->blur_rmesh);
	pi_entity_free(impl->blur_entity);
	pi_material_free(impl->blur_material);
	pi_camera_free(impl->blur_camera);

	pi_free(renderer->impl);
	pi_renderer_destroy(renderer);
}

void PI_API pi_preshadow_vsm_update_camera(PiRenderer* renderer, PiCamera* view_cam, PiCamera* shadow_cam)
{
	PreShadowRenderer* impl;
	_type_check(renderer);
	impl = (PreShadowRenderer*)renderer->impl;
	pi_camera_set_up(shadow_cam, 0, 0, -1);
	pi_camera_set_direction(shadow_cam, -impl->light_dir.x, -impl->light_dir.y, -impl->light_dir.z);
	renderutil_update_shadow_cam(view_cam, shadow_cam, (float)impl->shadow_map_size, (float)impl->shadow_map_size, impl->vsm_data.shadow_z_far);
}

void PI_API pi_preshadow_vsm_set_shadow_mapsize(PiRenderer* renderer, uint size)
{
	PreShadowRenderer* impl;
	_type_check(renderer);
	impl = (PreShadowRenderer*)renderer->impl;
	impl->shadow_map_size = size;
}

void PI_API pi_preshadow_vsm_set_zfar(PiRenderer* renderer, float z_far)
{
	PreShadowRenderer* impl;
	_type_check(renderer);
	impl = (PreShadowRenderer*)renderer->impl;
	impl->vsm_data.shadow_z_far = z_far;
}

void PI_API pi_preshadow_vsm_set_blur_pass(PiRenderer* renderer, uint num)
{
	PreShadowRenderer* impl;
	_type_check(renderer);
	impl = (PreShadowRenderer*)renderer->impl;
	impl->blur_pass = num;
}
