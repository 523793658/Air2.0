
#include <prelighting.h>
#include <pi_vector3.h>
#include <rendersystem.h>
#include <entity.h>
#include <material.h>
#include <environment.h>
#include <camera.h>
#include <lights.h>

/* 缓存的用来绘制light的entity */
typedef struct
{
	PiVector *entity_list;
	uint used_num;
	uint total_num;
} LightEntityCache;

/**
 * prelighting渲染器
 */
typedef struct
{
	PiBool is_deploy;
	PiBool is_init;

	/* 将要被用来绘制光源的entity */
	PiVector *light_entity_list;

	char *target_name;
	char *view_cam_name;

	char *lighting_diffuse_name;
	char *lighting_sepcular_name;

	char *point_light_list_name;
	char *spot_light_list_name;

	char *g_buffer_tex0_name;
	char *g_buffer_depth_name;

	PiCamera *view_camera;

	PiTexture *g_buffer_tex0;
	PiTexture *g_buffer_depth;

	PiRenderTarget *lighting_target;

	PiTexture *lighting_diffuse;
	PiTexture *lighting_specular;
	PiTexture *lighting_buffer_depth;

	PiRenderView *lighting_diffuse_view;
	PiRenderView *lighting_specular_view;
	PiRenderView *lighting_buffer_depth_view;

	PiCamera *screen_quad_camera;

	PiMesh *screen_quad_mesh;
	PiRenderMesh *screen_quad_rmesh;

	PiEntity *lighting_quad;
	PiMaterial *lighting_quad_material;

	PiMesh *point_light_mesh;
	PiRenderMesh *point_light_rmesh;

	PiMesh *spot_light_mesh;
	PiRenderMesh *spot_light_rmesh;

	LightEntityCache point_light_entity_cache;
	LightEntityCache spot_light_entity_cache;

	/* Unform的值 */
	PiMatrix4 view_proj_matrix_inverse;

	SamplerState g_buffer_tex0_sampler;
	SamplerState g_buffer_depth_sampler;

	/* 常量字符串 */
	char *U_ViewProjMatrixInverse;

	char *U_GBufferTex0;
	char *U_GBufferDepthTex;

	char *U_PointLightColor;
	char *U_PointLightPosition;
	char *U_PointLightRadius;
	char *U_PointLightExponent;

	char *U_SpotLightColor;
	char *U_SpotLightPosition;
	char *U_SpotLightRange;
	char *U_SpotLightExponent;
	char *U_SpotLightDirection;
	char *U_SpotLightInnerConeAngle;
	char *U_SpotLightOuterConeAngle;
	char *U_SpotLightPenumbraExponent;
} PreLightingRenderer;

static void _type_check(PiRenderer *renderer)
{
	PI_ASSERT(renderer->type == ERT_PRE_LIGHTING, "Renderer type error!");
}

static PiMesh *_create_cone_mesh(uint cone_edge_num)
{
	uint i, j;
	float r = 2.0f * (float)PI_PI / cone_edge_num;
	float *pos = pi_new0(float, (cone_edge_num + 1) * 3);
	uint32 *indices = pi_new0(uint32, cone_edge_num * 3 + (cone_edge_num - 2) * 3);
	PiMesh *mesh;

	for (i = 0; i < cone_edge_num; i++)
	{
		j = i * 3;

		pos[j] = pi_math_cos(r * i);
		pos[j + 1] = -1.0f;
		pos[j + 2] = pi_math_sin(r * i);
	}

	pos[cone_edge_num * 3] = 0.0f;
	pos[cone_edge_num * 3 + 1] = 0.0f;
	pos[cone_edge_num * 3 + 2] = 0.0f;

	for (i = 0; i < cone_edge_num; i++)
	{
		j = i * 3;

		indices[j] = i;
		indices[j + 1] = cone_edge_num;
		indices[j + 2] = (i + 1) % cone_edge_num;
	}

	for (i = 0; i < cone_edge_num - 2; i++)
	{
		j = (cone_edge_num + i) * 3;

		indices[j] = 0;
		indices[j + 1] = i + 1;
		indices[j + 2] = i + 2;
	}

	mesh = pi_mesh_create(EGOT_TRIANGLE_LIST, EINDEX_32BIT, cone_edge_num + 1, pos, NULL, NULL, NULL, cone_edge_num * 3 + (cone_edge_num - 2) * 3, indices);

	pi_free(pos);
	pi_free(indices);

	return mesh;
}

