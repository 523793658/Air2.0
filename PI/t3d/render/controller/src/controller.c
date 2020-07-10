#include <controller.h>

static PiBool _default_update(PiController *c, float tpf)
{
	return FALSE;
}

static PiBool _default_apply(PiController *c, ControllerApplyType type, void *obj)
{
	return TRUE;
}

PiController* PI_API pi_controller_new(ControllerType type, ControllerApplyFunc apply, ControllerUpdateFunc update, void *impl)
{
	PiController *c = pi_new0(PiController, 1);
	c->type = type;
	c->impl = impl;
	c->update_func = update != NULL ? update : _default_update;
	c->apply_func = apply != NULL ? apply : _default_apply;
	c->is_alive = TRUE;
	return c;
}

void PI_API pi_controller_free(PiController *r)
{
	pi_free(r);
}

PiBool PI_API pi_controller_update(PiController *c, float tpf)
{
	PiBool r = FALSE;

	PI_ASSERT(c != NULL && ( (c->type >= CT_USER) || (c->type >= 0 && c->type < CT_COUNT) ), "update invalid controller, type = %d", c->type);


	if(c->update_func != NULL)
	{
		r = c->update_func(c, tpf);
	}
	return r;
}

PiBool PI_API pi_controller_apply(PiController *c, ControllerApplyType type, void *obj)
{
	PiBool r = TRUE;
	if(c->apply_func != NULL)
	{
		r = c->apply_func(c, type, obj);
	}
	return r;
}
