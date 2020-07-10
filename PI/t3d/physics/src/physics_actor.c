#include "physics_actor.h"
#include "physics_wrap.h"

static PiActor* pi_physics_actor_new()
{
	PiActor* actor = pi_new0(PiActor, 1);
	pi_transform_set(&actor->shapeOffset, 0, 0, 0, 0, 0, 0, 1);
	actor->density = 1.0f;
	return actor;
}

void PI_API pi_physics_actor_free(PiActor* actor)
{
	physics_actor_release(actor);
	pi_free(actor);
}

PiActor* PI_API pi_physics_actor_create_dynamic(PiGeometry* g, PiPhyscisMaterial* material)
{
	PiActor* actor = pi_physics_actor_new();
	actor->type = AT_DYNAMIC;
	actor->geometry = g;
	actor->material = material;
	physics_actor_init(actor);
	return actor;
}

PiActor* PI_API pi_physics_actor_create_dynamic_with_density(PiGeometry* g, PiPhyscisMaterial* material, float density, TransformData* transform)
{
	PiActor* actor = pi_physics_actor_new();
	actor->type = AT_DYNAMIC;
	actor->geometry = g;
	actor->material = material;
	actor->density = density;
	if (transform)
	{
		pi_transform_copy(&actor->shapeOffset, transform);
	}
	physics_actor_init(actor);
	return actor;
}


PiActor* PI_API pi_physics_actor_create_static(PiGeometry* g, PiPhyscisMaterial* material, TransformData* transform)
{
	PiActor* actor = pi_physics_actor_new();
	actor->type = AT_STATIC;
	actor->geometry = g;
	actor->material = material;
	if (transform)
	{
		pi_transform_copy(&actor->shapeOffset, transform);
	}
	physics_actor_init(actor);
	return actor;
}

void PI_API pi_physics_actor_set_transform(PiActor* actor, TransformData* transform)
{
	physics_actor_apply_transform(actor, transform);
}

void PI_API pi_physics_actor_set_kinematic_target(PiActor* actor, TransformData* destination)
{
	physics_actor_set_kinematic_target(actor, destination);
}

void PI_API pi_physics_actor_get_world_transform(PiActor* actor, TransformData* dst)
{
	physics_actor_get_transform(actor, dst);
}

void PI_API pi_physics_actor_set_kinematic(PiActor* actor, PiBool is_kinematic)
{
	physics_actor_set_kinematic(actor, is_kinematic);
}


void PI_API pi_physics_actor_set_linear_velocity(PiActor* actor, float vx, float vy, float vz)
{
	physics_actor_set_linear_velocity(actor, vx, vy, vz);
}

void PI_API pi_physics_actor_set_force(PiActor* actor, float vx, float vy, float vz, int modeType)
{
	physics_actor_set_force(actor, vx, vy, vz, modeType);
}

void PI_API pi_physics_actor_wakeup(PiActor* actor)
{
	physics_actor_wakeup(actor);
}

void PI_API pi_physics_actor_set_angular_velocity(PiActor* actor, PiVector3* v)
{
	physics_actor_set_angular_velocity(actor, v);
}


void PI_API pi_physics_actor_set_sleep(PiActor* actor)
{
	physics_actor_set_sleep(actor);
}

PiBool PI_API pi_physics_actor_is_sleep(PiActor* actor)
{
	return physics_actor_is_sleep(actor);
}



void PI_API pi_physics_actor_set_linear_damp(PiActor* actor, float damp)
{
	physics_actor_set_linear_damp(actor, damp);
}

void PI_API pi_physics_actor_set_angular_damp(PiActor* actor, float damp)
{
	physics_actor_set_angular_damp(actor, damp);
}


