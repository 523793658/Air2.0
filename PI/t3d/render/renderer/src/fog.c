
#include "fog.h"
#include <rendersystem.h>
#include <entity.h>
#include "environment.h"

/**
 * Fog渲染器
 */
typedef struct
{
	PiBool is_deploy;

	char *target_name;
	char *view_cam_name;
	char *color_tex_name;
	char *depth_tex_name;
	char *env_name;

	PiCamera *quad_camera;

	PiMaterial *material;
	PiMesh *quad_mesh;
	PiRenderMesh *quad_rmesh;
	PiEntity *quad_entity;

	SamplerState color_tex_sampler;
	SamplerState depth_tex_sampler;
	SamplerState scatter_tex_sampler;
	PiTexture* scatter_tex;

	/* 逆矩阵， 用于逆向获取深度值 */
	PiMatrix4 proj_mat_inverse;

	/* 雾化参数 */
	FogMode mode;
	float start;
	float end;
	float scale;
	float density;
	float density_square;
	float colorDensity[4];
	PiBool isFlow;
	float flowData[4];

	/* 字符串常量 */
	char *FOG_LINEAR;
	char *FOG_EXP;
	char *FOG_EXP2;
	char *FOG_RANGE;
	char *FLOW;
	char *SCATTER_MAP;

	char *U_SceneColorTex;
	char *U_SceneDepthTex;
	char *U_ProjMatrixInverse;
	char *U_FogColorDensity;
	char *U_FogEnd;
	char *U_FogScale;
	char *U_FlowData;
	char *U_ViewProjMatrixInverse;
	char *U_ScatterTex;

} FOGRenderer;

static void _type_check(PiRenderer *renderer)
{
	PI_ASSERT((renderer)->type == ERT_FOG, "Renderer type error!");
}

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	float width, height;
	PiRenderTarget *target;
	FOGRenderer *impl;
	_type_check(renderer);
	impl = (FOGRenderer *)renderer->impl;

	if (!impl->is_deploy)
	{
		return FALSE;
	}

	pi_hash_lookup(resources, impl->target_name, (void **)&target);
	width = (float)target->width;
	height = (float)target->height;

	impl->quad_camera = pi_camera_new();
	pi_camera_set_location(impl->quad_camera, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->quad_camera, 0.0f, 0.0f, -1.0f);
	pi_camera_set_frustum(impl->quad_camera, -width / 2.0f + 0.5f, width / 2.0f + 0.5f, -height / 2.0f - 0.5f, height / 2.0f - 0.5f, 0.0f, 2.0f, TRUE);

	/* 屏幕网格构建 用于雾效处理 */
	impl->quad_mesh = pi_mesh_create_quad(NULL, NULL, 0.0f);
	impl->quad_rmesh = pi_rendermesh_new(impl->quad_mesh, TRUE);

	impl->quad_entity = pi_entity_new();
	pi_entity_set_material(impl->quad_entity, impl->material);
	pi_entity_set_mesh(impl->quad_entity, impl->quad_rmesh);
	pi_spatial_set_local_scaling(impl->quad_entity->spatial, width, height, 1.0f);
	pi_spatial_update(impl->quad_entity->spatial);

	/* 初始化纹理采样器 */
	pi_renderstate_set_default_sampler(&impl->color_tex_sampler);
	pi_sampler_set_addr_mode(&impl->color_tex_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->color_tex_sampler, TFO_MIN_MAG_POINT);

	pi_renderstate_set_default_sampler(&impl->depth_tex_sampler);
	pi_sampler_set_addr_mode(&impl->depth_tex_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->depth_tex_sampler, TFO_MIN_MAG_POINT);

	pi_renderstate_set_default_sampler(&impl->scatter_tex_sampler);
	pi_sampler_set_addr_mode(&impl->scatter_tex_sampler, TAM_WRAP, TAM_WRAP, TAM_WRAP);
	pi_sampler_set_filter(&impl->scatter_tex_sampler, TFO_MIN_MAG_LINEAR);
	if (impl->scatter_tex != NULL)
	{
		pi_material_set_def(impl->material, impl->SCATTER_MAP, TRUE);
		pi_sampler_set_texture(&impl->scatter_tex_sampler, impl->scatter_tex);
		pi_material_set_uniform(impl->material, impl->U_ScatterTex, UT_SAMPLER_2D, 1, &impl->scatter_tex_sampler, FALSE);
	}
	else
	{
		pi_material_set_def(impl->material, impl->SCATTER_MAP, FALSE);
	}

	/* 设置逆投影矩阵 */
	pi_material_set_uniform(impl->material, impl->U_ProjMatrixInverse, UT_MATRIX4, 1, &impl->proj_mat_inverse, FALSE);
	pi_material_set_uniform(impl->material, impl->U_SceneColorTex, UT_SAMPLER_2D, 1, &impl->color_tex_sampler, FALSE);
	pi_material_set_uniform(impl->material, impl->U_SceneDepthTex, UT_SAMPLER_2D, 1, &impl->depth_tex_sampler, FALSE);
	pi_material_set_uniform(impl->material, impl->U_FlowData, UT_VEC4, 1, impl->flowData, FALSE);

	pi_material_set_uniform(impl->material, impl->U_FogColorDensity, UT_VEC4, 1, impl->colorDensity, FALSE);
	pi_material_set_uniform_pack_flag(impl->material, impl->U_FogEnd, UT_FLOAT, 1, &impl->end, FALSE, TRUE);
	return TRUE;
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	PiRenderTarget *target;
	FOGRenderer *impl;
	_type_check(renderer);
	impl = (FOGRenderer *)renderer->impl;

	pi_hash_lookup(resources, impl->target_name, (void **)&target);

	pi_rendersystem_set_target(target);

	pi_rendersystem_set_camera(impl->quad_camera);

	pi_entity_draw(impl->quad_entity);
}

