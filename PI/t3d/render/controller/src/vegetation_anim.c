#include "vegetation_anim.h"
typedef struct _Params
{
	float leaf_amplitude;
	float leaf_frequency;
	float branch_amplitude;
	float branch_frequency;
}Params;

typedef struct
{
	PiEnvironment* env;

	PiVector4 wind_blend;
	float wind_scale;
	float wind_frequency;

	PiSpatial* bind_spatial;

	Params params;

	PiBool leaf_anim_enable;
	float individual_phase;
	PiBool vertex_color;
	PiTexture* leaf_tex;
	PiTexture* branch_tex;

	PiSphere* fall_generator;
    float fall_scale;
	float fall_intensity;
	float fall_dir[2];
	PiBool is_fall;

	SamplerState sampler;
	float time;

	char *VEGETATION_ANIM;
	char *VEGETATION_ANIM_LEAF;
	char *VEGETATION_LEAF_MAP;

	char *U_WindBlend;

	char *U_VegetationParams;
	char *U_VegetationLeafMap;
	char *U_VegetationBranchMap;
} VegetationAnim;

PiBool _vec2_normalise(float *dst, const float *src)
{
	float fLen = pi_math_sqrt(src[0] * src[0] + src[1] * src[1]);
	if(IS_FLOAT_EQUAL(fLen, 0.0f))
	{
		pi_memset_inline(dst, 0, sizeof(float) * 2);
		return FALSE;
	}
	dst[0] = src[0] / fLen;
	dst[1] = src[1] / fLen;
	return TRUE;
}

static PiBool _update(struct PiController *c, float tpf)
{
	VegetationAnim *impl = (VegetationAnim*)c->impl;
	PiEnvironment* env = impl->env;
	float intensity;
	float inherent_intensity;
	impl->time += tpf;
	if (impl->is_fall)
	{
		impl->fall_intensity *= 0.5;
		if (impl->fall_intensity < 0.1f)
		{
			impl->fall_dir[0] = impl->fall_dir[1] = 0;
			impl->is_fall = FALSE;
		}
	}


	intensity = env->wind_data.wind_intensity;
	inherent_intensity = intensity * 0.4f * (pi_math_sin(impl->time * 5 * impl->wind_frequency + impl->individual_phase) + 1.0f);
	intensity += inherent_intensity;

	impl->wind_blend.x = env->wind_data.wind_dir[0] * intensity;
	impl->wind_blend.y = env->wind_data.wind_dir[1] * intensity;
	PiVector3* pos;
	PiVector3 v = { 1.0f, 1.0f, 1.0f };
	pos = pi_spatial_get_world_translation(impl->bind_spatial);
	impl->wind_blend.w = pi_vec3_dot(pos, &v);
	if (impl->fall_generator)
	{
		float fall_dir[2];
		float distance;
		float fall_intensity;
		distance = pi_vec3_distance(pos, &impl->fall_generator->pos);
		fall_dir[0] = pos->x - impl->fall_generator->pos.x;
		fall_dir[1] = pos->z - impl->fall_generator->pos.z;
		_vec2_normalise(fall_dir, fall_dir);

		fall_intensity = impl->fall_generator->radius - distance;
		fall_intensity = MIN(MAX(fall_intensity, 0.0f), 1.0f) * impl->fall_scale;

		if (fall_intensity > impl->fall_intensity)
		{
			impl->fall_intensity = fall_intensity;
			impl->is_fall = TRUE;
			if (impl->fall_dir[0] == 0 && impl->fall_dir[1] == 0)
			{
				impl->fall_dir[0] = fall_dir[0];
				impl->fall_dir[1] = fall_dir[1];
			}
		}
	}

	impl->wind_blend.x += impl->fall_dir[0] * impl->fall_intensity;
	impl->wind_blend.y += impl->fall_dir[1] * impl->fall_intensity;

	impl->wind_blend.x *= impl->wind_scale;
	impl->wind_blend.y *= impl->wind_scale;

	{
		PiVector3 wind_tmp;
		PiQuaternion* rotation = pi_spatial_get_world_rotation(impl->bind_spatial);
		PiQuaternion temp_rotation;
		pi_quat_inverse(&temp_rotation, rotation);
		pi_vec3_set(&wind_tmp, impl->wind_blend.x, 0, impl->wind_blend.y);
		pi_quat_rotate_vec3(&wind_tmp, &wind_tmp, &temp_rotation);
		impl->wind_blend.x = wind_tmp.x;
		impl->wind_blend.y = wind_tmp.z;
	}
	return FALSE;
}

static PiBool _apply(PiController *c, ControllerApplyType type, void *obj)
{
	VegetationAnim *impl = (VegetationAnim*)c->impl;

	PI_ASSERT(type == CAT_ENTITY, "The vegetation controller can only support entity!");

	if (type == CAT_ENTITY)
	{
		PiEntity* entity = (PiEntity*)obj;
		PiMaterial* material = entity->material;
		
		pi_material_set_uniform(material, impl->U_WindBlend, UT_VEC4, 1, &impl->wind_blend, FALSE);
	}

	return TRUE;
}