static PiMesh *_create_sphere_mesh(float radius, uint32 slices, uint32 stacks)
{
	PiMesh *mesh;
	float dtheta = (float)(PI_PI) / (float)stacks;
	float dphi = 2.0f * (float)(PI_PI) / (float)slices;
	uint32 i, j;     // Looping variables
	uint32 num_vertex = slices * (stacks - 1) + 2;
	uint32 num_index = slices * (stacks - 1) * 2 * 3;
	float *pos = pi_new0(float, num_vertex * 3);
	uint32 *indices = pi_new0(uint32, num_index);
	uint32 n = 0;

	pos[0] = 0.0f;
	pos[1] = 1.0f;
	pos[2] = 0.0f;

	pos[3] = 0.0f;
	pos[4] = -1.0f;
	pos[5] = 0.0f;

	for (i = 1; i < stacks; i++)
	{
		float theta = (float)i * dtheta;

		float stheta = pi_math_sin(theta);
		float ctheta = pi_math_cos(theta);

		for (j = 0; j < slices; j++)
		{
			uint32 m = 2 + (i - 1) * slices + j;

			float phi = (float)j * dphi;

			pos[m * 3] = pi_math_cos(phi) * stheta;
			pos[m * 3 + 1] = ctheta;
			pos[m * 3 + 2] = -pi_math_sin(phi) * stheta;

			if (i < stacks - 1)
			{
				uint32 next_slice = (j + 1 == slices) ? (m - j) : (m + 1);
				uint32 next_stack = m + slices;

				indices[n++] = m;
				indices[n++] = next_stack;
				indices[n++] = next_slice;

				indices[n++] = next_stack;
				indices[n++] = next_slice + slices;
				indices[n++] = next_slice;
			}
		}
	}

	for (j = 0; j < slices; j++)
	{
		uint32 m = 2 + j;
		indices[n++] = 0;
		indices[n++] = m;
		indices[n++] = (j + 1 == slices) ? (m - j) : (m + 1);

		m = 2 + (stacks - 2) * slices + j;
		indices[n++] = m;
		indices[n++] = 1;
		indices[n++] = (j + 1 == slices) ? (m - j) : (m + 1);
	}

	mesh = pi_mesh_create(EGOT_TRIANGLE_LIST, EINDEX_32BIT, num_vertex, pos, NULL, NULL, NULL, num_index, indices);

	pi_free(pos);
	pi_free(indices);

	return mesh;
}

static void _init_const_string(PreLightingRenderer *impl)
{
	impl->U_ViewProjMatrixInverse = pi_conststr("u_ViewProjMatrixInverse");

	impl->U_GBufferTex0 = pi_conststr("u_GBufferTex0");
	impl->U_GBufferDepthTex = pi_conststr("u_GBufferDepthTex");

	impl->U_PointLightColor = pi_conststr("u_Color");
	impl->U_PointLightPosition = pi_conststr("u_Position");
	impl->U_PointLightRadius = pi_conststr("u_Radius");
	impl->U_PointLightExponent = pi_conststr("u_Exponent");

	impl->U_SpotLightColor = pi_conststr("u_Color");
	impl->U_SpotLightPosition = pi_conststr("u_Position");
	impl->U_SpotLightRange = pi_conststr("u_Range");
	impl->U_SpotLightExponent = pi_conststr("u_Exponent");

	impl->U_SpotLightDirection = pi_conststr("u_Direction");
	impl->U_SpotLightInnerConeAngle = pi_conststr("u_InnerConeAngle");
	impl->U_SpotLightOuterConeAngle = pi_conststr("u_OuterConeAngle");
	impl->U_SpotLightPenumbraExponent = pi_conststr("u_PenumbraExponent");
}

