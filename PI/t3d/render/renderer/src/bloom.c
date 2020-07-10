#include <bloom.h>

#include <pi_spatial.h>

#include <texture.h>
#include <rendertarget.h>
#include <rendersystem.h>

#include <entity.h>
#include <material.h>

/**
 * 全屏泛光 bloom 流程
 * 创建两张纹理，用于中间临时渲染目标：tmp0，tmp2, 宽，高均为input的1/4
 * 1. input作为采样纹理，tmp0作为渲染目标，运行fs：bright.fs，作用：提取原图亮度
 * 2. tmp0作为采样纹理，tmp1作为渲染目标，运行fs：blur_v.fs，作用：纵向高斯模糊
 * 3. tmp1作为采样纹理，tmp0作为渲染目标，运行fs：blur_h.fs，作用：横向高斯模糊
 * 4. tmp0作为采样纹理，output作为渲染目标，运行fs：bloom_blend.fs，作用：将模糊后的图片和原图混合
 */

typedef struct
{
	char *input_name;
	char *output_name;

	PiEntity *entity;

	PiCamera *camera;

	PiMesh *mesh;
	PiRenderMesh *rmesh;

	PiTexture *input_tex;
	SamplerState src_ss;

	PiMaterial *bright_material;
	PiMaterial *blur_h_material;
	PiMaterial *blur_v_material;
	PiMaterial *blend_material;

	uint width, height;
	PiTexture *tmp_1, *tmp_2;
	PiRenderView *tmp_view_1, *tmp_view_2;
	PiRenderView *bloom_dss_view;
	PiRenderTarget *bloom_rt;

	float weights[2];

	PiBool is_deploy;

	/* 常量字符串 */
	char *U_Tex;
	char *U_SrcTex;
	char *U_BlendWeight;
	char *U_BloomTex;
} Bloom;

static PiBool _init(PiRenderer *renderer, PiHash* resources)
{
	Bloom *impl = (Bloom *)(renderer->impl);
	PiRenderTarget *target;

	PI_USE_PARAM(resources);

	impl->U_Tex = pi_conststr("u_Tex");
	impl->U_SrcTex = pi_conststr("u_SrcTex");
	impl->U_BlendWeight = pi_conststr("u_BlendWeight");
	impl->U_BloomTex = pi_conststr("u_BloomTex");

	if(impl->is_deploy == FALSE)
	{
		return FALSE;
	}

	impl->weights[0] = 1.0f;
	impl->weights[1] = 0.65f;

	pi_hash_lookup(resources, impl->output_name, (void**)&target);
	impl->width = target->width;
	impl->height = target->height;

	impl->input_tex = NULL;
	{
		impl->bright_material = pi_material_new(RS_BLOOM_BRIGHT_VS, RS_BLOOM_BRIGHT_FS);
		pi_material_set_depthwrite_enable(impl->bright_material, FALSE);
		impl->blur_h_material = pi_material_new(RS_BLOOM_BLUR_H_VS, RS_BLOOM_BLUR_H_FS);
		pi_material_set_depthwrite_enable(impl->blur_h_material, FALSE);
		
		impl->blur_v_material = pi_material_new(RS_BLOOM_BLUR_V_VS, RS_BLOOM_BLUR_V_FS);
		pi_material_set_depthwrite_enable(impl->blur_v_material, FALSE);
		
		impl->blend_material = pi_material_new(RS_BLOOM_BLEND_VS, RS_BLOOM_BLEND_FS);
		pi_material_set_depthwrite_enable(impl->blend_material, FALSE);
	}

	pi_renderstate_set_default_sampler(&impl->src_ss);

	impl->mesh = pi_mesh_create_quad(NULL, NULL, 0.0f);
	impl->rmesh = pi_rendermesh_new(impl->mesh, TRUE);
	impl->entity = pi_entity_new();

	pi_entity_set_mesh(impl->entity, impl->rmesh);

	pi_spatial_set_local_scaling(impl->entity->spatial, (float)impl->width, (float)impl->height, 1.0f);
	pi_spatial_update(impl->entity->spatial);

	impl->bloom_rt = pi_rendertarget_new(TT_MRT, TRUE);

	impl->tmp_1 = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, impl->width / 4, impl->height / 4, TRUE);
	impl->tmp_2 = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, impl->width / 4, impl->height / 4, TRUE);
	impl->tmp_view_1 = pi_renderview_new_tex2d(RVT_COLOR, impl->tmp_1, 0, 0, TRUE);
	impl->tmp_view_2 = pi_renderview_new_tex2d(RVT_COLOR, impl->tmp_2, 0, 0, TRUE);

	impl->bloom_dss_view = pi_renderview_new(RVT_DEPTH_STENCIL, impl->width / 4, impl->height / 4, RF_D16, TRUE);
	pi_rendertarget_attach(impl->bloom_rt, ATT_DEPTHSTENCIL, impl->bloom_dss_view);

	impl->camera = pi_camera_new();
	pi_camera_set_location(impl->camera, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->camera, 0.0f, 0.0f, -1.0f);
	pi_camera_set_frustum(impl->camera, -(float)impl->width / 2.0f, impl->width / 2.0f, -(float)impl->height / 2.0f, (float)impl->height / 2.0f, 0.0f, 2.0f, TRUE);
	
	return TRUE;
}

