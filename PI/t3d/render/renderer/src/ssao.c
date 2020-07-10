#include "ssao.h"

#include <pi_spatial.h>
#include <entity.h>
#include <rendertarget.h>
#include <rendersystem.h>

/**
 * SSAO渲染器
 */
typedef struct
{
	PiMesh *mesh;
	PiRenderMesh *rmesh;
	PiEntity *entity;
	PiCamera *camera;

	char* depth_map_name;
	char* normal_map_name;
	char* scene_camera_name;
	char* output_name;
	SamplerState ss_src;

	PiMaterial *material;
	PiBool is_deploy;

	float reject_radius;
	float accept_radius;
	float normal_scale;
	float intensity;
	float sample_radius_scale;
	uint32 quality;
	PiBool independent_output;
	PiBool half_resolution_mode;

	PiRenderTarget* ao_rt;
	PiTexture *ao_tex;
	PiRenderView *ao_view;

	/* 常量字符串 */
	char *PRE_SET_1;
	char *PRE_SET_2;
	char *PRE_SET_3;
	char *PRE_SET_4;
	char *USE_NORMAL;
	char *HALF_RESOLUTION;

	char *U_RejectRadius;
	char *U_AcceptRadius;
	char *U_NormalScale;
	char *U_Intensity;
	char *U_SampleRadiusScale;
	char *U_RTSize;
	char *U_SrcDepthTex;
	char *U_ProjMatrixInverse;
	char *U_SrcNormalTex;
} SSAORenderer;

static void _type_check(PiRenderer* renderer)
{
	PI_ASSERT((renderer)->type == ERT_SSAO, "Renderer type error!");
}

static void _update_params(SSAORenderer* renderer)
{
	if(renderer->material == NULL)
	{
		return;
	}

	pi_material_set_uniform(renderer->material, renderer->U_RejectRadius, UT_FLOAT, 1, &renderer->reject_radius, TRUE);
	pi_material_set_uniform(renderer->material, renderer->U_AcceptRadius, UT_FLOAT, 1, &renderer->accept_radius, TRUE);
	pi_material_set_uniform(renderer->material, renderer->U_NormalScale, UT_FLOAT, 1, &renderer->normal_scale, TRUE);
	pi_material_set_uniform(renderer->material, renderer->U_Intensity, UT_FLOAT, 1, &renderer->intensity, TRUE);
	pi_material_set_uniform(renderer->material, renderer->U_SampleRadiusScale, UT_FLOAT, 1, &renderer->sample_radius_scale, TRUE);

	switch (renderer->quality)
	{
	case 1:
		pi_material_set_def(renderer->material, renderer->PRE_SET_1, TRUE);
		pi_material_set_def(renderer->material, renderer->PRE_SET_2, FALSE);
		pi_material_set_def(renderer->material, renderer->PRE_SET_3, FALSE);
		pi_material_set_def(renderer->material, renderer->PRE_SET_4, FALSE);
		break;
	case 2:
		pi_material_set_def(renderer->material, renderer->PRE_SET_1, FALSE);
		pi_material_set_def(renderer->material, renderer->PRE_SET_2, TRUE);
		pi_material_set_def(renderer->material, renderer->PRE_SET_3, FALSE);
		pi_material_set_def(renderer->material, renderer->PRE_SET_4, FALSE);
		break;
	case 3:
		pi_material_set_def(renderer->material, renderer->PRE_SET_1, FALSE);
		pi_material_set_def(renderer->material, renderer->PRE_SET_2, FALSE);
		pi_material_set_def(renderer->material, renderer->PRE_SET_3, TRUE);
		pi_material_set_def(renderer->material, renderer->PRE_SET_4, FALSE);
		break;
	case 4:
		pi_material_set_def(renderer->material, renderer->PRE_SET_1, FALSE);
		pi_material_set_def(renderer->material, renderer->PRE_SET_2, FALSE);
		pi_material_set_def(renderer->material, renderer->PRE_SET_3, FALSE);
		pi_material_set_def(renderer->material, renderer->PRE_SET_4, TRUE);
		break;
	default:
		pi_material_set_def(renderer->material, renderer->PRE_SET_1, FALSE);
		pi_material_set_def(renderer->material, renderer->PRE_SET_2, FALSE);
		pi_material_set_def(renderer->material, renderer->PRE_SET_3, FALSE);
		pi_material_set_def(renderer->material, renderer->PRE_SET_4, TRUE);
		break;
	}
}

