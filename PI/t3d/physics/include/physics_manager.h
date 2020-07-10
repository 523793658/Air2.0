#ifndef _Physics_Manager_H_
#define _Physics_Manager_H_
#include "pi_lib.h"
#include "physics_obj_binder.h"
#include "physics_scene.h"

typedef enum
{
	PMUS_CLAMP,
	PMUS_FIXTIME,
	PMUS_TIME_DIVISION
}PhysicsManagerUpdateStrategy;

typedef struct  
{
	PiVector* scene_list;
	PiVector* obj_binder_list_dynamic;
	PiVector* obj_binder_list_static;
	float time_remainder;
	float fixed_timestep;
	PhysicsManagerUpdateStrategy strategy;

}PiPhysicsManager;


PI_BEGIN_DECLS

PiPhysicsManager* PI_API pi_physics_manager_create(PhysicsManagerUpdateStrategy strategy);

void PI_API pi_physics_manager_free(PiPhysicsManager* manager);


void PI_API pi_physics_manager_add_scene(PiPhysicsManager* manager, PiPhysicsScene* scene);

void PI_API pi_physics_manager_remove_scene(PiPhysicsManager* manager, PiPhysicsScene* scene);


void PI_API pi_physics_manager_add_obj_binder(PiPhysicsManager* manager, PiObjectBinder* binder);

void PI_API pi_physics_manager_remove_obj_binder(PiPhysicsManager* manager, PiObjectBinder* binder);

void PI_API pi_physics_manager_update(PiPhysicsManager* manager, float tpf);


PI_END_DECLS





#endif