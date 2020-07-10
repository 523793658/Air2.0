#include <lighting.h>

#include <pi_vector3.h>
#include <rendersystem.h>
#include <entity.h>
#include <material.h>
#include <environment.h>
#include <preshadow_pcf.h>
#include <camera.h>
#include <lights.h>
#include "vector4.h"
#define MAX_FR_LOCAL_LIGHT_NUM 10

typedef struct _PointLights
{
	PiVector4 multi_light_pos_decay_cache[MAX_FR_LOCAL_LIGHT_NUM];
	PiVector4 multi_light_color_radius_cache[MAX_FR_LOCAL_LIGHT_NUM];
}PointLights;

/**
 * 光照渲染器
 */
typedef struct
{
	PiBool is_lighting_deploy;
	PiBool view_space_lighting;
	PiBool is_vl;

	char *entity_list_name;
	char *target_name;
	char *view_cam_name;
	char *env_name;

	PiBool is_shadow_deploy;
	char *shadow_data_name;

	PiBool is_decal_deploy;
	char *decal_map_name;
	char *decal_matrix_name;
	char *decal_z_far_name;

	PiBool is_refraction_deploy;
	char *refraction_map_name;

	PiEnvironment *env;

	PiCamera *view_cam;

	/* Unform的值*/
	PCFShadowPipelineData *shadow_data;

	SamplerState decal_map_sampler;
	PiTexture *decal_map;
	PiMatrix4 *decal_matrix;
	float *decal_z_far;

	SamplerState refraction_map_sampler;
	PiTexture *refraction_map;

	SamplerState cube_tex_sampler;
	PiVector *light_list;

	/* 常量字符串 */
	char *SHADOW;
	char *DECAL;
	char *light_pos_name;

	char *MULTI_LIGHTING;
	char *CYLINDER_MAPPING;

	char *U_DefaultLightData;

	char *U_ExtraLightData;

	char *U_EnvironmentMap;

	char *U_RefractionMap;

	char *U_ShadowMatrix;
	char *U_DecalMap;
	char *U_DecalMatrix;
	char *U_DecalZFar;

	char *U_PointLightNum;
	char *U_PointLightColorRadiusArray;
	char *U_ViewPointLightPosDecayArray;
	char *U_WorldPointLightPosDecayArray;
	PointLights point_lights_cache;
} LightingRenderer;

static void _type_check(PiRenderer *renderer)
{
	PI_ASSERT(renderer->type == ERT_FORWARD_LIGHTING, "Renderer type error!");
}

