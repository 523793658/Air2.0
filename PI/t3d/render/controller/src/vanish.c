
#include "vanish.h"

typedef struct
{
	char *VANISH;
	char *U_VanishSpeed;
	char *U_VanishGap;
} VanishConstString;

typedef struct
{
	float vanish_elapse;
	float vanish_time;
	float vanish_speed;
	float vanish_gap;
	PiBool end_visible;
} Vanish;

static VanishConstString *s_const_str = NULL;

static PiBool _update(struct PiController *c, float tpf)
{
	Vanish *impl = (Vanish *)c->impl;
	impl->vanish_elapse += tpf;
	impl->vanish_speed = impl->vanish_elapse / impl->vanish_time;
	return FALSE;
}

static PiBool _apply(PiController *c, ControllerApplyType type, void *obj)
{
	Vanish *impl = (Vanish *)c->impl;
	PiMaterial *material = (PiMaterial *)obj;
	if (impl->vanish_speed < 1.001f + impl->vanish_gap)
	{
		pi_material_set_def(material, s_const_str->VANISH, TRUE);
		pi_material_set_uniform(material, s_const_str->U_VanishSpeed, UT_FLOAT, 1, &impl->vanish_speed, FALSE);
	}
	else if (impl->end_visible)
	{
		pi_material_set_def(material, s_const_str->VANISH, FALSE);
	}
	return TRUE;
}

PiController *PI_API pi_vanish_new()
{
	Vanish *impl = pi_new0(Vanish, 1);
	PiController *c = pi_controller_new(CT_VANISH, _apply, _update, impl);

	impl->vanish_elapse = 0;
	impl->vanish_time = 4.0f;
	impl->vanish_gap = 0.1f;
	impl->vanish_speed = 0.0f;
	impl->end_visible = FALSE;

	if (!s_const_str)
	{
		s_const_str = pi_new0(VanishConstString, 1);

		s_const_str->VANISH = pi_conststr("VANISH");
		s_const_str->U_VanishGap = pi_conststr("u_VanishGap");
		s_const_str->U_VanishSpeed = pi_conststr("u_VanishSpeed");
	}
	return c;
}

void PI_API pi_vanish_set_parameter(PiController *c, float vanish_time, float vanish_gap, PiBool end_visible)
{
	Vanish *impl = (Vanish *)c->impl;
	impl->vanish_time = vanish_time;
	impl->vanish_gap = vanish_gap;
	impl->end_visible = end_visible;
}

void PI_API pi_vanish_reset(PiController *c, void *obj)
{
	Vanish *impl = (Vanish *)c->impl;
	PiMaterial *material = (PiMaterial *)obj;
	impl->vanish_elapse = 0;
	pi_material_set_def(material, s_const_str->VANISH, TRUE);
}

void PI_API pi_vanish_free(PiController *c)
{
	Vanish *impl = (Vanish *)c->impl;
	pi_free(impl);
	pi_controller_free(c);
}
