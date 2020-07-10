#ifndef _Physics_Force_Field_H_
#define _Physics_Force_Field_H_
#pragma once
#include "pi_lib.h"
#include "pi_vector3.h"

typedef void(*ForceFieldUpdateFunc) ();

typedef enum _ForceFieldType
{
	FFS_Sphere,
	FFS_Capsule,
	FFS_Cylinder,
	FFS_Cone,
	FFS_Box
}ForceFieldType;

typedef struct _SphereParams
{
	PiVector3 position;
	float radius;
}SphereParams;


typedef struct _ForceField
{
	ForceFieldType type;
	PiVector* last_frame_actors;
	PiVector* current_frame_actors;
	ForceFieldUpdateFunc update_func;
	float life_time;
	float start_value;
	float end_value;
	void* params;
}ForceField;





PI_BEGIN_DECLS
ForceField* PI_API pi_physics_sphere_force_field_create(float x, float y, float z, float radius, float life_time, float start_value, float end_value);
PI_END_DECLS





#endif