
#include <deferred_shading.h>
#include <pi_vector3.h>
#include <rendersystem.h>
#include <entity.h>
#include <material.h>
#include <environment.h>
#include <camera.h>
#include <preshadow_pcf.h>
#include <preshadow_vsm.h>

/**
 * deferred_shading渲染器
 */
typedef struct
{
	PiBool is_deploy;
	PiBool is_init;

	char *target_name;
	char *view_cam_name;
	char *env_name;

	char *g_buffer_tex0_name;
	char *g_buffer_tex1_name;
	char *g_buffer_tex2_name;
	char *g_buffer_tex3_name;
	char *g_buffer_depth_name;

	char *lighting_diffuse_name;
	char *lighting_sepcular_name;

	PiBool is_shadow_deploy;
	DeferredShadingShadowMapType shadow_type;
	char *shadow_data_name;

	PiBool is_decal_deploy;
	char *decal_map_name;
	char *decal_matrix_name;
	char *decal_z_far_name;

	PiCamera *screen_quad_camera;
	PiMesh *screen_quad_mesh;
	PiRenderMesh *screen_quad_rmesh;

	PiEntity *composition_quad;
	PiMaterial *composition_quad_material;

	/* 从渲染管线获取的各种资源*/
	PiTexture *lighting_diffuse;
	PiTexture *lighting_specular;

	PiTexture *g_buffer_tex0;
	PiTexture *g_buffer_tex1;
	PiTexture *g_buffer_tex2;
	PiTexture *g_buffer_tex3;
	PiTexture *g_buffer_depth;

	PiEnvironment *env;

	/* Unform的值*/
	PiVector3 view_position;
	PiMatrix4 view_proj_matrix_inverse;
	PiMatrix4 proj_mat_inverse;

	void *shadow_data;
	SamplerState jitter_sampler;
	SamplerState shadow_map_sampler;

	SamplerState decal_map_sampler;
	PiTexture *decal_map;
	PiMatrix4 *decal_matrix;
	float *decal_z_far;

	SamplerState cube_tex_sampler;

	SamplerState g_buffer_tex0_sampler;
	SamplerState g_buffer_tex1_sampler;
	SamplerState g_buffer_tex2_sampler;
	SamplerState g_buffer_tex3_sampler;
	SamplerState g_buffer_depth_sampler;

	SamplerState lighting_diffuse_sampler;
	SamplerState lighting_specular_sampler;

	/* 常量字符串 */
	char *SHADOW;
	char *SHADOW_PCF;
	char *SHADOW_VSM;
	char *PCF_HARD;
	char *PCF_8X;
	char *PCF_16X;
	char *PCF_32X;
	char *PCF_48X;
	char *PCF_64X;

	char *DECAL;

	char *ENVIRONMENT_MAPPING;
	char *CYLINDER_MAPPING;

	char *U_DirLightDir;
	char *U_DirLightColor;
	char *U_AmbientColor;

	char *U_EnvironmentMap;

	char *U_ShadowMap;
	char *U_ShadowMatrix;
	char *U_ShadowZFar;
	char *U_JitterTexture;
	char *U_PCFFilterSize;

	char *U_DecalMap;
	char *U_DecalMatrix;
	char *U_DecalZFar;

	char *U_ViewProjMatrixInverse;
	char *U_ProjMatrixInverse;
	char *U_WorldViewPosition;

	char *U_GBufferTex0;
	char *U_GBufferTex1;
	char *U_GBufferTex2;
	char *U_GBufferTex3;
	char *U_GBufferDepthTex;

	char *U_LightingDiffuse;
	char *U_LightingSpecular;
} DeferredShadingRenderer;

static void _type_check(PiRenderer *renderer)
{
	PI_ASSERT(renderer->type == ERT_DEFERRED_SHADING, "Renderer type error!");
}

