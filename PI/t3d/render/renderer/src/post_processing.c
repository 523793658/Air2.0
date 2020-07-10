
#include "post_processing.h"
#include <pi_spatial.h>
#include <entity.h>
#include <rendertarget.h>
#include <rendersystem.h>


const static char *RS_POST_PROCESSING_VS = "default.vs";
const static char *RS_POST_PROCESSING_FS = "post_processing.fs";
const static char *RS_FOG_FS = "fog.fs";
const static char *RS_GAUS_BLUR_VS = "gaus_blur.vs";
const static char *RS_GAUS_BLUR_FS = "gaus_blur.fs";

const static char *RS_EXPOSURE_VS = "default.vs";
const static char *RS_EXPOSURE_FS = "exposure.fs";

const static char *RS_DOWN_SAMPLE_VS = "default.vs";
const static char *RS_DOWN_SAMPLE_FS = "down_sample.fs";

const static char *RS_SIMPLE_VS = "default.vs";
const static char *RS_SIMPLE_FS = "simplest.fs";







typedef struct  
{
	PiVector3 fog_color;
	float fog_density;

	float fog_start;
	float fog_end;
	float flow_speed;
	float scale_ui;

	float src_width;
	float src_height;
	float bloom_threshold;
	float bloom_scale;
}PostProcessingParams;
typedef struct
{
	float fog_type;
	float range_enable;
	float flow_enable;
	float pading1;
}FogSwitch;

typedef struct  
{
	
	float fxaa_enable;
	float bloom_enable;
	float pading2;
	float pading3;
}PostProcessingSwitch;

/**
 * PostProcessing渲染器
 */
typedef struct
{
	PiBool is_deploy;
	PiBool bloom_enable;

	float texCoordOffset_4[8];

	float texCoordOffset_16[8];

	float texCoordBlur_h[24];
	float texCoordBlur_v[24];

	PiMesh *mesh;
	PiRenderMesh *rmesh;
	PiMaterial *material;
	PiEntity *entity;
	PiEntity *entity_4;
	PiEntity *entity_16;

	PiCamera *camera_4;
	PiCamera *camera;
	PiCamera *camera_16;

	PiMaterial *fog_material;

	PiMaterial *down_sampler_material_1_4;
	PiMaterial *down_sampler_material_4_16;
	PiMaterial *simple_material;
	PiMaterial *exposure_material;
	PiMaterial *blur_material;



	PiMatrix4 proj_mat_inverse;

	SamplerState scene_color_sampler;
	SamplerState scene_depth_sampler;
	SamplerState post_color_sampler;

	SamplerState post_color_sampler_4;
	SamplerState post_color_sampler_16_1;
	SamplerState post_color_sampler_16_2;

	SamplerState clut_sampler;
	float fog_density;

	PostProcessingParams params;
	PostProcessingSwitch switchs;
	FogSwitch fog_switchs;

	PiRenderTarget *post_screen_target;
	PiRenderTarget *post_screen_target_4;
	PiRenderTarget *post_screen_target_16_1;
	PiRenderTarget *post_screen_target_16_2;


	PiTexture *post_screen_color_texture;
	PiRenderView *post_screen_color_view;

	PiTexture *post_screen_color_texture_4;
	PiRenderView *post_screen_color_view_4;

	PiTexture *post_screen_color_texture_16_1;
	PiRenderView *post_screen_color_view_16_1;

	PiTexture *post_screen_color_texture_16_2;
	PiRenderView *post_screen_color_view_16_2;

	PiTexture *clut_texture;
	PiBool color_grading_enable;

	

	char *scene_color_name;
	char *scene_depth_name;
	char *view_camera_name;
	char *target_name;

	char *U_SceneColorTex;
	char *U_SceneDepthTex;
	char *U_CLUTTex;
	char *U_BloomTex;
	char *U_Params;
	char *U_Switchs;
	char *U_ProjMatrixInverse;
	char *u_SrcTex;
	char *U_Offset;
	char *U_DiffuseMap;
	char *COLOR_GRADING;
} PostProcessingRenderer;