static void _init_const_string(LightingRenderer *impl)
{
	impl->shadow_data = NULL;

	impl->decal_map = NULL;
	impl->decal_matrix = NULL;
	impl->decal_z_far = NULL;

	impl->env = NULL;

	impl->light_list = NULL;

	impl->view_cam = NULL;

	impl->refraction_map = NULL;

	impl->SHADOW = pi_conststr("SHADOW");

	impl->DECAL = pi_conststr("DECAL");

	impl->MULTI_LIGHTING = pi_conststr("MULTI_LIGHTING");
	impl->CYLINDER_MAPPING = pi_conststr("CYLINDER_MAPPING");

	impl->U_DefaultLightData = pi_conststr("u_DefaultLightData");

	impl->U_ExtraLightData = pi_conststr("u_ExtraLightData");

	impl->U_EnvironmentMap = pi_conststr("u_EnvironmentMap");

	impl->U_RefractionMap = pi_conststr("u_RefractionMap");

	impl->U_ShadowMatrix = pi_conststr("u_ShadowMatrix");

	impl->U_DecalMap = pi_conststr("u_DecalMap");
	impl->U_DecalMatrix = pi_conststr("u_DecalMatrix");
	impl->U_DecalZFar = pi_conststr("u_DecalZFar");

	impl->U_PointLightNum = pi_conststr("u_PointLightNum");
	impl->U_PointLightColorRadiusArray = pi_conststr("u_PointLightColorRadiusArray");
	impl->U_ViewPointLightPosDecayArray = pi_conststr("u_ViewPointLightPosDecayArray");
	impl->U_WorldPointLightPosDecayArray = pi_conststr("u_WorldPointLightPosDecayArray");
}

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	LightingRenderer *impl = (LightingRenderer *)renderer->impl;
	PiColor borderColor;
	color_set(&borderColor, 0.0f, 0.0f, 0.0f, 1.0f);

	if (!impl->is_lighting_deploy)
	{
		return FALSE;
	}

	_init_const_string(impl);
	if (impl->view_space_lighting)
	{
		impl->light_pos_name = impl->U_ViewPointLightPosDecayArray;
	}
	else
	{
		impl->light_pos_name = impl->U_WorldPointLightPosDecayArray;
	}

	pi_renderstate_set_default_sampler(&impl->decal_map_sampler);
	pi_sampler_set_addr_mode(&impl->decal_map_sampler, TAM_BORDER, TAM_BORDER, TAM_BORDER);
	pi_sampler_set_border_color(&impl->decal_map_sampler, &borderColor);
	pi_sampler_set_filter(&impl->decal_map_sampler, TFO_MIN_MAG_LINEAR);

	pi_renderstate_set_default_sampler(&impl->refraction_map_sampler);
	pi_sampler_set_addr_mode(&impl->refraction_map_sampler, TAM_BORDER, TAM_BORDER, TAM_BORDER);
	pi_sampler_set_filter(&impl->refraction_map_sampler, TFO_MIN_MAG_LINEAR);

	pi_renderstate_set_default_sampler(&impl->cube_tex_sampler);
	pi_sampler_set_addr_mode(&impl->cube_tex_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->cube_tex_sampler, TFO_MIN_MAG_MIP_LINEAR);

	return TRUE;
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	LightingRenderer *impl;
	PiRenderTarget *target;
	PiCamera *view_camera;
	PiVector *entity_list;
	PI_USE_PARAM(tpf);
	_type_check(renderer);
	impl = (LightingRenderer *)renderer->impl;

	pi_hash_lookup(resources, impl->entity_list_name, (void **)&entity_list);
	pi_hash_lookup(resources, impl->view_cam_name, (void **)&view_camera);
	pi_hash_lookup(resources, impl->target_name, (void **)&target);

	pi_rendersystem_set_target(target);
	pi_rendersystem_set_camera(view_camera);
	pi_entity_draw_list(entity_list);
}

static void _set_pcf_quality(LightingRenderer *renderer, PiMaterial *material, PCFShadowPipelineData *shadow_data)
{
	/*switch (shadow_data->shadow_samples)
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
	}*/
}

