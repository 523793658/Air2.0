#include "picture_switch.h"
#include "rendertarget.h"
#include "rendersystem.h"
#include "entity.h"
const static char *RS_PICTURE_SWITCH_VS = "default.vs";
typedef struct
{
	char* pitcture_from_name;
	char* pitcture_to_name;
	char* target_name;
	char* fs_shader_name;
	char* params_tex_name;

	PictureSwitchState state;
	float current_time;
	float process;
	float duration;
	PiVector4 params1;
	PiVector4 params2;

	SamplerState from_ss;
	SamplerState to_ss;
	SamplerState param_ss;


	PiMesh *mesh;
	PiRenderMesh *rmesh;
	PiMaterial *material;
	PiEntity *entity;

	PiCamera *camera;

	/*³£ÁÁ×Ö·û´®*/
	char* U_FromTex;
	char* U_ToTex;
	char* U_Process;
	char* U_Params;
	char* U_ParamsTex;
}PictureSwitchRenderer;

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	PictureSwitchRenderer* impl = renderer->impl;
	PiRenderTarget* target;
	uint32 width, height;
	pi_hash_lookup(resources, impl->target_name, (void**)&target);
	width = target->width;
	height = target->height;

	impl->U_FromTex = pi_conststr("u_FromTex");
	impl->U_ToTex = pi_conststr("u_ToTex");
	impl->U_Process = pi_conststr("u_Process");
	impl->U_Params = pi_conststr("u_Params");
	impl->U_ParamsTex = pi_conststr("u_ParamsTex");


	impl->mesh = pi_mesh_create_quad(NULL, NULL, 0.0f);
	impl->rmesh = pi_rendermesh_new(impl->mesh, TRUE);

	impl->material = pi_material_new(RS_PICTURE_SWITCH_VS, impl->fs_shader_name);
	impl->entity = pi_entity_new();

	pi_material_set_depth_enable(impl->material, FALSE);
	pi_material_set_depthwrite_enable(impl->material, FALSE);

	pi_renderstate_set_default_sampler(&impl->from_ss);
	pi_sampler_set_addr_mode(&impl->from_ss, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_material_set_uniform(impl->material, impl->U_FromTex, UT_SAMPLER_2D, 1, &impl->from_ss, FALSE);

	pi_renderstate_set_default_sampler(&impl->param_ss);
	pi_sampler_set_addr_mode(&impl->param_ss, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);


	pi_renderstate_set_default_sampler(&impl->to_ss);
	pi_sampler_set_addr_mode(&impl->to_ss, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_material_set_uniform(impl->material, impl->U_ToTex, UT_SAMPLER_2D, 1, &impl->to_ss, FALSE);

	pi_entity_set_mesh(impl->entity, impl->rmesh);
	pi_entity_set_material(impl->entity, impl->material);
	pi_spatial_set_local_scaling(impl->entity->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->entity->spatial);
	impl->camera = pi_camera_new();
	pi_camera_set_location(impl->camera, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->camera, 0.0f, 0.0f, -1.0f);
	pi_camera_set_frustum(impl->camera, -(float)width / 2.0f + 0.5f, width / 2.0f + 0.5f, -(float)height / 2.0f - 0.5f, height / 2.0f - 0.5f, 0.0f, 2.0f, TRUE);
	return TRUE;
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	PictureSwitchRenderer *impl = (PictureSwitchRenderer*)renderer->impl;
	PiRenderTarget *target;
	pi_hash_lookup(resources, impl->target_name, (void **)&target);
	pi_rendersystem_set_target(target);
	pi_rendersystem_set_camera(impl->camera);
	pi_entity_draw(impl->entity);
}

static void _update(PiRenderer *renderer, float tpf, PiHash *resources)
{
	PictureSwitchRenderer* impl;
	PiTexture* from_texture;
	PiTexture* to_texture;
	PiTexture* params_tex;
	impl = (PictureSwitchRenderer*)renderer->impl;
	if (impl->state == PSS_DURING)
	{
		impl->current_time += tpf;
		impl->process = impl->current_time / impl->duration;
		if (impl->process > 1.0f)
		{
			impl->process = 1.0f;
			impl->state = PSS_AFTER;
		}
	}
	pi_hash_lookup(resources, impl->pitcture_from_name, (void**)&from_texture);
	pi_hash_lookup(resources, impl->pitcture_to_name, (void**)&to_texture);
	pi_sampler_set_texture(&impl->from_ss, from_texture);
	pi_sampler_set_texture(&impl->to_ss, to_texture);
	pi_material_set_uniform_pack_flag(impl->material, impl->U_Process, UT_FLOAT, 1, &impl->process, FALSE, TRUE);
	pi_material_set_uniform_pack_flag(impl->material, impl->U_Params, UT_VEC4, 2, &impl->params1, FALSE, TRUE);
	if (impl->params_tex_name != NULL && pi_hash_lookup(resources, impl->params_tex_name, &params_tex))
	{
		pi_sampler_set_texture(&impl->param_ss, params_tex);
		pi_material_set_uniform(impl->material, impl->U_ParamsTex, UT_SAMPLER_2D, 1, &impl->param_ss, FALSE);
	}
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	PictureSwitchRenderer *impl;
	impl = (PictureSwitchRenderer *)renderer->impl;

	pi_spatial_set_local_scaling(impl->entity->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->entity->spatial);

	pi_camera_set_frustum(impl->camera, -(float)width / 2.0f + 0.5f, (float)width / 2.0f + 0.5f, -(float)height / 2.0f - 0.5f, (float)height / 2.0f - 0.5f, 0.0f, 2.0f, TRUE);

}

PiRenderer *PI_API pi_picture_switch_new_with_name(char* name)
{
	PictureSwitchRenderer* impl = pi_new0(PictureSwitchRenderer, 1);
	impl->duration = 1.0f;
	impl->state = PSS_BEFORE;
	return pi_renderer_create(ERT_PICTURE_SWITCH, name, _init, _resize, _update, _draw, impl);
}

PiRenderer *PI_API pi_picture_switch_new()
{
	return pi_picture_switch_new_with_name("picture_switch");
}

void PI_API pi_picture_switch_deploy(PiRenderer *renderer, char *src_name, char *dst_name, char* target_name, char* fs_shader_name)
{
	PictureSwitchRenderer* impl = renderer->impl;
	pi_free(impl->pitcture_from_name);
	pi_free(impl->pitcture_to_name);
	pi_free(impl->target_name);
	pi_free(impl->fs_shader_name);

	impl->pitcture_from_name = pi_str_dup(src_name);
	impl->pitcture_to_name = pi_str_dup(dst_name);
	impl->target_name = pi_str_dup(target_name);
	impl->fs_shader_name = pi_str_dup(fs_shader_name);
}

void PI_API pi_picture_switch_set_params_texture_name(PiRenderer* renderer, char* name)
{
	PictureSwitchRenderer* impl = renderer->impl;
	pi_free(impl->params_tex_name);
	impl->params_tex_name = pi_str_dup(name);
}

void PI_API pi_picture_switch_free(PiRenderer *renderer)
{
	PictureSwitchRenderer* impl = renderer->impl;
	pi_free(impl->pitcture_from_name);
	pi_free(impl->pitcture_to_name);
	pi_free(impl->target_name);
	pi_free(impl->fs_shader_name);
}

void PI_API pi_picture_switch_set_duration(PiRenderer *renderer, float duration)
{
	PictureSwitchRenderer* impl = renderer->impl;
	impl->duration = duration;
}

void PI_API pi_picture_switch_set_params(PiRenderer* renderer, float* data)
{
	PictureSwitchRenderer* impl = renderer->impl;
	pi_memcpy_inline(&impl->params1, data, sizeof(PiVector4) * 2);
}


void PI_API pi_picture_switch_change_state(PiRenderer *renderer, PictureSwitchState state)
{
	PictureSwitchRenderer* impl = renderer->impl;
	impl->state = state;
	impl->current_time = 0;
	switch (state)
	{
	case PSS_BEFORE:
		impl->process = 0.0f;
		break;
	case PSS_DURING:
		impl->process = 0.0f;
		break;
	case PSS_AFTER:
		impl->process = 1.0f;
		break;
	default:
		break;
	}
}

void PI_API pi_picture_switch_set_filter(PiRenderer *renderer, TexFilterOp filter)
{

}