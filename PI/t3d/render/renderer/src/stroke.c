#include "stroke.h"
#include "entity.h"
#include "camera.h"
#include "material.h"
#include "rendertarget.h"
#include "rendersystem.h"
typedef struct  
{
	PiMesh *mesh;
	PiRenderMesh *rmesh;
	PiEntity * entity;
	PiCamera *camera;

	char* input_name;
	char* output_name;
	SamplerState ss_src;

	SamplerState ss_alpha;
	PiTexture* alpha_map;

	int width;

	PiMaterial *material;
	PiBool is_deploy;

	char* U_SrcSize;
	char* U_SrcTex;
	char* U_StrokeColor;
	char* U_Width;
	char* ALPHA_MAP;
	char* U_AlphaMap;
	PiVector3 color;
}PiStrokeRenderer;

static void _type_check(PiRenderer *renderer)
{
	PI_ASSERT((renderer)->type == ERT_STROKE, "Renderer type error");
}

static PiBool _init(PiRenderer* renderer, PiHash *resources)
{
	PiRenderTarget *target;
	uint32 width, height;
	PiStrokeRenderer *impl = (PiStrokeRenderer*)renderer->impl;
	float size[2];

	if (!impl->is_deploy)
	{
		return FALSE;
	}

	pi_hash_lookup(resources, impl->output_name, (void**)&target);
	width = target->width;
	height = target->height;

	impl->mesh = pi_mesh_create_quad(NULL, NULL, 0.0f);
	impl->rmesh = pi_rendermesh_new(impl->mesh, TRUE);

	impl->material = pi_material_new(RS_STROKE_VS, RS_STROKE_FS);
	impl->entity = pi_entity_new();

	pi_material_set_uniform(impl->material, impl->U_Width, UT_INT, 1, &impl->width, TRUE);
	pi_material_set_uniform(impl->material, impl->U_StrokeColor, UT_VEC3, 1, &impl->color, FALSE);

	if (impl->alpha_map)
	{
		pi_material_set_def(impl->material, impl->ALPHA_MAP, TRUE);
		pi_material_set_uniform(impl->material, impl->U_AlphaMap, UT_SAMPLER_2D, 1, &impl->ss_alpha, FALSE);
	}

	pi_renderstate_set_default_sampler(&impl->ss_src);
	pi_sampler_set_filter(&impl->ss_src, TFO_MIN_MAG_LINEAR);
	pi_sampler_set_addr_mode(&impl->ss_src, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	size[0] = (float)width;
	size[1] = (float)height;
	pi_material_set_uniform(impl->material, impl->U_SrcSize, UT_VEC2, 1, size, TRUE);
	pi_material_set_depth_enable(impl->material, FALSE);
	pi_material_set_depthwrite_enable(impl->material, FALSE);

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
	PiStrokeRenderer* impl = (PiStrokeRenderer*)renderer->impl;
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
	PiStrokeRenderer* impl;
	PiTexture* src_tex;
	PI_USE_PARAM(tpf);
	impl = (PiStrokeRenderer*)renderer->impl;
	pi_hash_lookup(resources, impl->input_name, (void**)&src_tex);
	pi_sampler_set_texture(&impl->ss_src, src_tex);
	pi_material_set_uniform(impl->material, impl->U_SrcTex, UT_SAMPLER_2D, 1, &impl->ss_src, TRUE);
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	PiStrokeRenderer* impl = (PiStrokeRenderer*)renderer->impl;
	float size[2];
	pi_camera_set_frustum(impl->camera, -(float)width / 2.0f + 0.5f, (float)width / 2.0f + 0.5f, -(float)height / 2.0f - 0.5f, (float)height / 2.0f - 0.5f, 0.0f, 2.0f, TRUE);

	pi_spatial_set_local_scaling(impl->entity->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->entity->spatial);

	size[0] = (float)width;
	size[1] = (float)height;
	pi_material_set_uniform(impl->material, impl->U_SrcSize, UT_VEC2, 1, size, TRUE);
}


PiRenderer* PI_API pi_stroke_new()
{
	PiRenderer* renderer;
	PiStrokeRenderer* impl = pi_new0(PiStrokeRenderer, 1);
	impl->U_SrcSize = "u_SrcSize";
	impl->U_SrcTex = "u_SrcTex";
	impl->U_StrokeColor = "u_StrokeColor";
	impl->U_Width = "u_Width";
	impl->ALPHA_MAP = "ALPHA_MAP";
	impl->U_AlphaMap = "u_AlphaMap";

	impl->width = 1;
	pi_vec3_set(&impl->color, 1.0, 0.0, 0.0);
	renderer = pi_renderer_create(ERT_STROKE, "stroke", _init, _resize, _update, _draw, impl);
	return renderer;
}

void PI_API pi_stroke_deploy(PiRenderer* renderer, char* input_name, char* output_name)
{
	PiStrokeRenderer* impl;
	_type_check(renderer);

	impl = (PiStrokeRenderer*)renderer->impl;
	if (impl->input_name != NULL)
	{
		pi_free(impl->input_name);
	}
	if (impl->output_name != NULL)
	{
		pi_free(impl->output_name);
	}
	impl->input_name = pi_str_dup(input_name);
	impl->output_name = pi_str_dup(output_name);

	impl->is_deploy = TRUE;
}

void PI_API pi_stroke_free(PiRenderer* renderer)
{
	PiStrokeRenderer* impl;
	_type_check(renderer);
	impl = (PiStrokeRenderer*)renderer->impl;

	pi_mesh_free(impl->mesh);
	pi_rendermesh_free(impl->rmesh);
	pi_entity_free(impl->entity);
	pi_camera_free(impl->camera);

	pi_material_free(impl->material);
	pi_free(impl);
	pi_renderer_destroy(renderer);
}

void PI_API pi_stroke_set_params(PiRenderer* renderer, uint width, float r, float g, float b)
{
	PiStrokeRenderer* impl;
	_type_check(renderer);
	impl = (PiStrokeRenderer*)renderer->impl;

	impl->width = width;
	pi_vec3_set(&impl->color, r, g, b);
}

void PI_API pi_stroke_set_alpha_map(PiRenderer* renderer, PiTexture* texture)
{
	PiStrokeRenderer* impl;
	_type_check(renderer);
	impl = (PiStrokeRenderer*)renderer->impl;
	impl->alpha_map = texture;
	pi_sampler_set_texture(&impl->ss_alpha, texture);
}