static void adjust_gaus_params(PostProcessingRenderer* impl, uint width, uint height)
{
	int i;
	impl->texCoordOffset_4[0] = -1.0f / width;
	impl->texCoordOffset_4[1] = -1.0f / height;

	impl->texCoordOffset_4[2] = 1.0f / width;
	impl->texCoordOffset_4[3] = -1.0f / height;

	impl->texCoordOffset_4[4] = -1.0f / width;
	impl->texCoordOffset_4[5] = 1.0f / height;

	impl->texCoordOffset_4[6] = 1.0f / width;
	impl->texCoordOffset_4[7] = 1.0f / height;


	impl->texCoordOffset_16[0] = -4.0f / width;
	impl->texCoordOffset_16[1] = -4.0f / height;

	impl->texCoordOffset_16[2] = 4.0f / width;
	impl->texCoordOffset_16[3] = -4.0f / height;

	impl->texCoordOffset_16[4] = -4.0f / width;
	impl->texCoordOffset_16[5] = 4.0f / height;

	impl->texCoordOffset_16[6] = 4.0f / width;
	impl->texCoordOffset_16[7] = 4.0f / height;

	for (i = 0; i < 6; i++)
	{
		impl->texCoordBlur_v[i * 4] = -16.0f * (i + 1) / width;
		impl->texCoordBlur_v[i * 4 + 1] = 0.0f;

		impl->texCoordBlur_v[i * 4 + 2] = 16.0f * (i + 1) / width;
		impl->texCoordBlur_v[i * 4 + 3] = 0.0f;

		impl->texCoordBlur_h[i * 4] = 0.0f;
		impl->texCoordBlur_h[i * 4 + 1] = -16.0f * (i + 1) / height;
		impl->texCoordBlur_h[i * 4 + 2] = 0.0f;
		impl->texCoordBlur_h[i * 4 + 3] = 16.0f * (i + 1) / height;
	}
}

