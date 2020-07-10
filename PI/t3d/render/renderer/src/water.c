#include "water.h"

#include <pi_spatial.h>
#include <entity.h>
#include <rendertarget.h>
#include <rendersystem.h>
#include <local_water.h>
#include <environment.h>

const static char *RS_WATER_SURFACE_VS = "water.vs";
const static char *RS_WATER_MUDDY_FS = "muddy.fs";
const static char *RS_GLASS_FS = "mirror.fs";

const static char *RS_WATER_POST_VS = "default.vs";
const static char *RS_WATER_POST_FS = "waterpost.fs";

typedef struct _WaterEnvironment
{
	PiVector3 light_dir;
	float pad1;

	PiVector3 light_color;
	float pad2;
	PiVector3 ambient_color;
	float pad3;

	PiMatrix4 viewProjMatrixInverse;
}WaterEnvironment;

/**
 * 水体渲染器
 */
typedef struct
{
	PiMesh *mesh;
	PiRenderMesh *rmesh;
	PiTexture *normal_map;
	PiTexture *caustics_map;
	PiBool caustics_map_change;

	WaterEnvironment environment;
	PiBool environment_enable;
	PiBool is_intz_support;

	PiCamera *post_camera;
	PiEntity *post_quad;

	uint width;
	uint height;

	PiRenderTarget *water_rt;
	PiTexture *water_map;
	PiTexture *water_depth;
	PiTexture *water_depth_stencil;

	PiRenderView *water_map_view;
	PiRenderView *water_depth_view;
	PiRenderView *water_depth_stencial_view;

	char *local_water_list_name;

	char *reflection_cam_name;
	char *reflection_list_name;
	PiRenderTarget *reflection_rt;
	PiTexture *reflection_tex;
	PiTexture *reflection_depth_texture;
	PiRenderView *reflection_view;
	PiRenderView *reflection_depth_view;
	PiRenderView *reflection_depth;
	PiMatrix4 reflection_inv_mat;
	PiTexture *env_tex;
	PiBool is_cylinder;

	char *view_cam_name;
	char *scene_color_name;
	char *scene_depth_name;
	char *output_name;
	char *env_name;
	
	PiMaterial *post_matl;
	PiBool has_local_water;

	float sea_level;
	PiBool caustics_enable;
	PiBool hdr_mode;

	PiBool is_deploy;

	float size[4];
	SamplerState global_normal_ss;
	SamplerState global_caustics_ss;
	SamplerState environment_ss;
	SamplerState scene_color_ss;
	SamplerState scene_depth_ss;
	SamplerState water_color_ss;
	SamplerState water_depth_ss;
	SamplerState reflection_ss;
	SamplerState reflection_depth_ss;


	char *U_NormalMap;
	char *U_CausticsMap;
	char *U_EnvironmentMap;
	char *U_SceneColorTex;
	char *U_SceneDepthTex;
	char *U_WaterColorTex;
	char *U_WaterDepthTex;
	char *U_ReflectionTex;
	char *U_ReflectionDepthTex;
	char *U_PlaneParams;

	/* 常量字符串 */
	char *FLOWING;
	char *CAUSTICS;
	char *REFLECTION;
	char *ENVIRONMENT_MAPPING;
	char *CYLINDER_MAPPING;
	char *DEPTH;
	char *DIRECTLY_MAPPING;

	char *U_RenderTargetSize;
	char *U_Params;
	char *U_LightDir;
	char *U_LightColor;
	char *U_AmbientColor;
	char *U_ViewProjMatrixInverse;
	char *U_SeaLevel;
	char *U_ReflectionMatrix;
	char *U_ReflectionMatrixInv;


	char* VERTEX_SHADER;
	char* FRAGMENT_SHADER;
} WaterRenderer;

static void _type_check(PiRenderer *renderer)
{
	PI_ASSERT(renderer->type == ERT_WATER, "Renderer type error!");
}

