#include "Classes/Engine/StaticMesh.h"
#include "StaticMeshResources.h"
#include "SimpleReflection.h"
#include "Misc/App.h"
namespace Air
{
	RStaticMesh::RStaticMesh(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{

	}

	bool operator == (const MeshSectionInfo& a, const MeshSectionInfo& b)
	{
		return a.mMaterialIndex == b.mMaterialIndex
			&& a.bEnableCollision == b.bEnableCollision
			&& a.bCastShadow == b.bCastShadow;
	}

	bool operator !=(const MeshSectionInfo& a, const MeshSectionInfo& b)
	{
		return !(a == b);
	}

	static uint32 getMeshMaterialKey(int32 LODIndex, int32 sectionIndex)
	{
		return ((LODIndex & 0xffff) << 16) | (sectionIndex & 0xffff);
	}

	MeshSectionInfo MeshSectionInfoMap::get(int32 LODIndex, int32 sectionIndex) const
	{
		uint32 key = getMeshMaterialKey(LODIndex, sectionIndex);
		auto &infoIt = mMap.find(key);
		if (infoIt == mMap.end())
		{
			key = getMeshMaterialKey(0, sectionIndex);
			infoIt = mMap.find(key);
		}
		if (infoIt == mMap.end())
		{
			return MeshSectionInfo(sectionIndex);
		}
		return infoIt->second;
	}

	void MeshSectionInfoMap::set(int32 LODIndex, int32 sectionIndex, MeshSectionInfo info)
	{
		uint32 key = getMeshMaterialKey(LODIndex, sectionIndex);
		mMap.emplace(key, info);
	}

	bool operator == (const StaticMaterial& lhs, const StaticMaterial& rhs)
	{
		return (lhs.mMaterialInterface == rhs.mMaterialInterface &&
			lhs.mMaterialSlotName == rhs.mMaterialSlotName
#if WITH_EDITORONLY_DATA
			&& lhs.mImportedMaterialSlotName == rhs.mImportedMaterialSlotName
#endif
			);
	}

	bool operator == (const StaticMaterial& lhs, const MaterialInterface& rhs)
	{
		return (lhs.mMaterialInterface == &rhs);
	}

	bool operator == (const MaterialInterface& lhs, const StaticMaterial& rhs)
	{
		return (&lhs == rhs.mMaterialInterface);
	}

	void MeshSectionInfoMap::remove(int32 LODIndex, int32 sectionIndex)
	{
		uint32 key = getMeshMaterialKey(LODIndex, sectionIndex);
		mMap.erase(key);
	}

	StaticMeshSourceModel::StaticMeshSourceModel()
	{
#if WITH_EDITOR
		mRawMeshBulkData = new RawMeshBulkData();
#endif
	}

	StaticMeshSourceModel::~StaticMeshSourceModel()
	{
#if WITH_EDITOR
		if (mRawMeshBulkData)
		{
			delete mRawMeshBulkData;
			mRawMeshBulkData = nullptr;
		}
#endif

		
	}

	void RStaticMesh::postLoad()
	{
		if (App::canEverRender() && !hasAnyFlags(RF_ClassDefaultObject))
		{
			initResource();
		}
	}

	void RStaticMesh::initResource()
	{
		updateUVChannelData(false);
		if (mRenderData)
		{
			mRenderData->initResource(this);
		}
	}

	void RStaticMesh::updateUVChannelData(bool bRebuildAll)
	{

	}

	DECLARE_SIMPLER_REFLECTION(RStaticMesh);
}