#include "inverse_renderer.h"
#include <entity.h>
#include <rendertarget.h>
#include <rendersystem.h>
#include <renderwrap.h>

/**
* FBO¿½±´äÖÈ¾Æ÷
*/
typedef struct
{
	PiMesh *mesh;
	PiRenderMesh *rmesh;
	PiMaterial *material;
	PiEntity *entity;

	PiCamera *camera;

	char *src_name;
	char *dst_name;

	SamplerState ss_src;
	SamplerState ss_depth_src;
	TexFilterOp filter;

	PiBool is_deploy;

	PiTexture *src_tex;
	PiTexture *src_depth;
	PiRenderTarget* src_rt;

	/* ³£Á¿×Ö·û´® */
	char *U_ColorTex;
	char *U_DepthTex;
	char *DEPTH;
} FBOCopyRenderer;

static void _type_check(PiRenderer *renderer)
{
	PI_ASSERT((renderer)->type == ERT_INVERSE, "Renderer type error!");
}

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	PiRenderTarget *target;
	uint32 width, height;
	FBOCopyRenderer *impl = (FBOCopyRenderer *)renderer->impl;

	impl->U_ColorTex = pi_conststr("u_ColorTex");
	impl->U_DepthTex = pi_conststr("u_DepthTex");
	impl->DEPTH = pi_conststr("DEPTH");

	if (!impl->is_deploy)
	{
		return FALSE;
	}

	pi_hash_lookup(resources, impl->dst_name, (void **)&target);
	width = target->width;
	height = target->height;

	impl->mesh = pi_mesh_create_quad(NULL, NULL, 0.0f);
	impl->rmesh = pi_rendermesh_new(impl->mesh, TRUE);

	impl->material = pi_material_new(RS_INVERSE_VS, RS_INVERSE_PS);
	impl->entity = pi_entity_new();

	pi_renderstate_set_default_sampler(&impl->ss_src);
	pi_sampler_set_addr_mode(&impl->ss_src, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_material_set_blend(impl->material, TRUE);
	pi_material_set_blend_factor(impl->material, BF_DST_ALPHA, BF_ONE, BF_DST_ALPHA, BF_ZERO);
	pi_entity_set_mesh(impl->entity, impl->rmesh);
	pi_entity_set_material(impl->entity, impl->material);
	pi_material_set_depth_enable(impl->material, FALSE);

	pi_spatial_set_local_scaling(impl->entity->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->entity->spatial);

	impl->camera = pi_camera_new();

	pi_camera_set_location(impl->camera, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->camera, 0.0f, 0.0f, -1.0f);
	pi_camera_set_frustum(impl->camera, -(float)width / 2.0f + 0.5f, width / 2.0f + 0.5f, -(float)height / 2.0f - 0.5f, (float)height / 2.0f - 0.5f, 0.0f, 2.0f, TRUE);

	return TRUE;
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	FBOCopyRenderer *impl = (FBOCopyRenderer *)renderer->impl;
	PiRenderTarget *target;
	PI_USE_PARAM(tpf);
	_type_check(renderer);
	pi_hash_lookup(resources, impl->dst_name, (void **)&target);

	pi_rendersystem_set_target(target);

	pi_rendersystem_set_camera(impl->camera);
	pi_entity_draw(impl->entity);
}

static void _update(PiRenderer *renderer, float tpf, PiHash *resources)
{
	FBOCopyRenderer* impl;
	PiRenderTarget* src_rt;
	PiTexture* color_tex;
	PI_USE_PARAM(tpf);
	impl = (FBOCopyRenderer *)renderer->impl;

	pi_hash_lookup(resources, impl->src_name, (void**)&src_rt);
	color_tex = src_rt->views[ATT_COLOR0]->data.tex_2d.tex;
	impl->src_rt = src_rt;

	impl->src_tex = color_tex;
	if (src_rt->views[ATT_COLOR1] != NULL)
	{
		impl->src_depth = src_rt->views[ATT_COLOR1]->data.tex_2d.tex;
		pi_material_set_def(impl->material, impl->DEPTH, TRUE);
		pi_sampler_set_filter(&impl->ss_depth_src, impl->filter);
		pi_sampler_set_texture(&impl->ss_depth_src, impl->src_depth);
		pi_material_set_uniform(impl->material, impl->U_DepthTex, UT_SAMPLER_2D, 1, &impl->ss_depth_src, TRUE);
		pi_material_set_depth_enable(impl->material, TRUE);
		pi_material_set_depthwrite_enable(impl->material, TRUE);
	}
	else
	{
		pi_material_set_def(impl->material, impl->DEPTH, FALSE);
		pi_material_set_depth_enable(impl->material, FALSE);
	}

	pi_sampler_set_filter(&impl->ss_src, impl->filter);
	pi_sampler_set_texture(&impl->ss_src, color_tex);
	pi_material_set_uniform(impl->material, impl->U_ColorTex, UT_SAMPLER_2D, 1, &impl->ss_src, TRUE);
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	FBOCopyRenderer *impl;
	_type_check(renderer);
	impl = (FBOCopyRenderer *)renderer->impl;

	pi_spatial_set_local_scaling(impl->entity->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->entity->spatial);

	pi_camera_set_frustum(impl->camera, -(float)width / 2.0f + 0.5f, (float)width / 2.0f + 0.5f, -(float)height / 2.0f - 0.5f, (float)height / 2.0f - 0.5f, 0.0f, 2.0f, TRUE);
}


PiRenderer *PI_API pi_inverse_new()
{
	PiRenderer *renderer;
	FBOCopyRenderer *impl = pi_new0(FBOCopyRenderer, 1);

	impl->filter = TFO_MIN_MAG_POINT;

	renderer = pi_renderer_create(ERT_INVERSE, "fbo_copy", _init, _resize, _update, _draw, impl);
	return renderer;
}

void PI_API pi_inverse_deploy(PiRenderer *renderer, char *src_name, char *dst_name)
{
	FBOCopyRenderer *impl;
	_type_check(renderer);
	impl = (FBOCopyRenderer *)renderer->impl;

	pi_free(impl->src_name);
	pi_free(impl->dst_name);

	impl->src_name = pi_str_dup(src_name);
	impl->dst_name = pi_str_dup(dst_name);

	impl->is_deploy = TRUE;
}

void PI_API pi_inverse_free(PiRenderer *renderer)
{
	FBOCopyRenderer *impl;
	_type_check(renderer);
	impl = (FBOCopyRenderer *)renderer->impl;

	pi_entity_free(impl->entity);
	pi_rendermesh_free(impl->rmesh);
	pi_mesh_free(impl->mesh);
	pi_material_free(impl->material);

	pi_camera_free(impl->camera);

	pi_free(impl->src_name);
	pi_free(impl->dst_name);

	pi_free(renderer->impl);
	pi_renderer_destroy(renderer);
}