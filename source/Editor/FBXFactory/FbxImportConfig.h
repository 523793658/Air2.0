#pragma once
#include "CoreMinimal.h"
#include "Containers/EnumAsByte.h"
#include "FbxStaticMeshImportData.h"
namespace Air
{
	enum EFBXImportType
	{
		FBXIT_StaticMesh,
		FBXIT_SkeletalMesh,
		FBXIT_Animation,
		FBXIT_SubDSurface,
		FBXIT_Max,
	};

	class FbxImportConfig
	{
	public:
		FbxImportConfig();

		wstring mFileName;
		
		TEnumAsByte<enum EFBXImportType> mOriginalImportType;
		
		TEnumAsByte<enum EFBXImportType> mMeshTypeToImport;

		bool bImportAsSkeletal;

		bool bImportAsSubDSurface;

		bool bUsedAsFullName;

		class FbxStaticMeshImportData* mStaticMeshImportData;

		void setMeshTypeToImport()
		{
			mMeshTypeToImport = bImportAsSkeletal ? FBXIT_SkeletalMesh : FBXIT_StaticMesh;
			if (bImportAsSubDSurface)
			{
				mMeshTypeToImport = FBXIT_SubDSurface;
			}
		}

	};
}