static void _update(PiRenderer *renderer, float tpf, PiHash* inputs)
{
	Bloom *impl = (Bloom *)(renderer->impl);
	PiBool is_lookup = pi_hash_lookup(inputs, impl->input_name, &impl->input_tex);
	PI_USE_PARAM(tpf);
	PI_USE_PARAM(is_lookup);
	PI_ASSERT(is_lookup, "bloom, input texture must already yet, input_name = %s", impl->input_name);
}

static void _draw(PiRenderer *renderer, float tpf, PiHash* outputs)
{
	PiBool is_lookup;
	SamplerState ss, blend_ss;
	Bloom *impl = (Bloom *)(renderer->impl);
	PiRenderTarget *screen_rt = NULL;

	PiColor clr;
	color_set(&clr, 0.0f, 0.0f, 0.0f, 1.0f);

	PI_USE_PARAM(tpf);
	is_lookup = pi_hash_lookup(outputs, impl->output_name, &screen_rt);

	pi_renderstate_set_default_sampler(&ss);
	pi_sampler_set_filter(&ss, TFO_MIN_MAG_LINEAR);

	/* 取亮度：用input_tex做采样，用tmp_1做目标 */
	pi_sampler_set_texture(&ss, impl->input_tex);
	pi_material_set_uniform(impl->bright_material, impl->U_Tex, UT_SAMPLER_2D, 1, &ss, TRUE);

	pi_entity_set_material(impl->entity, impl->bright_material);
	pi_rendertarget_attach(impl->bloom_rt, ATT_COLOR0, impl->tmp_view_1);
	pi_rendertarget_set_viewport(impl->bloom_rt, 0, 0, impl->width / 4, impl->height / 4);
	pi_rendersystem_set_target(impl->bloom_rt);
	pi_rendersystem_clearview(TBM_ALL, &clr, 1.0f, 0);
	pi_rendersystem_set_camera(impl->camera);
	pi_entity_draw(impl->entity);

	/* 横向高斯模糊，用tmp_1做纹理，tmp_2做目标 */
	pi_sampler_set_texture(&ss, impl->tmp_1);
	pi_material_set_uniform(impl->blur_h_material, impl->U_Tex, UT_SAMPLER_2D, 1, &ss, TRUE);

	pi_entity_set_material(impl->entity, impl->blur_h_material);
	pi_rendertarget_attach(impl->bloom_rt, ATT_COLOR0, impl->tmp_view_2);
	pi_rendertarget_set_viewport(impl->bloom_rt, 0, 0, impl->width / 4, impl->height / 4);
	pi_rendersystem_set_target(impl->bloom_rt);

	pi_rendersystem_clearview(TBM_ALL, &clr, 1.0f, 0);
	pi_entity_draw(impl->entity);

	/* 纵向高斯模糊，用tmp_2做纹理，tmp_1做目标 */
	pi_sampler_set_texture(&ss, impl->tmp_2);
	pi_material_set_uniform(impl->blur_v_material, impl->U_Tex, UT_SAMPLER_2D, 1, &ss, TRUE);

	pi_entity_set_material(impl->entity, impl->blur_v_material);
	pi_rendertarget_attach(impl->bloom_rt, ATT_COLOR0, impl->tmp_view_1);
	pi_rendertarget_set_viewport(impl->bloom_rt, 0, 0, impl->width / 4, impl->height / 4);
	pi_rendersystem_set_target(impl->bloom_rt);
	pi_rendersystem_clearview(TBM_ALL, &clr, 1.0f, 0);
	pi_entity_draw(impl->entity);

	/* 混合，用tmp_1和input_tex做纹理，直接混合 */
	pi_renderstate_set_default_sampler(&blend_ss);
	pi_sampler_set_filter(&blend_ss, TFO_MIN_MAG_LINEAR);

	pi_sampler_set_texture(&ss, impl->input_tex);
	pi_material_set_uniform(impl->blend_material, impl->U_SrcTex, UT_SAMPLER_2D, 1, &ss, TRUE);
	pi_material_set_uniform(impl->blend_material, impl->U_BlendWeight, UT_VEC2, 1, &impl->weights, TRUE);

	pi_sampler_set_texture(&blend_ss, impl->tmp_1);
	pi_material_set_uniform(impl->blend_material, impl->U_BloomTex, UT_SAMPLER_2D, 1, &blend_ss, TRUE);

	pi_rendertarget_set_viewport(impl->bloom_rt, 0, 0, screen_rt->width, screen_rt->height);
	pi_rendersystem_set_target(screen_rt);

	pi_entity_set_material(impl->entity, impl->blend_material);
	pi_rendersystem_clearview(TBM_ALL, &clr, 1.0f, 0);
	pi_entity_draw(impl->entity);
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	SamplerState ss;
	Bloom *impl = renderer->impl;

	impl->width = width / 4;
	impl->height = height / 4;
	pi_renderstate_set_default_sampler(&ss);

	pi_rendertarget_detach(impl->bloom_rt, ATT_COLOR0);
	pi_rendertarget_detach(impl->bloom_rt, ATT_DEPTHSTENCIL);

	pi_texture_free(impl->tmp_1);
	pi_renderview_free(impl->tmp_view_1);
	impl->tmp_1 = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, width / 4, height / 4, TRUE);
	impl->tmp_view_1 = pi_renderview_new_tex2d(RVT_COLOR, impl->tmp_1, 0, 0, TRUE);

	pi_texture_free(impl->tmp_2);
	pi_renderview_free(impl->tmp_view_2);
	impl->tmp_2 = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, width / 4, height / 4, TRUE);
	impl->tmp_view_2 = pi_renderview_new_tex2d(RVT_COLOR, impl->tmp_2, 0, 0, TRUE);

	pi_renderview_free(impl->bloom_dss_view);
	impl->bloom_dss_view = pi_renderview_new(RVT_DEPTH_STENCIL, width / 4, height / 4, RF_D16, TRUE);
	pi_rendertarget_attach(impl->bloom_rt, ATT_DEPTHSTENCIL, impl->bloom_dss_view);
}