static void _apply_material(WaterRenderer *renderer, PiMaterial *material, PiLocalWater *water)
{
	//TODO:整合Uniform和shader切换优化
	//水的基础属性
	pi_material_set_uniform(material, renderer->U_Params, UT_VEC4, 5, &water->params, FALSE);
	//全局环境属性
	pi_material_set_uniform_pack_flag(material, renderer->U_LightDir, UT_VEC3, 1, &renderer->environment.light_dir, FALSE, TRUE);
	pi_material_set_uniform_pack_flag(material, renderer->U_LightColor, UT_VEC3, 1, &renderer->environment.light_color, FALSE, TRUE);
	pi_material_set_uniform_pack_flag(material, renderer->U_AmbientColor, UT_VEC3, 1, &renderer->environment.ambient_color, FALSE, TRUE);
	pi_material_set_uniform(material, renderer->U_ViewProjMatrixInverse, UT_MATRIX4, 1, &renderer->environment.viewProjMatrixInverse, FALSE);

	pi_material_set_def(material, renderer->FLOWING, water->flowing);

	if (water->normal_map != NULL)
	{
		pi_sampler_set_texture(&water->normal_ss, water->normal_map);
		pi_material_set_uniform(material, renderer->U_NormalMap, UT_SAMPLER_2D, 1, &water->normal_ss, FALSE);
	}
	else
	{
		pi_material_set_uniform(material, renderer->U_NormalMap, UT_SAMPLER_2D, 1, &renderer->global_normal_ss, FALSE);
	}
}

static void _apply_global_water_normal_map(WaterRenderer *impl)
{
	pi_sampler_set_texture(&impl->global_normal_ss, impl->normal_map);
}

static void _apply_global_water_caustice_map(WaterRenderer *impl)
{
	if (impl->caustics_enable && impl->caustics_map != NULL)
	{
		pi_sampler_set_texture(&impl->global_caustics_ss, impl->caustics_map);
	}

}

static void _create_renderview(WaterRenderer *impl)
{
	impl->water_map = pi_texture_2d_create(impl->hdr_mode ? RF_ABGR16F : RF_ABGR8, TU_COLOR, 1, 1, impl->width, impl->height, TRUE);
	impl->water_map_view = pi_renderview_new_tex2d(RVT_COLOR, impl->water_map, 0, 0, TRUE);

	if (impl->is_intz_support)
	{
		impl->water_depth = pi_texture_2d_create(RF_INTZ, TU_DEPTH_STENCIL, 1, 1, impl->width, impl->height, TRUE);
		impl->water_depth_view = pi_renderview_new_tex2d(RVT_DEPTH_STENCIL, impl->water_depth, 0, 0, TRUE);
		pi_rendertarget_attach(impl->water_rt, ATT_DEPTHSTENCIL, impl->water_depth_view);

	}
	else
	{
		impl->water_depth = pi_texture_2d_create(RF_R32F, TU_COLOR, 1, 1, impl->width, impl->height, TRUE);
		impl->water_depth_view = pi_renderview_new_tex2d(RVT_COLOR, impl->water_depth, 0, 0, TRUE);
		pi_rendertarget_attach(impl->water_rt, ATT_COLOR1, impl->water_depth_view);

		impl->water_depth_stencil = pi_texture_2d_create(RF_D24S8, TU_DEPTH_STENCIL, 1, 1, impl->width, impl->height, TRUE);
		impl->water_depth_stencial_view = pi_renderview_new_tex2d(RVT_DEPTH_STENCIL, impl->water_depth_stencil, 0, 0, TRUE);
		pi_rendertarget_attach(impl->water_rt, ATT_DEPTHSTENCIL, impl->water_depth_stencial_view);
	}


	



	pi_rendertarget_attach(impl->water_rt, ATT_COLOR0, impl->water_map_view);
	pi_rendertarget_set_viewport(impl->water_rt, 0, 0, impl->width, impl->height);

	pi_renderstate_set_default_sampler(&impl->water_color_ss);
	pi_sampler_set_addr_mode(&impl->water_color_ss, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->water_color_ss, TFO_MIN_MAG_LINEAR);
	pi_sampler_set_texture(&impl->water_color_ss, impl->water_map);
	pi_material_set_uniform(impl->post_matl, impl->U_WaterColorTex, UT_SAMPLER_2D, 1, &impl->water_color_ss, FALSE);


	pi_renderstate_set_default_sampler(&impl->environment_ss);
	pi_sampler_set_filter(&impl->environment_ss, TFO_MIN_MAG_MIP_LINEAR);

	pi_renderstate_set_default_sampler(&impl->water_depth_ss);
	pi_sampler_set_addr_mode(&impl->water_depth_ss, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->water_depth_ss, TFO_MIN_MAG_LINEAR);
	pi_sampler_set_texture(&impl->water_depth_ss, impl->water_depth);
	pi_material_set_uniform(impl->post_matl, impl->U_WaterDepthTex, UT_SAMPLER_2D, 1, &impl->water_depth_ss, FALSE);

	impl->reflection_tex = pi_texture_2d_create(impl->hdr_mode ? RF_ABGR16F : RF_ABGR8, TU_COLOR, 1, 1, impl->width, impl->height, TRUE);
	impl->reflection_view = pi_renderview_new_tex2d(RVT_COLOR, impl->reflection_tex, 0, 0, TRUE);
	impl->reflection_depth_texture = pi_texture_2d_create(RF_R16F, TU_COLOR, 1, 1, impl->width, impl->height, TRUE);
	impl->reflection_depth_view = pi_renderview_new_tex2d(RVT_COLOR, impl->reflection_depth_texture, 0, 0, TRUE);
	impl->reflection_depth = pi_renderview_new(RVT_DEPTH_STENCIL, impl->width, impl->height, RF_D16, TRUE);
	pi_rendertarget_attach(impl->reflection_rt, ATT_DEPTHSTENCIL, impl->reflection_depth);
	pi_rendertarget_attach(impl->reflection_rt, ATT_COLOR0, impl->reflection_view);
	pi_rendertarget_attach(impl->reflection_rt, ATT_COLOR1, impl->reflection_depth_view);
	pi_rendertarget_set_viewport(impl->reflection_rt, 0, 0, impl->width, impl->height);

	impl->size[0] = 1.0f / impl->width;
	impl->size[1] = 1.0f / impl->height;


	pi_material_set_def(impl->post_matl, impl->DIRECTLY_MAPPING, TRUE);
	pi_material_set_uniform_pack_flag(impl->post_matl, impl->U_RenderTargetSize, UT_VEC2, 1, impl->size, FALSE, TRUE);

	pi_renderstate_set_default_sampler(&impl->reflection_depth_ss);
	pi_sampler_set_addr_mode(&impl->reflection_depth_ss, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->reflection_depth_ss, TFO_MIN_MAG_LINEAR);
	pi_sampler_set_texture(&impl->reflection_depth_ss, impl->reflection_depth_texture);
	pi_renderstate_set_default_sampler(&impl->reflection_ss);
	pi_sampler_set_addr_mode(&impl->reflection_ss, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->reflection_ss, TFO_MIN_MAG_LINEAR);
	pi_sampler_set_texture(&impl->reflection_ss, impl->reflection_tex);
}