static void _init_sampler(PreLightingRenderer *impl)
{
	pi_renderstate_set_default_sampler(&impl->g_buffer_tex0_sampler);
	pi_sampler_set_addr_mode(&impl->g_buffer_tex0_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->g_buffer_tex0_sampler, TFO_MIN_MAG_POINT);

	pi_renderstate_set_default_sampler(&impl->g_buffer_depth_sampler);
	pi_sampler_set_addr_mode(&impl->g_buffer_depth_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->g_buffer_depth_sampler, TFO_MIN_MAG_POINT);
}

static void _init_screen_quad_camera(PreLightingRenderer *impl, uint32 width, uint32 height)
{
	impl->screen_quad_camera = pi_camera_new();
	pi_camera_set_location(impl->screen_quad_camera, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->screen_quad_camera, 0.0f, 0.0f, -1.0f);
	pi_camera_set_frustum(impl->screen_quad_camera, -(float)width / 2.0f, width / 2.0f, -(float)height / 2.0f, (float)height / 2.0f, 0.0f, 2.0f, TRUE);
}

static void _init_screen_quad(PreLightingRenderer *impl, uint32 width, uint32 height)
{
	impl->lighting_quad_material = pi_material_new(RS_PRE_LIGHTING_VS, RS_PRE_LIGHTING_FS);

	pi_material_set_depth_enable(impl->lighting_quad_material, TRUE);
	pi_material_set_depthwrite_enable(impl->lighting_quad_material, TRUE);
	pi_material_set_depth_compfunc(impl->lighting_quad_material, CF_ALWAYSPASS);

	impl->lighting_quad = pi_entity_new();
	pi_entity_set_material(impl->lighting_quad, impl->lighting_quad_material);

	impl->screen_quad_mesh = pi_mesh_create_quad(NULL, NULL, 0.0f);
	impl->screen_quad_rmesh = pi_rendermesh_new(impl->screen_quad_mesh, TRUE);
	pi_entity_set_mesh(impl->lighting_quad, impl->screen_quad_rmesh);

	pi_spatial_set_local_scaling(impl->lighting_quad->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->lighting_quad->spatial);
}

static void _init_lighting_target(PreLightingRenderer *impl, uint32 width, uint32 height)
{
	impl->lighting_diffuse = pi_texture_2d_create(RF_B10G11R11F, TU_COLOR, 1, 1, width, height, TRUE);
	impl->lighting_specular = pi_texture_2d_create(RF_B10G11R11F, TU_COLOR, 1, 1, width, height, TRUE);
	impl->lighting_buffer_depth = pi_texture_2d_create(RF_D24S8, TU_DEPTH_STENCIL, 1, 1, width, height, TRUE);

	impl->lighting_diffuse_view = pi_renderview_new_tex2d(RVT_COLOR, impl->lighting_diffuse, 0, 0, TRUE);
	impl->lighting_specular_view = pi_renderview_new_tex2d(RVT_COLOR, impl->lighting_specular, 0, 0, TRUE);
	impl->lighting_buffer_depth_view = pi_renderview_new_tex2d(RVT_DEPTH_STENCIL, impl->lighting_buffer_depth, 0, 0, TRUE);

	pi_rendertarget_attach(impl->lighting_target, ATT_COLOR0, impl->lighting_diffuse_view);
	pi_rendertarget_attach(impl->lighting_target, ATT_COLOR1, impl->lighting_specular_view);
	pi_rendertarget_attach(impl->lighting_target, ATT_DEPTHSTENCIL, impl->lighting_buffer_depth_view);

	pi_rendertarget_set_viewport(impl->lighting_target, 0, 0, width, height);
}