static PiBool _init(PiRenderer* renderer, PiHash *resources)
{
	uint32 width, height;
	SSAORenderer *impl = (SSAORenderer*)renderer->impl;
	uint size[2];

	if(!impl->is_deploy)
	{
		return FALSE;
	}

	impl->PRE_SET_1 = pi_conststr("PRE_SET_1");
	impl->PRE_SET_2 = pi_conststr("PRE_SET_2");
	impl->PRE_SET_3 = pi_conststr("PRE_SET_3");
	impl->PRE_SET_4 = pi_conststr("PRE_SET_4");
	impl->USE_NORMAL = pi_conststr("USE_NORMAL");
	impl->HALF_RESOLUTION = pi_conststr("HALF_RESOLUTION");

	impl->U_RejectRadius = pi_conststr("u_RejectRadius");
	impl->U_AcceptRadius = pi_conststr("u_AcceptRadius");
	impl->U_NormalScale = pi_conststr("u_NormalScale");
	impl->U_Intensity = pi_conststr("u_Intensity");
	impl->U_SampleRadiusScale = pi_conststr("u_SampleRadiusScale");
	impl->U_RTSize = pi_conststr("u_RTSize");
	impl->U_SrcDepthTex = pi_conststr("u_SrcDepthTex");
	impl->U_ProjMatrixInverse = pi_conststr("u_ProjMatrixInverse");
	impl->U_SrcNormalTex = pi_conststr("u_SrcNormalTex");

	impl->mesh = pi_mesh_create_quad(NULL, NULL, 0.0f);
	impl->rmesh = pi_rendermesh_new(impl->mesh, TRUE);

	impl->material = pi_material_new(RS_SSAO_VS, RS_SSAO_FS);
	
	impl->entity = pi_entity_new();

	pi_renderstate_set_default_sampler(&impl->ss_src);
	pi_sampler_set_filter(&impl->ss_src, TFO_MIN_MAG_POINT);
	pi_sampler_set_addr_mode(&impl->ss_src, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_material_set_depth_enable(impl->material, FALSE);
	pi_material_set_depthwrite_enable(impl->material, FALSE);
	if(!impl->independent_output)
	{
		PiRenderTarget* target;
		pi_hash_lookup(resources, impl->output_name, (void**)&target);
		width = target->width;
		height = target->height;
		pi_material_set_blend(impl->material, TRUE);
		pi_material_set_blend_factor(impl->material,  BF_ZERO, BF_SRC_COLOR, BF_ZERO, BF_ONE);
	}
	else
	{
		PiTexture* srcTex;
		pi_hash_lookup(resources, impl->depth_map_name, (void**)&srcTex);
		width = srcTex->width;
		height = srcTex->height;
		if(impl->half_resolution_mode)
		{
			width /= 2;
			height /= 2;
			pi_sampler_set_filter(&impl->ss_src, TFO_MIN_MAG_LINEAR);
			pi_material_set_def(impl->material, impl->HALF_RESOLUTION, TRUE);
		}
		impl->ao_rt = pi_rendertarget_new(TT_MRT, TRUE);
		impl->ao_tex = pi_texture_2d_create(RF_A8, TU_COLOR, 1, 1, width, height, TRUE);
		impl->ao_view = pi_renderview_new_tex2d(RVT_COLOR, impl->ao_tex, 0, 0, TRUE);
		pi_rendertarget_attach(impl->ao_rt, ATT_COLOR0, impl->ao_view);
		pi_rendertarget_set_viewport(impl->ao_rt, 0, 0, width, height);
	}
	_update_params(impl);

	size[0] = width;
	size[1] = height;
	pi_material_set_uniform(impl->material, impl->U_RTSize, UT_IVEC2, 1, &size, TRUE);

	pi_entity_set_mesh(impl->entity, impl->rmesh);
	pi_entity_set_material(impl->entity, impl->material);

	pi_spatial_set_local_scaling(impl->entity->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->entity->spatial);

	impl->camera = pi_camera_new();

	pi_camera_set_location(impl->camera, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->camera, 0.0f, 0.0f, -1.0f);
	pi_camera_set_frustum(impl->camera, -(float)width / 2.0f, width / 2.0f, -(float)height / 2.0f, (float)height / 2.0f, 0.0f, 2.0f, TRUE);

	return TRUE;
}

static void _draw(PiRenderer* renderer, float tpf, PiHash* resources)
{
	SSAORenderer* impl = (SSAORenderer*)renderer->impl;
	PiRenderTarget* target;
	PI_USE_PARAM(tpf);
	_type_check(renderer);

	if (impl->independent_output)
	{
		target = impl->ao_rt;
	}
	else
	{
		pi_hash_lookup(resources, impl->output_name, (void**)&target);
	}
	pi_rendersystem_set_target(target);

	pi_rendersystem_set_camera(impl->camera);

	pi_entity_draw(impl->entity);
}

static void _update(PiRenderer* renderer, float tpf, PiHash* resources)
{
	SSAORenderer* impl;
	PiTexture* depth_map;
	PiTexture* normal_map = NULL;
	PiCamera* scene_camera;
	PiMatrix4 proj_mat_inverse;
	PI_USE_PARAM(tpf);
	impl = (SSAORenderer*)renderer->impl;

	pi_hash_lookup(resources, impl->depth_map_name, (void**)&depth_map);
	pi_hash_lookup(resources, impl->scene_camera_name, (void**)&scene_camera);
	pi_hash_lookup(resources, impl->normal_map_name, (void**)&normal_map);

	pi_sampler_set_texture(&impl->ss_src, depth_map);
	pi_material_set_uniform(impl->material, impl->U_SrcDepthTex, UT_SAMPLER_2D, 1, &impl->ss_src, TRUE);
	pi_mat4_inverse(&proj_mat_inverse, pi_camera_get_projection_matrix(scene_camera));
	pi_material_set_uniform(impl->material, impl->U_ProjMatrixInverse, UT_MATRIX4, 1, &proj_mat_inverse, TRUE);

	if(impl->normal_map_name != NULL && normal_map != NULL)
	{
		pi_material_set_def(impl->material, impl->USE_NORMAL, TRUE);
		pi_sampler_set_texture(&impl->ss_src, normal_map);
		pi_material_set_uniform(impl->material, impl->U_SrcNormalTex, UT_SAMPLER_2D, 1, &impl->ss_src, TRUE);
	}
	else
	{
		pi_material_set_def(impl->material, impl->USE_NORMAL, FALSE);
	}

	if(impl->independent_output)
	{
		pi_hash_insert(resources, impl->output_name, impl->ao_tex);
	}
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	PI_USE_PARAM(renderer);
	PI_USE_PARAM(width);
	PI_USE_PARAM(height);
	//TODO:加入resize实现
}


PiRenderer* PI_API pi_ssao_new()
{
	PiRenderer* renderer;
	SSAORenderer* impl = pi_new0(SSAORenderer, 1);
	impl->reject_radius = 0.8f;
	impl->accept_radius = 0.0003f;
	impl->normal_scale = 1;
	impl->intensity = 2;
	impl->sample_radius_scale = 1;
	impl->quality = 4;
	impl->independent_output = FALSE;
	impl->half_resolution_mode = FALSE;

	renderer = pi_renderer_create(ERT_SSAO, "ssao", _init, _resize, _update, _draw, impl);
	return renderer;
}

void PI_API pi_ssao_deploy(PiRenderer* renderer, char* depth_map_name, char* output_name, char* scene_camera_name)
{
	SSAORenderer* impl;
	_type_check(renderer);
	impl = (SSAORenderer*)renderer->impl;

	pi_free(impl->depth_map_name);
	pi_free(impl->output_name);
	pi_free(impl->scene_camera_name);

	impl->depth_map_name = pi_str_dup(depth_map_name);
	impl->output_name = pi_str_dup(output_name);
	impl->scene_camera_name = pi_str_dup(scene_camera_name);

	impl->is_deploy = TRUE;
}

void PI_API pi_ssao_deploy_normal(PiRenderer* renderer, char* normal_map_name)
{
	SSAORenderer* impl;
	_type_check(renderer);
	impl = (SSAORenderer*)renderer->impl;

	pi_free(impl->normal_map_name);

	impl->normal_map_name = pi_str_dup(normal_map_name);
}

void PI_API pi_ssao_free(PiRenderer* renderer)
{
	SSAORenderer* impl;
	_type_check(renderer);
	impl = (SSAORenderer*)renderer->impl;

	pi_mesh_free(impl->mesh);
	pi_rendermesh_free(impl->rmesh);
	pi_entity_free(impl->entity);
	pi_camera_free(impl->camera);
	pi_material_free(impl->material);
	if(impl->ao_rt)
	{
		pi_rendertarget_free(impl->ao_rt);
		pi_texture_free(impl->ao_tex);
		pi_renderview_free(impl->ao_view);
	}
	pi_free(impl->depth_map_name);
	pi_free(impl->normal_map_name);
	pi_free(impl->output_name);
	pi_free(impl->scene_camera_name);

	pi_free(renderer->impl);
	pi_renderer_destroy(renderer);
}

void PI_API pi_ssao_set_reject_radius(PiRenderer* renderer, float radius)
{
	SSAORenderer* impl;
	_type_check(renderer);
	impl = (SSAORenderer*)renderer->impl;

	impl->reject_radius = radius;
	_update_params(impl);
}

void PI_API pi_ssao_set_accept_radius(PiRenderer* renderer, float radius)
{
	SSAORenderer* impl;
	_type_check(renderer);
	impl = (SSAORenderer*)renderer->impl;

	impl->accept_radius = radius;
	_update_params(impl);
}

void PI_API pi_ssao_set_normal_scale(PiRenderer* renderer, float scale)
{
	SSAORenderer* impl;
	_type_check(renderer);
	impl = (SSAORenderer*)renderer->impl;

	impl->normal_scale = scale;
	_update_params(impl);
}

void PI_API pi_ssao_set_intensity(PiRenderer* renderer, float intensity)
{
	SSAORenderer* impl;
	_type_check(renderer);
	impl = (SSAORenderer*)renderer->impl;

	impl->intensity = intensity;
	_update_params(impl);
}

void PI_API pi_ssao_set_sample_radius_scale(PiRenderer* renderer, float scale)
{
	SSAORenderer* impl;
	_type_check(renderer);
	impl = (SSAORenderer*)renderer->impl;

	impl->sample_radius_scale = scale;
	_update_params(impl);
}

void PI_API pi_ssao_set_quality(PiRenderer* renderer, uint level)
{
	SSAORenderer* impl;
	_type_check(renderer);
	impl = (SSAORenderer*)renderer->impl;

	impl->quality = level;
	_update_params(impl);
}

void PI_API pi_ssao_set_independent_output(PiRenderer* renderer, PiBool is_enable)
{
	SSAORenderer* impl;
	_type_check(renderer);
	impl = (SSAORenderer*)renderer->impl;

	impl->independent_output = is_enable;
}

void PI_API pi_ssao_set_half_resolution_mode(PiRenderer* renderer, PiBool is_enable)
{
	SSAORenderer* impl;
	_type_check(renderer);
	impl = (SSAORenderer*)renderer->impl;

	impl->half_resolution_mode = is_enable;
}