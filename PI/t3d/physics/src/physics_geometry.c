#include "physics_geometry.h"
#include "physics_wrap.h"

PiGeometry* PI_API pi_physics_geometry_triangle_mesh_create(PhysicsTriangleMesh* mesh)
{
	PiGeometry* geometry = pi_new0(PiGeometry, 1);
	geometry->buffer = mesh;
	geometry->type = GT_TriangleMesh;
	physics_geometry_init(geometry);
	return geometry;
}

PiGeometry* PI_API pi_physics_factory_create_box_geometry(float width, float height, float depth)
{
	PiGeometry* g = pi_new(PiGeometry, 1);
	g->width = width;
	g->height = height;
	g->depth = depth;
	g->type = GT_Box;
	physics_geometry_init(g);
	return g;
}

PiGeometry* PI_API pi_physics_factory_create_sphere_geometry(float radius)
{
	PiGeometry* g = pi_new(PiGeometry, 1);
	g->radius = radius;
	g->type = GT_Sphere;
	physics_geometry_init(g);
	return g;

}

PiGeometry* PI_API pi_physics_factory_create_capsule_geometry(float radius, float height)
{
	PiGeometry* g = pi_new(PiGeometry, 1);
	g->radius = radius;
	g->height = height;
	g->type = GT_Capsule;
	physics_geometry_init(g);
	return g;
}

PiGeometry* PI_API pi_physics_factory_create_plane_geometry()
{
	PiGeometry* g = pi_new(PiGeometry, 1);
	g->type = GT_Plane;
	physics_geometry_init(g);
	return g;
}

void PI_API pi_physics_geometry_free(PiGeometry* geometry)
{
	physics_geometry_release(geometry);
	pi_free(geometry);
}

PiGeometry* PI_API pi_physics_factory_create_height_field_geometry(HeightField* height_field)
{
	PiGeometry* g = pi_new(PiGeometry, 1);
	g->buffer = height_field;
	g->type = GT_HeightField;
	physics_geometry_init(g);
	return g;
}