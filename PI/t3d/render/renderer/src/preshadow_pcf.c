#include "preshadow_pcf.h"

#include <pi_vector3.h>
#include <pi_spatial.h>
#include <rendersystem.h>
#include <entity.h>
#include <material.h>
#include <renderutil.h>
#include <environment.h>
#include <pi_random.h>

/**
 * PCFÒõÓ°Í¼äÖÈ¾Æ÷
 */
typedef struct
{
	PiVector3 light_dir;
	char *entity_list_name;
	char *pcf_data_name;
	char *view_cam_name;
	char *shadow_cam_name;
	char *env_name;
	uint shadow_map_size;
	PiRenderTarget *shadow_rt;
	PiRenderView *color_view;
	PiRenderView *shadow_map_view;

	PCFShadowPipelineData pcf_data;

	PiBool is_deploy;
} PrePCFShadowRenderer;

static void _type_check(PiRenderer *renderer)
{
	PI_ASSERT((renderer)->type == ERT_PRE_SHADOW_PCF, "Renderer type error!");
}

static PiTexture *_create_jitter_texture(PCFShadowSamples samples)
{
	uint size = 16;
	uint samples_u;
	uint samples_v;
	unsigned char *data;
	float v[4], d[4];
	uint x, y, i, j, k;
	PiTexture *res;

	switch (samples)
	{
	case PCF_SS_8X:
		samples_u = 4;
		samples_v = 2;
		break;
	case PCF_SS_16X:
		samples_u = 4;
		samples_v = 4;
		break;
	case PCF_SS_32X:
		samples_u = 8;
		samples_v = 4;
		break;
	case PCF_SS_48X:
		samples_u = 8;
		samples_v = 6;
		break;
	case PCF_SS_64X:
		samples_u = 8;
		samples_v = 8;
		break;
	default:
		return NULL;
		break;
	}

	data = pi_new(unsigned char, size * size * samples_u * samples_v * 4 / 2);

	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size; j++)
		{
			for (k = 0; k < samples_u * samples_v / 2; k++)
			{
				x = k % (samples_u / 2);
				y = (samples_v - 1) - k / (samples_u / 2);

				v[0] = (float)(x * 2 + 0.5f) / samples_u;
				v[1] = (float)(y + 0.5f) / samples_v;
				v[2] = (float)(x * 2 + 1 + 0.5f) / samples_u;
				v[3] = v[1];

				v[0] += ((float)pi_random_float(-1, 1)) * (0.5f / samples_u);
				v[1] += ((float)pi_random_float(-1, 1)) * (0.5f / samples_v);
				v[2] += ((float)pi_random_float(-1, 1)) * (0.5f / samples_u);
				v[3] += ((float)pi_random_float(-1, 1)) * (0.5f / samples_v);

				d[0] = pi_math_sqrt(v[1]) * pi_math_cos(2 * 3.1415926f * v[0]);
				d[1] = pi_math_sqrt(v[1]) * pi_math_sin(2 * 3.1415926f * v[0]);
				d[2] = pi_math_sqrt(v[3]) * pi_math_cos(2 * 3.1415926f * v[2]);
				d[3] = pi_math_sqrt(v[3]) * pi_math_sin(2 * 3.1415926f * v[2]);

				data[(k * size * size + j * size + i) * 4 + 0] = (unsigned char)((d[0] + 1.0f) * 127);
				data[(k * size * size + j * size + i) * 4 + 1] = (unsigned char)((d[1] + 1.0f) * 127);
				data[(k * size * size + j * size + i) * 4 + 2] = (unsigned char)((d[2] + 1.0f) * 127);
				data[(k * size * size + j * size + i) * 4 + 3] = (unsigned char)((d[3] + 1.0f) * 127);
			}
		}
	}

	res = pi_texture_3d_create(RF_ABGR8, TU_NORMAL, 1, size, size, samples_u * samples_v / 2, TRUE);
	pi_texture_3d_update(res, 0, 0, 0, 0, size, size, samples_u * samples_v / 2, size * size * samples_u * samples_v * 4 / 2, (byte *)data);

	pi_free(data);

	return res;
}

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	PrePCFShadowRenderer *impl;

	PI_USE_PARAM(resources);

	impl = (PrePCFShadowRenderer *)renderer->impl;

	if (!impl->is_deploy)
	{
		return FALSE;
	}

	impl->shadow_rt = pi_rendertarget_new(TT_MRT, TRUE);
	
	impl->pcf_data.shadow_map = pi_texture_2d_create(RF_D16, TU_DEPTH_STENCIL, 1, 1, impl->shadow_map_size, impl->shadow_map_size, TRUE);
	impl->shadow_map_view = pi_renderview_new_tex2d(RVT_DEPTH_STENCIL, impl->pcf_data.shadow_map, 0, 0, TRUE);
	pi_rendertarget_attach(impl->shadow_rt, ATT_DEPTHSTENCIL, impl->shadow_map_view);

	if (pi_rendercap_get_bool(pi_rendersystem_get_cap(), CT_NULL_RENDER_TARGET_SUPPORT))
	{
		impl->color_view = pi_renderview_new(RVT_COLOR, impl->shadow_map_size, impl->shadow_map_size, RF_NULL, TRUE);
	}
	else
	{
		impl->color_view = pi_renderview_new(RVT_COLOR, impl->shadow_map_size, impl->shadow_map_size, RF_ABGR8, TRUE);
	}
	impl->pcf_data.sizeInv = 1.0f / impl->shadow_map_size;
	pi_rendertarget_attach(impl->shadow_rt, ATT_COLOR0, impl->color_view);
	pi_rendertarget_set_viewport(impl->shadow_rt, 0, 0, impl->shadow_map_size, impl->shadow_map_size);

	return TRUE;
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	PrePCFShadowRenderer *impl;
	PiColor background;
	PiVector *entity_list;
	PiCamera *shadow_camera;
	PI_USE_PARAM(tpf);
	_type_check(renderer);
	impl = (PrePCFShadowRenderer *)renderer->impl;
	pi_rendersystem_set_target(impl->shadow_rt);
	pi_rendersystem_clearview(TBM_DEPTH, &background, 1.0f, 0);

	pi_hash_lookup(resources, impl->shadow_cam_name, (void **)&shadow_camera);
	pi_hash_lookup(resources, impl->entity_list_name, (void **)&entity_list);

	pi_rendersystem_set_camera(shadow_camera);
	pi_entity_draw_list(entity_list);
}

