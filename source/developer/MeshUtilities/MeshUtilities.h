#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Classes/Engine/StaticMesh.h"
namespace Air
{
	namespace ETangentOptions
	{
		enum Type
		{
			None = 0,
			BlendOverlappingNormals = 0x1,
			IgnoreDegenerateTriangles = 0x2,
		};
	}

	class StaticMeshRenderData;


	class IMeshReduction
	{

	};

	class IMeshUtilities : public IModuleInterface
	{
	public:
		virtual bool buildStaticMesh(StaticMeshRenderData& outRenderData, TArray<StaticMeshSourceModel>& sourceModels, const StaticMeshLODGroup& LODGroup) = 0;

		virtual void buildStaticMeshVertexAndIndexBuffers(TArray<StaticMeshBuildVertex>& outVertices,
			TArray<TArray<uint32>>& outPerSectionIndices,
			TArray<int32>& outWedgeMap,
			const RawMesh& rawMesh,
			const std::multimap<int32, int32>& overlappingCorners,
			const TMap<uint32, uint32>& materialToSectionMapping,
			float comparisonThreshold,
			float3 buildScale) = 0;
	};


	
}