static void _init_const_string(DeferredShadingRenderer *impl)
{
	impl->shadow_data = NULL;

	impl->decal_map = NULL;
	impl->decal_matrix = NULL;
	impl->decal_z_far = NULL;

	impl->g_buffer_tex0 = NULL;
	impl->g_buffer_tex1 = NULL;
	impl->g_buffer_tex2 = NULL;
	impl->g_buffer_tex3 = NULL;
	impl->g_buffer_depth = NULL;

	impl->env = NULL;

	impl->SHADOW = pi_conststr("SHADOW");
	impl->SHADOW_PCF = pi_conststr("SHADOW_PCF");
	impl->SHADOW_VSM = pi_conststr("SHADOW_VSM");
	impl->PCF_HARD = pi_conststr("PCF_HARD");
	impl->PCF_8X = pi_conststr("PCF_8X");
	impl->PCF_16X = pi_conststr("PCF_16X");
	impl->PCF_32X = pi_conststr("PCF_32X");
	impl->PCF_48X = pi_conststr("PCF_48X");
	impl->PCF_64X = pi_conststr("PCF_64X");

	impl->DECAL = pi_conststr("DECAL");

	impl->ENVIRONMENT_MAPPING = pi_conststr("ENVIRONMENT_MAPPING");
	impl->CYLINDER_MAPPING = pi_conststr("CYLINDER_MAPPING");

	impl->U_DirLightDir = pi_conststr("u_DirLightDir");
	impl->U_DirLightColor = pi_conststr("u_DirLightColor");
	impl->U_AmbientColor = pi_conststr("u_AmbientColor");

	impl->U_EnvironmentMap = pi_conststr("u_EnvironmentMap");

	impl->U_ShadowMap = pi_conststr("u_ShadowMap");
	impl->U_ShadowMatrix = pi_conststr("u_ShadowMatrix");
	impl->U_ShadowZFar = pi_conststr("u_ShadowZFar");
	impl->U_JitterTexture = pi_conststr("u_JitterTexture");
	impl->U_PCFFilterSize = pi_conststr("u_PCFFilterSize");

	impl->U_DecalMap = pi_conststr("u_DecalMap");
	impl->U_DecalMatrix = pi_conststr("u_DecalMatrix");
	impl->U_DecalZFar = pi_conststr("u_DecalZFar");

	impl->U_ProjMatrixInverse = pi_conststr("u_ProjMatrixInverse");
	impl->U_WorldViewPosition = pi_conststr("u_WorldViewPosition");
	impl->U_ViewProjMatrixInverse = pi_conststr("u_ViewProjMatrixInverse");

	impl->U_GBufferTex0 = pi_conststr("u_GBufferTex0");
	impl->U_GBufferTex1 = pi_conststr("u_GBufferTex1");
	impl->U_GBufferTex2 = pi_conststr("u_GBufferTex2");
	impl->U_GBufferTex3 = pi_conststr("u_GBufferTex3");
	impl->U_GBufferDepthTex = pi_conststr("u_GBufferDepthTex");

	impl->U_LightingDiffuse = pi_conststr("u_LightingDiffuse");
	impl->U_LightingSpecular = pi_conststr("u_LightingSpecular");
}