static void _update(PiRenderer *renderer, float tpf, PiHash *resources)
{
	PiCamera *view_camera;
	PiTexture *color_tex, *depth_tex;
	FOGRenderer *impl;
	_type_check(renderer);
	impl = (FOGRenderer *)renderer->impl;

	pi_hash_lookup(resources, impl->view_cam_name, (void **)&view_camera);
	pi_mat4_inverse(&impl->proj_mat_inverse, pi_camera_get_projection_matrix(view_camera));

	/* 颜色纹理 */
	pi_hash_lookup(resources, impl->color_tex_name, (void **)&color_tex);
	pi_sampler_set_texture(&impl->color_tex_sampler, color_tex);

	/* 深度纹理 */
	pi_hash_lookup(resources, impl->depth_tex_name, (void **)&depth_tex);
	pi_sampler_set_texture(&impl->depth_tex_sampler, depth_tex);

	if (impl->isFlow && impl->env_name != NULL){
		PiEnvironment* env;
		PiCamera* scene_camera;
		PiMatrix4 view_proj_mat_inverse;
		pi_hash_lookup(resources, impl->env_name, &env);
		impl->flowData[2] = env->wind_data.wind_dir[0];
		impl->flowData[3] = env->wind_data.wind_dir[1];
		pi_hash_lookup(resources, impl->view_cam_name, (void **)&scene_camera);
		pi_mat4_inverse(&view_proj_mat_inverse, pi_camera_get_view_projection_matrix(scene_camera));
		pi_material_set_uniform(impl->material, impl->U_ViewProjMatrixInverse, UT_MATRIX4, 1, &view_proj_mat_inverse, TRUE);
	}
	else
	{
		pi_material_set_def(impl->material, impl->FLOW, FALSE);
	}
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	float w = (float)width;
	float h = (float)height;
	FOGRenderer *impl;
	_type_check(renderer);
	impl = (FOGRenderer *)renderer->impl;

	pi_spatial_set_local_scaling(impl->quad_entity->spatial, w, h, 1.0f);
	pi_spatial_update(impl->quad_entity->spatial);

	pi_camera_set_frustum(impl->quad_camera, -w / 2.0f + 0.5f, w / 2.0f + 0.5f, -h / 2.0f - 0.5f, h / 2.0f - 0.5f, 0.0f, 2.0f, TRUE);
}

