#pragma once
#include "Classes/EditorFramework/AssetImportData.h"
#include "FbxSceneImportData.h"
#include "config.h"
namespace Air
{
	class Fbx_API FbxAssetImportData : public AssetImportData
	{
		float3 mImportTranslation;

		Rotator mImportRotation;

		float mImportConstantScale;

		bool bConvertScene{ true };

		bool bForceFrontXAxis;

		bool bConvertSceneUnit;

		bool bImportAsSene;

		FbxSceneImportData* mFbxSceneImportDataReference;
	};
}