#include "physics_material.h"
#include "physics_wrap.h"
PiPhyscisMaterial* PI_API pi_physics_material_create(float staticFriction, float dynamicFriction, float restitution)
{
	PiPhyscisMaterial* material = pi_new(PiPhyscisMaterial, 1);
	physics_material_create(material, staticFriction, dynamicFriction, restitution);
	return material;
}

void PI_API pi_physics_material_free(PiPhyscisMaterial* material)
{
	physics_material_free(material);
	pi_free(material);
}