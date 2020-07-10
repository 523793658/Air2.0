
#include "shiny.h"

typedef struct
{
	PiBool is_show;
	PiQuaternion rotation;
	PiVector3 shiny_color;
	PiVector3 light_dir;
	float speed;
	float current_angle;
	SamplerState sampler_clamp;
	float shiny_width;
	PiSequence *color_sequence;
	float color_time;
	float current_color_time;

	char *SHINY;
	char *U_ShinyLightDir;
	char *U_ShinyColor;
	char *U_ShinyWidth;
} Shiny;

static PiBool _update(struct PiController *c, float tpf)
{
	float factor = tpf;
	Shiny *impl = c->impl;
	if (impl)
	{
		impl->current_angle += impl->speed * factor;
		if (impl->current_angle > 2.0f * PI_PI)
		{
			impl->current_angle = 0.0f;
		}

		impl->current_color_time += factor / impl->color_time;
		if (impl->current_color_time > 1.0f)
		{
			impl->current_color_time = 0.0f;
		}

		if (impl->is_show)
		{
			PiVector3 *color = NULL;
			PiVector3 y_axis;
			pi_vec3_set(&y_axis, 0.0f, 1.0f, 0.0f);
			pi_quat_from_angle_axis(&impl->rotation, &y_axis, impl->current_angle);

			pi_sequence_set_time(impl->color_sequence, impl->current_color_time);
			color = (PiVector3 *)pi_sequence_get_value(impl->color_sequence);
			impl->shiny_color.x = color->x;
			impl->shiny_color.y = color->y;
			impl->shiny_color.z = color->z;
		}

		return TRUE;
	}

	return FALSE;
}

static PiBool _apply(PiController *c, ControllerApplyType type, void *obj)
{
	Shiny *impl = c->impl;
	PiEntity *entity = (PiEntity *)obj;
	if (impl)
	{
		pi_material_set_def(entity->material, impl->SHINY, impl->is_show);

		if (impl->is_show)
		{
			PiMatrix4 *world_matrix;
			PiVector3 default_light_dir;
			pi_vec3_set(&default_light_dir, 1.0f, 0.0f, 0.0f);

			world_matrix = pi_entity_get_world_matrix(entity);
			pi_quat_rotate_vec3(&impl->light_dir, &default_light_dir, &impl->rotation);
			pi_mat4_apply_vector(&impl->light_dir, &impl->light_dir, world_matrix);

			pi_material_set_def(entity->material, impl->SHINY, impl->is_show);
			pi_material_set_uniform(entity->material, impl->U_ShinyLightDir, UT_VEC3, 1, &impl->light_dir, FALSE);
			pi_material_set_uniform(entity->material, impl->U_ShinyColor, UT_VEC3, 1, &impl->shiny_color, FALSE);
			pi_material_set_uniform(entity->material, impl->U_ShinyWidth, UT_FLOAT, 1, &impl->shiny_width, FALSE);
		}

		return TRUE;
	}

	return FALSE;
}

PiController *PI_API pi_shiny_new()
{
	Shiny *impl = pi_new0(Shiny, 1);
	PiController *c = pi_controller_new(CT_SHINY, _apply, _update, impl);

	impl->is_show = TRUE;
	impl->color_sequence = NULL;

	pi_renderstate_set_default_sampler(&(impl->sampler_clamp));
	pi_sampler_set_addr_mode(&(impl->sampler_clamp), TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&(impl->sampler_clamp), TFO_MIN_MAG_LINEAR);

	impl->SHINY = pi_conststr("SHINY");
	impl->U_ShinyLightDir = pi_conststr("u_ShinyLightDir");
	impl->U_ShinyColor = pi_conststr("u_ShinyColor");
	impl->U_ShinyWidth = pi_conststr("u_ShinyWidth");

	return c;
}

void PI_API pi_shiny_free(PiController *c)
{
	Shiny *impl = c->impl;
	pi_free(impl);
	pi_controller_free(c);
}

/*
 * the range of lightwidth is [0,1]
 */
void PI_API pi_shiny_set_width(PiController *c, float width)
{
	Shiny *impl = c->impl;
	if (impl)
	{
		impl->shiny_width = width;
	}
}

void PI_API pi_shiny_show(PiController *c)
{
	Shiny *impl = c->impl;
	if (impl)
	{
		impl->is_show = TRUE;
	}
}

void PI_API pi_shiny_hide(PiController *c)
{
	Shiny *impl = c->impl;
	if (impl)
	{
		impl->is_show = FALSE;
	}
}

void PI_API pi_shiny_set_speed(PiController *c, float speed)
{
	Shiny *impl = c->impl;
	if (impl)
	{
		impl->speed = speed;
	}
}

void PI_API pi_shiny_set_color_sequence(PiController *c, PiSequence *color_sequence)
{
	Shiny *impl = c->impl;
	if (impl)
	{
		impl->color_sequence = color_sequence;
	}
}

void PI_API pi_shiny_set_color_time(PiController *c, float time)
{
	Shiny *impl = c->impl;
	if (impl)
	{
		impl->color_time = time;
	}
}
