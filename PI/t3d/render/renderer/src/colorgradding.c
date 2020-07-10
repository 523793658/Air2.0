#include <colorgrading.h>
#include <entity.h>
#include <rendersystem.h>
#include <pi_spatial.h>

/**
 * color grading
 */
typedef struct
{
	PiEntity *entity;

	PiCamera *camera;

	char *input_name;
	char *output_name;

	uint lut_size;
	SamplerState ss_lut;
	SamplerState ss_src;

	PiMesh *mesh;
	PiRenderMesh *rmesh;

	PiMaterial *material;

	PiBool is_deploy;

	/* ³£Á¿×Ö·û´® */
	char *U_LutTex;
	char *U_Scale;
	char *U_Offset;
	char *U_SrcTex;
} ColorGrading;

static void _type_check(PiRenderer *renderer)
{
	PI_ASSERT(renderer->type == ERT_COLOR_GRADING, "Renderer type error!");
}

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	PiRenderTarget *target;
	float w, h;
	float scale, offset;
	ColorGrading *impl = (ColorGrading *)(renderer->impl);

	impl->U_LutTex = pi_conststr("u_LutTex");
	impl->U_Scale = pi_conststr("u_Scale");
	impl->U_Offset = pi_conststr("u_Offset");
	impl->U_SrcTex = pi_conststr("u_SrcTex");

	if (!impl->is_deploy)
	{
		return FALSE;
	}

	pi_hash_lookup(resources, impl->output_name, (void **)&target);
	w = (float)target->width;
	h = (float)target->height;

	impl->mesh = pi_mesh_create_quad(NULL, NULL, 0.0f);
	impl->material = pi_material_new(RS_COLOR_GRADING_VS, RS_COLOR_GRADING_FS);

	pi_material_set_uniform(impl->material, impl->U_LutTex, UT_SAMPLER_3D, 1, &impl->ss_lut, TRUE);

	scale = ( impl->lut_size - 1.0f ) / impl->lut_size;
	offset = 1.0f / (2.0f * impl->lut_size );
	pi_material_set_uniform(impl->material, impl->U_Scale, UT_FLOAT, 1, &scale, TRUE);
	pi_material_set_uniform(impl->material, impl->U_Offset, UT_FLOAT, 1, &offset, TRUE);

	_type_check(renderer);

	impl->entity = pi_entity_new();
	impl->rmesh = pi_rendermesh_new(impl->mesh, TRUE);

	pi_entity_set_mesh(impl->entity, impl->rmesh);
	pi_entity_set_material(impl->entity, impl->material);

	pi_spatial_set_local_scaling(impl->entity->spatial, (float)w, (float)h, 1.0f);
	pi_spatial_update(impl->entity->spatial);

	impl->camera = pi_camera_new();
	pi_camera_set_location(impl->camera, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->camera, 0.0f, 0.0f, -1.0f);
	pi_camera_set_frustum(impl->camera, -w / 2.0f, w / 2.0f, -h / 2.0f, h / 2.0f, 0.0f, 2.0f, TRUE);
	return TRUE;
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	ColorGrading *impl = (ColorGrading *)(renderer->impl);
	PiRenderTarget *target;

	PI_USE_PARAM(tpf);
	_type_check(renderer);

	pi_hash_lookup(resources, impl->output_name, (void **)&target);
	pi_rendersystem_set_target(target);

	pi_rendersystem_set_camera(impl->camera);

	pi_entity_draw(impl->entity);
}

static void _update(PiRenderer *renderer, float tpf, PiHash *resources)
{
	ColorGrading *impl;
	PiTexture *src_tex;

	_type_check(renderer);
	PI_USE_PARAM(tpf);

	impl = (ColorGrading *)(renderer->impl);
	pi_hash_lookup(resources, impl->input_name, (void **)&src_tex);
	pi_sampler_set_texture(&impl->ss_src, src_tex);
	pi_material_set_uniform(impl->material, impl->U_SrcTex, UT_SAMPLER_2D, 1, &impl->ss_src, TRUE);
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	ColorGrading *impl = (ColorGrading *)(renderer->impl);

	pi_camera_set_frustum(impl->camera, -(float)width / 2.0f, (float)width / 2.0f, -(float)height / 2.0f, (float)height / 2.0f, 0.0f, 2.0f, TRUE);

	pi_spatial_set_local_scaling(impl->entity->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->entity->spatial);
}

PiRenderer *PI_API pi_colorgrading_new()
{
	PiRenderer *renderer;
	ColorGrading *impl = pi_new0(ColorGrading, 1);

	renderer = pi_renderer_create(ERT_COLOR_GRADING, "color grading", _init, _resize, _update, _draw, impl);

	return renderer;
}

void PI_API pi_colorgrading_deploy(PiRenderer *renderer, char *input_name, char *output_name)
{
	ColorGrading *impl;
	_type_check(renderer);
	impl = (ColorGrading *)(((PiRenderer *)renderer)->impl);

	pi_free(impl->input_name);
	pi_free(impl->output_name);

	impl->input_name = pi_str_dup(input_name);
	impl->output_name = pi_str_dup(output_name);

	impl->is_deploy = TRUE;
}

void PI_API pi_colorgrading_free(PiRenderer *renderer)
{
	ColorGrading *impl = (ColorGrading *)(((PiRenderer *)renderer)->impl);
	_type_check(renderer);

	pi_mesh_free(impl->mesh);
	pi_rendermesh_free(impl->rmesh);
	pi_material_free(impl->material);

	pi_camera_free(impl->camera);

	pi_free(renderer->impl);
	pi_renderer_destroy(renderer);
}

void PI_API pi_colorgrading_set_lut(PiRenderer *renderer, PiTexture *lut)
{
	ColorGrading *impl = (ColorGrading *)(((PiRenderer *)renderer)->impl);
	_type_check(renderer);

	impl->lut_size = lut ? lut->width : 0;
	pi_renderstate_set_default_sampler(&impl->ss_lut);
	pi_sampler_set_texture(&impl->ss_lut, lut);
	pi_sampler_set_filter(&impl->ss_lut, TFO_MIN_MAG_LINEAR);
	pi_sampler_set_addr_mode(&impl->ss_lut, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);

	pi_renderstate_set_default_sampler(&impl->ss_src);
}
