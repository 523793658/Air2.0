#include "fxaa.h"

#include <pi_spatial.h>
#include <entity.h>
#include <rendertarget.h>
#include <rendersystem.h>

/**
 * FXAAäÖÈ¾Æ÷
 */
typedef struct
{
	PiMesh *mesh;
	PiRenderMesh *rmesh;
	PiEntity *entity;
	PiCamera *camera;

	char* input_name;
	char* output_name;
	SamplerState ss_src;

	PiMaterial *material;
	PiBool is_deploy;

	uint preset;

	/* ³£Á¿×Ö·û´® */
	char *FXAA_PRESET_1;

	char *U_RenderTargetSize;
	char *U_SrcSize;
	char *U_SrcTex;
} FXAARenderer;

static void _type_check(PiRenderer* renderer)
{
	PI_ASSERT((renderer)->type == ERT_FXAA, "Renderer type error!");
}

static PiBool _init(PiRenderer* renderer, PiHash *resources)
{
	PiRenderTarget *target;
	uint32 width, height;
	FXAARenderer *impl = (FXAARenderer*)renderer->impl;
	float size[2];

	if(!impl->is_deploy)
	{
		return FALSE;
	}
	
	impl->FXAA_PRESET_1 = pi_conststr("FXAA_PRESET_1");

	impl->U_SrcSize = pi_conststr("u_SrcSize");
	impl->U_SrcTex = pi_conststr("u_SrcTex");
	impl->U_RenderTargetSize = pi_conststr("u_RenderTargetSize");

	pi_hash_lookup(resources, impl->output_name, (void**)&target);
	width = target->width;
	height = target->height;

	impl->mesh = pi_mesh_create_quad(NULL, NULL, 0.0f);
	impl->rmesh = pi_rendermesh_new(impl->mesh, TRUE);

	impl->material = pi_material_new(RS_FXAA_VS, RS_FXAA_FS);
	impl->entity = pi_entity_new();

	pi_renderstate_set_default_sampler(&impl->ss_src);
	pi_sampler_set_filter(&impl->ss_src, TFO_MIN_MAG_LINEAR);
	pi_sampler_set_addr_mode(&impl->ss_src, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	size[0] = (float)width;
	size[1] = (float)height;
	pi_material_set_uniform(impl->material, impl->U_SrcSize, UT_VEC2, 1, size, TRUE);
	pi_material_set_depth_enable(impl->material, FALSE);
	pi_material_set_depthwrite_enable(impl->material, FALSE);
	if(impl->preset == 1)
	{
		pi_material_set_def(impl->material, impl->FXAA_PRESET_1, TRUE);
	}

	pi_entity_set_mesh(impl->entity, impl->rmesh);
	pi_entity_set_material(impl->entity, impl->material);

	pi_spatial_set_local_scaling(impl->entity->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->entity->spatial);
	
	impl->camera = pi_camera_new();

	pi_camera_set_location(impl->camera, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->camera, 0.0f, 0.0f, -1.0f);
	pi_camera_set_frustum(impl->camera, -(float)width / 2.0f + 0.5f, (float)width / 2.0f + 0.5f, -(float)height / 2.0f - 0.5f, (float)height / 2.0f - 0.5f, 0.0f, 2.0f, TRUE);

	return TRUE;
}

static void _draw(PiRenderer* renderer, float tpf, PiHash* resources)
{
	FXAARenderer* impl = (FXAARenderer*)renderer->impl;
	PiRenderTarget* target;
	PI_USE_PARAM(tpf);
	_type_check(renderer);
	pi_hash_lookup(resources, impl->output_name, (void**)&target);
	pi_rendersystem_set_target(target);

	pi_rendersystem_set_camera(impl->camera);

	pi_entity_draw(impl->entity);
}

static void _update(PiRenderer* renderer, float tpf, PiHash* resources)
{
	FXAARenderer* impl;
	PiTexture* src_tex;
	PI_USE_PARAM(tpf);
	impl = (FXAARenderer*)renderer->impl;
	pi_hash_lookup(resources, impl->input_name, (void**)&src_tex);
	pi_sampler_set_texture(&impl->ss_src, src_tex);
	pi_material_set_uniform(impl->material, impl->U_SrcTex, UT_SAMPLER_2D, 1, &impl->ss_src, TRUE);
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	FXAARenderer* impl = (FXAARenderer*)renderer->impl;
	float size[2];
	pi_camera_set_frustum(impl->camera, -(float)width / 2.0f + 0.5f, (float)width / 2.0f + 0.5f, -(float)height / 2.0f - 0.5f, (float)height / 2.0f - 0.5f, 0.0f, 2.0f, TRUE);
	
	pi_spatial_set_local_scaling(impl->entity->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->entity->spatial);

	size[0] = (float)width;
	size[1] = (float)height;
	pi_material_set_uniform(impl->material, impl->U_SrcSize, UT_VEC2, 1, size, TRUE);
}


PiRenderer* PI_API pi_fxaa_new_with_name(char* name)
{
	PiRenderer* renderer;
	FXAARenderer* impl = pi_new0(FXAARenderer, 1);
	impl->preset = 2;

	renderer = pi_renderer_create(ERT_FXAA, name, _init, _resize, _update, _draw, impl);
	return renderer;
}

PiRenderer* PI_API pi_fxaa_new()
{
	return pi_fxaa_new_with_name("fxaa");
}

void PI_API pi_fxaa_deploy(PiRenderer* renderer, char* input_name, char* output_name)
{
	FXAARenderer* impl;
	_type_check(renderer);
	impl = (FXAARenderer*)renderer->impl;

	pi_free(impl->input_name);
	pi_free(impl->output_name);

	impl->input_name = pi_str_dup(input_name);
	impl->output_name = pi_str_dup(output_name);

	impl->is_deploy = TRUE;
}

void PI_API pi_fxaa_free(PiRenderer* renderer)
{
	FXAARenderer* impl;
	_type_check(renderer);
	impl = (FXAARenderer*)renderer->impl;

	pi_mesh_free(impl->mesh);
	pi_rendermesh_free(impl->rmesh);
	pi_entity_free(impl->entity);
	pi_camera_free(impl->camera);
	pi_material_free(impl->material);
	pi_free(impl->input_name);
	pi_free(impl->output_name);

	pi_free(renderer->impl);
	pi_renderer_destroy(renderer);
}

void PI_API pi_fxaa_set_preset(PiRenderer* renderer, uint n)
{
	FXAARenderer* impl;
	_type_check(renderer);
	impl = (FXAARenderer*)renderer->impl;

	impl->preset = n;
	if (impl->material)
	{
		if (n == 1)
		{
			pi_material_set_def(impl->material, impl->FXAA_PRESET_1, TRUE);
		}
		else
		{
			pi_material_set_def(impl->material, impl->FXAA_PRESET_1, FALSE);
		}
	}
}