#ifndef _Physics_Scene_H_
#define _Physics_Scene_H_
#define _Physics_Scene_H_
#include "pi_lib.h"
#include "physics_actor.h"
#include "physics_ragdoll.h"
typedef struct  
{
	void* impl;
}PiPhysicsScene;

PI_BEGIN_DECLS

PiPhysicsScene* PI_API pi_physics_scene_create(float x, float y, float z);

void PI_API pi_physics_scene_free(PiPhysicsScene* scene);

void PI_API pi_physics_scene_add_actor(PiPhysicsScene* scene, PiActor* actor);

void PI_API pi_physics_scene_remove_actor(PiPhysicsScene* scene, PiActor* actor);

void PI_API pi_physics_scene_add_ragdoll(PiPhysicsScene* scene, Ragdoll* ragdoll);

void PI_API pi_physics_scene_remove_ragdoll(PiPhysicsScene* scene, Ragdoll* ragdoll);

void PI_API pi_physics_scene_simulate(PiPhysicsScene* scene, float tpf);

void PI_API pi_physics_scene_fetch_results(PiPhysicsScene* scene, PiBool block, uint32* errorState);

PI_END_DECLS
#endif