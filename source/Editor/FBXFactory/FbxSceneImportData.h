#pragma once
#include "config.h"
namespace Air
{
	class Fbx_API FbxSceneImportData
	{
		wstring mSourceFbxFile;

		bool bImportScene;

		bool bCreateFolderHierachy;
	};
}