static void _type_check(PiRenderer *renderer)
{
	PI_ASSERT((renderer)->type == ERT_POST_PROCESSING, "Renderer type error!");
}

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	PiRenderTarget *target;
	uint32 width, height;
	PostProcessingRenderer *impl = (PostProcessingRenderer *)renderer->impl;
	if (!impl->is_deploy)
	{
		return FALSE;
	}
	impl->U_SceneColorTex = pi_conststr("u_SceneColorTex");
	impl->U_SceneDepthTex = pi_conststr("u_SceneDepthTex");
	impl->U_Params = pi_conststr("u_Params");
	impl->U_Switchs = pi_conststr("u_Switchs");
	impl->U_ProjMatrixInverse = pi_conststr("u_ProjMatrixInverse");
	impl->U_CLUTTex = pi_conststr("u_CLUTTex");
	impl->COLOR_GRADING = pi_conststr("COLOR_GRADING");
	impl->U_BloomTex = pi_conststr("u_BloomTex");
	impl->u_SrcTex = pi_conststr("u_SrcTex");
	impl->U_Offset = pi_conststr("u_Offset");
	impl->U_DiffuseMap = pi_conststr("u_DiffuseMap");

	pi_hash_lookup(resources, impl->target_name, (void **)&target);
	width = target->width;
	height = target->height;

	adjust_gaus_params(impl, width, height);

	impl->params.src_width = 1.0f / width;
	impl->params.src_height = 1.0f / height;

	impl->post_screen_target = pi_rendertarget_new(TT_MRT, TRUE);
	impl->post_screen_target_4 = pi_rendertarget_new(TT_MRT, TRUE);
	impl->post_screen_target_16_1 = pi_rendertarget_new(TT_MRT, TRUE);
	impl->post_screen_target_16_2 = pi_rendertarget_new(TT_MRT, TRUE);

	impl->post_screen_color_texture = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, width, height, TRUE);
	impl->post_screen_color_texture_4 = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, width / 4, height / 4, TRUE);

	impl->post_screen_color_texture_16_1 = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, width / 16, height / 16, TRUE);

	impl->post_screen_color_texture_16_2 = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, width / 16, height / 16, TRUE);



	impl->post_screen_color_view = pi_renderview_new_tex2d(RVT_COLOR, impl->post_screen_color_texture, 0, 0, TRUE);

	impl->post_screen_color_view_4 = pi_renderview_new_tex2d(RVT_COLOR, impl->post_screen_color_texture_4, 0, 0, TRUE);

	impl->post_screen_color_view_16_1 = pi_renderview_new_tex2d(RVT_COLOR, impl->post_screen_color_texture_16_1, 0, 0, TRUE);

	impl->post_screen_color_view_16_2 = pi_renderview_new_tex2d(RVT_COLOR, impl->post_screen_color_texture_16_2, 0, 0, TRUE);

	pi_rendertarget_attach(impl->post_screen_target, ATT_COLOR0, impl->post_screen_color_view);

	pi_rendertarget_attach(impl->post_screen_target_4, ATT_COLOR0, impl->post_screen_color_view_4);

	pi_rendertarget_attach(impl->post_screen_target_16_1, ATT_COLOR0, impl->post_screen_color_view_16_1);

	pi_rendertarget_attach(impl->post_screen_target_16_2, ATT_COLOR0, impl->post_screen_color_view_16_2);

	pi_renderstate_set_default_sampler(&impl->scene_color_sampler);
	pi_sampler_set_filter(&impl->scene_color_sampler, TFO_MIN_MAG_LINEAR);
	pi_sampler_set_addr_mode(&impl->scene_color_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);

	pi_renderstate_set_default_sampler(&impl->scene_depth_sampler);
	pi_sampler_set_filter(&impl->scene_depth_sampler, TFO_MIN_MAG_LINEAR);
	pi_sampler_set_addr_mode(&impl->scene_depth_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);

	pi_renderstate_set_default_sampler(&impl->post_color_sampler);
	pi_sampler_set_filter(&impl->post_color_sampler, TFO_MIN_MAG_LINEAR);
	pi_sampler_set_addr_mode(&impl->post_color_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_texture(&impl->post_color_sampler, impl->post_screen_color_texture);


	pi_renderstate_set_default_sampler(&impl->post_color_sampler_4);
	pi_sampler_set_filter(&impl->post_color_sampler_4, TFO_MIN_MAG_LINEAR);
	pi_sampler_set_addr_mode(&impl->post_color_sampler_4, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_texture(&impl->post_color_sampler_4, impl->post_screen_color_texture_4);


	pi_renderstate_set_default_sampler(&impl->post_color_sampler_16_1);
	pi_sampler_set_filter(&impl->post_color_sampler_16_1, TFO_MIN_MAG_LINEAR);
	pi_sampler_set_addr_mode(&impl->post_color_sampler_16_1, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_texture(&impl->post_color_sampler_16_1, impl->post_screen_color_texture_16_1);

	pi_renderstate_set_default_sampler(&impl->post_color_sampler_16_2);
	pi_sampler_set_filter(&impl->post_color_sampler_16_2, TFO_MIN_MAG_LINEAR);
	pi_sampler_set_addr_mode(&impl->post_color_sampler_16_2, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_texture(&impl->post_color_sampler_16_2, impl->post_screen_color_texture_16_2);

	pi_renderstate_set_default_sampler(&impl->clut_sampler);
	pi_sampler_set_texture(&impl->clut_sampler, impl->clut_texture);
	pi_sampler_set_filter(&impl->clut_sampler, TFO_MIN_MAG_LINEAR);
	pi_sampler_set_addr_mode(&impl->clut_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);

	impl->fog_material = pi_material_new(RS_POST_PROCESSING_VS, RS_FOG_FS);
	pi_material_set_depth_enable(impl->fog_material, FALSE);
	pi_material_set_depthwrite_enable(impl->fog_material, FALSE);
	pi_material_set_uniform(impl->fog_material, impl->U_SceneColorTex, UT_SAMPLER_2D, 1, &impl->scene_color_sampler, FALSE);
	pi_material_set_uniform(impl->fog_material, impl->U_SceneDepthTex, UT_SAMPLER_2D, 1, &impl->scene_depth_sampler, FALSE);
	pi_material_set_uniform(impl->fog_material, impl->U_Switchs, UT_IVEC4, sizeof(FogSwitch) / sizeof(float) / 4, &impl->fog_switchs, FALSE);
	pi_material_set_uniform(impl->fog_material, impl->U_Params, UT_VEC4, sizeof(PostProcessingParams) / sizeof(float) / 4, &impl->params, FALSE);

	impl->material = pi_material_new(RS_POST_PROCESSING_VS, RS_POST_PROCESSING_FS);
	pi_material_set_depth_enable(impl->material, FALSE);
	pi_material_set_depthwrite_enable(impl->material, FALSE);
	pi_material_set_uniform(impl->material, impl->U_SceneColorTex, UT_SAMPLER_2D, 1, &impl->post_color_sampler, FALSE);
	pi_material_set_uniform(impl->material, impl->U_SceneDepthTex, UT_SAMPLER_2D, 1, &impl->scene_depth_sampler, FALSE);
	pi_material_set_uniform(impl->material, impl->U_CLUTTex, UT_SAMPLER_3D, 1, &impl->clut_sampler, FALSE);

	impl->down_sampler_material_1_4 = pi_material_new(RS_DOWN_SAMPLE_VS, RS_DOWN_SAMPLE_FS);
	pi_material_set_depth_enable(impl->down_sampler_material_1_4, FALSE);
	pi_material_set_depthwrite_enable(impl->down_sampler_material_1_4, FALSE);
	pi_material_set_uniform(impl->down_sampler_material_1_4, impl->U_Offset, UT_VEC4, 2, impl->texCoordOffset_4, FALSE);

	impl->down_sampler_material_4_16 = pi_material_new(RS_DOWN_SAMPLE_VS, RS_DOWN_SAMPLE_FS);
	pi_material_set_depth_enable(impl->down_sampler_material_4_16, FALSE);
	pi_material_set_depthwrite_enable(impl->down_sampler_material_4_16, FALSE);
	pi_material_set_uniform(impl->down_sampler_material_4_16, impl->U_Offset, UT_VEC4, 2, impl->texCoordOffset_16, FALSE);

	impl->exposure_material = pi_material_new(RS_EXPOSURE_VS, RS_EXPOSURE_FS);
	pi_material_set_depth_enable(impl->exposure_material, FALSE);
	pi_material_set_depthwrite_enable(impl->exposure_material, FALSE);
	pi_material_set_uniform_pack_flag(impl->exposure_material, impl->U_Params, UT_FLOAT, 1, &impl->params.bloom_threshold, FALSE, TRUE);
	pi_material_set_uniform(impl->exposure_material, impl->u_SrcTex, UT_SAMPLER_2D, 1, &impl->post_color_sampler_16_1, FALSE);

	impl->blur_material = pi_material_new(RS_GAUS_BLUR_VS, RS_GAUS_BLUR_FS);
	pi_material_set_depth_enable(impl->blur_material, FALSE);
	pi_material_set_depthwrite_enable(impl->blur_material, FALSE);
	pi_material_set_uniform_pack_flag(impl->blur_material, impl->U_Params, UT_FLOAT, 1, &impl->params.bloom_scale, FALSE, TRUE);

	impl->simple_material = pi_material_new(RS_SIMPLE_VS, RS_SIMPLE_FS);
	pi_material_set_depth_enable(impl->simple_material, FALSE);
	pi_material_set_depthwrite_enable(impl->simple_material, FALSE);
	pi_material_set_uniform(impl->simple_material, impl->U_DiffuseMap, UT_SAMPLER_2D, 1, &impl->post_color_sampler_16_2, FALSE);



	pi_material_set_uniform(impl->material, impl->U_BloomTex, UT_SAMPLER_2D, 1, &impl->post_color_sampler_4, FALSE);

 	pi_material_set_uniform(impl->material, impl->U_Params, UT_VEC4, sizeof(PostProcessingParams) / sizeof(float) / 4, &impl->params, FALSE);
	pi_material_set_uniform(impl->material, impl->U_Switchs, UT_IVEC4, sizeof(PostProcessingSwitch) / sizeof(float) / 4, &impl->switchs, FALSE);
	pi_material_set_def(impl->material, impl->COLOR_GRADING, impl->color_grading_enable && impl->clut_texture != NULL);

	impl->mesh = pi_mesh_create_quad(NULL, NULL, 0.0f);
	impl->rmesh = pi_rendermesh_new(impl->mesh, TRUE);

	impl->entity = pi_entity_new();
	pi_entity_set_mesh(impl->entity, impl->rmesh);
	pi_spatial_set_local_scaling(impl->entity->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->entity->spatial);

	impl->entity_4 = pi_entity_new();
	pi_entity_set_mesh(impl->entity_4, impl->rmesh);
	pi_spatial_set_local_scaling(impl->entity_4->spatial, (float)width / 4, (float)height / 4, 1.0f);
	pi_spatial_update(impl->entity_4->spatial);

	impl->entity_16 = pi_entity_new();
	pi_entity_set_mesh(impl->entity_16, impl->rmesh);
	pi_spatial_set_local_scaling(impl->entity_16->spatial, (float)width / 16, (float)height / 16, 1.0f);
	pi_spatial_update(impl->entity_16->spatial);



	impl->camera = pi_camera_new();
	pi_camera_set_location(impl->camera, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->camera, 0.0f, 0.0f, -1.0f);
	pi_camera_set_up(impl->camera, 0.0f, 1.0f, 0.0f);

	pi_camera_set_frustum(impl->camera, -(float)width / 2.0f + 0.5f, (float)width / 2.0f + 0.5f, -(float)height / 2.0f - 0.5f, 
		(float)height / 2.0f - 0.5f, 0.0f, 2.0f, TRUE);


	impl->camera_4 = pi_camera_new();
	pi_camera_set_location(impl->camera_4, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->camera_4, 0.0f, 0.0f, -1.0f);
	pi_camera_set_up(impl->camera_4, 0.0f, 1.0f, 0.0f);

	pi_camera_set_frustum(impl->camera_4, -(float)width / 8.0f + 0.5f, (float)width / 8.0f + 0.5f, -(float)height / 8.0f - 0.5f,
		(float)height / 8.0f - 0.5f, 0.0f, 2.0f, TRUE);



	impl->camera_16 = pi_camera_new();
	pi_camera_set_location(impl->camera_16, 0.0f, 0.0f, 1.0f);
	pi_camera_set_up(impl->camera_16, 0.0f, 1.0f, 0.0f);

	pi_camera_set_direction(impl->camera_16, 0.0f, 0.0f, -1.0f);
	pi_camera_set_frustum(impl->camera_16, -(float)width / 32.0f + 0.5f, (float)width / 32.0f + 0.5f, -(float)height / 32.0f - 0.5f,
		(float)height / 32.0f - 0.5f, 0.0f, 2.0f, TRUE);

	return TRUE;
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	PostProcessingRenderer *impl;
	PiRenderTarget *target;

	_type_check(renderer);

	impl = (PostProcessingRenderer *)renderer->impl;

	pi_hash_lookup(resources, impl->target_name, (void **)&target);
	pi_rendersystem_set_camera(impl->camera);


	//后期雾处理
	pi_rendersystem_set_target(impl->post_screen_target);
	pi_entity_set_material(impl->entity, impl->fog_material);
	pi_entity_draw(impl->entity);

	if (impl->bloom_enable)
	{
		//缩小到1/4倍
		pi_rendersystem_set_target(impl->post_screen_target_4);
		pi_rendersystem_set_camera(impl->camera_4);
		pi_entity_set_material(impl->entity_4, impl->down_sampler_material_1_4);
		pi_material_set_uniform(impl->down_sampler_material_1_4, impl->u_SrcTex, UT_SAMPLER_2D, 1, &impl->post_color_sampler, FALSE);
		pi_entity_draw(impl->entity_4);

		//缩小到1/16倍
		pi_rendersystem_set_target(impl->post_screen_target_16_1);
		pi_rendersystem_set_camera(impl->camera_16);
		pi_entity_set_material(impl->entity_16, impl->down_sampler_material_4_16);
		pi_material_set_uniform(impl->down_sampler_material_4_16, impl->u_SrcTex, UT_SAMPLER_2D, 1, &impl->post_color_sampler_4, FALSE);
		pi_entity_draw(impl->entity_16);

		//提取高亮部分
		pi_rendersystem_set_target(impl->post_screen_target_16_2);
		pi_entity_set_material(impl->entity_16, impl->exposure_material);
		pi_entity_draw(impl->entity_16);

		//横向高斯模糊
		pi_rendersystem_set_target(impl->post_screen_target_16_1);
		pi_entity_set_material(impl->entity_16, impl->blur_material);
		pi_material_set_uniform(impl->blur_material, impl->u_SrcTex, UT_SAMPLER_2D, 1, &impl->post_color_sampler_16_2, FALSE);
		pi_material_set_uniform(impl->blur_material, impl->U_Offset, UT_VEC4, 6, &impl->texCoordBlur_v, FALSE);
		pi_entity_draw(impl->entity_16);

		//纵向高斯模糊
		pi_rendersystem_set_target(impl->post_screen_target_16_2);
		pi_material_set_uniform(impl->blur_material, impl->u_SrcTex, UT_SAMPLER_2D, 1, &impl->post_color_sampler_16_1, FALSE);
		pi_material_set_uniform(impl->blur_material, impl->U_Offset, UT_VEC4, 6, &impl->texCoordBlur_h, FALSE);
		pi_entity_draw(impl->entity_16);

		//横向高斯模糊
		pi_rendersystem_set_target(impl->post_screen_target_16_1);
		pi_material_set_uniform(impl->blur_material, impl->u_SrcTex, UT_SAMPLER_2D, 1, &impl->post_color_sampler_16_2, FALSE);
		pi_material_set_uniform(impl->blur_material, impl->U_Offset, UT_VEC4, 6, &impl->texCoordBlur_v, FALSE);
		pi_entity_draw(impl->entity_16);

		//纵向高斯模糊
		pi_rendersystem_set_target(impl->post_screen_target_16_2);
		pi_material_set_uniform(impl->blur_material, impl->u_SrcTex, UT_SAMPLER_2D, 1, &impl->post_color_sampler_16_1, FALSE);
		pi_material_set_uniform(impl->blur_material, impl->U_Offset, UT_VEC4, 6, &impl->texCoordBlur_h, FALSE);
		pi_entity_draw(impl->entity_16);

		//放大到1/4大小
		pi_rendersystem_set_target(impl->post_screen_target_4);
		pi_entity_set_material(impl->entity_4, impl->simple_material);
		pi_rendersystem_set_camera(impl->camera_4);
		pi_entity_draw(impl->entity_4);
	}
	pi_rendersystem_set_camera(impl->camera);
	pi_rendersystem_set_target(target);
	pi_entity_set_material(impl->entity, impl->material);
	pi_entity_draw(impl->entity);
}

static void _update(PiRenderer *renderer, float tpf, PiHash *resources)
{
	PostProcessingRenderer *impl;
	PiTexture *scene_color_tex;
	PiTexture *scene_depth_tex;
	PiCamera *view_camera;
	_type_check(renderer);

	impl = (PostProcessingRenderer *)renderer->impl;

	pi_hash_lookup(resources, impl->scene_color_name, (void **)&scene_color_tex);
	pi_sampler_set_texture(&impl->scene_color_sampler, scene_color_tex);

	pi_hash_lookup(resources, impl->scene_depth_name, (void **)&scene_depth_tex);
	pi_sampler_set_texture(&impl->scene_depth_sampler, scene_depth_tex);

	pi_hash_lookup(resources, impl->view_camera_name, (void **)&view_camera);
	pi_mat4_inverse(&impl->proj_mat_inverse, pi_camera_get_projection_matrix(view_camera));
	pi_material_set_uniform(impl->fog_material, impl->U_ProjMatrixInverse, UT_MATRIX4, 1, &impl->proj_mat_inverse, FALSE);

	}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	PostProcessingRenderer *impl;
	int width_4 = width / 4, height_4 = height / 4, width_16 = width / 16, height_16 = height / 16;

	_type_check(renderer);
	impl = (PostProcessingRenderer *)renderer->impl;
	adjust_gaus_params(impl, width, height);

	pi_spatial_set_local_scaling(impl->entity->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->entity->spatial);

	pi_spatial_set_local_scaling(impl->entity_4->spatial, (float)width_4, (float)height_4, 1.0f);
	pi_spatial_update(impl->entity_4->spatial);

	pi_spatial_set_local_scaling(impl->entity_16->spatial, (float)width_16, (float)height_16, 1.0f);
	pi_spatial_update(impl->entity_16->spatial);

	pi_camera_set_frustum(impl->camera, -(width / 2.0f) + 0.5f, width / 2.0f + 0.5f, -(height / 2.0f) - 0.5f, height / 2.0f - 0.5f, 0.0f, 2.0f, TRUE);

	pi_camera_set_frustum(impl->camera_4, -(width_4 / 2.0f) + 0.5f, width_4 / 2.0f + 0.5f, -(height_4 / 2.0f) - 0.5f, height_4 / 2.0f - 0.5f, 0.0f, 2.0f, TRUE);

	pi_camera_set_frustum(impl->camera_16, -(width_16 / 2.0f) + 0.5f, width_16 / 2.0f + 0.5f, -(height_16 / 2.0f) - 0.5f, height_16 / 2.0f - 0.5f, 0.0f, 2.0f, TRUE);


	impl->params.src_height = 1.0f / height;
	impl->params.src_width = 1.0f / width;

	pi_renderview_free(impl->post_screen_color_view);
	pi_renderview_free(impl->post_screen_color_view_4);
	pi_renderview_free(impl->post_screen_color_view_16_1);
	pi_renderview_free(impl->post_screen_color_view_16_2);

	pi_texture_free(impl->post_screen_color_texture);
	pi_texture_free(impl->post_screen_color_texture_4);
	pi_texture_free(impl->post_screen_color_texture_16_1);
	pi_texture_free(impl->post_screen_color_texture_16_2);

	impl->post_screen_color_texture = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, width, height, TRUE);
	impl->post_screen_color_texture_4 = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, width_4, height_4, TRUE);
	impl->post_screen_color_texture_16_1 = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, width_16, height_16, TRUE);
	impl->post_screen_color_texture_16_2 = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, width_16, height_16, TRUE);

	impl->post_screen_color_view = pi_renderview_new_tex2d(RVT_COLOR, impl->post_screen_color_texture, 0, 0, TRUE);
	impl->post_screen_color_view_4 = pi_renderview_new_tex2d(RVT_COLOR, impl->post_screen_color_texture_4, 0, 0, TRUE);
	impl->post_screen_color_view_16_1 = pi_renderview_new_tex2d(RVT_COLOR, impl->post_screen_color_texture_16_1, 0, 0, TRUE);
	impl->post_screen_color_view_16_2 = pi_renderview_new_tex2d(RVT_COLOR, impl->post_screen_color_texture_16_2, 0, 0, TRUE);

	pi_rendertarget_attach(impl->post_screen_target, ATT_COLOR0, impl->post_screen_color_view);
	pi_rendertarget_attach(impl->post_screen_target_4, ATT_COLOR0, impl->post_screen_color_view_4);
	pi_rendertarget_attach(impl->post_screen_target_16_1, ATT_COLOR0, impl->post_screen_color_view_16_1);
	pi_rendertarget_attach(impl->post_screen_target_16_2, ATT_COLOR0, impl->post_screen_color_view_16_2);

	pi_rendertarget_set_viewport(impl->post_screen_target, 0, 0, width, height);
	pi_rendertarget_set_viewport(impl->post_screen_target_4, 0, 0, width_4, height_4);
	pi_rendertarget_set_viewport(impl->post_screen_target_16_1, 0, 0, width_16, height_16);
	pi_rendertarget_set_viewport(impl->post_screen_target_16_2, 0, 0, width_16, height_16);


	pi_sampler_set_texture(&impl->post_color_sampler, impl->post_screen_color_texture);
	pi_sampler_set_texture(&impl->post_color_sampler_4, impl->post_screen_color_texture_4);
	pi_sampler_set_texture(&impl->post_color_sampler_16_1, impl->post_screen_color_texture_16_1);
	pi_sampler_set_texture(&impl->post_color_sampler_16_2, impl->post_screen_color_texture_16_2);
}