static void _init_sampler(DeferredShadingRenderer *impl)
{
	PiColor borderColor;
	color_set(&borderColor, 0.0f, 0.0f, 0.0f, 1.0f);

	pi_renderstate_set_default_sampler(&impl->shadow_map_sampler);
	pi_sampler_set_addr_mode(&impl->shadow_map_sampler, TAM_BORDER, TAM_BORDER, TAM_BORDER);
	pi_sampler_set_filter(&impl->shadow_map_sampler, TFO_MIN_MAG_LINEAR);
	pi_sampler_set_compare_func(&impl->shadow_map_sampler, CF_LESSEQUAL);

	pi_renderstate_set_default_sampler(&impl->decal_map_sampler);
	pi_sampler_set_addr_mode(&impl->decal_map_sampler, TAM_BORDER, TAM_BORDER, TAM_BORDER);
	pi_sampler_set_border_color(&impl->decal_map_sampler, &borderColor);
	pi_sampler_set_filter(&impl->decal_map_sampler, TFO_MIN_MAG_LINEAR);

	pi_renderstate_set_default_sampler(&impl->cube_tex_sampler);
	pi_sampler_set_addr_mode(&impl->cube_tex_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->cube_tex_sampler, TFO_MIN_MAG_MIP_LINEAR);

	pi_renderstate_set_default_sampler(&impl->g_buffer_tex0_sampler);
	pi_sampler_set_addr_mode(&impl->g_buffer_tex0_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->g_buffer_tex0_sampler, TFO_MIN_MAG_POINT);

	pi_renderstate_set_default_sampler(&impl->g_buffer_tex1_sampler);
	pi_sampler_set_addr_mode(&impl->g_buffer_tex1_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->g_buffer_tex1_sampler, TFO_MIN_MAG_POINT);

	pi_renderstate_set_default_sampler(&impl->g_buffer_tex2_sampler);
	pi_sampler_set_addr_mode(&impl->g_buffer_tex2_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->g_buffer_tex2_sampler, TFO_MIN_MAG_POINT);

	pi_renderstate_set_default_sampler(&impl->g_buffer_tex3_sampler);
	pi_sampler_set_addr_mode(&impl->g_buffer_tex3_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->g_buffer_tex3_sampler, TFO_MIN_MAG_POINT);

	pi_renderstate_set_default_sampler(&impl->g_buffer_depth_sampler);
	pi_sampler_set_addr_mode(&impl->g_buffer_depth_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->g_buffer_depth_sampler, TFO_MIN_MAG_POINT);

	pi_renderstate_set_default_sampler(&impl->lighting_diffuse_sampler);
	pi_sampler_set_addr_mode(&impl->lighting_diffuse_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->lighting_diffuse_sampler, TFO_MIN_MAG_POINT);

	pi_renderstate_set_default_sampler(&impl->lighting_specular_sampler);
	pi_sampler_set_addr_mode(&impl->lighting_specular_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->lighting_specular_sampler, TFO_MIN_MAG_POINT);
}

static void _init_screen_quad_camera(DeferredShadingRenderer *impl, uint32 width, uint32 height)
{
	impl->screen_quad_camera = pi_camera_new();
	pi_camera_set_location(impl->screen_quad_camera, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->screen_quad_camera, 0.0f, 0.0f, -1.0f);
	pi_camera_set_frustum(impl->screen_quad_camera, -(float)width / 2.0f, width / 2.0f, -(float)height / 2.0f, (float)height / 2.0f, 0.0f, 2.0f, TRUE);
}