static PiMaterial* _create_local_material(WaterRenderer* renderer, PiLocalWater* water)
{
	PiMaterial* material;
	if (water->reflection_type == RT_GLASS)
	{
		material = pi_material_new(renderer->VERTEX_SHADER, RS_GLASS_FS);
		pi_material_set_uniform(material, renderer->U_ReflectionTex, UT_SAMPLER_2D, 1, &renderer->reflection_ss, FALSE);
		pi_material_set_uniform(material, renderer->U_SceneColorTex, UT_SAMPLER_2D, 1, &renderer->scene_color_ss, FALSE);
	}
	else if (water->muddy)
	{
		material = pi_material_new(RS_WATER_SURFACE_VS, RS_WATER_MUDDY_FS);
	}
	else
	{
		material = pi_material_new(renderer->VERTEX_SHADER, renderer->FRAGMENT_SHADER);
		pi_material_set_uniform(material, renderer->U_ReflectionTex, UT_SAMPLER_2D, 1, &renderer->reflection_ss, FALSE);
		pi_material_set_uniform(material, renderer->U_SceneColorTex, UT_SAMPLER_2D, 1, &renderer->scene_color_ss, FALSE);
	}
	pi_material_set_def(material, renderer->DEPTH, !renderer->is_intz_support);
	pi_material_set_uniform(material, renderer->U_SceneDepthTex, UT_SAMPLER_2D, 1, &renderer->scene_depth_ss, FALSE);

	pi_material_set_def(material, renderer->DIRECTLY_MAPPING, TRUE);
	pi_material_set_cull_mode(material, CM_NO);
	pi_material_set_uniform_pack_flag(material, renderer->U_RenderTargetSize, UT_VEC2, 1, renderer->size, FALSE, TRUE);
	water->material = material;
	if (renderer->caustics_enable && renderer->caustics_map != NULL && !water->muddy)
	{
		pi_material_set_def(material, renderer->CAUSTICS, TRUE);
		pi_material_set_uniform(material, renderer->U_CausticsMap, UT_SAMPLER_2D, 1, &renderer->global_caustics_ss, FALSE);
	}
	pi_entity_set_material(water->entity, material);
	_apply_material(renderer, material, water);
	return material;
}

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{

	WaterRenderer *impl = (WaterRenderer *)renderer->impl;
	PiQuaternion quat;
	PiRenderTarget *target;
	PiEnvironment *env = NULL;

	PI_USE_PARAM(resources);

	if (!impl->is_deploy)
	{
		return FALSE;
	}
	if (impl->normal_map == NULL)
	{
		return FALSE;
	}

	impl->FLOWING = pi_conststr("FLOWING");
	impl->CAUSTICS = pi_conststr("CAUSTICS");
	impl->REFLECTION = pi_conststr("REFLECTION");
	impl->ENVIRONMENT_MAPPING = pi_conststr("ENVIRONMENT_MAPPING");
	impl->CYLINDER_MAPPING = pi_conststr("CYLINDER_MAPPING");
	impl->DEPTH = pi_conststr("DEPTH");
	impl->DIRECTLY_MAPPING = pi_conststr("DIRECTLY_MAPPING");

	impl->U_RenderTargetSize = pi_conststr("u_RenderTargetSize");
	impl->U_Params = pi_conststr("u_Params");
	impl->U_NormalMap = pi_conststr("u_NormalMap");
	impl->U_CausticsMap = pi_conststr("u_CausticsMap");
	impl->U_EnvironmentMap = pi_conststr("u_EnvironmentMap");
	impl->U_WaterColorTex = pi_conststr("u_WaterColorTex");
	impl->U_WaterDepthTex = pi_conststr("u_WaterDepthTex");
	impl->U_ReflectionTex = pi_conststr("u_ReflectionTex");
	impl->U_PlaneParams = pi_conststr("u_PlaneParams");
	impl->U_LightDir = pi_conststr("u_LightDir");
	impl->U_LightColor = pi_conststr("u_LightColor");
	impl->U_AmbientColor = pi_conststr("u_AmbientColor");
	impl->U_ViewProjMatrixInverse = pi_conststr("u_ViewProjMatrixInverse");
	impl->U_SceneColorTex = pi_conststr("u_SceneColorTex");
	impl->U_SceneDepthTex = pi_conststr("u_SceneDepthTex");
	impl->U_SeaLevel = pi_conststr("u_SeaLevel");
	impl->U_ReflectionMatrix = pi_conststr("u_ReflectionMatrix");
	impl->U_ReflectionMatrixInv = pi_conststr("u_ReflectionMatrixInv");
	impl->U_ReflectionDepthTex = pi_conststr("u_ReflectionDepthTex");
	pi_hash_lookup(resources, impl->output_name, (void **)&target);
	impl->width = target->width;
	impl->height = target->height;

	impl->mesh = pi_mesh_create_quad(NULL, NULL, 0.0f);
	impl->rmesh = pi_rendermesh_new(impl->mesh, TRUE);

	pi_hash_lookup(resources, impl->env_name, (void **)&env);
	impl->env_tex = env->env_tex;
	impl->is_cylinder = env->is_cylinder;
	pi_quat_from_angle_axis(&quat, pi_vec3_get_xunit(), -(float)(PI_PI / 2.0));

	impl->post_camera = pi_camera_new();
	pi_camera_set_location(impl->post_camera, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->post_camera, 0.0f, 0.0f, -1.0f);
	pi_camera_set_frustum(impl->post_camera, -(float)impl->width / 2.0f, (float)impl->width / 2.0f, -(float)impl->height / 2.0f, (float)impl->height / 2.0f, 0.0f, 2.0f, TRUE);

	impl->post_matl = pi_material_new(RS_WATER_POST_VS, RS_WATER_POST_FS);

	impl->post_quad = pi_entity_new();
	pi_entity_set_mesh(impl->post_quad, impl->rmesh);
	pi_entity_set_material(impl->post_quad, impl->post_matl);
	pi_material_set_blend(impl->post_matl, TRUE);
	pi_material_set_blend_factor(impl->post_matl, BF_SRC_ALPHA, BF_INV_SRC_ALPHA, BF_ZERO, BF_ONE);
	pi_spatial_set_local_scaling(impl->post_quad->spatial, (float)impl->width, (float)impl->height, 1.0f);
	pi_spatial_update(impl->post_quad->spatial);


	pi_renderstate_set_default_sampler(&impl->global_normal_ss);
	pi_sampler_set_filter(&impl->global_normal_ss, TFO_MIN_POINT_MAG_LINEAR);

	pi_renderstate_set_default_sampler(&impl->global_caustics_ss);
	pi_sampler_set_filter(&impl->global_caustics_ss, TFO_MIN_MAG_LINEAR);

	_apply_global_water_normal_map(impl);
	_apply_global_water_caustice_map(impl);

	impl->water_rt = pi_rendertarget_new(TT_MRT, TRUE);

	impl->reflection_rt = pi_rendertarget_new(TT_MRT, TRUE);

	_create_renderview(impl);
	return TRUE;
}
static void update_camera(PiCamera* view_camera, PiCamera* reflection_camera, PiLocalWater *water)
{
	PiVector3 reflection_dir, reflection_up, reflection_pos;
	pi_camera_get_direction(view_camera, &reflection_dir);
	pi_plane_symmetry_vector(&reflection_dir, &reflection_dir, &water->plane);
	pi_camera_get_up(view_camera, &reflection_up);
	pi_plane_symmetry_vector(&reflection_up, &reflection_up, &water->plane);
	pi_plane_symmetry_point(&reflection_pos, pi_camera_get_location(view_camera), &water->plane);
	pi_camera_set_frustum(reflection_camera, view_camera->frustum_left, view_camera->frustum_right, view_camera->frustum_bottom, view_camera->frustum_top, 
		view_camera->frustum_near, view_camera->frustum_far, view_camera->is_ortho);
	pi_camera_set_location(reflection_camera, reflection_pos.x, reflection_pos.y, reflection_pos.z);
	pi_camera_set_direction(reflection_camera, reflection_dir.x, reflection_dir.y, reflection_dir.z);
	pi_camera_set_up(reflection_camera, reflection_up.x, reflection_up.y, reflection_up.z);
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	WaterRenderer *impl = (WaterRenderer *)renderer->impl;
	PiRenderTarget *target;
	PiColor background;
	uint i;
	PiVector *local_water_list = NULL;
	PiVector *reflection_list = NULL;
	PiCamera *view_cam;
	PiCamera *reflection_cam;
	PI_USE_PARAM(tpf);
	_type_check(renderer);

	pi_hash_lookup(resources, impl->view_cam_name, (void **)&view_cam);
	pi_hash_lookup(resources, impl->reflection_cam_name, (void **)&reflection_cam);
	pi_hash_lookup(resources, impl->reflection_list_name, (void **)&reflection_list);
	pi_hash_lookup(resources, impl->local_water_list_name, (void **)&local_water_list);

	if (impl->has_local_water)
	{
		pi_rendersystem_set_target(impl->water_rt);
		color_set(&background, 1.0f, 1.0f, 1.0f, 1.0f);
		pi_rendersystem_clearview(TBM_COLOR | TBM_DEPTH, &background, 1.0f, 0);
		pi_rendersystem_set_camera(view_cam);

		uint size = pi_vector_size(local_water_list);
		for (i = 0; i < size; ++i)
		{
			PiLocalWater *local_water = (PiLocalWater *)pi_vector_get(local_water_list, i);
			if (impl->caustics_map_change)
			{
				if (!local_water->muddy)
				{
					if (impl->caustics_enable && impl->caustics_map != NULL)
					{
						pi_material_set_def(local_water->material, impl->CAUSTICS, TRUE);
						pi_material_set_uniform(local_water->material, impl->U_CausticsMap, UT_SAMPLER_2D, 1, &impl->global_caustics_ss, FALSE);
					}
					else
					{
						pi_material_set_def(local_water->material, impl->CAUSTICS, FALSE);
					}
				}
			}
			if (local_water->mesh)
			{
				pi_entity_set_mesh(local_water->entity, local_water->mesh);
			}
			else
			{
				pi_entity_set_mesh(local_water->entity, impl->rmesh);
			}

			if (local_water->reflection && reflection_list != NULL)
			{
				uint reflection_entity_count, j;
				reflection_entity_count = pi_vector_size(reflection_list);
				for (j = 0; j < reflection_entity_count; ++j)
				{
					PiEntity* entity = pi_vector_get(reflection_list, j);
					pi_material_set_uniform(entity->material, impl->U_PlaneParams, UT_VEC4, 1, &local_water->plane, FALSE);
				}
				update_camera(view_cam, reflection_cam, local_water);
				pi_rendersystem_set_target(impl->reflection_rt);
				color_set(&background, 1.0f, 0.0f, 0.0f, 0.0f);
				pi_rendersystem_clearview(TBM_COLOR | TBM_DEPTH, &background, 1.0f, 0);
				pi_rendersystem_set_camera(reflection_cam);

				pi_entity_draw_list(reflection_list);
				pi_material_set_def(local_water->material, impl->REFLECTION, TRUE);
				pi_material_set_uniform(local_water->material, impl->U_ReflectionTex, UT_SAMPLER_2D, 1, &impl->reflection_ss, FALSE);
				pi_mat4_inverse(&impl->reflection_inv_mat, pi_camera_get_view_projection_matrix(reflection_cam));
				pi_material_set_uniform(local_water->material, impl->U_ReflectionDepthTex, UT_SAMPLER_2D, 1, &impl->reflection_depth_ss, FALSE);
				pi_material_set_uniform(local_water->material, impl->U_ReflectionMatrix, UT_MATRIX4, 1, pi_camera_get_view_projection_matrix(reflection_cam), FALSE);
				pi_material_set_uniform(local_water->material, impl->U_ReflectionMatrixInv, UT_MATRIX4, 1, &impl->reflection_inv_mat, FALSE);
			}
			else
			{
				pi_material_set_def(local_water->material, impl->REFLECTION, FALSE);
			}
			pi_rendersystem_set_target(impl->water_rt);
			pi_rendersystem_set_camera(view_cam);
			pi_entity_draw(local_water->entity);
		}
		pi_hash_lookup(resources, impl->output_name, (void **)&target);
		pi_rendersystem_set_target(target);
		pi_rendersystem_set_camera(impl->post_camera);
		pi_entity_draw(impl->post_quad);
	}
	if (impl->caustics_map_change)
	{
		impl->caustics_map_change = FALSE;
	}
}

