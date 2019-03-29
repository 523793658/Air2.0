#pragma once
#include "EngineMininal.h"
#include "Serialization/bulkData.h"
#include "misc/Guid.h"
namespace Air
{
	enum
	{
		MAX_MESH_TEXTURE_COORDS = 8,
	};

	struct RawMesh
	{
		TArray<int32> mFaceMaterialIndices;
		TArray<uint32> mFaceSmoothingMasks;

		TArray<float3> mVertexPositions;

		TArray<uint32> mWedgeIndices;

		TArray<float3> mWedgeTangentX;

		TArray<float3> mWedgeTangentY;

		TArray<float3> mWedgeTangentZ;

		TArray<float2> mWedgeTexCoords[MAX_MESH_TEXTURE_COORDS];

		TArray<Color> mWedgeColors;

		TArray<int32> mMaterialIndexToImportIndex;

		ENGINE_API void empty();

		ENGINE_API bool isValid() const;

		FORCEINLINE float3 getWedgePosition(int32 wedgeIndex) const
		{
			return mVertexPositions[mWedgeIndices[wedgeIndex]];
		}

		void compactMaterialIndices();
	};


	class RawMeshBulkData
	{
		ByteBulkData mBulkData;
		Guid mGuid;
		bool bGuidIsHash;

	public:
		ENGINE_API RawMeshBulkData();
#if WITH_EDITORONLY_DATA
		FORCEINLINE bool isEmpty() const
		{
			return mBulkData.getBulkDataSize() == 0;
		}

		ENGINE_API void loadRawMesh(struct RawMesh& outMesh);

		ENGINE_API void saveRawMesh(struct RawMesh& outMesh);
#endif

	};
}