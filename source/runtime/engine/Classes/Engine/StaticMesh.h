#pragma once
#include "EngineMininal.h"
#include "ObjectMacros.h"
#include "Object.h"
#include "RawMesh.h"
#include "Components.h"
#include "Classes/Engine/EngineType.h"
#include "StaticMeshResources.h"
#include "Classes/Engine/MeshMerging.h"


namespace Air
{
	struct StaticMeshSourceModel
	{
#if WITH_EDITOR
		class RawMeshBulkData* mRawMeshBulkData;
#endif
		ENGINE_API StaticMeshSourceModel();
		ENGINE_API ~StaticMeshSourceModel();

		MeshReductionSettings mReductionSettings;

		float mScreenSize;

		MeshBuildSettings mBuildttings;
	};

	struct StaticMaterial
	{
		StaticMaterial()
			:mMaterialInterface()
			,mMaterialSlotName(Name_None)
			,mImportedMaterialSlotName(Name_None)
		{}

		StaticMaterial(std::shared_ptr<class MaterialInterface>& inMaterialInterface, wstring inMaterialSlotName = Name_None
#if WITH_EDITORONLY_DATA
			, wstring inImportedMaterialSlotName = Name_None
#endif
		)
			:mMaterialInterface(inMaterialInterface)
			, mMaterialSlotName(inMaterialSlotName)
#if WITH_EDITORONLY_DATA
			, mImportedMaterialSlotName(inImportedMaterialSlotName)
#endif
		{

		}
		friend Archive& operator << (Archive& ar, StaticMaterial& elem);

		ENGINE_API friend bool operator == (const StaticMaterial& lhs, const StaticMaterial& rhs);
		ENGINE_API friend bool operator == (const StaticMaterial& lhs, const MaterialInterface& rhs);
		ENGINE_API friend bool operator == (const MaterialInterface& lhs, const StaticMaterial& rhs);

		std::shared_ptr<class MaterialInterface> mMaterialInterface;

		wstring mMaterialSlotName;

#if WITH_EDITORONLY_DATA
		wstring mImportedMaterialSlotName;
#endif

		MeshUVChannelInfo mUVChannelData;
	};

	struct MeshSectionInfo
	{
		int32 mMaterialIndex;

		bool bEnableCollision;

		bool bCastShadow;

		MeshSectionInfo()
			:mMaterialIndex(0)
			,bEnableCollision(false)
			,bCastShadow(true)
		{}

		explicit MeshSectionInfo(int32 inMaterialIndex)
			:mMaterialIndex(inMaterialIndex)
			,bEnableCollision(false)
			,bCastShadow(true)
		{}
	};

	bool operator == (const MeshSectionInfo& a, const MeshSectionInfo& b);

	bool operator != (const MeshSectionInfo& a, const MeshSectionInfo& b);


	struct MeshSectionInfoMap
	{
		TMap<uint32, MeshSectionInfo> mMap;

		void serialize(Archive& ar);

		ENGINE_API MeshSectionInfo get(int32 LODIndex, int32 sectionIndex) const;

		ENGINE_API void set(int32 LODIndex, int32 sectionIndex, MeshSectionInfo info);

		ENGINE_API void remove(int32 LODIndex, int32 sectionIndex);
	};

	class ENGINE_API RStaticMesh : public Object
	{
		GENERATED_RCLASS_BODY(RStaticMesh, Object)
	public:
		virtual void postLoad() override;

		virtual void initResource();

		void updateUVChannelData(bool bRebuildAll);

	private:
		void cacheDerivedData();
	public:

		std::unique_ptr<class StaticMeshRenderData> mRenderData;

#if WITH_EDITORONLY_DATA

		TArray<StaticMeshSourceModel> mSourceModels;

		static const float mMinimumAutoLODPixelError;

		class AssetImportData* mAssetImportData;

#endif

		uint32 bLODsShareStaticLighting : 1;

		uint32 bAutoComputeLODScreenSize : 1;

		Guid mLightingGuid;

		TArray<StaticMaterial> mStaticMaterials;

		MeshSectionInfoMap mSectionInfoMap;

		wstring mLODGroup;

		int32 mMinLOD;

		bool bAllowCPUAccess;
	};
}