static void _update(PiRenderer *renderer, float tpf, PiHash *resources)
{
	WaterRenderer *impl;
	PiTexture *color_tex = NULL;
	PiTexture *depth_tex = NULL;
	PiCamera *view_cam = NULL;
	PiEnvironment *env = NULL;
	PiVector* local_water_list;
	uint i, size;
	PiVector* reflection_entity_list = NULL;

	PI_USE_PARAM(tpf);
	impl = (WaterRenderer *)renderer->impl;

	pi_hash_lookup(resources, impl->env_name, (void **)&env);
	pi_hash_lookup(resources, impl->view_cam_name, (void **)&view_cam);


	pi_hash_lookup(resources, impl->local_water_list_name, (void **)&local_water_list);
	pi_hash_lookup(resources, impl->reflection_list_name, (void **)&reflection_entity_list);

	impl->env_tex = env->env_tex;
	pi_sampler_set_texture(&impl->environment_ss, impl->env_tex);
	size = pi_vector_size(local_water_list);
	impl->has_local_water = size > 0;
	if (impl->has_local_water)
	{
		pi_mat4_inverse(&impl->environment.viewProjMatrixInverse, pi_camera_get_view_projection_matrix(view_cam));
		pi_hash_lookup(resources, impl->scene_color_name, (void **)&color_tex);
		pi_hash_lookup(resources, impl->scene_depth_name, (void **)&depth_tex);
		pi_sampler_set_texture(&impl->scene_color_ss, color_tex);
		pi_sampler_set_texture(&impl->scene_depth_ss, depth_tex);
		pi_vec3_copy(&impl->environment.light_dir, &env->default_light.diffuse_dir);
		pi_vec3_copy(&impl->environment.light_color, &env->default_light.diffuse_color);
		pi_vec3_copy(&impl->environment.ambient_color, &env->default_light.ambient_color);
		if (reflection_entity_list)
		{
			uint size = pi_vector_size(reflection_entity_list);
			for (i = 0; i < size; ++i)
			{
				PiEntity* entity = pi_vector_get(reflection_entity_list, i);
				pi_material_set_uniform_pack_flag(entity->material, impl->U_LightDir, UT_VEC3, 1, &impl->environment.light_dir, FALSE, TRUE);
				pi_material_set_uniform_pack_flag(entity->material, impl->U_LightColor, UT_VEC3, 1, &impl->environment.light_color, FALSE, TRUE);
				pi_material_set_uniform_pack_flag(entity->material, impl->U_AmbientColor, UT_VEC3, 1, &impl->environment.ambient_color, FALSE, TRUE);
			}
		}
		for (i = 0; i < size; i++)
		{
			PiLocalWater* local_water = pi_vector_get(local_water_list, i);
			PiMaterial *matl = local_water->material;
			if (matl == NULL)
			{
				matl = _create_local_material(impl, local_water);
			}
			if (impl->environment_enable)
			{
				pi_material_set_def(matl, impl->ENVIRONMENT_MAPPING, env->env_tex != NULL);
				if (env->env_tex != NULL)
				{
					pi_sampler_set_texture(&impl->environment_ss, env->env_tex);
					pi_material_set_def(matl, impl->CYLINDER_MAPPING, env->is_cylinder);
					if (env->is_cylinder)
					{
						pi_material_set_uniform(matl, impl->U_EnvironmentMap, UT_SAMPLER_2D, 1, &impl->environment_ss, FALSE);
					}
					else
					{
						pi_material_set_def(matl, impl->CYLINDER_MAPPING, FALSE);
						pi_material_set_uniform(matl, impl->U_EnvironmentMap, UT_SAMPLER_CUBE, 1, &impl->environment_ss, FALSE);
					}
				}
			}
			else
			{
				pi_material_set_def(matl, impl->ENVIRONMENT_MAPPING, FALSE);
			}
		}
	}
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	WaterRenderer *impl = (WaterRenderer *)renderer->impl;

	impl->width = width;
	impl->height = height;

	pi_rendertarget_detach(impl->water_rt, ATT_COLOR0);
	pi_rendertarget_detach(impl->water_rt, ATT_COLOR1);
	pi_rendertarget_detach(impl->water_rt, ATT_DEPTHSTENCIL);

	pi_texture_free(impl->water_map);
	pi_texture_free(impl->water_depth);
	pi_texture_free(impl->water_depth_stencil);

	pi_renderview_free(impl->water_depth_view);
	pi_renderview_free(impl->water_map_view);
	pi_renderview_free(impl->water_depth_stencial_view);

	pi_rendertarget_detach(impl->reflection_rt, ATT_COLOR0);
	pi_rendertarget_detach(impl->reflection_rt, ATT_DEPTHSTENCIL);

	pi_texture_free(impl->reflection_tex);
	pi_renderview_free(impl->reflection_view);
	pi_renderview_free(impl->reflection_depth);


	_create_renderview(impl);

	pi_camera_set_frustum(impl->post_camera, -(float)width / 2.0f, (float)width / 2.0f, -(float)height / 2.0f, (float)height / 2.0f, 0.0f, 2.0f, TRUE);

	pi_spatial_set_local_scaling(impl->post_quad->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->post_quad->spatial);
}

