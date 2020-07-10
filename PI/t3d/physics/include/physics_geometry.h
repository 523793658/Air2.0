#ifndef _Physics_Geometry_H_
#define _Physics_Geometry_H_
#include "pi_lib.h"
#include "physics_mesh.h"
typedef enum
{
	GT_Box,
	GT_Plane,
	GT_Sphere,
	GT_Capsule,
	GT_HeightField,
	GT_TriangleMesh
}PiGeometryType;

typedef struct
{
	PiGeometryType type;
	float width;
	float height;
	float depth;
	float radius;
	void* buffer;
	void* impl;
}PiGeometry;

PI_BEGIN_DECLS

PiGeometry* PI_API pi_physics_factory_create_box_geometry(float width, float height, float depth);

PiGeometry* PI_API pi_physics_factory_create_sphere_geometry(float radius);

PiGeometry* PI_API pi_physics_factory_create_capsule_geometry(float radius, float height);

PiGeometry* PI_API pi_physics_factory_create_plane_geometry();

PiGeometry* PI_API pi_physics_factory_create_height_field_geometry(HeightField* height_field);

void PI_API pi_physics_geometry_free(PiGeometry* geometry);

PiGeometry* PI_API pi_physics_geometry_triangle_mesh_create(PhysicsTriangleMesh* mesh);

PI_END_DECLS



#endif