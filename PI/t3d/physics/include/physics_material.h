#ifndef _Physics_Material_H_
#define _Physics_Material_H_
#include "pi_lib.h"

typedef struct
{
	void* impl;
}PiPhyscisMaterial;


PI_BEGIN_DECLS

PiPhyscisMaterial* PI_API pi_physics_material_create(float staticFriction, float dynamicFriction, float restitution);

void PI_API pi_physics_material_free(PiPhyscisMaterial* material);

PI_END_DECLS

#endif