static void _init_screen_quad(DeferredShadingRenderer *impl, uint32 width, uint32 height)
{
	impl->composition_quad_material = pi_material_new(RS_DEFERRED_SHADING_VS, RS_DEFERRED_SHADING_FS);

	pi_material_set_depth_enable(impl->composition_quad_material, TRUE);
	pi_material_set_depthwrite_enable(impl->composition_quad_material, TRUE);

	pi_material_set_uniform(impl->composition_quad_material, impl->U_ViewProjMatrixInverse, UT_MATRIX4, 1, &impl->view_proj_matrix_inverse, FALSE);
	pi_material_set_uniform(impl->composition_quad_material, impl->U_ProjMatrixInverse, UT_MATRIX4, 1, &impl->proj_mat_inverse, FALSE);
	pi_material_set_uniform(impl->composition_quad_material, impl->U_WorldViewPosition, UT_VEC3, 1, &impl->view_position, FALSE);

	impl->composition_quad = pi_entity_new();
	pi_entity_set_material(impl->composition_quad, impl->composition_quad_material);

	impl->screen_quad_mesh = pi_mesh_create_quad(NULL, NULL, 0.0f);
	impl->screen_quad_rmesh = pi_rendermesh_new(impl->screen_quad_mesh, TRUE);
	pi_entity_set_mesh(impl->composition_quad, impl->screen_quad_rmesh);

	pi_spatial_set_local_scaling(impl->composition_quad->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->composition_quad->spatial);
}

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	DeferredShadingRenderer *impl = (DeferredShadingRenderer *)renderer->impl;
	PiRenderTarget *target;
	uint32 width, height;

	if (!impl->is_deploy)
	{
		return FALSE;
	}

	_init_const_string(impl);

	_init_sampler(impl);

	pi_hash_lookup(resources, impl->target_name, (void **)&target);
	width = target->width;
	height = target->height;

	_init_screen_quad_camera(impl, width, height);

	_init_screen_quad(impl, width, height);

	impl->is_init = TRUE;

	return TRUE;
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	DeferredShadingRenderer *impl;
	PiRenderTarget *target;

	PI_USE_PARAM(tpf);
	_type_check(renderer);
	impl = (DeferredShadingRenderer *)renderer->impl;

	pi_hash_lookup(resources, impl->target_name, (void **)&target);

	pi_rendersystem_set_target(target);
	pi_rendersystem_set_camera(impl->screen_quad_camera);
	pi_entity_draw(impl->composition_quad);
}

static void _set_pcf_quality(DeferredShadingRenderer *renderer, PiMaterial *material, PCFShadowPipelineData *shadow_data)
{
	switch (shadow_data->shadow_samples)
	{
		case PCF_SS_8X:
			pi_material_set_def(material, renderer->PCF_HARD, FALSE);
			pi_material_set_def(material, renderer->PCF_8X, TRUE);
			pi_material_set_def(material, renderer->PCF_16X, FALSE);
			pi_material_set_def(material, renderer->PCF_32X, FALSE);
			pi_material_set_def(material, renderer->PCF_48X, FALSE);
			pi_material_set_def(material, renderer->PCF_64X, FALSE);
			break;

		case PCF_SS_16X:
			pi_material_set_def(material, renderer->PCF_HARD, FALSE);
			pi_material_set_def(material, renderer->PCF_8X, FALSE);
			pi_material_set_def(material, renderer->PCF_16X, TRUE);
			pi_material_set_def(material, renderer->PCF_32X, FALSE);
			pi_material_set_def(material, renderer->PCF_48X, FALSE);
			pi_material_set_def(material, renderer->PCF_64X, FALSE);
			break;

		case PCF_SS_32X:
			pi_material_set_def(material, renderer->PCF_HARD, FALSE);
			pi_material_set_def(material, renderer->PCF_8X, FALSE);
			pi_material_set_def(material, renderer->PCF_16X, FALSE);
			pi_material_set_def(material, renderer->PCF_32X, TRUE);
			pi_material_set_def(material, renderer->PCF_48X, FALSE);
			pi_material_set_def(material, renderer->PCF_64X, FALSE);
			break;

		case PCF_SS_48X:
			pi_material_set_def(material, renderer->PCF_HARD, FALSE);
			pi_material_set_def(material, renderer->PCF_8X, FALSE);
			pi_material_set_def(material, renderer->PCF_16X, FALSE);
			pi_material_set_def(material, renderer->PCF_32X, FALSE);
			pi_material_set_def(material, renderer->PCF_48X, TRUE);
			pi_material_set_def(material, renderer->PCF_64X, FALSE);
			break;

		case PCF_SS_64X:
			pi_material_set_def(material, renderer->PCF_HARD, FALSE);
			pi_material_set_def(material, renderer->PCF_8X, FALSE);
			pi_material_set_def(material, renderer->PCF_16X, FALSE);
			pi_material_set_def(material, renderer->PCF_32X, FALSE);
			pi_material_set_def(material, renderer->PCF_48X, FALSE);
			pi_material_set_def(material, renderer->PCF_64X, TRUE);
			break;

		default:
			pi_material_set_def(material, renderer->PCF_HARD, TRUE);
			pi_material_set_def(material, renderer->PCF_8X, FALSE);
			pi_material_set_def(material, renderer->PCF_16X, FALSE);
			pi_material_set_def(material, renderer->PCF_32X, FALSE);
			pi_material_set_def(material, renderer->PCF_48X, FALSE);
			pi_material_set_def(material, renderer->PCF_64X, FALSE);
			break;
	}
}