static void _update(PiRenderer *renderer, float tpf, PiHash *resources)
{
	PrePCFShadowRenderer *impl;
	PiCamera *view_camera = NULL;
	PiCamera *shadow_camera = NULL;
	PiEnvironment *env;
	PI_USE_PARAM(tpf);
	impl = (PrePCFShadowRenderer *)renderer->impl;

	pi_hash_lookup(resources, impl->view_cam_name, (void **)&view_camera);
	pi_hash_lookup(resources, impl->shadow_cam_name, (void **)&shadow_camera);
	pi_hash_lookup(resources, impl->env_name, (void **)&env);
	pi_vec3_set(&impl->light_dir, env->default_light.diffuse_dir.x, env->default_light.diffuse_dir.y, env->default_light.diffuse_dir.z);

	pi_mat4_copy(&impl->pcf_data.shadow_matrix, pi_camera_get_view_matrix(view_camera));
	pi_mat4_inverse(&impl->pcf_data.shadow_matrix, &impl->pcf_data.shadow_matrix);
	pi_mat4_mul(&impl->pcf_data.shadow_matrix, pi_camera_get_view_projection_matrix(shadow_camera), &impl->pcf_data.shadow_matrix);

	pi_hash_insert(resources, impl->pcf_data_name, &impl->pcf_data);
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	PI_USE_PARAM(renderer);
	PI_USE_PARAM(width);
	PI_USE_PARAM(height);
}

