#include "physics_mesh.h"
#include "physics_wrap.h"
uint PI_API pi_physics_mesh_get_height_field_size(HeightFieldData* data)
{
	return data->size;
}

void* PI_API pi_physics_mesh_get_height_field_data(HeightFieldData* data)
{
	return data->data;
}

void PI_API pi_physics_mesh_height_field_data_free(HeightFieldData* data)
{
	pi_free(data->data);
	pi_free(data);
}

HeightField* PI_API pi_physics_mesh_load_height_field(void* data, uint size)
{
	HeightField* height_field = pi_new(HeightField, 1);
	physics_height_field_init(height_field, data, size);
	return height_field;
}

void PI_API pi_physics_mesh_height_field_free(HeightField* hf)
{
	physics_height_field_free(hf);
	pi_free(hf);
}

PhysicsTriangleMesh* PI_API pi_physics_mesh_create_triangle_mesh(PiMesh* mesh)
{
	PhysicsTriangleMesh* triangleMesh = pi_new0(PhysicsTriangleMesh, 1);
	physics_cook_triangle_mesh(mesh, triangleMesh);
	return triangleMesh;
}

void PI_API pi_physics_mesh_free_triangle_mesh(PhysicsTriangleMesh* mesh)
{
	physics_triangle_mesh_free(mesh);
	pi_free(mesh);
}