PiRenderer *PI_API pi_water_new_with_name(char* name)
{
	PiRenderer *renderer;
	WaterRenderer *impl = pi_new0(WaterRenderer, 1);

	impl->sea_level = 0;
	impl->caustics_enable = TRUE;
	impl->hdr_mode = FALSE;
	impl->environment_enable = TRUE;
	impl->is_intz_support = pi_rendercap_is_format_support(pi_rendersystem_get_cap(), CFT_TEXTURE_FORMAT, RF_INTZ);
	renderer = pi_renderer_create(ERT_WATER, name, _init, _resize, _update, _draw, impl);
	return renderer;
}


PiRenderer *PI_API pi_water_new()
{
	return pi_water_new_with_name("water");
}

void PI_API pi_water_deploy(PiRenderer *renderer, char *view_cam_name, char *scene_color_name, char *scene_depth_name, char *output_name, char *env_name, char* vs_key, char* fs_key)
{
	WaterRenderer *impl;
	_type_check(renderer);
	impl = (WaterRenderer *)renderer->impl;
	if (impl->is_deploy)
	{
		pi_free(impl->scene_color_name);
		pi_free(impl->scene_depth_name);
		pi_free(impl->output_name);
		pi_free(impl->env_name);
		pi_free(impl->view_cam_name);
		pi_free(impl->VERTEX_SHADER);
		pi_free(impl->FRAGMENT_SHADER);
	}
	impl->view_cam_name = pi_str_dup(view_cam_name);
	impl->scene_color_name = pi_str_dup(scene_color_name);

	impl->scene_depth_name = pi_str_dup(scene_depth_name);
	impl->output_name = pi_str_dup(output_name);
	impl->env_name = pi_str_dup(env_name);
	impl->VERTEX_SHADER = pi_str_dup(vs_key);
	impl->FRAGMENT_SHADER = pi_str_dup(fs_key);
	impl->is_deploy = TRUE;
}

