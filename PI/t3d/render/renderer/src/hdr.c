#include "hdr.h"

#include <pi_spatial.h>
#include <entity.h>
#include <rendertarget.h>
#include <rendersystem.h>

#define BLOOM_LEVEL 4
#define BLOOM_DOWN_SAMPLER 2
#define DOWN_SAMPLE_LEVEL 5

/**
 * HDRäÖÈ¾Æ÷
 */
typedef struct
{
	PiMesh *mesh;
	PiRenderMesh *rmesh;
	PiEntity *entity;
	PiCamera *camera;

	float min_tone_luminance;
	PiVector4 tone_map_params;
	PiVector4 light_pass_params;
	PiVector4 luminance_params;

	char *input_name;
	char *output_name;
	SamplerState ss_src;
	SamplerState ss_bloom[BLOOM_LEVEL];
	SamplerState ss_acc;
	SamplerState ss_down_sample[DOWN_SAMPLE_LEVEL];

	PiMaterial *light_pass_material;
	PiMaterial *tone_mapping_material;
	PiMaterial *acc_material;
	PiMaterial *down_sample_material;
	PiMaterial *mipmap_material;
	PiMaterial *bloom_down_sample_material;

	PiRenderTarget *bloom_rt[BLOOM_LEVEL];
	PiTexture *bloom_tex[BLOOM_LEVEL];
	PiRenderView *bloom_view[BLOOM_LEVEL];

	PiRenderTarget *acc_rt;
	PiTexture *acc_tex;
	PiRenderView *acc_view;

	PiRenderTarget *down_sample_rt[DOWN_SAMPLE_LEVEL];
	PiTexture *down_sample_tex[DOWN_SAMPLE_LEVEL];
	PiRenderView *down_sample_view[DOWN_SAMPLE_LEVEL];

	PiBool is_deploy;

	/* ³£Á¿×Ö·û´® */
	char *U_SrcTex;
	char *U_LightPassParams;
	char *U_ToneMapParams;
	char *U_LuminanceParams;
	char *U_BloomTex[BLOOM_LEVEL];

	char *U_SceneTex;
	char *U_LuminanceMap;
	char *U_AccTex;
	char *U_RenderTargetSize;
	char *U_offsetUV;

	char *DIRECTLY_MAPPING;
	char *SAMPLER4X4;
	char *TONE_MAPPING;
} HDRRenderer;

