#ifndef _Physics_Actor_H_
#define _Physics_Actor_H_
#include "pi_lib.h"
#include "pi_matrix4.h"
#include "pi_transform.h"
#include "physics_material.h"
#include "physics_geometry.h"

typedef enum
{
	AT_STATIC,
	AT_DYNAMIC
}PiActorType;



typedef struct
{
	int64_t id;
	void* impl;
	TransformData transform;
	PiActorType type;
	PiGeometry* geometry;
	float density;
	PiPhyscisMaterial* material;
	TransformData shapeOffset;
}PiActor;

PI_BEGIN_DECLS

PiActor* PI_API pi_physics_actor_create_dynamic(PiGeometry* g, PiPhyscisMaterial* material);

PiActor* PI_API pi_physics_actor_create_dynamic_with_density(PiGeometry* g, PiPhyscisMaterial* material, float density, TransformData* transform);

PiActor* PI_API pi_physics_actor_create_static(PiGeometry* g, PiPhyscisMaterial* material, TransformData* offset);

void PI_API pi_physics_actor_free(PiActor* actor);

void PI_API pi_physics_actor_set_transform(PiActor* actor, TransformData* transform);


void PI_API pi_physics_actor_set_kinematic_target(PiActor* actor, TransformData* destination);

void PI_API pi_physics_actor_get_world_transform(PiActor* actor, TransformData* dst);

void PI_API pi_physics_actor_set_kinematic(PiActor* actor, PiBool is_kinematic);

void PI_API pi_physics_actor_set_linear_velocity(PiActor* actor, float vx, float vy, float vz);

void PI_API pi_physics_actor_set_force(PiActor* actor, float vx, float vy, float vz, int modeType);

void PI_API pi_physics_actor_wakeup(PiActor* actor);

void PI_API pi_physics_actor_set_angular_velocity(PiActor* actor, PiVector3* v);

void PI_API pi_physics_actor_set_sleep(PiActor* actor);

PiBool PI_API pi_physics_actor_is_sleep(PiActor* actor);

void PI_API pi_physics_actor_set_linear_damp(PiActor* actor, float damp);

void PI_API pi_physics_actor_set_angular_damp(PiActor* actor, float damp);




PI_END_DECLS

#endif