PiRenderer *PI_API pi_post_processing_new()
{
	PiRenderer *renderer;
	PostProcessingRenderer *impl = pi_new0(PostProcessingRenderer, 1);
	renderer = pi_renderer_create(ERT_POST_PROCESSING, "post_processing", _init, _resize, _update, _draw, impl);
	return renderer;
}

void PI_API pi_post_processing_deploy(PiRenderer *renderer, char *scene_color_name, char *scene_depth_name, char *target_name, char *view_camera_name)
{
	PostProcessingRenderer *impl;

	_type_check(renderer);

	impl = (PostProcessingRenderer *)renderer->impl;

	pi_free(impl->scene_color_name);
	pi_free(impl->target_name);

	impl->scene_color_name = pi_str_dup(scene_color_name);
	impl->scene_depth_name = pi_str_dup(scene_depth_name);
	impl->target_name = pi_str_dup(target_name);
	impl->view_camera_name = pi_str_dup(view_camera_name);

	impl->is_deploy = TRUE;
}

void PI_API pi_post_processing_free(PiRenderer *renderer)
{
	PostProcessingRenderer *impl;

	_type_check(renderer);

	impl = (PostProcessingRenderer *)renderer->impl;

	pi_entity_free(impl->entity);
	pi_material_free(impl->material);
	pi_rendermesh_free(impl->rmesh);
	pi_mesh_free(impl->mesh);
	pi_camera_free(impl->camera);
	pi_material_free(impl->exposure_material);
	pi_material_free(impl->fog_material);
	pi_rendertarget_free(impl->post_screen_target);
	pi_renderview_free(impl->post_screen_color_view);
	pi_texture_free(impl->post_screen_color_texture);
	pi_free(impl->scene_color_name);
	pi_free(impl->scene_depth_name);
	pi_free(impl->target_name);

	pi_free(renderer->impl);
	pi_renderer_destroy(renderer);
}

