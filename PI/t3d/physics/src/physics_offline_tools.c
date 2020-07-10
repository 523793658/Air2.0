#include "physics_offline_tools.h"
#include "physics_wrap.h"

HeightFieldData* PI_API pi_physics_tools_cook_height_field(int nx, int nz, float* heightBuffer, char* path)
{
	HeightFieldData* data = pi_new0(HeightFieldData, 1);
	data->size = physics_cook_terrain(nx, nz, heightBuffer, &data->data);
	return data;
}