static void _type_check(PiRenderer *renderer)
{
	PI_ASSERT((renderer)->type == ERT_HDR, "Renderer type error!");
}

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	PiRenderTarget *target;
	uint32 width, height;
	HDRRenderer *impl = (HDRRenderer *)renderer->impl;
	uint w, h, i;
	float size[2];
	if (!impl->is_deploy)
	{
		return FALSE;
	}

	impl->U_SrcTex = "u_SrcTex";
	impl->U_BloomTex[0] = "u_BloomTex0";
	impl->U_BloomTex[1] = "u_BloomTex1";
	impl->U_BloomTex[2] = "u_BloomTex2";
	impl->U_BloomTex[3] = "u_BloomTex3";

	impl->U_LightPassParams = "u_LightPassParams";
	impl->U_SceneTex = "u_SceneTex";
	impl->U_LuminanceMap = "u_LuminanceMap";
	impl->U_AccTex = "u_AccTex";
	impl->U_RenderTargetSize = "u_RenderTargetSize";
	impl->U_offsetUV = "u_offsetUV";
	impl->U_ToneMapParams = "u_ToneMapParams";
	impl->U_LuminanceParams = "u_LuminanceParams";


	impl->DIRECTLY_MAPPING = "DIRECTLY_MAPPING";
	impl->SAMPLER4X4 = "SAMPLER4X4";
	impl->TONE_MAPPING = "TONE_MAPPING";

	pi_hash_lookup(resources, impl->output_name, (void **)&target);
	w = width = target->width;
	h = height = target->height;
	for (i = 0; i < BLOOM_LEVEL; i++)
	{
		w /= BLOOM_DOWN_SAMPLER;
		h /= BLOOM_DOWN_SAMPLER;
		if (w > 0 && h > 0)
		{

			impl->bloom_rt[i] = pi_rendertarget_new(TT_MRT, TRUE);
			impl->bloom_tex[i] = pi_texture_2d_create(RF_ABGR16F, TU_COLOR, 1, 1, w, h, TRUE);
			impl->bloom_view[i] = pi_renderview_new_tex2d(RVT_COLOR, impl->bloom_tex[i], 0, 0, TRUE);
			pi_rendertarget_attach(impl->bloom_rt[i], ATT_COLOR0, impl->bloom_view[i]);
			pi_rendertarget_set_viewport(impl->bloom_rt[i], 0, 0, w, h);
		}
	}

	impl->acc_rt = pi_rendertarget_new(TT_MRT, TRUE);
	impl->acc_tex = pi_texture_2d_create(RF_R16F, TU_COLOR, 1, 1, 1, 1, TRUE);
	impl->acc_view = pi_renderview_new_tex2d(RVT_COLOR, impl->acc_tex, 0, 0, TRUE);
	pi_rendertarget_attach(impl->acc_rt, ATT_COLOR0, impl->acc_view);
	pi_rendertarget_set_viewport(impl->acc_rt, 0, 0, 1, 1);

	w = h = 1024;
	for (i = 0; i < DOWN_SAMPLE_LEVEL; i++)
	{
		w /= 4;
		h /= 4;
		impl->down_sample_rt[i] = pi_rendertarget_new(TT_MRT, TRUE);
		impl->down_sample_tex[i] = pi_texture_2d_create(RF_R16F, TU_COLOR, 1, 1, w, h, TRUE);
		impl->down_sample_view[i] = pi_renderview_new_tex2d(RVT_COLOR, impl->down_sample_tex[i], 0, 0, TRUE);
		pi_rendertarget_attach(impl->down_sample_rt[i], ATT_COLOR0, impl->down_sample_view[i]);
		pi_rendertarget_set_viewport(impl->down_sample_rt[i], 0, 0, w, h);
	}

	impl->mesh = pi_mesh_create_quad(NULL, NULL, 0.0f);
	impl->rmesh = pi_rendermesh_new(impl->mesh, TRUE);

	impl->light_pass_material = pi_material_new(RS_LIGHT_PASS_VS, RS_LIGHT_PASS_FS);
	impl->tone_mapping_material = pi_material_new(RS_TONE_MAPPING_VS, RS_TONE_MAPPING_FS);
	impl->acc_material = pi_material_new(RS_LUMINANCE_ACCUMULATION_VS, RS_LUMINANCE_ACCUMULATION_FS);
	impl->down_sample_material = pi_material_new(RS_DOWN_SAMPLE_VS, RS_DOWN_SAMPLE_FS);
	impl->mipmap_material = pi_material_new(RS_MIPMAP_VS, RS_MIPMAP_FS);
	impl->bloom_down_sample_material = pi_material_new(RS_MIPMAP_VS, RS_MIPMAP_FS);
	pi_material_set_def(impl->tone_mapping_material, impl->TONE_MAPPING, TRUE);
	pi_material_set_def(impl->mipmap_material, impl->DIRECTLY_MAPPING, TRUE);
	pi_material_set_def(impl->mipmap_material, impl->SAMPLER4X4, TRUE);
	pi_material_set_def(impl->bloom_down_sample_material, impl->DIRECTLY_MAPPING, TRUE);

	impl->entity = pi_entity_new();

	pi_renderstate_set_default_sampler(&impl->ss_src);
	pi_sampler_set_filter(&impl->ss_src, TFO_MIN_MAG_LINEAR);
	pi_sampler_set_addr_mode(&impl->ss_src, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);

	for (i = 0; i < BLOOM_LEVEL; i++)
	{
		pi_renderstate_set_default_sampler(&impl->ss_bloom[i]);
		pi_sampler_set_filter(&impl->ss_bloom[i], TFO_MIN_MAG_MIP_LINEAR);
		pi_sampler_set_addr_mode(&impl->ss_bloom[i], TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
		pi_sampler_set_texture(&impl->ss_bloom[i], impl->bloom_tex[i]);
	}

	pi_renderstate_set_default_sampler(&impl->ss_acc);
	pi_sampler_set_filter(&impl->ss_acc, TFO_MIN_MAG_MIP_LINEAR);
	pi_sampler_set_addr_mode(&impl->ss_acc, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_texture(&impl->ss_acc, impl->acc_tex);

	for (i = 0; i < DOWN_SAMPLE_LEVEL; i++)
	{
		pi_renderstate_set_default_sampler(&impl->ss_down_sample[i]);
		pi_sampler_set_filter(&impl->ss_down_sample[i], TFO_MIN_MAG_MIP_LINEAR);
		pi_sampler_set_addr_mode(&impl->ss_down_sample[i], TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
		pi_sampler_set_texture(&impl->ss_down_sample[i], impl->down_sample_tex[i]);
	}
	
	pi_material_set_depth_enable(impl->light_pass_material, FALSE);
	size[0] = 1.0f / width;
	size[1] = 1.0f / height;

	pi_material_set_depthwrite_enable(impl->light_pass_material, FALSE);

	pi_material_set_uniform(impl->light_pass_material, impl->U_LightPassParams, UT_VEC4, 1, &impl->light_pass_params, FALSE);
	pi_material_set_uniform(impl->light_pass_material, impl->U_RenderTargetSize, UT_VEC2, 1, size, TRUE);
	pi_material_set_def(impl->light_pass_material, impl->DIRECTLY_MAPPING, TRUE);

	pi_material_set_depth_enable(impl->tone_mapping_material, FALSE);
	pi_material_set_depthwrite_enable(impl->tone_mapping_material, FALSE);
	for (i = 0; i < BLOOM_LEVEL; i++)
	{
		pi_material_set_uniform(impl->tone_mapping_material, impl->U_BloomTex[i], UT_SAMPLER_2D, 1, &impl->ss_bloom[i], FALSE);
	}
	pi_material_set_uniform(impl->tone_mapping_material, impl->U_LuminanceMap, UT_SAMPLER_2D, 1, &impl->ss_acc, FALSE);

	pi_material_set_uniform(impl->tone_mapping_material, impl->U_ToneMapParams, UT_VEC4, 1, &impl->tone_map_params, FALSE);
	pi_material_set_def(impl->tone_mapping_material, impl->DIRECTLY_MAPPING, TRUE);
	pi_material_set_uniform(impl->tone_mapping_material, impl->U_RenderTargetSize, UT_VEC2, 1, size, TRUE);

	pi_material_set_depth_enable(impl->down_sample_material, FALSE);
	pi_material_set_depthwrite_enable(impl->down_sample_material, FALSE);
	pi_material_set_def(impl->down_sample_material, impl->DIRECTLY_MAPPING, TRUE);
	size[0] = 1.0f / impl->down_sample_tex[0]->width;
	size[1] = 1.0f / impl->down_sample_tex[0]->width;

	pi_material_set_uniform(impl->down_sample_material, impl->U_RenderTargetSize, UT_VEC2, 1, size, TRUE);

	pi_material_set_depth_enable(impl->acc_material, FALSE);
	pi_material_set_depthwrite_enable(impl->acc_material, FALSE);
	pi_material_set_uniform(impl->acc_material, impl->U_SrcTex, UT_SAMPLER_2D, 1, &impl->ss_down_sample[4], FALSE);
	pi_material_set_uniform(impl->acc_material, impl->U_AccTex, UT_SAMPLER_2D, 1, &impl->ss_acc, FALSE);
	pi_material_set_def(impl->acc_material, impl->DIRECTLY_MAPPING, TRUE);
	size[0] = 1.0f / impl->acc_tex->width;
	size[1] = 1.0f / impl->acc_tex->height;
	pi_material_set_uniform(impl->acc_material, impl->U_RenderTargetSize, UT_VEC2, 1, size, TRUE);

	pi_entity_set_mesh(impl->entity, impl->rmesh);

	pi_spatial_set_local_scaling(impl->entity->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->entity->spatial);

	impl->camera = pi_camera_new();

	pi_camera_set_location(impl->camera, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->camera, 0.0f, 0.0f, -1.0f);
	pi_camera_set_frustum(impl->camera, -(float)width / 2.0f, (float)width / 2.0f, -(float)height / 2.0f, (float)height / 2.0f, 0.0f, 2.0f, TRUE);
	return TRUE;
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	HDRRenderer *impl = (HDRRenderer *)renderer->impl;
	PiRenderTarget *target;
	uint i;
	float size[2];
	float offsetUV[2];
	PI_USE_PARAM(tpf);
	_type_check(renderer);
	pi_hash_lookup(resources, impl->output_name, (void **)&target);

	pi_rendersystem_set_target(impl->bloom_rt[0]);
	pi_rendersystem_set_camera(impl->camera);
	pi_entity_set_material(impl->entity, impl->light_pass_material);
	pi_entity_draw(impl->entity);

	pi_rendersystem_set_target(impl->down_sample_rt[0]);
	pi_entity_set_material(impl->entity, impl->down_sample_material);
	pi_entity_draw(impl->entity);

	for (i = 1; i < BLOOM_LEVEL; ++i)
	{
		pi_rendersystem_set_target(impl->bloom_rt[i]);
		pi_material_set_uniform(impl->bloom_down_sample_material, impl->U_SrcTex, UT_SAMPLER_2D, 1, &impl->ss_bloom[i - 1], FALSE);
		size[0] = 1.0f / impl->bloom_tex[i]->width;
		size[1] = 1.0f / impl->bloom_tex[i]->height;
		pi_material_set_uniform(impl->bloom_down_sample_material, impl->U_RenderTargetSize, UT_VEC2, 1, size, TRUE);
		pi_entity_set_material(impl->entity, impl->bloom_down_sample_material);
		pi_entity_draw(impl->entity);
	}

	for (i = 1; i < DOWN_SAMPLE_LEVEL; ++i)
	{
		pi_rendersystem_set_target(impl->down_sample_rt[i]);
		pi_material_set_uniform(impl->mipmap_material, impl->U_SrcTex, UT_SAMPLER_2D, 1, &impl->ss_down_sample[i - 1], FALSE);
		size[0] = 1.0f / impl->down_sample_tex[i]->width;
		size[1] = 1.0f / impl->down_sample_tex[i]->height;
		offsetUV[0] = 2.0f / impl->down_sample_tex[i - 1]->width;
		offsetUV[1] = 2.0f / impl->down_sample_tex[i - 1]->height;
		pi_material_set_uniform(impl->mipmap_material, impl->U_RenderTargetSize, UT_VEC2, 1, size, TRUE);
		pi_material_set_uniform(impl->mipmap_material, impl->U_offsetUV, UT_VEC2, 1, offsetUV, TRUE);
		pi_entity_set_material(impl->entity, impl->mipmap_material);
		pi_entity_draw(impl->entity);
	}

	pi_rendersystem_set_target(impl->acc_rt);
	pi_entity_set_material(impl->entity, impl->acc_material);
	pi_entity_draw(impl->entity);


	pi_rendersystem_set_target(target);
	pi_entity_set_material(impl->entity, impl->tone_mapping_material);
	pi_entity_draw(impl->entity);


}

static void _update(PiRenderer *renderer, float tpf, PiHash *resources)
{
	HDRRenderer *impl;
	PiTexture *src_tex;
	impl = (HDRRenderer *)renderer->impl;
	pi_hash_lookup(resources, impl->input_name, (void **)&src_tex);
	pi_sampler_set_texture(&impl->ss_src, src_tex);
	pi_material_set_uniform(impl->light_pass_material, impl->U_SrcTex, UT_SAMPLER_2D, 1, &impl->ss_src, FALSE);
	pi_material_set_uniform(impl->tone_mapping_material, impl->U_SceneTex, UT_SAMPLER_2D, 1, &impl->ss_src, FALSE);
	pi_material_set_uniform(impl->down_sample_material, impl->U_SrcTex, UT_SAMPLER_2D, 1, &impl->ss_src, FALSE);
	impl->luminance_params.x = tpf;
	pi_material_set_uniform(impl->acc_material, impl->U_LuminanceParams, UT_VEC4, 1, &impl->luminance_params, FALSE);
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{

}

PiRenderer *PI_API pi_hdr_new_with_name(char* name)
{
	PiRenderer *renderer;
	HDRRenderer *impl = pi_new0(HDRRenderer, 1);
	impl->tone_map_params.y = 0.5f;
	impl->tone_map_params.x = 10.0f;
	impl->min_tone_luminance = 0.07f;
	impl->light_pass_params.y = 0.6f;
	impl->light_pass_params.z = 1.0f - impl->light_pass_params.y;
	impl->light_pass_params.x = 1.0f;

	renderer = pi_renderer_create(ERT_HDR, name, _init, _resize, _update, _draw, impl);
	return renderer;
}

PiRenderer *PI_API pi_hdr_new()
{
	return pi_hdr_new_with_name("HDR");
}

void PI_API pi_hdr_deploy(PiRenderer *renderer, char *input_name, char *output_name)
{
	HDRRenderer *impl;
	_type_check(renderer);
	impl = (HDRRenderer *)renderer->impl;

	pi_free(impl->input_name);
	pi_free(impl->output_name);

	impl->input_name = pi_str_dup(input_name);
	impl->output_name = pi_str_dup(output_name);

	impl->is_deploy = TRUE;
}

void PI_API pi_hdr_free(PiRenderer *renderer)
{
	uint i;
	HDRRenderer *impl;
	_type_check(renderer);
	impl = (HDRRenderer *)renderer->impl;

	pi_mesh_free(impl->mesh);
	pi_rendermesh_free(impl->rmesh);
	pi_entity_free(impl->entity);
	pi_camera_free(impl->camera);
	pi_material_free(impl->light_pass_material);
	pi_material_free(impl->tone_mapping_material);
	pi_material_free(impl->acc_material);
	pi_material_free(impl->down_sample_material);
	pi_material_free(impl->mipmap_material);
	pi_material_free(impl->bloom_down_sample_material);


	for (i = 0; i < BLOOM_LEVEL; ++i)
	{
		pi_renderview_free(impl->bloom_view[i]);
		pi_rendertarget_free(impl->bloom_rt[i]);
		pi_texture_free(impl->bloom_tex[i]);
	}

	for (i = 0; i < DOWN_SAMPLE_LEVEL; ++i)
	{
		pi_renderview_free(impl->down_sample_view[i]);
		pi_rendertarget_free(impl->down_sample_rt[i]);
		pi_texture_free(impl->down_sample_tex[i]);
	}

	pi_renderview_free(impl->acc_view);
	pi_rendertarget_free(impl->acc_rt);
	pi_texture_free(impl->acc_tex);

	pi_free(impl->input_name);
	pi_free(impl->output_name);

	pi_free(renderer->impl);
	pi_renderer_destroy(renderer);
}

void PI_API pi_hdr_set_min_tone_luminance(PiRenderer *renderer, float tone_luminance)
{
	HDRRenderer *impl;
	_type_check(renderer);
	impl = (HDRRenderer *)renderer->impl;
	impl->luminance_params.y = tone_luminance;
}

void PI_API pi_hdr_set_gaussian_scalar(PiRenderer *renderer, float scalar)
{
	HDRRenderer *impl;
	_type_check(renderer);
	impl = (HDRRenderer *)renderer->impl;
	impl->tone_map_params.x = scalar;
}

void PI_API pi_hdr_set_exposure(PiRenderer *renderer, float exposure)
{
	HDRRenderer *impl;
	_type_check(renderer);
	impl = (HDRRenderer *)renderer->impl;
	impl->tone_map_params.y = exposure;
}

void PI_API pi_hdr_set_bloom_brightness_threshold(PiRenderer *renderer, float threshold)
{
	HDRRenderer *impl;
	_type_check(renderer);
	impl = (HDRRenderer *)renderer->impl;
	impl->light_pass_params.y = threshold;
	impl->light_pass_params.z = 1.0f - impl->light_pass_params.y;
}

void PI_API pi_hdr_set_bloom_scale(PiRenderer *renderer, float scale)
{
	HDRRenderer *impl;
	_type_check(renderer);
	impl = (HDRRenderer *)renderer->impl;
	impl->light_pass_params.x = scale;
}