void PI_API pi_post_processing_set_fog_color(PiRenderer *renderer, float r, float g, float b)
{
	PostProcessingRenderer *impl;

	_type_check(renderer);

	impl = (PostProcessingRenderer *)renderer->impl;
	impl->params.fog_color.x = r;
	impl->params.fog_color.y = g;
	impl->params.fog_color.z = b;
}

void PI_API pi_post_processing_set_fog_mode(PiRenderer *renderer, FogMode mode)
{
	PostProcessingRenderer *impl;

	_type_check(renderer);

	impl = (PostProcessingRenderer *)renderer->impl;

	impl->fog_switchs.fog_type = (float)mode;
	switch (mode)
	{
	case FM_LINEAR:
		impl->params.fog_density = 1.0f / (impl->params.fog_end - impl->params.fog_start);
		break;
	case FM_EXP:
		impl->params.fog_density = impl->fog_density;
		break;
	case FM_EXP2:
		impl->params.fog_density = impl->fog_density * impl->fog_density;
		break;
	default:
		break;
	}

}

void PI_API pi_post_processing_set_fog_start_end(PiRenderer *renderer, float start, float end)
{
	PostProcessingRenderer *impl;

	_type_check(renderer);

	impl = (PostProcessingRenderer *)renderer->impl;
	impl->params.fog_start = start;
	impl->params.fog_end = end;
	if (impl->fog_switchs.fog_type == FM_LINEAR)
	{
		impl->params.fog_density = 1.0f / (end - start);
	}
}

