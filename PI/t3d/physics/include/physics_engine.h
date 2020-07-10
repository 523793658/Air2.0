#ifndef _Physic_Engine_H_
#define _Physic_Engine_H_

#include "pi_lib.h"
#include "physics_scene.h"
typedef enum
{
	PT_DEBUG,
	PT_RELEASE
}InitialType;

typedef struct  
{
	void* impl;
}PhysicsEngine;

PI_BEGIN_DECLS

void PI_API pi_physics_engine_init(InitialType type);

void PI_API pi_physics_engine_init_by_url(InitialType type, char* url);

PiBool PI_API pi_physics_engine_simulate(float tpf);

void PI_API pi_physics_engine_destroy();

PI_END_DECLS
#endif