static void _update(PiRenderer *renderer, float tpf, PiHash *resources)
{
	DeferredShadingRenderer *impl;

	PI_USE_PARAM(tpf);
	_type_check(renderer);
	impl = (DeferredShadingRenderer *)renderer->impl;

	if (impl->is_shadow_deploy)
	{
		pi_hash_lookup(resources, impl->shadow_data_name, (void **)&impl->shadow_data);
	}
	else
	{
		impl->shadow_data = NULL;
	}

	if (impl->is_decal_deploy)
	{
		pi_hash_lookup(resources, impl->decal_map_name, (void **)&impl->decal_map);
		pi_hash_lookup(resources, impl->decal_matrix_name, (void **)&impl->decal_matrix);
		pi_hash_lookup(resources, impl->decal_z_far_name, (void **)&impl->decal_z_far);
	}
	else
	{
		impl->decal_map = NULL;
	}

	if (impl->shadow_data != NULL)
	{
		PCFShadowPipelineData *pcf_data = NULL;
		VSMShadowPipelineData *vsm_data = NULL;

		pi_material_set_def(impl->composition_quad_material, impl->SHADOW, TRUE);

		switch (impl->shadow_type)
		{
			case DS_SMT_PCF:
				pcf_data = (PCFShadowPipelineData *)impl->shadow_data;
				pi_sampler_set_texture(&impl->shadow_map_sampler, pcf_data->shadow_map);
				pi_sampler_set_texture(&impl->jitter_sampler, pcf_data->jitter_texture);
				pi_material_set_uniform(impl->composition_quad_material, impl->U_ShadowMap, UT_SAMPLER_2D_SHADOW, 1, &impl->shadow_map_sampler, FALSE);
				pi_material_set_uniform(impl->composition_quad_material, impl->U_ShadowMatrix, UT_MATRIX4, 1, &pcf_data->shadow_matrix, FALSE);
				pi_material_set_uniform(impl->composition_quad_material, impl->U_ShadowZFar, UT_FLOAT, 1, &pcf_data->shadow_z_far, FALSE);
				pi_material_set_uniform(impl->composition_quad_material, impl->U_PCFFilterSize, UT_FLOAT, 1, &pcf_data->filter_size, FALSE);
				pi_material_set_uniform(impl->composition_quad_material, impl->U_JitterTexture, UT_SAMPLER_3D, 1, &impl->jitter_sampler, FALSE);
				pi_material_set_def(impl->composition_quad_material, impl->SHADOW_PCF, TRUE);
				pi_material_set_def(impl->composition_quad_material, impl->SHADOW_VSM, FALSE);
				_set_pcf_quality(impl, impl->composition_quad_material, (PCFShadowPipelineData *)impl->shadow_data);
				break;

			case DS_SMT_VSM:
				vsm_data = (VSMShadowPipelineData *)impl->shadow_data;
				pi_sampler_set_texture(&impl->shadow_map_sampler, vsm_data->shadow_map);
				pi_material_set_uniform(impl->composition_quad_material, impl->U_ShadowMap, UT_SAMPLER_2D, 1, &impl->shadow_map_sampler, FALSE);
				pi_material_set_uniform(impl->composition_quad_material, impl->U_ShadowMatrix, UT_MATRIX4, 1, &vsm_data->shadow_matrix, FALSE);
				pi_material_set_uniform(impl->composition_quad_material, impl->U_ShadowZFar, UT_FLOAT, 1, &vsm_data->shadow_z_far, FALSE);
				pi_material_set_def(impl->composition_quad_material, impl->SHADOW_PCF, FALSE);
				pi_material_set_def(impl->composition_quad_material, impl->SHADOW_VSM, TRUE);
				break;

			default:
				break;
		}
	}
	else
	{
		pi_material_set_def(impl->composition_quad_material, impl->SHADOW, FALSE);
	}

	if (impl->decal_map != NULL)
	{
		pi_sampler_set_texture(&impl->decal_map_sampler, impl->decal_map);
		pi_material_set_uniform(impl->composition_quad_material, impl->U_DecalMap, UT_SAMPLER_2D, 1, &impl->decal_map_sampler, FALSE);
		pi_material_set_uniform(impl->composition_quad_material, impl->U_DecalMatrix, UT_MATRIX4, 1, impl->decal_matrix, FALSE);
		pi_material_set_uniform(impl->composition_quad_material, impl->U_DecalZFar, UT_FLOAT, 1, impl->decal_z_far, FALSE);

		pi_material_set_def(impl->composition_quad_material, impl->DECAL, TRUE);
	}
	else
	{
		pi_material_set_def(impl->composition_quad_material, impl->DECAL, FALSE);
	}

	pi_hash_lookup(resources, impl->env_name, (void **)&impl->env);

	pi_material_set_uniform(impl->composition_quad_material, impl->U_DirLightDir, UT_VEC3, 1, &impl->env->diffuse_dir, FALSE);
	pi_material_set_uniform(impl->composition_quad_material, impl->U_DirLightColor, UT_VEC3, 1, &impl->env->diffuse_color, FALSE);
	pi_material_set_uniform(impl->composition_quad_material, impl->U_AmbientColor, UT_VEC3, 1, &impl->env->ambient_color, FALSE);

	if (impl->env->env_tex != NULL)
	{
		pi_material_set_def(impl->composition_quad_material, impl->ENVIRONMENT_MAPPING, TRUE);
		pi_sampler_set_texture(&impl->cube_tex_sampler, impl->env->env_tex);
		pi_material_set_def(impl->composition_quad_material, impl->CYLINDER_MAPPING, impl->env->is_cylinder);
		if (impl->env->is_cylinder)
		{
			pi_material_set_uniform(impl->composition_quad_material, impl->U_EnvironmentMap, UT_SAMPLER_2D, 1, &impl->cube_tex_sampler, FALSE);
		}
		else
		{
			pi_material_set_uniform(impl->composition_quad_material, impl->U_EnvironmentMap, UT_SAMPLER_CUBE, 1, &impl->cube_tex_sampler, FALSE);
		}
	}
	else
	{
		pi_material_set_def(impl->composition_quad_material, impl->ENVIRONMENT_MAPPING, FALSE);
	}

	{
		PiCamera *view_camera;
		pi_hash_lookup(resources, impl->view_cam_name, (void **)&view_camera);

		pi_mat4_inverse(&impl->view_proj_matrix_inverse, pi_camera_get_view_projection_matrix(view_camera));
		pi_mat4_inverse(&impl->proj_mat_inverse, pi_camera_get_projection_matrix(view_camera));
		pi_vec3_copy(&impl->view_position, pi_camera_get_location(view_camera));
	}

	pi_hash_lookup(resources, impl->g_buffer_tex0_name, (void **)&impl->g_buffer_tex0);
	pi_hash_lookup(resources, impl->g_buffer_tex1_name, (void **)&impl->g_buffer_tex1);
	pi_hash_lookup(resources, impl->g_buffer_tex2_name, (void **)&impl->g_buffer_tex2);
	pi_hash_lookup(resources, impl->g_buffer_tex3_name, (void **)&impl->g_buffer_tex3);
	pi_hash_lookup(resources, impl->g_buffer_depth_name, (void **)&impl->g_buffer_depth);

	pi_sampler_set_texture(&impl->g_buffer_tex0_sampler, impl->g_buffer_tex0);
	pi_sampler_set_texture(&impl->g_buffer_tex1_sampler, impl->g_buffer_tex1);
	pi_sampler_set_texture(&impl->g_buffer_tex2_sampler, impl->g_buffer_tex2);
	pi_sampler_set_texture(&impl->g_buffer_tex3_sampler, impl->g_buffer_tex3);
	pi_sampler_set_texture(&impl->g_buffer_depth_sampler, impl->g_buffer_depth);

	pi_material_set_uniform(impl->composition_quad_material, impl->U_GBufferTex0, UT_SAMPLER_2D, 1, &impl->g_buffer_tex0_sampler, FALSE);
	pi_material_set_uniform(impl->composition_quad_material, impl->U_GBufferTex1, UT_SAMPLER_2D, 1, &impl->g_buffer_tex1_sampler, FALSE);
	pi_material_set_uniform(impl->composition_quad_material, impl->U_GBufferTex2, UT_SAMPLER_2D, 1, &impl->g_buffer_tex2_sampler, FALSE);
	pi_material_set_uniform(impl->composition_quad_material, impl->U_GBufferTex3, UT_SAMPLER_2D, 1, &impl->g_buffer_tex3_sampler, FALSE);
	pi_material_set_uniform(impl->composition_quad_material, impl->U_GBufferDepthTex, UT_SAMPLER_2D, 1, &impl->g_buffer_depth_sampler, FALSE);

	pi_hash_lookup(resources, impl->lighting_diffuse_name, (void **)&impl->lighting_diffuse);
	pi_hash_lookup(resources, impl->lighting_sepcular_name, (void **)&impl->lighting_specular);

	pi_sampler_set_texture(&impl->lighting_diffuse_sampler, impl->lighting_diffuse);
	pi_sampler_set_texture(&impl->lighting_specular_sampler, impl->lighting_specular);

	pi_material_set_uniform(impl->composition_quad_material, impl->U_LightingDiffuse, UT_SAMPLER_2D, 1, &impl->lighting_diffuse_sampler, FALSE);
	pi_material_set_uniform(impl->composition_quad_material, impl->U_LightingSpecular, UT_SAMPLER_2D, 1, &impl->lighting_specular_sampler, FALSE);
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	DeferredShadingRenderer *impl;
	_type_check(renderer);
	impl = (DeferredShadingRenderer *)renderer->impl;

	pi_spatial_set_local_scaling(impl->composition_quad->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->composition_quad->spatial);

	pi_camera_set_frustum(impl->screen_quad_camera, -(float)width / 2.0f, width / 2.0f, -(float)height / 2.0f, (float)height / 2.0f, 0.0f, 2.0f, TRUE);
}