void PI_API pi_post_processing_set_fog_density(PiRenderer *renderer, float density)
{
	PostProcessingRenderer *impl;

	_type_check(renderer);

	impl = (PostProcessingRenderer *)renderer->impl;
	impl->fog_density = density;
	if (impl->fog_switchs.fog_type == FM_EXP)
	{
		impl->params.fog_density = density;
	}
	else if (impl->fog_switchs.fog_type == FM_EXP2)
	{
		impl->params.fog_density = density * density;
	}

}
void PI_API pi_post_processing_set_fog_range_enable(PiRenderer *renderer, PiBool enable)
{
	PostProcessingRenderer *impl;

	_type_check(renderer);

	impl = (PostProcessingRenderer *)renderer->impl;
	impl->fog_switchs.range_enable = (float)enable;
}

void PI_API pi_post_processing_set_fog_flow(PiRenderer *renderer, PiBool flow_enable, float flow_speed, float scale_ui)
{
	PostProcessingRenderer *impl;

	_type_check(renderer);

	impl = (PostProcessingRenderer *)renderer->impl;
	impl->fog_switchs.flow_enable = (float)flow_enable;
	impl->params.flow_speed = flow_speed;
	impl->params.scale_ui = scale_ui;
}

void PI_API pi_post_processing_set_fxaa_enable(PiRenderer* renderer, PiBool enable)
{
	PostProcessingRenderer *impl;

	_type_check(renderer);

	impl = (PostProcessingRenderer *)renderer->impl;

	impl->switchs.fxaa_enable = (float)enable;
}