PiRenderer *PI_API pi_preshadow_pcf_new_with_name(char *name)
{
	PiRenderer *renderer;
	PrePCFShadowRenderer *impl = pi_new0(PrePCFShadowRenderer, 1);

	impl->shadow_map_size = 1024;
	impl->pcf_data.shadow_z_far = 25;
	impl->pcf_data.shadow_samples = PCF_SS_16X;

	renderer = pi_renderer_create(ERT_PRE_SHADOW_PCF, name, _init, _resize, _update, _draw, impl);
	//pi_renderer_set_enable(renderer, FALSE);
	return renderer;
}


PiRenderer *PI_API pi_preshadow_pcf_new()
{
	return pi_preshadow_pcf_new_with_name("pre shadow pcf");
}

void PI_API pi_preshadow_pcf_deploy(PiRenderer *renderer, char *shadow_data_name, char *view_cam_name, char *shadow_cam_name, char *entity_list_name, char *env_name)
{
	PrePCFShadowRenderer *impl;
	_type_check(renderer);
	impl = (PrePCFShadowRenderer *)renderer->impl;

	pi_free(impl->pcf_data_name);
	pi_free(impl->shadow_cam_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->entity_list_name);
	pi_free(impl->env_name);
	impl->shadow_cam_name = pi_str_dup(shadow_cam_name);
	impl->view_cam_name = pi_str_dup(view_cam_name);
	impl->entity_list_name = pi_str_dup(entity_list_name);
	impl->pcf_data_name = pi_str_dup(shadow_data_name);
	impl->env_name = pi_str_dup(env_name);


	impl->is_deploy = TRUE;
}

void PI_API pi_preshadow_pcf_free(PiRenderer *renderer)
{
	PrePCFShadowRenderer *impl;
	_type_check(renderer);
	impl = (PrePCFShadowRenderer *)renderer->impl;

	pi_free(impl->pcf_data_name);
	pi_free(impl->shadow_cam_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->entity_list_name);
	pi_rendertarget_free(impl->shadow_rt);

	pi_texture_free(impl->pcf_data.shadow_map);
	pi_texture_free(impl->pcf_data.depth_stencil);

	pi_renderview_free(impl->shadow_map_view);
	pi_renderview_free(impl->color_view);

	pi_free(renderer->impl);
	pi_renderer_destroy(renderer);
}

void PI_API pi_preshadow_pcf_update_camera(PiRenderer *renderer, PiCamera *view_cam, PiCamera *shadow_cam)
{
	PrePCFShadowRenderer *impl;
	_type_check(renderer);
	impl = (PrePCFShadowRenderer *)renderer->impl;
	pi_camera_set_up(shadow_cam, 0, 0, -1);
	pi_camera_set_direction(shadow_cam, -impl->light_dir.x, -impl->light_dir.y, -impl->light_dir.z);
	renderutil_update_shadow_cam(view_cam, shadow_cam, (float)impl->shadow_map_size, (float)impl->shadow_map_size, impl->pcf_data.shadow_z_far);
}

void PI_API pi_preshadow_pcf_set_shadow_mapsize(PiRenderer *renderer, uint size)
{
	PrePCFShadowRenderer *impl;
	_type_check(renderer);
	impl = (PrePCFShadowRenderer *)renderer->impl;
	impl->shadow_map_size = size;
	impl->pcf_data.sizeInv = 1.0f / size;
}

void PI_API pi_preshadow_pcf_set_zfar(PiRenderer *renderer, float z_far)
{
	PrePCFShadowRenderer *impl;
	_type_check(renderer);
	impl = (PrePCFShadowRenderer *)renderer->impl;
	impl->pcf_data.shadow_z_far = z_far;
}

void PI_API pi_preshadow_pcf_set_quality(PiRenderer *renderer, PCFShadowSamples samples)
{
	PrePCFShadowRenderer *impl;
	_type_check(renderer);
	impl = (PrePCFShadowRenderer *)renderer->impl;

	if (impl->pcf_data.shadow_samples != samples)
	{
		impl->pcf_data.shadow_samples = samples;
	}
}

void PI_API pi_preshadow_pcf_set_filter_size(PiRenderer *renderer, float filter_size)
{
}