void PI_API pi_water_deploy_local_water(PiRenderer *renderer, char *local_water_list_name)
{
	WaterRenderer *impl;
	_type_check(renderer);
	impl = (WaterRenderer *)renderer->impl;
	pi_free(impl->local_water_list_name);
	impl->local_water_list_name = pi_str_dup(local_water_list_name);
}

void PI_API pi_water_deploy_reflection(PiRenderer *renderer, char *reflection_cam_name, char *reflection_list_name)
{
	WaterRenderer *impl;
	_type_check(renderer);
	impl = (WaterRenderer *)renderer->impl;
	pi_free(impl->reflection_cam_name);
	pi_free(impl->reflection_list_name);
	impl->reflection_cam_name = pi_str_dup(reflection_cam_name);
	impl->reflection_list_name = pi_str_dup(reflection_list_name);
}

void PI_API pi_water_free(PiRenderer *renderer)
{
	WaterRenderer *impl;
	_type_check(renderer);
	impl = (WaterRenderer *)renderer->impl;
	if (impl->is_deploy)
	{
		pi_free(impl->scene_color_name);
		pi_free(impl->scene_depth_name);
		pi_free(impl->local_water_list_name);
		pi_free(impl->output_name);
		pi_free(impl->view_cam_name);
		pi_free(impl->reflection_cam_name);
		pi_free(impl->reflection_list_name);
		pi_free(impl->env_name);
		pi_free(impl->VERTEX_SHADER);
		pi_free(impl->FRAGMENT_SHADER);
	}
	pi_mesh_free(impl->mesh);
	pi_rendermesh_free(impl->rmesh);
	pi_camera_free(impl->post_camera);
	pi_material_free(impl->post_matl);
	pi_entity_free(impl->post_quad);
	pi_rendertarget_free(impl->water_rt);
	pi_rendertarget_free(impl->reflection_rt);
	pi_renderview_free(impl->reflection_view);
	pi_renderview_free(impl->reflection_depth);
	pi_texture_free(impl->water_map);
	pi_texture_free(impl->water_depth);
	pi_texture_free(impl->water_depth_stencil);
	pi_renderview_free(impl->water_map_view);
	pi_renderview_free(impl->water_depth_view);
	pi_renderview_free(impl->water_depth_stencial_view);

	pi_texture_free(impl->reflection_tex);

	pi_free(renderer->impl);
	pi_renderer_destroy(renderer);
}

