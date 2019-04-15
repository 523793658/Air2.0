#include "RawMesh.h"
#include "Serialization/BufferReader.h"
#include "Serialization/MemoryWriter.h"
#include "HAL/PlatformMisc.h"
namespace Air
{
	enum
	{
		RAW_MESH_VER = 0,
	};

	Archive& operator<<(Archive& ar, RawMesh& rawMesh)
	{
		int32 version = RAW_MESH_VER;
		ar << version;
		ar << rawMesh.mFaceMaterialIndices;
		ar << rawMesh.mFaceSmoothingMasks;
		ar << rawMesh.mVertexPositions;
		ar << rawMesh.mWedgeIndices;
		ar << rawMesh.mWedgeTangentX;
		ar << rawMesh.mWedgeTangentY;
		ar << rawMesh.mWedgeTangentZ;
		for (int32 i = 0; i < MAX_MESH_TEXTURE_COORDS; ++i)
		{
			ar << rawMesh.mWedgeTexCoords[i];
		}
		ar << rawMesh.mWedgeColors;

		ar << rawMesh.mMaterialIndexToImportIndex;
		return ar;
	}

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

	void RawMeshBulkData::saveRawMesh(struct RawMesh& inMesh)
	{
		TArray<uint8> tempBytes;
		MemoryWriter ar(tempBytes, true);
		ar << inMesh;
		mBulkData.lock(LOCK_READ_WRITE);
		uint8* dest = (uint8*)mBulkData.realloc(tempBytes.size());
		Memory::memcpy(dest, tempBytes.getData(), tempBytes.size());
		mBulkData.unlock();
		PlatformMisc::createGuid(mGuid);
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

	bool RawMesh::isValidOrFixable() const
	{
		int32 numVertices = mVertexPositions.size();
		int32 numWedges = mWedgeIndices.size();
		int32 numFaces = numWedges / 3;
		int32 numTexCoords = mWedgeTexCoords[0].size();
		int32 numFaceSmoothingMasks = mFaceSmoothingMasks.size();
		int32 numFaceMaterialIndices = mFaceMaterialIndices.size();

		bool bValidOrFixable = numVertices > 0
			&& numWedges > 0
			&& numFaces > 0
			&& (numWedges / 3) == numFaces
			&& numFaceMaterialIndices == numFaces
			&& numFaceSmoothingMasks == numFaces
			&& validateArraySize(mWedgeColors, numWedges)
			&& numTexCoords == numWedges;
		for (int32 texcoordIndex = 1; texcoordIndex < MAX_MESH_TEXTURE_COORDS; ++texcoordIndex)
		{
			bValidOrFixable = bValidOrFixable && validateArraySize(mWedgeTexCoords[texcoordIndex], numWedges);
		}
		int32 wedgeIndex = 0; 
		while (bValidOrFixable && wedgeIndex < numWedges)
		{
			bValidOrFixable = bValidOrFixable && (mWedgeIndices[wedgeIndex] < (uint32)numVertices);
			wedgeIndex++;
		}
		return bValidOrFixable;
	}

	void RawMesh::compactMaterialIndices()
	{
		mMaterialIndexToImportIndex.reset();
		if (isValidOrFixable())
		{
			TArray<int32, TInlineAllocator<8>> numTrianglesPerSection;
			int32 numFaces = mFaceMaterialIndices.size();
			for (int32 faceIndex = 0; faceIndex < numFaces; ++faceIndex)
			{
				int32 materialIndex = mFaceMaterialIndices[faceIndex];
				if (materialIndex >= numTrianglesPerSection.size())
				{
					numTrianglesPerSection.addZeroed(materialIndex - numTrianglesPerSection.size() + 1);
				}
				if (materialIndex >= 0)
				{
					numTrianglesPerSection[materialIndex]++;
				}
			}
			TArray<int32, TInlineAllocator<8>> importIndexToMaterialIndex;
			for (int32 sectionIndex = 0; sectionIndex < numTrianglesPerSection.size(); ++sectionIndex)
			{
				int32 newMaterialIndex = INDEX_NONE;
				if (numTrianglesPerSection[sectionIndex] > 0)
				{
					newMaterialIndex = mMaterialIndexToImportIndex.add(sectionIndex);
				}
				importIndexToMaterialIndex.add(newMaterialIndex);
			}
			if (mMaterialIndexToImportIndex.size() != importIndexToMaterialIndex.size())
			{
				for (int32 faceIndex = 0; faceIndex < numFaces; ++faceIndex)
				{
					int32 materialIndex = mFaceMaterialIndices[faceIndex];
					mFaceMaterialIndices[faceIndex] = importIndexToMaterialIndex[materialIndex];

				}
			}
			else
			{
				mMaterialIndexToImportIndex.reset();
			}
		}
	}
}