#include "PxHeightFieldDesc.h"
#include "PxHeightFieldSample.h"
#include "PxDefaultStreams.h"
#include "physx_offline_tools.h"
#include "physics_wrap.h"
#include "PxSimpleFactory.h"
#include "PxCoreUtilityTypes.h"
#include "physx_engine.h"
#include "PxScene.h"
#include "physx_scene.h"
#include "physics_offline_tools.h"
extern "C"
{

	void PI_API physics_triangle_mesh_free(PhysicsTriangleMesh* mesh)
	{
		PxTriangleMesh* tmesh = static_cast<PxTriangleMesh*>(mesh->impl);
		tmesh->release();
	}

void PI_API physics_cook_triangle_mesh(PiMesh* mesh, PhysicsTriangleMesh* triangleMesh)
{
	PhysXEngine* engine = PhysXEngine::getInstance();
	PxTriangleMeshDesc desc;
	VertexElement *r = pi_renderdata_get_vertex(&mesh->data, EVS_POSITION);
	uint16_t materialIndex = 0;

	desc.points.count = mesh->data.vertex_num;
	desc.points.stride = pi_get_vertex_type_size(r->type, r->num);
	desc.points.data = r->data;




	desc.triangles.data = mesh->data.idata.data;
	desc.triangles.count = mesh->data.idata.num / 3;
	desc.triangles.stride = (mesh->data.idata.type == EINDEX_16BIT ? sizeof(uint16_t) : sizeof(uint32_t)) * 3;

	desc.materialIndices.data = &materialIndex;
	desc.materialIndices.stride = sizeof(uint16_t);
	desc.flags = (PxMeshFlags)0;
	PxCooking& cooking = engine->getCooking();
	const PxCookingParams currentParams = cooking.getParams();
	PxCookingParams newParams = currentParams;

	//不需要碰撞返回mesh数据所以直接设置成true， 如果需要就设置成false
	newParams.suppressTriangleMeshRemapTable = true;
	cooking.setParams(newParams);

	PxTriangleMesh* impl = cooking.createTriangleMesh(desc, engine->getPhysics().getPhysicsInsertionCallback());

	cooking.setParams(currentParams);
	triangleMesh->impl = impl;
}


uint PI_API physics_cook_terrain(uint32 nx, uint32 nz, float* heightBuffer, void** output_data)
{
	PhysXEngine* engine = PhysXEngine::getInstance();
	PxHeightFieldDesc hfDesc;
	hfDesc.format = PxHeightFieldFormat::eS16_TM;
	hfDesc.nbColumns = nz;
	hfDesc.nbRows = nx;
	hfDesc.samples.stride = sizeof(PxHeightFieldSample);
	hfDesc.samples.data = pi_malloc0(hfDesc.samples.stride * nx * nz);


	PxTriangleMeshDesc dsc;

	PxHeightFieldSample* sample = (PxHeightFieldSample*)(hfDesc.samples.data);
	for (uint32 i = 0; i < nz; i++)
	{
		for (uint32 j = 0; j < nx; j++)
		{
			sample[j * nz + i].height = (PxI16)(heightBuffer[i * nx + j] * 20);
		}
	}
	PxDefaultMemoryOutputStream outputStream;
	engine->getCooking().cookHeightField(hfDesc, outputStream);
	void* data = pi_malloc(outputStream.getSize());
	pi_memcpy_inline(data, outputStream.getData(), outputStream.getSize());
	pi_free(sample);
	*output_data = data;
	return outputStream.getSize();
}

}
