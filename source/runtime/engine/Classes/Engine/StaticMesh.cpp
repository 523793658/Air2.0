#include "Classes/Engine/StaticMesh.h"
#include "StaticMeshResources.h"
#include "SimpleReflection.h"
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

	DECLARE_SIMPLER_REFLECTION(RStaticMesh);
}