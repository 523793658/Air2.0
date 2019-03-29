#pragma once
#include "Containers/EnumAsByte.h"
#include "FbxAssetImportData.h"
namespace Air
{
	enum EFBXNormalImportMethod
	{
		FBXNIM_ComputeNormals,
		FBXNIM_ImportNormals,
		FBXNIM_ImportNormalsAndTangents,
		FBXNIM_MAX,
	};

	namespace EFBXNormalGenerationMethod
	{
		enum Type
		{
			Builtin,
			MikkTSpace,
		};
	}
	
	class FbxMeshImportData : public FbxAssetImportData
	{
	public:
		bool bTransformVertexToAbsolute;

		uint32 bImportMeshLODs : 1;

		TEnumAsByte<enum EFBXNormalImportMethod> mNormalImportMethod;

		TEnumAsByte<enum EFBXNormalGenerationMethod::Type> mNormalGenerationMethod;
	};
}