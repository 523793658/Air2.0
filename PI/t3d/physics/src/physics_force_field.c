#include "physics_force_field.h"



ForceField* PI_API pi_physics_force_field_create(float life_time, float start_value, float end_value, void* params, ForceFieldUpdateFunc update_func)
{
	ForceField* forcefield = pi_new0(ForceField, 1);
	forcefield->type = FFS_Sphere;
	forcefield->end_value = end_value;
	forcefield->start_value = start_value;
	forcefield->life_time = life_time;
	forcefield->params = params;
	forcefield->update_func = update_func;
	return forcefield;
}


void _sphere_update()
{

}

ForceField* PI_API pi_physics_sphere_force_field_create(float x, float y, float z, float radius, float life_time, float start_value, float end_value)
{
	SphereParams* params = pi_new(SphereParams, 1);
	pi_vec3_set(&params->position, x, y, z);
	params->radius = radius;
	return pi_physics_force_field_create(life_time, start_value, end_value, params, _sphere_update);
}

void PI_API pi_physics_force_field_update()
{

}