void PI_API pi_post_processing_set_color_grading_enable(PiRenderer* renderer, PiBool enable)
{
	PostProcessingRenderer *impl;

	_type_check(renderer);

	impl = (PostProcessingRenderer *)renderer->impl;

	impl->color_grading_enable = enable;

	if (renderer->is_init)
	{
		pi_material_set_def(impl->material, impl->COLOR_GRADING, enable && impl->clut_texture != NULL);
	}
}


void PI_API pi_post_processing_set_color_grading_clut(PiRenderer* renderer, PiTexture* texture)
{
	PostProcessingRenderer *impl;

	_type_check(renderer);

	impl = (PostProcessingRenderer *)renderer->impl;

	impl->clut_texture = texture;

	pi_sampler_set_texture(&impl->clut_sampler, texture);

	if (renderer->is_init)
	{
		pi_material_set_def(impl->material, impl->COLOR_GRADING, impl->color_grading_enable && impl->clut_texture != NULL);
	}
}


void PI_API pi_post_processing_set_bloom_enable(PiRenderer* renderer, PiBool enable)
{
	PostProcessingRenderer *impl;

	_type_check(renderer);

	impl = (PostProcessingRenderer *)renderer->impl;

	impl->bloom_enable = enable;

	if (impl->bloom_enable)
	{
		impl->switchs.bloom_enable = 1.0;
	}
	else
	{
		impl->switchs.bloom_enable = 0.0;
	}
}
void PI_API pi_post_processing_set_bloom_params(PiRenderer* renderer, float threshold, float scale)
{
	PostProcessingRenderer *impl;

	_type_check(renderer);

	impl = (PostProcessingRenderer *)renderer->impl;

	impl->params.bloom_threshold = threshold;
	impl->params.bloom_scale = scale;
}


