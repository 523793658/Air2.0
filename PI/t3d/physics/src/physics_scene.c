#include "physics_wrap.h"
#include "physics_scene.h"


PiPhysicsScene* PI_API pi_physics_scene_create(float x, float y, float z)
{
	PiPhysicsScene* scene = pi_new0(PiPhysicsScene, 1);
	PiVector3 g = { x, y, z };
	physics_scene_create(scene, &g, NULL);
	return scene;
}

void PI_API pi_physics_scene_free(PiPhysicsScene* scene)
{
	physics_scene_free(scene);
	pi_free(scene);
}

void PI_API pi_physics_scene_add_actor(PiPhysicsScene* scene, PiActor* actor)
{
	physics_scene_add_actor(scene, actor);
}

void PI_API pi_physics_scene_remove_actor(PiPhysicsScene* scene, PiActor* actor)
{
	physics_scene_remove_actor(scene, actor);
}

void PI_API pi_physics_scene_add_ragdoll(PiPhysicsScene* scene, Ragdoll* ragdoll)
{
	physics_scene_add_aggregate(scene, ragdoll->aggregate);
}

void PI_API pi_physics_scene_remove_ragdoll(PiPhysicsScene* scene, Ragdoll* ragdoll)
{
	physics_scene_remove_aggregate(scene, ragdoll->aggregate);
}

void PI_API pi_physics_scene_simulate(PiPhysicsScene* scene, float tpf)
{
	physics_scene_simulate(scene, tpf);
}

void PI_API pi_physics_scene_fetch_results(PiPhysicsScene* scene, PiBool block, uint32* errorState)
{
	physics_scene_fetch_results(scene, block, errorState);
}