void PI_API pi_water_update_camera(PiRenderer *renderer, PiCamera *view_cam, PiCamera *reflection_cam)
{
	WaterRenderer *impl;
	float reflection_height;
	PiVector3 reflection_dir;
	PiVector3 reflection_up;
	_type_check(renderer);
	impl = (WaterRenderer *)renderer->impl;

	reflection_height = 2 * impl->sea_level - view_cam->location.y;
	pi_camera_get_direction(view_cam, &reflection_dir);
	reflection_dir.y = -reflection_dir.y;
	pi_camera_get_up(view_cam, &reflection_up);
	reflection_up.y = -reflection_up.y;
	pi_camera_set_frustum(reflection_cam, view_cam->frustum_left, view_cam->frustum_right, view_cam->frustum_bottom, view_cam->frustum_top, view_cam->frustum_near, view_cam->frustum_far, view_cam->is_ortho);
	pi_camera_set_location(reflection_cam, view_cam->location.x, reflection_height, view_cam->location.z);
	pi_camera_set_direction(reflection_cam, reflection_dir.x, reflection_dir.y, reflection_dir.z);
	pi_camera_set_up(reflection_cam, reflection_up.x, reflection_up.y, reflection_up.z);
}

void PI_API pi_water_set_normal_map(PiRenderer *renderer, PiTexture *normal_map)
{
	WaterRenderer *impl;
	_type_check(renderer);
	impl = (WaterRenderer *)renderer->impl;
	impl->normal_map = normal_map;
	_apply_global_water_normal_map(impl);
}

void PI_API pi_water_set_caustics_map(PiRenderer *renderer, PiTexture *caustics_map)
{
	WaterRenderer *impl;
	_type_check(renderer);
	impl = (WaterRenderer *)renderer->impl;
	impl->caustics_map = caustics_map;

	_apply_global_water_caustice_map(impl);
	impl->caustics_map_change = TRUE;
}

void PI_API pi_water_set_caustics_enable(PiRenderer *renderer, PiBool is_enable)
{
	WaterRenderer *impl;
	_type_check(renderer);
	impl = (WaterRenderer *)renderer->impl;
	impl->caustics_enable = is_enable;
	_apply_global_water_caustice_map(impl);
	impl->caustics_map_change = TRUE;
}

void PI_API pi_water_set_hdr_mode_enable(PiRenderer *renderer, PiBool is_enable)
{
	WaterRenderer *impl;
	_type_check(renderer);
	impl = (WaterRenderer *)renderer->impl;
	impl->hdr_mode = is_enable;
}

void PI_API pi_water_set_environment_enable(PiRenderer* renderer, PiBool is_enable)
{
	WaterRenderer *impl;
	_type_check(renderer);
	impl = (WaterRenderer *)renderer->impl;
	impl->environment_enable = is_enable;
}
