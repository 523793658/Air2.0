#include "FbxStaticMeshImportData.h"
#include "Classes/Engine/StaticMesh.h"
#include "StaticMeshResources.h"
namespace Air
{
	FbxStaticMeshImportData::FbxStaticMeshImportData()
	{
		mStaticMeshLODGroup = Name_None;
		bRemoveDegenerates = true;
		bBuildAdjacencyBuffer = true;
		bBuildReversedIndexBuffer = true;
		bTransformVertexToAbsolute = true;
		mVertexOverrideColor = Color(255, 255, 255, 255);
		bCombineMeshes = true;
	}

	FbxStaticMeshImportData* FbxStaticMeshImportData::getImportDataForStaticMesh(RStaticMesh* staticMesh, FbxStaticMeshImportData* templateForCreation)
	{
		BOOST_ASSERT(staticMesh);
		FbxStaticMeshImportData* importData = check_cast<FbxStaticMeshImportData*>(staticMesh->mAssetImportData);
		if (!importData)
		{
			importData = new FbxStaticMeshImportData();
			if (staticMesh->mAssetImportData != nullptr)
			{
				importData->mSourceData = staticMesh->mAssetImportData->mSourceData;
			}
			staticMesh->mAssetImportData = importData;
		}
		return importData;
	}
}