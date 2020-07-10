#include "radialblur.h"

#include <pi_spatial.h>
#include <entity.h>
#include <rendertarget.h>
#include <rendersystem.h>

/**
 * radial blur 渲染器
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

	uint width;
	uint height;

	PiMaterial *material;

	/* 3个参数，请参见 radialblur.fs 里的对应变量 */
	float progress;
	float sample_dist;
	float sample_strength;

	PiBool is_deploy;

	/* 常量字符串 */
	char *U_Progress;
	char *U_SampleDist;
	char *U_SampleStrength;
	char *U_SrcTex;

} RadialBlurRenderer;

static void _type_check(PiRenderer* renderer)
{
	PI_ASSERT((renderer)->type == ERT_RADIALBLUR, "Renderer type error!");
}

static PiBool _init(PiRenderer* renderer, PiHash* resources)
{
	RadialBlurRenderer *impl = (RadialBlurRenderer*)renderer->impl;

	PI_USE_PARAM(resources);

	if(!impl->is_deploy)
	{
		return FALSE;
	}

	impl->U_Progress = pi_conststr("u_Progress");
	impl->U_SampleDist = pi_conststr("u_SampleDist");
	impl->U_SampleStrength = pi_conststr("u_SampleStrength");
	impl->U_SrcTex = pi_conststr("u_SrcTex");

	impl->mesh = pi_mesh_create_quad(NULL, NULL, 0.0f);
	impl->rmesh = pi_rendermesh_new(impl->mesh, TRUE);

	impl->material = pi_material_new(RS_RADIALBLUR_VS, RS_RADIALBLUR_FS);
	impl->entity = pi_entity_new();

	pi_renderstate_set_default_sampler(&impl->ss_src);
	pi_sampler_set_filter(&impl->ss_src, TFO_MIN_MAG_LINEAR);
	
	pi_entity_set_mesh(impl->entity, impl->rmesh);
	pi_entity_set_material(impl->entity, impl->material);

	pi_spatial_set_local_scaling(impl->entity->spatial, (float)impl->width, (float)impl->height, 1.0f);
	pi_spatial_update(impl->entity->spatial);

	impl->camera = pi_camera_new();

	pi_camera_set_location(impl->camera, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->camera, 0.0f, 0.0f, -1.0f);
	pi_camera_set_frustum(impl->camera, -(float)impl->width / 2.0f, impl->width / 2.0f, -(float)impl->height / 2.0f, (float)impl->height / 2.0f, 0.0f, 2.0f, TRUE);
	return TRUE;
}

static void _draw(PiRenderer* renderer, float tpf, PiHash* resources)
{
	RadialBlurRenderer* impl = (RadialBlurRenderer*)renderer->impl;
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
	/* TEST
	static float s_tTime = 0.0f;
	static int s_iSign = 1;
	*/

	RadialBlurRenderer* impl;
	PiTexture* src_tex;
	PI_USE_PARAM(tpf);
	impl = (RadialBlurRenderer*)renderer->impl;
	pi_hash_lookup(resources, impl->input_name, (void**)&src_tex);
	pi_sampler_set_texture(&impl->ss_src, src_tex);

	/* TEST
	s_tTime += s_iSign * tpf;
	if (s_tTime > 1.0f) {
	    s_tTime = 1.0f;
	    s_iSign = -1;
	} else if (s_tTime < 0.0f) {
	    s_tTime = 0.0f;
	    s_iSign = 1;
	}
	impl->progress = s_tTime / 1.0f;
	*/

	pi_material_set_uniform(impl->material, impl->U_Progress, UT_FLOAT, 1, &impl->progress, TRUE);
	pi_material_set_uniform(impl->material, impl->U_SampleDist, UT_FLOAT, 1, &impl->sample_dist, TRUE);
	pi_material_set_uniform(impl->material, impl->U_SampleStrength, UT_FLOAT, 1, &impl->sample_strength, TRUE);
	pi_material_set_uniform(impl->material, impl->U_SrcTex, UT_SAMPLER_2D, 1, &impl->ss_src, TRUE);
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	PI_USE_PARAM(renderer);
	PI_USE_PARAM(width);
	PI_USE_PARAM(height);
	//TODO:加入resize实现
}

PiRenderer* PI_API pi_radialblur_new()
{
	PiRenderer* renderer;
	RadialBlurRenderer* impl = pi_new0(RadialBlurRenderer, 1);

	renderer = pi_renderer_create(ERT_RADIALBLUR, "radial blur", _init, _resize, _update, _draw, impl);
	return renderer;
}

void PI_API pi_radialblur_deploy(PiRenderer* renderer, uint width, uint height, char* input_name, char* output_name)
{
	RadialBlurRenderer* impl;
	_type_check(renderer);
	impl = (RadialBlurRenderer*)(((PiRenderer*)renderer)->impl);

	impl->width = width;
	impl->height = height;

	pi_free(impl->input_name);
	pi_free(impl->output_name);

	impl->input_name = pi_str_dup(input_name);
	impl->output_name = pi_str_dup(output_name);

	impl->progress = 1.0f;
	impl->sample_dist = 1.0f;
	impl->sample_strength = 2.2f;

	impl->is_deploy = TRUE;
}

void PI_API pi_radialblur_free(PiRenderer* renderer)
{
	RadialBlurRenderer* impl;
	_type_check(renderer);
	impl = (RadialBlurRenderer*)(((PiRenderer*)renderer)->impl);

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

/* 进度设置，范围是[0.0, 1.0]，默认是1.0，0表示没有效果 1表示最大效果 */
void PI_API pi_radialblur_set_progress(PiRenderer* renderer, float progress)
{
	RadialBlurRenderer* impl;
	_type_check(renderer);
	impl = (RadialBlurRenderer*)(((PiRenderer*)renderer)->impl);
	if (impl)
	{
		impl->progress = progress;
	}
}

/* 采样距离设置，默认是1，值越大表示采样的点离当前像素点越远 */
void PI_API pi_radialblur_set_sample_dist(PiRenderer* renderer, float sample_dist)
{
	RadialBlurRenderer* impl;
	_type_check(renderer);
	impl = (RadialBlurRenderer*)(((PiRenderer*)renderer)->impl);
	if (impl)
	{
		impl->sample_dist = sample_dist;
	}
}

/* 力度，默认是2.2，越大表示越糊 */
void PI_API pi_radialblur_set_sample_strength(PiRenderer* renderer, float sample_strength)
{
	RadialBlurRenderer* impl;
	_type_check(renderer);
	impl = (RadialBlurRenderer*)(((PiRenderer*)renderer)->impl);
	if (impl)
	{
		impl->sample_strength = sample_strength;
	}
}