static PiMaterial *_init_light_entity_material(PreLightingRenderer *impl, PiMaterial *material)
{
	pi_material_set_depthwrite_enable(material, FALSE);

	pi_material_set_blend(material, TRUE);
	pi_material_set_blend_factor(material, BF_ONE, BF_ONE, BF_ZERO, BF_ONE);

	pi_material_set_uniform(material, impl->U_GBufferTex0, UT_SAMPLER_2D, 1, &impl->g_buffer_tex0_sampler, FALSE);
	pi_material_set_uniform(material, impl->U_GBufferDepthTex, UT_SAMPLER_2D, 1, &impl->g_buffer_depth_sampler, FALSE);

	pi_material_set_uniform(material, impl->U_ViewProjMatrixInverse, UT_MATRIX4, 1, &impl->view_proj_matrix_inverse, FALSE);

	return material;
}

static void _set_non_intersected_light_material(PiMaterial *material)
{
	pi_material_set_cull_mode(material, CM_BACK);

	pi_material_set_depth_enable(material, TRUE);
}

static void _set_intersected_light_material(PiMaterial *material)
{
	pi_material_set_cull_mode(material, CM_FRONT);

	pi_material_set_depth_enable(material, FALSE);
}

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	PreLightingRenderer *impl = (PreLightingRenderer *)renderer->impl;
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

	impl->lighting_target = pi_rendertarget_new(TT_MRT, TRUE);
	_init_lighting_target(impl, width, height);

	impl->point_light_mesh = _create_sphere_mesh(1.0f, 9, 6);
	impl->point_light_rmesh = pi_rendermesh_new(impl->point_light_mesh, TRUE);

	impl->spot_light_mesh = _create_cone_mesh(30);
	impl->spot_light_rmesh = pi_rendermesh_new(impl->spot_light_mesh, TRUE);

	impl->point_light_entity_cache.entity_list = pi_vector_new();
	impl->point_light_entity_cache.total_num = 0;
	impl->point_light_entity_cache.used_num = 0;

	impl->spot_light_entity_cache.entity_list = pi_vector_new();
	impl->spot_light_entity_cache.total_num = 0;
	impl->spot_light_entity_cache.used_num = 0;

	impl->light_entity_list = pi_vector_new();

	impl->is_init = TRUE;

	return TRUE;
}

static PiEntity *_get_point_light_entity(PreLightingRenderer *impl)
{
	PiEntity *entity;

	if (impl->point_light_entity_cache.used_num < impl->point_light_entity_cache.total_num)
	{
		entity = (PiEntity *)pi_vector_get(impl->point_light_entity_cache.entity_list, impl->point_light_entity_cache.used_num);
	}
	else
	{
		PiMaterial *material = pi_material_new(RS_LIGHT_VS, RS_POINT_LIGHT_FS);

		entity = pi_entity_new();

		pi_entity_set_material(entity, material);

		_init_light_entity_material(impl, material);

		pi_vector_push(impl->point_light_entity_cache.entity_list, entity);

		impl->point_light_entity_cache.total_num++;
	}

	impl->point_light_entity_cache.used_num++;

	return entity;
}

static PiEntity *_get_spot_light_entity(PreLightingRenderer *impl)
{
	PiEntity *entity;

	if (impl->spot_light_entity_cache.used_num < impl->spot_light_entity_cache.total_num)
	{
		entity = (PiEntity *)pi_vector_get(impl->spot_light_entity_cache.entity_list, impl->spot_light_entity_cache.used_num);
	}
	else
	{
		PiMaterial *material = pi_material_new(RS_LIGHT_VS, RS_SPOT_LIGHT_FS);

		entity = pi_entity_new();

		pi_entity_set_material(entity, material);

		_init_light_entity_material(impl, material);

		pi_vector_push(impl->spot_light_entity_cache.entity_list, entity);

		impl->spot_light_entity_cache.total_num++;
	}

	impl->spot_light_entity_cache.used_num++;
	return entity;
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	PreLightingRenderer *impl;

	PI_USE_PARAM(tpf);
	_type_check(renderer);
	impl = (PreLightingRenderer *)renderer->impl;

	pi_rendersystem_set_target(impl->lighting_target);

	pi_rendersystem_set_camera(impl->screen_quad_camera);
	pi_entity_draw(impl->lighting_quad);

	pi_rendersystem_set_camera(impl->view_camera);
	pi_entity_draw_list(impl->light_entity_list);
}