static void _update(PiRenderer *renderer, float tpf, PiHash *resources)
{
	uint i, n;
	LightingRenderer *impl;
	PiEntity *entity;
	PiMaterial *material;
	PiVector *entity_list;
	PiRenderSystem *sys = pi_rendersystem_get_instance();


	PI_USE_PARAM(tpf);
	_type_check(renderer);
	impl = (LightingRenderer *)renderer->impl;

	pi_hash_lookup(resources, impl->env_name, (void **)&impl->env);
	pi_hash_lookup(resources, impl->view_cam_name, (void **)&impl->view_cam);
	if (impl->is_refraction_deploy)
	{
		pi_hash_lookup(resources, impl->refraction_map_name, (void **)&impl->refraction_map);
		pi_sampler_set_texture(&impl->refraction_map_sampler, impl->refraction_map);
	}

	if (impl->is_shadow_deploy)
	{
		pi_hash_lookup(resources, impl->shadow_data_name, (void **)&impl->shadow_data);
		impl->env->default_light.shadow_z_far = impl->shadow_data->shadow_z_far;
		impl->env->default_light.shadow_map_size_inv = impl->shadow_data->sizeInv;
		impl->env->extra_light.shadow_z_far = impl->shadow_data->shadow_z_far;
		impl->env->extra_light.shadow_map_size_inv = impl->shadow_data->sizeInv;
		renderutil_set_shadow_data(&sys->gdv, impl->shadow_data->shadow_map);
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

	if (impl->env->env_tex != NULL)
	{
		pi_sampler_set_texture(&impl->cube_tex_sampler, impl->env->env_tex);
	}

	pi_hash_lookup(resources, impl->entity_list_name, (void **)&entity_list);
	n = pi_vector_size(entity_list);

	for (i = 0; i < n; ++i)
	{
		entity = (PiEntity *)pi_vector_get(entity_list, i);
		material = entity->material;
		impl->light_list = (PiVector *)pi_entity_get_bind(entity, EBT_LIGHT_LIST);

		pi_material_set_uniform(material, impl->U_DefaultLightData, UT_STRUCT, 1, &impl->env->default_light, FALSE);
		pi_material_set_uniform(material, impl->U_ExtraLightData, UT_STRUCT, 1, &impl->env->extra_light, FALSE);

		if (impl->refraction_map != NULL)
		{
			pi_material_set_uniform(material, impl->U_RefractionMap, UT_SAMPLER_2D, 1, &impl->refraction_map_sampler, FALSE);
		}
		if (impl->shadow_data != NULL && !pi_entity_get_bind(entity, EBT_DISCARD_SHADOW))
		{
			PCFShadowPipelineData *pcf_data = NULL;
			pcf_data = impl->shadow_data;
			pi_material_set_def(material, impl->SHADOW, TRUE);
			pi_material_set_uniform(material, impl->U_ShadowMatrix, UT_MATRIX4, 1, &pcf_data->shadow_matrix, FALSE);
		}
		else
		{
			pi_material_set_def(material, impl->SHADOW, FALSE);
		}

		if (impl->decal_map != NULL)
		{
			pi_sampler_set_texture(&impl->decal_map_sampler, impl->decal_map);
			pi_material_set_uniform(material, impl->U_DecalMap, UT_SAMPLER_2D, 1, &impl->decal_map_sampler, FALSE);
			pi_material_set_uniform(material, impl->U_DecalMatrix, UT_MATRIX4, 1, impl->decal_matrix, FALSE);
			pi_material_set_uniform(material, impl->U_DecalZFar, UT_VEC4, 1, impl->decal_z_far, FALSE);
			pi_material_set_def(material, impl->DECAL, TRUE);
		}
		else
		{
			pi_material_set_def(material, impl->DECAL, FALSE);
		}
		if (impl->env->env_tex != NULL)
		{
			pi_material_set_def(material, impl->CYLINDER_MAPPING, impl->env->is_cylinder);
			if (impl->env->is_cylinder)
			{
				pi_material_set_uniform(material, impl->U_EnvironmentMap, UT_SAMPLER_2D, 1, &impl->cube_tex_sampler, FALSE);
			}
			else
			{
				pi_material_set_uniform(material, impl->U_EnvironmentMap, UT_SAMPLER_CUBE, 1, &impl->cube_tex_sampler, FALSE);
			}
		}

		if (impl->light_list != NULL)
		{
			int j;
			PointLight *light;
			PiVector3 light_pos;
			int num_light = pi_vector_size(impl->light_list);

			if (num_light > 0)
			{
				material->uniforms_buffer_int[0].x = num_light = MIN(num_light, MAX_FR_LOCAL_LIGHT_NUM);
				pi_material_set_def(material, impl->MULTI_LIGHTING, TRUE);
				pi_material_set_uniform_pack_flag(material, impl->U_PointLightNum, UT_INT, 1, &material->uniforms_buffer_int[0], FALSE, TRUE);

				for (j = 0; j < num_light; j++)
				{
					light = (PointLight *)pi_vector_get(impl->light_list, j);

					if (impl->view_space_lighting)
					{
						pi_mat4_apply_point(&light_pos, &light->pos, pi_camera_get_view_matrix(impl->view_cam));		//TODO:每个光源每一帧只需要计算一次，考虑将此处计算移动至别处
					}
					else
					{
						pi_vec3_copy(&light_pos, &light->pos);
					}
					pi_math_vec4_copy_from_vec3(&impl->point_lights_cache.multi_light_color_radius_cache[j], &light->color);
					impl->point_lights_cache.multi_light_color_radius_cache[j].w = light->radius;

					pi_math_vec4_copy_from_vec3(&impl->point_lights_cache.multi_light_pos_decay_cache[j], &light_pos);
					impl->point_lights_cache.multi_light_pos_decay_cache[j].w = light->decay;
				}

				pi_material_set_uniform(material, impl->U_PointLightColorRadiusArray, UT_VEC4, num_light, &impl->point_lights_cache.multi_light_color_radius_cache, TRUE);
				pi_material_set_uniform(material, impl->light_pos_name, UT_VEC4, num_light, &impl->point_lights_cache.multi_light_pos_decay_cache, TRUE);
			}
			else
			{
				pi_material_set_def(material, impl->MULTI_LIGHTING, FALSE);
			}
		}
		else
		{
			pi_material_set_def(material, impl->MULTI_LIGHTING, FALSE);
		}
	}
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	PI_USE_PARAM(renderer);
	PI_USE_PARAM(width);
	PI_USE_PARAM(height);
}

PiRenderer *PI_API pi_lighting_new()
{
	return pi_lighting_new_with_name("postlight");
}

PiRenderer *PI_API pi_lighting_new_with_name(char* name)
{
	PiRenderer *renderer;
	LightingRenderer *impl = pi_new0(LightingRenderer, 1);
	impl->view_space_lighting = FALSE;
	impl->is_vl = FALSE;

	renderer = pi_renderer_create(ERT_FORWARD_LIGHTING, name, _init, _resize, _update, _draw, impl);
	return renderer;
}

void PI_API pi_lighting_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *entity_list_name, char *env_name, PiBool is_vl)
{
	LightingRenderer *impl;
	_type_check(renderer);
	impl = (LightingRenderer *)renderer->impl;
	pi_free(impl->target_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->entity_list_name);
	pi_free(impl->env_name);
	impl->target_name = pi_str_dup(target_name);
	impl->view_cam_name = pi_str_dup(view_cam_name);
	impl->entity_list_name = pi_str_dup(entity_list_name);
	impl->env_name = pi_str_dup(env_name);
	impl->is_vl = is_vl;

	impl->is_lighting_deploy = TRUE;
}

