
#include "swing.h"

typedef struct
{
	PiBool is_show;
	ENUM_SWING_MAP_MODE current_mode;
	PiMatrix4 rotation_matrix;
	PiVector3 swing_color;
	float speed;
	float current_angle;

	SamplerState sampler_clamp;
	PiTexture *cube_tex;
	PiTexture *planar_tex;

	PiSequence *color_sequence;
	float current_color_time;
	float color_time;

	char *SWING_USE_CUBEMAP;
	char *SWING;
	char *U_SwingCubeTex;
	char *U_SwingPlanarTex;
	char *U_SwingRotationMatrix;
	char *U_SwingColor;
} Swing;

static PiBool _update(PiController *c, float tpf)
{
	Swing *impl = c->impl;
	if (impl)
	{
		impl->current_angle += impl->speed * tpf;
		if (impl->current_angle > 2.0f * PI_PI)
		{
			impl->current_angle = 0.0f;
		}

		impl->current_color_time += tpf / impl->color_time;
		if (impl->current_color_time > 1.0f)
		{
			impl->current_color_time = 0.0f;
		}

		if (impl->is_show)
		{
			PiVector3 *color = NULL;
			PiQuaternion rotation;
			PiVector3 y_axis;

			pi_vec3_set(&y_axis, 0.0f, 1.0f, 0.0f);
			pi_quat_from_angle_axis(&rotation, &y_axis, impl->current_angle);
			pi_mat4_build_rotate(&impl->rotation_matrix, &rotation);

			pi_sequence_set_time(impl->color_sequence, impl->current_color_time);
			color = (PiVector3 *)pi_sequence_get_value(impl->color_sequence);
			impl->swing_color.x = color->x;
			impl->swing_color.y = color->y;
			impl->swing_color.z = color->z;
		}

		return TRUE;
	}

	return FALSE;
}

static PiBool _apply(PiController *c, ControllerApplyType type, void *obj)
{
	Swing *impl = c->impl;
	PiEntity *entity = (PiEntity *)obj;
	if (impl)
	{
		pi_material_set_def(entity->material, impl->SWING, impl->is_show);
		if (impl->is_show)
		{
			if (impl->current_mode == ENUM_SWING_CUBE_MAP)
			{
				pi_material_set_def(entity->material, impl->SWING_USE_CUBEMAP, TRUE);
				pi_sampler_set_texture(&impl->sampler_clamp, impl->cube_tex);
				pi_material_set_uniform(entity->material, impl->U_SwingCubeTex, UT_SAMPLER_CUBE, 1, &impl->sampler_clamp, FALSE);
			}
			else if (impl->current_mode == ENUM_SWING_PLANAR_MAP)
			{
				pi_material_set_def(entity->material, impl->SWING_USE_CUBEMAP, FALSE);
				pi_sampler_set_texture(&impl->sampler_clamp, impl->planar_tex);
				pi_material_set_uniform(entity->material, impl->U_SwingPlanarTex, UT_SAMPLER_2D, 1, &impl->sampler_clamp, FALSE);
			}
			pi_material_set_uniform(entity->material, impl->U_SwingRotationMatrix, UT_MATRIX4, 1, &impl->rotation_matrix, FALSE);
			pi_material_set_uniform(entity->material, impl->U_SwingColor, UT_VEC3, 1, &(impl->swing_color), FALSE);
		}

		return TRUE;
	}

	return FALSE;
}

PiController *PI_API pi_swing_new()
{
	Swing *impl = pi_new0(Swing, 1);
	PiController *c = pi_controller_new(CT_SWING, _apply, _update, impl);

	impl->is_show = TRUE;
	impl->color_sequence = NULL;

	pi_renderstate_set_default_sampler(&impl->sampler_clamp);
	pi_sampler_set_addr_mode(&impl->sampler_clamp, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->sampler_clamp, TFO_MIN_MAG_LINEAR);
	pi_mat4_set_identity(&impl->rotation_matrix);
	impl->planar_tex = NULL;
	impl->cube_tex = NULL;

	impl->SWING = pi_conststr("SWING");
	impl->SWING_USE_CUBEMAP = pi_conststr("SWING_USE_CUBEMAP");
	impl->U_SwingCubeTex = pi_conststr("u_SwingCubeTex");
	impl->U_SwingPlanarTex = pi_conststr("u_SwingPlanarTex");
	impl->U_SwingRotationMatrix = pi_conststr("u_SwingRotationMatrix");
	impl->U_SwingColor = pi_conststr("u_SwingColor");

	impl->current_mode = ENUM_SWING_CUBE_MAP;
	impl->swing_color.x = 1.0f;
	impl->swing_color.y = 1.0f;
	impl->swing_color.z = 1.0f;
	impl->speed = 0.0f;
	impl->current_angle = 0.0f;

	impl->color_time = 1.0f;
	impl->current_color_time = 0.0f;

	return c;
}

void PI_API pi_swing_free(PiController *c)
{
	Swing *impl = c->impl;
	pi_free(impl);
	pi_controller_free(c);
}

void PI_API pi_swing_set_planar_map(PiController *c, PiTexture *planar_tex)
{
	Swing *impl = c->impl;
	if (impl)
	{
		impl->planar_tex = planar_tex;
	}
}

void PI_API pi_swing_set_cube_map(PiController *c, PiTexture *cube_tex)
{
	Swing *impl = c->impl;
	if (impl)
	{
		impl->cube_tex = cube_tex;
	}
}

void PI_API pi_swing_set_map_mode(PiController *c, ENUM_SWING_MAP_MODE mode)
{
	Swing *impl = c->impl;
	if (impl)
	{
		impl->current_mode = mode;
	}
}

void PI_API pi_swing_show(PiController *c)
{
	Swing *impl = c->impl;
	if (impl)
	{
		impl->is_show = TRUE;
	}
}

void PI_API pi_swing_hide(PiController *c)
{
	Swing *impl = c->impl;
	if (impl)
	{
		impl->is_show = FALSE;
	}
}

void PI_API pi_swing_set_speed(PiController *c, float speed)
{
	Swing *impl = c->impl;
	if (impl)
	{
		impl->speed = speed;
	}
}

void PI_API pi_swing_set_color_sequence(PiController *c, PiSequence *color_sequence)
{
	Swing *impl = c->impl;
	if (impl)
	{
		impl->color_sequence = color_sequence;
	}
}

void PI_API pi_swing_set_color_time(PiController *c, float time)
{
	Swing *impl = c->impl;
	if (impl)
	{
		impl->color_time = time;
	}
}