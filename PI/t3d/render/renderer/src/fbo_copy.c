#include "fbo_copy.h"
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
	FBOCopyBlendMode blend;
	TexFilterOp filter;

	SamplerState ss_depthSrc;

	PiBool is_deploy;
	
	PiTexture *src_tex;
	PiRenderTarget* src_rt;

	SamplerState ss_alpha;
	PiTexture* alpha_map;

	PiBool is_lightmap;
	
	/* ³£Á¿×Ö·û´® */
	char *U_ColorTex;
	char *U_DepthTex;
	char *COPY_DEPTH;
	char *ALPHA_MAP;
	char *U_AlphaMap;
	char *LIGHTMAP;
} FBOCopyRenderer;

static void _type_check(PiRenderer *renderer)
{
	PI_ASSERT((renderer)->type == ERT_FBO_COPY, "Renderer type error!");
}

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	PiRenderTarget *target;
	uint32 width, height;
	FBOCopyRenderer *impl = (FBOCopyRenderer *)renderer->impl;

	impl->U_ColorTex = pi_conststr("u_ColorTex");
	impl->U_DepthTex = pi_conststr("u_DepthTex");
	impl->COPY_DEPTH = pi_conststr("COPY_DEPTH");
	impl->ALPHA_MAP = pi_conststr("ALPHA_MAP");
	impl->U_AlphaMap = pi_conststr("u_AlphaMap");
	impl->LIGHTMAP = pi_conststr("LIGHTMAP");
	if (!impl->is_deploy)
	{
		return FALSE;
	}

	pi_hash_lookup(resources, impl->dst_name, (void **)&target);
	width = target->width;
	height = target->height;

	impl->mesh = pi_mesh_create_quad(NULL, NULL, 0.0f);
	impl->rmesh = pi_rendermesh_new(impl->mesh, TRUE);

	impl->material = pi_material_new(RS_FBO_COPY_VS, RS_FBO_COPY_FS);
	impl->entity = pi_entity_new();

	pi_renderstate_set_default_sampler(&impl->ss_src);
	pi_sampler_set_addr_mode(&impl->ss_src, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	if (impl->blend == FCBM_DEPTH)
	{
		pi_material_set_depth_enable(impl->material, TRUE);
		pi_material_set_depthwrite_enable(impl->material, TRUE);
		pi_renderstate_set_default_sampler(&impl->ss_depthSrc);
		pi_sampler_set_addr_mode(&impl->ss_src, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	}
	else
	{
		pi_material_set_depth_enable(impl->material, FALSE);
		pi_material_set_depthwrite_enable(impl->material, FALSE);
	}
	pi_material_set_def(impl->material, impl->LIGHTMAP, impl->is_lightmap);


	if (impl->alpha_map){
		pi_material_set_def(impl->material, impl->ALPHA_MAP, TRUE);
		pi_renderstate_set_default_sampler(&impl->ss_alpha);
		pi_sampler_set_filter(&impl->ss_alpha, TFO_CMP_MIN_MAG_LINEAR_MIP_POINT);
		pi_sampler_set_texture(&impl->ss_alpha, impl->alpha_map);
		pi_material_set_uniform(impl->material, impl->U_AlphaMap, UT_SAMPLER_2D, 1, &impl->ss_alpha, FALSE);
	}
	else{
		pi_material_set_def(impl->material, impl->ALPHA_MAP, FALSE);
	}

	pi_entity_set_mesh(impl->entity, impl->rmesh);
	pi_entity_set_material(impl->entity, impl->material);

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
	PiTexture* depth_tex;
	PI_USE_PARAM(tpf);
	impl = (FBOCopyRenderer *)renderer->impl;

	pi_hash_lookup(resources, impl->src_name, (void**)&src_rt);
	color_tex = src_rt->views[ATT_COLOR0]->data.tex_2d.tex;
	impl->src_rt = src_rt;
	
	impl->src_tex = color_tex;
	
	pi_sampler_set_filter(&impl->ss_src, impl->filter);
	pi_sampler_set_texture(&impl->ss_src, color_tex);
	pi_material_set_uniform(impl->material, impl->U_ColorTex, UT_SAMPLER_2D, 1, &impl->ss_src, TRUE);

	if (impl->blend == FCBM_DEPTH)
	{
		depth_tex = src_rt->views[ATT_COLOR1]->data.tex_2d.tex;
		pi_material_set_def(impl->material, impl->COPY_DEPTH, TRUE);
		pi_sampler_set_filter(&impl->ss_src, impl->filter);
		pi_sampler_set_texture(&impl->ss_depthSrc, depth_tex);
		pi_material_set_uniform(impl->material, impl->U_DepthTex, UT_SAMPLER_2D, 1, &impl->ss_depthSrc, TRUE);
	}
	switch (impl->blend)
	{
	case FCBM_NONE:
		pi_material_set_blend(impl->material, FALSE);
		break;
	case FTBM_COLOR_MULT:
		pi_material_set_blend(impl->material, TRUE);
		pi_material_set_blend_factor(impl->material,  BF_ZERO, BF_SRC_COLOR, BF_ZERO, BF_ONE);
		break;
	case FCBM_ALPHA:
		pi_material_set_blend(impl->material, TRUE);
		pi_material_set_blend_factor(impl->material,  BF_SRC_ALPHA, BF_INV_SRC_ALPHA, BF_ZERO, BF_ONE);
		break;
	case FCBM_ALPHA_MULT:
		pi_material_set_blend(impl->material, TRUE);
		pi_material_set_blend_factor(impl->material,  BF_ZERO, BF_SRC_ALPHA, BF_ZERO, BF_ONE);
		break;
	case FCBM_DEPTH:
		pi_material_set_blend(impl->material, TRUE);
		pi_material_set_blend_factor(impl->material, BF_ZERO, BF_ONE, BF_ZERO, BF_ONE);
	default:
		pi_material_set_blend(impl->material, FALSE);
		break;
	}
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


PiRenderer *PI_API pi_fbo_copy_new_with_name(char* name)
{
	PiRenderer *renderer;
	FBOCopyRenderer *impl = pi_new0(FBOCopyRenderer, 1);

	impl->blend = FCBM_NONE;
	impl->filter = TFO_MIN_MAG_POINT;

	renderer = pi_renderer_create(ERT_FBO_COPY, name, _init, _resize, _update, _draw, impl);
	return renderer;
}

PiRenderer *PI_API pi_fbo_copy_new()
{
	return pi_fbo_copy_new_with_name("fbo_copy");
}

void PI_API pi_fbo_copy_light_map(PiRenderer* renderer, PiBool is_lightmap)
{
	FBOCopyRenderer *impl;
	_type_check(renderer);
	impl = (FBOCopyRenderer *)renderer->impl;
	impl->is_lightmap = is_lightmap;
}

void PI_API pi_fbo_copy_deploy(PiRenderer *renderer, char *src_name, char *dst_name)
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

void PI_API pi_fbo_copy_free(PiRenderer *renderer)
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

void PI_API pi_fbo_copy_set_blend_mode(PiRenderer *renderer, FBOCopyBlendMode blend)
{
	FBOCopyRenderer *impl;
	_type_check(renderer);
	impl = (FBOCopyRenderer *)renderer->impl;

	impl->blend = blend;
}

void PI_API pi_fbo_copy_set_filter(PiRenderer *renderer, TexFilterOp filter)
{
	FBOCopyRenderer *impl;
	_type_check(renderer);
	impl = (FBOCopyRenderer *)renderer->impl;

	impl->filter = filter;
}

void PI_API pi_fob_copy_set_alpha_map(PiRenderer *renderer, PiTexture* texture)
{
	FBOCopyRenderer* impl;
	_type_check(renderer);
	impl = (FBOCopyRenderer*)renderer->impl;
	impl->alpha_map = texture;
	pi_sampler_set_texture(&impl->ss_alpha, texture);
}