PiRenderer *PI_API pi_fog_new_with_name(char* name)
{
	PiRenderer *renderer;

	FOGRenderer *impl = pi_new0(FOGRenderer, 1);
	impl->FOG_LINEAR = pi_conststr("FOG_LINEAR");
	impl->FOG_EXP = pi_conststr("FOG_EXP");
	impl->FOG_EXP2 = pi_conststr("FOG_EXP2");
	impl->FOG_RANGE = pi_conststr("FOG_RANGE");
	impl->FLOW = pi_conststr("FLOW");
	impl->SCATTER_MAP = pi_conststr("SCATTER_MAP");

	impl->U_SceneColorTex = pi_conststr("u_SceneColorTex");
	impl->U_SceneDepthTex = pi_conststr("u_SceneDepthTex");
	impl->U_ProjMatrixInverse = pi_conststr("u_ProjMatrixInverse");

	impl->U_FogColorDensity = pi_conststr("u_FogColorDensity");
	impl->U_FogEnd = pi_conststr("u_FogEnd");
	impl->U_FogScale = pi_conststr("u_FogScale");
	impl->U_FlowData = pi_conststr("u_FlowData");
	impl->U_ViewProjMatrixInverse = pi_conststr("u_ViewProjMatrixInverse");
	impl->U_ScatterTex = pi_conststr("u_ScatterTex");

	/* 默认雾化参数 */
	impl->mode = FM_EXP;
	impl->colorDensity[0] = 0.0f;
	impl->colorDensity[1] = 0.0f;
	impl->colorDensity[2] = 0.0f;
	impl->colorDensity[3] = 1.0f;
	impl->density = 1.0f;
	impl->density_square = 1.0f;
	impl->start = 0.0f;
	impl->end = 1.0f;
	impl->scale = 1.0f;

	/* 创建材质 专门用于雾效处理 */
	impl->material = pi_material_new(RS_FOG_VS, RS_FOG_FS);
	pi_material_set_depth_enable(impl->material, FALSE);
	/* 相关开关 */
	pi_material_set_def(impl->material, impl->FOG_EXP, TRUE);
	pi_material_set_def(impl->material, impl->FOG_RANGE, TRUE);

	renderer = pi_renderer_create(ERT_FOG, name, _init, _resize, _update, _draw, impl);

	return renderer;
}

PiRenderer *PI_API pi_fog_new()
{
	return pi_fog_new_with_name("fog");
}

void PI_API pi_fog_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *color_tex_name, char *depth_tex_name)
{
	FOGRenderer *impl;
	_type_check(renderer);
	impl = (FOGRenderer *)renderer->impl;

	pi_free(impl->target_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->color_tex_name);
	pi_free(impl->depth_tex_name);

	impl->target_name = pi_str_dup(target_name);
	impl->view_cam_name = pi_str_dup(view_cam_name);
	impl->color_tex_name = pi_str_dup(color_tex_name);
	impl->depth_tex_name = pi_str_dup(depth_tex_name);

	impl->is_deploy = TRUE;
}

void PI_API pi_fog_deploy_env(PiRenderer* renderer, char *env_name)
{
	FOGRenderer *impl;
	_type_check(renderer);
	impl = (FOGRenderer *)renderer->impl;
	pi_free(impl->env_name);
	impl->env_name = pi_str_dup(env_name);
}

void PI_API pi_fog_free(PiRenderer *renderer)
{
	FOGRenderer *impl;
	_type_check(renderer);
	impl = (FOGRenderer *)renderer->impl;

	pi_camera_free(impl->quad_camera);

	pi_entity_free(impl->quad_entity);
	pi_rendermesh_free(impl->quad_rmesh);
	pi_mesh_free(impl->quad_mesh);
	pi_material_free(impl->material);

	pi_free(impl->target_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->color_tex_name);
	pi_free(impl->depth_tex_name);

	pi_free(renderer->impl);
	pi_renderer_destroy(renderer);
}