PiRenderer *PI_API pi_deferred_shading_new()
{
	PiRenderer *renderer;
	DeferredShadingRenderer *impl = pi_new0(DeferredShadingRenderer, 1);
	renderer = pi_renderer_create(ERT_DEFERRED_SHADING, "deferred shading renderer", _init, _resize, _update, _draw, impl);
	return renderer;
}

void PI_API pi_deferred_shading_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *env_name, char *lighting_diffuse_name, char *lighting_sepcular_name,
                                       char *g_buffer_tex0_name, char *g_buffer_tex1_name, char *g_buffer_tex2_name, char *g_buffer_tex3_name, char *g_buffer_depth_name)
{
	DeferredShadingRenderer *impl;
	_type_check(renderer);
	impl = (DeferredShadingRenderer *)renderer->impl;

	pi_free(impl->target_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->env_name);

	pi_free(impl->lighting_diffuse_name);
	pi_free(impl->lighting_sepcular_name);

	pi_free(impl->g_buffer_tex0_name);
	pi_free(impl->g_buffer_tex1_name);
	pi_free(impl->g_buffer_tex2_name);
	pi_free(impl->g_buffer_tex3_name);
	pi_free(impl->g_buffer_depth_name);

	impl->target_name = pi_str_dup(target_name);
	impl->view_cam_name = pi_str_dup(view_cam_name);
	impl->env_name = pi_str_dup(env_name);

	impl->lighting_diffuse_name = pi_str_dup(lighting_diffuse_name);
	impl->lighting_sepcular_name = pi_str_dup(lighting_sepcular_name);

	impl->g_buffer_tex0_name = pi_str_dup(g_buffer_tex0_name);
	impl->g_buffer_tex1_name = pi_str_dup(g_buffer_tex1_name);
	impl->g_buffer_tex2_name = pi_str_dup(g_buffer_tex2_name);
	impl->g_buffer_tex3_name = pi_str_dup(g_buffer_tex3_name);
	impl->g_buffer_depth_name = pi_str_dup(g_buffer_depth_name);

	impl->is_deploy = TRUE;
}

