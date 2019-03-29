#include "RawMesh.h"
#include "Serialization/BufferReader.h"
namespace Air
{
	RawMeshBulkData::RawMeshBulkData()
		:bGuidIsHash(false)
	{}

	void RawMeshBulkData::loadRawMesh(struct RawMesh& outMesh)
	{
		outMesh.empty();

		if (mBulkData.getElementCount() > 0)
		{
			BufferReader ar(
				mBulkData.lock(LOCK_READ_ONLY), mBulkData.getElementCount(), false, true);
			ar << outMesh;
			mBulkData.unlock();
		}
	}

	void RawMesh::empty()
	{
		mFaceMaterialIndices.empty();
		mFaceSmoothingMasks.empty();
		mVertexPositions.empty();
		mWedgeIndices.empty();
		mWedgeTangentX.empty();
		mWedgeTangentY.empty();
		mWedgeTangentZ.empty();
		mWedgeColors.empty();
		for (int32 i = 0; i < MAX_MESH_TEXTURE_COORDS; ++i)
		{
			mWedgeTexCoords[i].empty();
		}
	}

	template<typename ArrayType>
	bool validateArraySize(ArrayType const & arr, int32 expectedSize)
	{
		return arr.size() == 0 || arr.size() == expectedSize;
	}

	bool RawMesh::isValid() const
	{
		int32 numVertices = mVertexPositions.size();
		int32 numWedges = mWedgeIndices.size();
		int32 numFaces = numWedges / 3;
		bool bValid = numVertices > 0
			&& numWedges > 0
			&& numFaces > 0
			&& (numWedges / 3) == numFaces
			&& validateArraySize(mFaceMaterialIndices, numFaces)
			&& validateArraySize(mFaceSmoothingMasks, numFaces)
			&& validateArraySize(mWedgeTangentX, numWedges)
			&& validateArraySize(mWedgeTangentY, numWedges)
			&& validateArraySize(mWedgeTangentZ, numWedges)
			&& validateArraySize(mWedgeColors, numWedges)
			&& mWedgeTexCoords[0].size() == numWedges;
		for (int32 texcoordIndex = 1; texcoordIndex < MAX_MESH_TEXTURE_COORDS; ++texcoordIndex)
		{
			bValid = bValid && validateArraySize(mWedgeTexCoords[texcoordIndex], numWedges);
		}
		int32 wedgeIndex = 0; 
		while (bValid && wedgeIndex < numWedges)
		{
			bValid = bValid && (mWedgeIndices[wedgeIndex] < (uint32)numVertices);
			wedgeIndex++;
		}
		return bValid;

	}

	void RawMesh::compactMaterialIndices()
	{
		mMaterialIndexToImportIndex.reset();
		if(isvaof)
	}
}