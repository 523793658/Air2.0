#include "pi_lib.h"
#include "renderer.h"
#include "pi_rendermesh.h"
#include "material.h"
#include "entity.h"
#include "rendertarget.h"
#include "rendersystem.h"
#include "gaussian_blur.h"

const static char *RS_GAUSSIAN_BLUR_VS = "default.vs";
const static char *RS_GAUSSIAN_BLUR_FS = "blur.fs";

typedef struct _GaussianBlurRenderer_
{
	char* input_name;
	char* output_name;
	PiBool is_deploy;
	PiMesh* mesh;
	PiRenderMesh* rmesh;
	PiMaterial* material;
	PiEntity* entity;
	SamplerState ss_src;

	PiCamera* camera;

	char* U_SrcTex;
	char* U_Size;
}GaussianBlurRenderer;


static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	GaussianBlurRenderer* impl = renderer->impl;
	PiColor color = { 1.0f, 1.0f, 1.0f, 0.0f };
	if (!impl->is_deploy)
	{
		return FALSE;
	}
	impl->mesh = pi_mesh_create_quad(NULL, NULL, 0.5);
	impl->rmesh = pi_rendermesh_new(impl->mesh, TRUE);
	impl->material = pi_material_new(RS_GAUSSIAN_BLUR_VS, RS_GAUSSIAN_BLUR_FS);
	impl->entity = pi_entity_new();
	pi_entity_set_material(impl->entity, impl->material);
	pi_renderstate_set_default_sampler(&impl->ss_src);
	pi_sampler_set_filter(&impl->ss_src, TFO_CMP_MIN_MAG_MIP_POINT);
	pi_sampler_set_addr_mode(&impl->ss_src, TAM_BORDER, TAM_BORDER, TAM_BORDER);

	pi_sampler_set_border_color(&impl->ss_src, &color);
	pi_material_set_depth_enable(impl->material, FALSE);
	pi_material_set_blend(impl->material, FALSE);
	impl->mesh = pi_mesh_create_quad(NULL, NULL, 0.5);
	impl->rmesh = pi_rendermesh_new(impl->mesh, TRUE);
	pi_entity_set_mesh(impl->entity, impl->rmesh);

	impl->camera = pi_camera_new();
	pi_camera_set_location(impl->camera, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->camera, 0.0f, 0.0f, -1.0f);

	impl->U_SrcTex = pi_conststr("u_SrcTex");
	impl->U_Size = pi_conststr("u_Size");
	return TRUE;
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	GaussianBlurRenderer* impl = renderer->impl;
	PiRenderTarget *target;
	pi_hash_lookup(resources, impl->output_name, (void**)&target);
	pi_rendersystem_set_target(target);
	pi_rendersystem_set_camera(impl->camera);
	pi_entity_draw(impl->entity);
}

static void _update(PiRenderer *renderer, float tpf, PiHash *resources)
{
	uint32 width, height;
	PiRenderTarget* input_target;
	PiTexture* texture;
	PiRenderTarget* target;
	GaussianBlurRenderer* impl = renderer->impl;
	float size[2];
	pi_hash_lookup(resources, impl->input_name, (void**)&input_target);
	texture = input_target->views[ATT_COLOR0]->data.tex_2d.tex;

	width = texture->width;
	height = texture->height;

	size[0] = 1.0f / width;
	size[1] = 1.0f / height;
	pi_material_set_uniform(impl->material, impl->U_Size, UT_VEC2, 1, size, TRUE);

	pi_sampler_set_texture(&impl->ss_src, texture);
	pi_material_set_uniform(impl->material, impl->U_SrcTex, UT_SAMPLER_2D, 1, &impl->ss_src, FALSE);

	pi_hash_lookup(resources, impl->output_name, (void*)&target);
	width = target->width;
	height = target->height;

	pi_spatial_set_local_scaling(impl->entity->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->entity->spatial);

	pi_camera_set_frustum(impl->camera, -(float)width / 2.0f + 0.5f, width / 2.0f + 0.5f, -(float)height / 2.0f - 0.5f, height / 2.0f - 0.5f, 0.0f, 2.0f, TRUE);
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{

}

PiRenderer* PI_API pi_gaussian_blur_renderer_new()
{
	GaussianBlurRenderer* renderer = pi_new0(GaussianBlurRenderer, 1);

	return pi_renderer_create(ERT_GAUSSIAN_BLUR, "gaussian_blur", _init, _resize, _update, _draw, renderer);
}

void PI_API pi_gaussian_blur_renderer_deploy(PiRenderer* renderer, char* input_name, char* output_name)
{
	GaussianBlurRenderer* impl = (GaussianBlurRenderer*)renderer->impl;
	impl->input_name = pi_str_dup(input_name);
	impl->output_name = pi_str_dup(output_name);
	impl->is_deploy = TRUE;
}


void PI_API pi_gaussian_blur_renderer_free(PiRenderer* renderer)
{
	GaussianBlurRenderer* impl = renderer->impl;
	pi_camera_free(impl->camera);
	pi_rendermesh_free(impl->rmesh);
	pi_mesh_free(impl->mesh);
	pi_entity_free(impl->entity);
	pi_material_free(impl->material);
	pi_free(impl);
	pi_renderer_destroy(renderer);
}