PiRenderer* PI_API pi_bloom_new()
{
	PiRenderer *renderer;
	Bloom *impl = pi_new0(Bloom, 1);
	impl->is_deploy = FALSE;
	renderer = pi_renderer_create(ERT_BLOOM, "bloom", _init, _resize, _update, _draw, impl);
	return renderer;
}

void PI_API pi_bloom_deploy(PiRenderer *renderer, char *input_name, char *output_name)
{
	Bloom *impl = renderer->impl;

	pi_free(impl->input_name);
	pi_free(impl->output_name);

	impl->input_name = pi_str_dup(input_name);
	impl->output_name = pi_str_dup(output_name);

	impl->is_deploy = TRUE;
}

void PI_API pi_bloom_set_weights(PiRenderer *renderer, float sharp, float blur)
{
	Bloom *impl = renderer->impl;
	impl->weights[0] = sharp;
	impl->weights[1] = blur;
}

void PI_API pi_bloom_free(PiRenderer *renderer)
{
	Bloom *impl = renderer->impl;

	pi_material_free(impl->blend_material);
	pi_material_free(impl->blur_h_material);
	pi_material_free(impl->blur_v_material);
	pi_material_free(impl->bright_material);

	pi_renderview_free(impl->tmp_view_1);
	pi_renderview_free(impl->tmp_view_2);
	pi_renderview_free(impl->bloom_dss_view);

	pi_texture_free(impl->tmp_1);
	pi_texture_free(impl->tmp_2);

	pi_rendertarget_free(impl->bloom_rt);

	pi_entity_free(impl->entity);
	pi_rendermesh_free(impl->rmesh);

	pi_free(impl);
	pi_renderer_destroy(renderer);
}