void PI_API pi_deferred_shading_deploy_shadow(PiRenderer *renderer, DeferredShadingShadowMapType type, char *shadow_data_name)
{
	DeferredShadingRenderer *impl;
	_type_check(renderer);
	impl = (DeferredShadingRenderer *)renderer->impl;
	pi_free(impl->shadow_data_name);

	impl->shadow_type = type;

	if (type != DS_SMT_NONE && shadow_data_name)
	{
		impl->shadow_data_name = pi_str_dup(shadow_data_name);

		impl->is_shadow_deploy = TRUE;
	}
	else
	{
		impl->shadow_data_name = NULL;

		impl->is_shadow_deploy = FALSE;
	}
}

void PI_API pi_deferred_shading_deploy_decal(PiRenderer *renderer, char *decal_map_name, char *decal_matrix_name, char *decal_z_far_name)
{
	DeferredShadingRenderer *impl;
	_type_check(renderer);
	impl = (DeferredShadingRenderer *)renderer->impl;
	pi_free(impl->decal_map_name);
	pi_free(impl->decal_matrix_name);
	pi_free(impl->decal_z_far_name);

	if (decal_map_name && decal_matrix_name && decal_z_far_name)
	{
		impl->decal_map_name = pi_str_dup(decal_map_name);
		impl->decal_matrix_name = pi_str_dup(decal_matrix_name);
		impl->decal_z_far_name = pi_str_dup(decal_z_far_name);

		impl->is_decal_deploy = TRUE;
	}
	else
	{
		impl->decal_map_name = NULL;
		impl->decal_matrix_name = NULL;
		impl->decal_z_far_name = NULL;

		impl->is_decal_deploy = FALSE;
	}
}

void PI_API pi_deferred_shading_free(PiRenderer *renderer)
{
	DeferredShadingRenderer *impl;
	_type_check(renderer);
	impl = (DeferredShadingRenderer *)renderer->impl;

	pi_free(impl->target_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->env_name);

	pi_free(impl->lighting_diffuse_name);
	pi_free(impl->lighting_sepcular_name);

	pi_free(impl->g_buffer_tex0_name);
	pi_free(impl->g_buffer_tex1_name);
	pi_free(impl->g_buffer_tex2_name);
	pi_free(impl->g_buffer_tex3_name);
	pi_free(impl->g_buffer_depth_name);

	pi_free(impl->shadow_data_name);

	pi_free(impl->decal_map_name);
	pi_free(impl->decal_matrix_name);
	pi_free(impl->decal_z_far_name);

	if (!impl->is_init)
	{
		return;
	}

	pi_camera_free(impl->screen_quad_camera);

	pi_entity_free(impl->composition_quad);
	pi_material_free(impl->composition_quad_material);

	pi_rendermesh_free(impl->screen_quad_rmesh);
	pi_mesh_free(impl->screen_quad_mesh);

	pi_free(impl);

	pi_renderer_destroy(renderer);
}