PiController* PI_API pi_vegetation_anim_new()
{
	VegetationAnim *impl = pi_new0(VegetationAnim, 1);
	PiController *c = pi_controller_new(CT_VEGETATION_ANIM, _apply, _update, impl);
	pi_renderstate_set_default_sampler(&impl->sampler);
	pi_sampler_set_addr_mode(&impl->sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->sampler, TFO_MIN_MAG_POINT);
	
	impl->VEGETATION_ANIM = pi_conststr("VEGETATION_ANIM");
	impl->VEGETATION_ANIM_LEAF = pi_conststr("VEGETATION_ANIM_LEAF");
	impl->VEGETATION_LEAF_MAP = pi_conststr("VEGETATION_LEAF_MAP");

	impl->U_WindBlend = pi_conststr("u_WindBlend");
	impl->U_VegetationParams = pi_conststr("u_VegetationParams");

	impl->U_VegetationLeafMap = pi_conststr("u_VegetationLeafMap");
	impl->U_VegetationBranchMap = pi_conststr("u_VegetationBranchMap");

	impl->wind_blend.z = 0.1f;
	impl->wind_scale = 1;
	impl->wind_frequency = 1;
	impl->leaf_anim_enable = FALSE;
	impl->params.leaf_amplitude = 0.1;
	impl->params.leaf_frequency= 1;
	impl->params.branch_amplitude = 0.02;
	impl->params.branch_frequency= 1;
    impl->fall_scale = 30.0f;

	return c;
}

void PI_API pi_vegetation_anim_free(PiController *c)
{
	VegetationAnim *impl = (VegetationAnim*)c->impl;

	pi_free(impl);
	pi_controller_free(c);
}

void PI_API pi_vegetation_anim_apply_params(PiController *c, PiEntity* entity)
{
	VegetationAnim *impl = (VegetationAnim*)c->impl;
	PiMaterial* material = entity->material;

	pi_material_set_def(material, impl->VEGETATION_ANIM, TRUE);
	pi_material_set_def(material, impl->VEGETATION_ANIM_LEAF, impl->leaf_anim_enable);
	pi_material_set_uniform(material, impl->U_VegetationParams, UT_VEC4, 1, &impl->params, FALSE);
	pi_material_set_uniform(material, impl->U_WindBlend, UT_VEC4, 1, &impl->wind_blend, FALSE);

	if (impl->leaf_tex != NULL)
	{
		pi_material_set_def(material, impl->VEGETATION_LEAF_MAP, TRUE);
		pi_sampler_set_texture(&impl->sampler, impl->leaf_tex);
		pi_material_set_uniform(material, impl->U_VegetationLeafMap, UT_SAMPLER_2D, 1, &impl->sampler, TRUE);
	}
}

void PI_API pi_vegetation_anim_set_environment(PiController *c, PiEnvironment* env)
{
	VegetationAnim *impl = (VegetationAnim*)c->impl;

	impl->env = env;
}

void PI_API pi_vegetation_anim_set_trunk_param(PiController *c, float trunk_flexibility, float wind_scale, float wind_frequency)
{
	VegetationAnim *impl = (VegetationAnim*)c->impl;

	impl->wind_blend.z = trunk_flexibility;
	impl->wind_scale = wind_scale;
	impl->wind_frequency = wind_frequency;
}

void PI_API pi_vegetation_anim_set_leaf_anim_enable(PiController *c, PiBool is_enable)
{
	VegetationAnim *impl = (VegetationAnim*)c->impl;

	impl->leaf_anim_enable = is_enable;
}

void PI_API pi_vegetation_anim_set_leaf_param(PiController *c, float leaf_amplitude, float leaf_frequency, float branch_amplitude, float branch_frequency)
{
	VegetationAnim *impl = (VegetationAnim*)c->impl;

	impl->params.leaf_amplitude = leaf_amplitude * 0.1f;
	impl->params.leaf_frequency = leaf_frequency;
	impl->params.branch_amplitude = branch_amplitude * 0.02f;
	impl->params.branch_frequency = branch_frequency;
}

void PI_API pi_vegetation_anim_set_leaf_attenuation(PiController *c, PiBool vertex_color, PiTexture* leaf_tex, PiTexture* branch_tex)
{
	VegetationAnim *impl = (VegetationAnim*)c->impl;

	impl->vertex_color = vertex_color;
	impl->leaf_tex = leaf_tex;
	impl->branch_tex = branch_tex;
}

void PI_API pi_vegetation_anim_set_individual_phase(PiController *c, float phase)
{
	VegetationAnim *impl = (VegetationAnim*)c->impl;

	impl->individual_phase = phase;
}

void PI_API pi_vegetation_anim_set_fall_generator(PiController *c, PiSphere *generator, float fall_scale)
{
	VegetationAnim *impl = (VegetationAnim*)c->impl;

	impl->fall_generator = generator;
    impl->fall_scale = fall_scale;
}

void PI_API pi_vegetation_anim_set_bind_spatial(PiController *c, PiSpatial* spatial)
{
	VegetationAnim* impl = (VegetationAnim*)c->impl;
	impl->bind_spatial = spatial;
}