static void _update(PiRenderer *renderer, float tpf, PiHash *resources)
{
	PreLightingRenderer *impl;

	PI_USE_PARAM(tpf);
	_type_check(renderer);
	impl = (PreLightingRenderer *)renderer->impl;

	pi_vector_clear(impl->light_entity_list, FALSE);

	pi_hash_lookup(resources, impl->g_buffer_tex0_name, (void **)&impl->g_buffer_tex0);
	pi_hash_lookup(resources, impl->g_buffer_depth_name, (void **)&impl->g_buffer_depth);

	pi_sampler_set_texture(&impl->g_buffer_tex0_sampler, impl->g_buffer_tex0);
	pi_sampler_set_texture(&impl->g_buffer_depth_sampler, impl->g_buffer_depth);

	pi_material_set_uniform(impl->lighting_quad_material, impl->U_GBufferDepthTex, UT_SAMPLER_2D, 1, &impl->g_buffer_depth_sampler, FALSE);

	pi_hash_lookup(resources, impl->view_cam_name, (void **)&impl->view_camera);
	pi_mat4_inverse(&impl->view_proj_matrix_inverse, pi_camera_get_view_projection_matrix(impl->view_camera));

	{
		uint i, n;
		PiVector3 dir;
		float distance;
		PiVector3 near_clip_plane_center;
		PiVector3 camera_direction;
		PiVector3 *view_position = pi_camera_get_location(impl->view_camera);

		pi_camera_get_direction(impl->view_camera, &camera_direction);
		pi_vec3_scale(&near_clip_plane_center, &camera_direction, pi_camera_get_frustum_near(impl->view_camera));
		pi_vec3_add(&near_clip_plane_center, &near_clip_plane_center, view_position);

		{
			PiVector *point_light_list;

			impl->point_light_entity_cache.used_num = 0;

			pi_hash_lookup(resources, impl->point_light_list_name, (void **)&point_light_list);
			n = pi_vector_size(point_light_list);

			for (i = 0; i < n; ++i)
			{
				PointLight *point_light = (PointLight *)pi_vector_get(point_light_list, i);

				PiEntity *light_entity = _get_point_light_entity(impl);
				pi_entity_set_mesh(light_entity, impl->point_light_rmesh);

				pi_spatial_set_local_translation(light_entity->spatial, point_light->pos.x, point_light->pos.y, point_light->pos.z);
				pi_spatial_set_local_scaling(light_entity->spatial, point_light->radius, point_light->radius, point_light->radius);
				pi_spatial_update(light_entity->spatial);

				pi_material_set_uniform(light_entity->material, impl->U_PointLightColor, UT_VEC3, 1, &point_light->color, FALSE);
				pi_material_set_uniform(light_entity->material, impl->U_PointLightPosition, UT_VEC3, 1, &point_light->pos, FALSE);
				pi_material_set_uniform(light_entity->material, impl->U_PointLightRadius, UT_FLOAT, 1, &point_light->radius, FALSE);
				pi_material_set_uniform(light_entity->material, impl->U_PointLightExponent, UT_FLOAT, 1, &point_light->decay, FALSE);

				pi_vec3_sub(&dir, &point_light->pos, &near_clip_plane_center);
				distance = ABS(pi_vec3_dot(&dir, &camera_direction));

				if (distance <= point_light->radius)
				{
					_set_intersected_light_material(light_entity->material);
				}
				else
				{
					_set_non_intersected_light_material(light_entity->material);
				}

				pi_vector_push(impl->light_entity_list, light_entity);
			}
		}

		{
			PiVector *spot_light_list;
			PiVector3 cone_default_dir;

			impl->spot_light_entity_cache.used_num = 0;

			pi_vec3_set(&cone_default_dir, 0.0f, 1.0f, 0.0f);
			pi_hash_lookup(resources, impl->spot_light_list_name, (void **)&spot_light_list);
			n = pi_vector_size(spot_light_list);

			for (i = 0; i < n; ++i)
			{
				float r;
				PiQuaternion rotation;

				SpotLight *spot_light = (SpotLight *)pi_vector_get(spot_light_list, i);

				PiEntity *light_entity = _get_spot_light_entity(impl);
				pi_entity_set_mesh(light_entity, impl->spot_light_rmesh);

				r = pi_math_tan(spot_light->outer_cone_angle) * spot_light->range;
				pi_quat_rotate_to(&rotation, &cone_default_dir, &spot_light->direction);

				pi_spatial_set_local_scaling(light_entity->spatial, r, spot_light->range, r);
				pi_spatial_set_local_translation(light_entity->spatial, spot_light->position.x, spot_light->position.y, spot_light->position.z);
				pi_spatial_set_local_rotation(light_entity->spatial, rotation.w, rotation.x, rotation.y, rotation.z);
				pi_spatial_update(light_entity->spatial);

				pi_material_set_uniform(light_entity->material, impl->U_SpotLightColor, UT_VEC3, 1, &spot_light->color, FALSE);
				pi_material_set_uniform(light_entity->material, impl->U_SpotLightPosition, UT_VEC3, 1, &spot_light->position, FALSE);
				pi_material_set_uniform(light_entity->material, impl->U_SpotLightRange, UT_FLOAT, 1, &spot_light->range, FALSE);
				pi_material_set_uniform(light_entity->material, impl->U_SpotLightExponent, UT_FLOAT, 1, &spot_light->exponent, FALSE);

				pi_material_set_uniform(light_entity->material, impl->U_SpotLightDirection, UT_VEC3, 1, &spot_light->direction, FALSE);
				pi_material_set_uniform(light_entity->material, impl->U_SpotLightInnerConeAngle, UT_FLOAT, 1, &spot_light->inner_cone_angle, FALSE);
				pi_material_set_uniform(light_entity->material, impl->U_SpotLightOuterConeAngle, UT_FLOAT, 1, &spot_light->outer_cone_angle, FALSE);
				pi_material_set_uniform(light_entity->material, impl->U_SpotLightPenumbraExponent, UT_FLOAT, 1, &spot_light->penumbra_exponent, FALSE);

				// TODO:可能需要使用更精确的方式来判断圆锥与平面是否相交
				pi_vec3_sub(&dir, &spot_light->position, &near_clip_plane_center);
				distance = ABS(pi_vec3_dot(&dir, &camera_direction));

				if (distance <= spot_light->range)
				{
					_set_intersected_light_material(light_entity->material);
				}
				else
				{
					_set_non_intersected_light_material(light_entity->material);
				}

				pi_vector_push(impl->light_entity_list, light_entity);
			}
		}
	}

	pi_hash_enter(resources, impl->lighting_diffuse_name, impl->lighting_diffuse, NULL);
	pi_hash_enter(resources, impl->lighting_sepcular_name, impl->lighting_specular, NULL);
}