void PI_API pi_fog_set_color(PiRenderer *renderer, float r, float g, float b)
{
	FOGRenderer *impl;
	_type_check(renderer);
	impl = (FOGRenderer *)renderer->impl;

	impl->colorDensity[0] = r;
	impl->colorDensity[1] = g;
	impl->colorDensity[2] = b;
}

void PI_API pi_fog_set_mode(PiRenderer *renderer, FogMode mode)
{
	FOGRenderer *impl;
	_type_check(renderer);
	impl = (FOGRenderer *)renderer->impl;

	impl->mode = mode;
	switch (mode)
	{
	case FM_EXP:
		pi_material_set_def(impl->material, impl->FOG_LINEAR, FALSE);
		pi_material_set_def(impl->material, impl->FOG_EXP, TRUE);
		pi_material_set_def(impl->material, impl->FOG_EXP2, FALSE);
		impl->colorDensity[3] = impl->density;
		break;
	case FM_EXP2:
		pi_material_set_def(impl->material, impl->FOG_LINEAR, FALSE);
		pi_material_set_def(impl->material, impl->FOG_EXP, FALSE);
		pi_material_set_def(impl->material, impl->FOG_EXP2, TRUE);
		impl->colorDensity[3] = impl->density_square;
		break;
	case FM_LINEAR:
		pi_material_set_def(impl->material, impl->FOG_LINEAR, TRUE);
		pi_material_set_def(impl->material, impl->FOG_EXP, FALSE);
		pi_material_set_def(impl->material, impl->FOG_EXP2, FALSE);
		impl->colorDensity[3] = impl->scale;
		break;
	}
}

void PI_API pi_fog_set_start_end(PiRenderer *renderer, float start, float end)
{
	FOGRenderer *impl;
	_type_check(renderer);
	impl = (FOGRenderer *)renderer->impl;

	impl->start = start;
	impl->end = end;
	impl->scale = 1.0f / (end - start);
	if (impl->mode == FM_LINEAR)
	{
		impl->colorDensity[3] = impl->scale;
	}
}

void PI_API pi_fog_set_density(PiRenderer *renderer, float density)
{
	FOGRenderer *impl;
	_type_check(renderer);
	impl = (FOGRenderer *)renderer->impl;

	impl->density = density;
	impl->density_square = density * density;
	switch (impl->mode)
	{
	case FM_EXP:
		impl->colorDensity[3] = impl->density;
		break;
	case FM_EXP2:
		impl->colorDensity[3] = impl->density_square;
		break;
	}
}

void PI_API pi_fog_set_range_enable(PiRenderer *renderer, PiBool enable)
{
	FOGRenderer *impl;
	_type_check(renderer);
	impl = (FOGRenderer *)renderer->impl;

	pi_material_set_def(impl->material, impl->FOG_RANGE, enable);
}

void PI_API pi_fog_set_flow(PiRenderer *renderer, PiBool isFlow, float flow_speed, float scale_uv)
{
	FOGRenderer *impl;
	_type_check(renderer);
	impl = (FOGRenderer *)renderer->impl;
	impl->isFlow = isFlow;
	impl->flowData[0] = flow_speed;
	impl->flowData[1] = scale_uv;
	if (isFlow)
	{
		pi_material_set_def(impl->material, impl->FLOW, TRUE);
	}
	else
	{
		pi_material_set_def(impl->material, impl->FLOW, FALSE);
	}
}

void PI_API pi_fog_set_texture(PiRenderer* renderer, PiTexture* tex)
{
	FOGRenderer *impl;
	_type_check(renderer);
	impl = (FOGRenderer *)renderer->impl;
	impl->scatter_tex = tex;
	if (tex)
	{
		pi_material_set_def(impl->material, impl->SCATTER_MAP, TRUE);
		pi_sampler_set_texture(&impl->scatter_tex_sampler, tex);
		pi_material_set_uniform(impl->material, impl->U_ScatterTex, UT_SAMPLER_2D, 1, &impl->scatter_tex_sampler, FALSE);
	}
	else
	{
		pi_material_set_def(impl->material, impl->SCATTER_MAP, FALSE);
	}
}
