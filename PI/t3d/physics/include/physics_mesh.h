#ifndef _Physics_Mesh_H_
#define _Physics_Mesh_H_
#include "pi_lib.h"
#include "pi_mesh.h"
typedef struct _PhysicsTriangleMesh
{
	void* impl;
}PhysicsTriangleMesh;

typedef struct
{
	void* data;
	uint size;
}HeightFieldData;

typedef struct  
{
	void* impl;
}HeightField;

PI_BEGIN_DECLS

uint PI_API pi_physics_mesh_get_height_field_size(HeightFieldData* data);

void PI_API pi_physics_mesh_height_field_data_free(HeightFieldData* data);

void* PI_API pi_physics_mesh_get_height_field_data(HeightFieldData* data);

HeightField* PI_API pi_physics_mesh_load_height_field(void* data, uint size);

void PI_API pi_physics_mesh_height_field_free(HeightField* hf);

PhysicsTriangleMesh* PI_API pi_physics_mesh_create_triangle_mesh(PiMesh* mesh);

void PI_API pi_physics_mesh_free_triangle_mesh(PhysicsTriangleMesh* mesh);

PI_END_DECLS

#endif