static void _renderview_free(PreLightingRenderer *impl)
{
	pi_rendertarget_detach(impl->lighting_target, ATT_COLOR0);
	pi_rendertarget_detach(impl->lighting_target, ATT_COLOR1);
	pi_rendertarget_detach(impl->lighting_target, ATT_DEPTHSTENCIL);

	pi_renderview_free(impl->lighting_diffuse_view);
	pi_renderview_free(impl->lighting_specular_view);
	pi_renderview_free(impl->lighting_buffer_depth_view);

	pi_texture_free(impl->lighting_diffuse);
	pi_texture_free(impl->lighting_specular);
	pi_texture_free(impl->lighting_buffer_depth);
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	PreLightingRenderer *impl;
	_type_check(renderer);
	impl = (PreLightingRenderer *)renderer->impl;

	_renderview_free(impl);

	_init_lighting_target(impl, width, height);

	pi_spatial_set_local_scaling(impl->lighting_quad->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->lighting_quad->spatial);

	pi_camera_set_frustum(impl->screen_quad_camera, -(float)width / 2.0f, (float)width / 2.0f, -(float)height / 2.0f, (float)height / 2.0f, 0.0f, 2.0f, TRUE);
}

PiRenderer *PI_API pi_prelighting_new()
{
	PiRenderer *renderer;
	PreLightingRenderer *impl = pi_new0(PreLightingRenderer, 1);
	renderer = pi_renderer_create(ERT_PRE_LIGHTING, "prelighting", _init, _resize, _update, _draw, impl);
	return renderer;
}

