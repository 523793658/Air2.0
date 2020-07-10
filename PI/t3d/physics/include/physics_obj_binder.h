#ifndef _Phsics_Obj_Binder_H_
#define _Phsics_Obj_Binder_H_

#include "pi_lib.h"
#include "pi_spatial.h"
#include "physics_actor.h"
typedef struct  
{
	PiSpatial* spatial;
	PiActor* actor;
	PiBool sleeping;
}PiObjectBinder;

PI_BEGIN_DECLS


PiObjectBinder* PI_API pi_physics_obj_binder_create(PiSpatial* spatial, PiActor* actor);

void PI_API pi_physics_obj_binder_free(PiObjectBinder* binder);

void PI_API pi_physics_obj_binder_synch(PiObjectBinder* binder);

void PI_API pi_physics_obj_binder_update(PiObjectBinder* binder);

PI_END_DECLS




#endif