void PI_API pi_lighting_deploy_shadow(PiRenderer *renderer, char *shadow_data_name)
{
	LightingRenderer *impl;
	_type_check(renderer);
	impl = (LightingRenderer *)renderer->impl;
	pi_free(impl->shadow_data_name);

	if (shadow_data_name)
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

void PI_API pi_lighting_deploy_decal(PiRenderer *renderer, char *decal_map_name, char *decal_matrix_name, char *decal_z_far_name)
{
	LightingRenderer *impl;
	_type_check(renderer);
	impl = (LightingRenderer *)renderer->impl;
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

void PI_API pi_lighting_deploy_refraction(PiRenderer *renderer, char *refraction_map_name)
{
	LightingRenderer *impl;
	_type_check(renderer);
	impl = (LightingRenderer *)renderer->impl;
	pi_free(impl->refraction_map_name);
	impl->refraction_map_name = pi_str_dup(refraction_map_name);

	impl->is_refraction_deploy = TRUE;
}

void PI_API pi_lighting_free(PiRenderer *renderer)
{
	LightingRenderer *impl;
	_type_check(renderer);
	impl = (LightingRenderer *)renderer->impl;
	
	pi_free(impl->target_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->entity_list_name);
	
	pi_free(impl->shadow_data_name);
	
	pi_free(impl->decal_map_name);
	pi_free(impl->decal_matrix_name);
	pi_free(impl->decal_z_far_name);
	
	pi_free(renderer->impl);

	pi_renderer_destroy(renderer);
}

void PI_API pi_lighting_set_light_pass(PiRenderer* renderer, PiBool is_vs)
{
	LightingRenderer* impl;
	_type_check(renderer);
	impl = (LightingRenderer*)renderer->impl;
	impl->is_vl = is_vs;
}

void PI_API pi_lighting_set_view_space_lighting(PiRenderer *renderer, PiBool is_view_space)
{
	LightingRenderer *impl;
	_type_check(renderer);
	impl = (LightingRenderer *)renderer->impl;
	impl->view_space_lighting = is_view_space;
	if (renderer->is_init)
	{
		if (impl->view_space_lighting)
		{
			impl->light_pos_name = impl->U_ViewPointLightPosDecayArray;
		}
		else
		{
			impl->light_pos_name = impl->U_WorldPointLightPosDecayArray;
		}
	}
}