void PI_API pi_prelighting_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *point_light_list_name, char *spot_light_list_name,
                                  char *lighting_diffuse_name, char *lighting_sepcular_name, char *g_buffer_tex0_name, char *g_buffer_depth_name)
{
	PreLightingRenderer *impl;
	_type_check(renderer);
	impl = (PreLightingRenderer *)renderer->impl;

	impl->target_name = pi_str_dup(target_name);

	impl->view_cam_name = pi_str_dup(view_cam_name);

	impl->point_light_list_name = pi_str_dup(point_light_list_name);
	impl->spot_light_list_name = pi_str_dup(spot_light_list_name);

	impl->lighting_diffuse_name = pi_str_dup(lighting_diffuse_name);
	impl->lighting_sepcular_name = pi_str_dup(lighting_sepcular_name);

	impl->g_buffer_tex0_name = pi_str_dup(g_buffer_tex0_name);
	impl->g_buffer_depth_name = pi_str_dup(g_buffer_depth_name);

	impl->is_deploy = TRUE;
}

static void _entity_cache_free(LightEntityCache *cache)
{
	uint i;

	for (i = 0; i < cache->total_num; i++)
	{
		PiEntity *entity = (PiEntity *)pi_vector_get(cache->entity_list, i);
		pi_material_free(entity->material);
		pi_entity_free(entity);
	}

	pi_vector_free(cache->entity_list);
}

void PI_API pi_prelighting_free(PiRenderer *renderer)
{
	PreLightingRenderer *impl;
	_type_check(renderer);
	impl = (PreLightingRenderer *)renderer->impl;

	if (!impl->is_deploy)
	{
		return;
	}

	pi_free(impl->target_name);
	pi_free(impl->view_cam_name);

	pi_free(impl->point_light_list_name);
	pi_free(impl->spot_light_list_name);

	pi_free(impl->lighting_diffuse_name);
	pi_free(impl->lighting_sepcular_name);

	pi_free(impl->g_buffer_tex0_name);
	pi_free(impl->g_buffer_depth_name);

	if (!impl->is_init)
	{
		return;
	}

	_renderview_free(impl);
	pi_rendertarget_free(impl->lighting_target);

	pi_vector_free(impl->light_entity_list);

	pi_entity_free(impl->lighting_quad);
	pi_material_free(impl->lighting_quad_material);
	pi_rendermesh_free(impl->screen_quad_rmesh);
	pi_mesh_free(impl->screen_quad_mesh);

	pi_camera_free(impl->screen_quad_camera);

	_entity_cache_free(&impl->point_light_entity_cache);
	_entity_cache_free(&impl->spot_light_entity_cache);

	pi_rendermesh_free(impl->point_light_rmesh);
	pi_mesh_free(impl->point_light_mesh);

	pi_rendermesh_free(impl->spot_light_rmesh);
	pi_mesh_free(impl->spot_light_mesh);

	pi_free(impl);

	pi_renderer_destroy(renderer);
}
