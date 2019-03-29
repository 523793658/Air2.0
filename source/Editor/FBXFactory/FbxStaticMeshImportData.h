#pragma once
#include "CoreMinimal.h"
#include "FbxImportConfig.h"
#include "FbxMeshImportData.h"
namespace Air
{
	class RStaticMesh;

	namespace EVertexColorImportOption
	{
		enum Type
		{
			Replace,
			Ignore,
			Override,
		};
	}

	class FbxStaticMeshImportData : public FbxMeshImportData
	{
	public:
		wstring mStaticMeshLODGroup;

		TEnumAsByte<EVertexColorImportOption::Type> mVertexColorImportOption;

		Color mVertexOverrideColor;

		uint32 bRemoveDegenerates : 1;

		uint32 bBuildAdjacencyBuffer : 1;

		uint32 bBuildReversedIndexBuffer : 1;

		uint32 bCombineMeshes : 1;

		static FbxStaticMeshImportData* getImportDataForStaticMesh(RStaticMesh* staticMesh, FbxStaticMeshImportData* templateForCreation);


	};


}