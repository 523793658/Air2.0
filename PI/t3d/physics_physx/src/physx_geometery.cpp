#include "physics_wrap.h"
#include "PxBoxGeometry.h"
#include "PxSphereGeometry.h"
#include "PxCapsuleGeometry.h"
#include "PxPlaneGeometry.h"
#include "PxDefaultStreams.h"
#include "PxHeightField.h"
#include "physx_engine.h"
#include "PxHeightFieldGeometry.h"
using namespace physx;

extern "C"
{

	void PI_API physics_geometry_init(PiGeometry* g)
	{
		switch (g->type)
		{
		case GT_Box:
			g->impl = new PxBoxGeometry(g->width, g->height, g->depth);
		break;

		case GT_Sphere:
			g->impl = new PxSphereGeometry(g->radius);
		break;

		case GT_Capsule:
			g->impl = new PxCapsuleGeometry(g->radius, g->height);
			break;

		case GT_Plane:
			g->impl = new PxPlaneGeometry();
			break;
		case GT_HeightField:
		{
			HeightField* height_field = static_cast<HeightField*>(g->buffer);
			PxHeightField* hf = static_cast<PxHeightField*>(height_field->impl);
			g->impl = new PxHeightFieldGeometry(hf, PxMeshGeometryFlags(), 0.05f, 1.0f, 1.0f);
		}
		break;
		case GT_TriangleMesh:
		{
			PxTriangleMesh* mesh = static_cast<PxTriangleMesh*>(g->buffer);
			g->impl = new PxTriangleMeshGeometry(mesh);
		}
		break;
		default:
			break;
		}
	}


	void PI_API physics_geometry_release(PiGeometry* g)
	{
		PxGeometry* impl = static_cast<PxGeometry*>(g->impl);
		delete impl;
	}







}

