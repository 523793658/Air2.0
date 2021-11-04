#pragma once
#include "CoreMinimal.h"
#include "FbxFactory.h"
#include "Containers/Array.h"
#include "Misc/SecureHash.h"
#include "FbxStaticMeshImportData.h"
#include <fbxsdk.h>
#include "Classes/Materials/Material.h"
#include "RawMesh.h"

namespace AirFbx
{

	struct FbxMeshInfo
	{
		wstring name;
		uint64 uniqueId;
		int32 faceNum;
		int32 vertexNum;
		bool bTriangulated;
		int32 materialNum;
		bool bIsSkelMesh;
		wstring skeletonRoot;
		int32 skeletonElemeNum;
		wstring LODGroup;
		wstring LODLevel;
		int32 norphNum;
	};

	struct FbxNodeInfo
	{
		const char* objectName;
		uint64 uniqueId;
		FbxAMatrix transform;
		FbxVector4 rotationPivot;
		FbxVector4 scalePivot;
		const char* attributeName;
		uint64 attributeUniqueId;
		const char* attributeType;

		const char* parentName;
		uint64 parentUniqueId;
	};

	struct FbxSceneInfo
	{
		int32 mNonSkinnedMeshNum;
		int32 SkinnedMeshNum;
		int32 totalGeometryNum;
		int32 totalMaterialNum;
		int32 totalTextureNum;

		TArray<FbxMeshInfo> meshInfo;
		TArray<FbxNodeInfo> hierarchyInfo;

		bool bHasAnimation;
		double frameRate;
		double totalTime;
		void reset()
		{
			mNonSkinnedMeshNum = 0;
			SkinnedMeshNum = 0;
			totalGeometryNum = 0;
			totalMaterialNum = 0;
			totalTextureNum = 0;
			meshInfo.empty();
			hierarchyInfo.empty();
			bHasAnimation = false;
			frameRate = 0.0;
			totalTime = 0.0;
		}
	};

	struct FBXImporterOptions
	{
		bool bConvertScene{ true };
		bool bUsedAsFullName;
		bool bCombineToSingle;
		EVertexColorImportOption::Type mVertexColorImportOption;
		Color mVertexOverrideColor;
		bool bTransformVertexToAbsolute;
		bool bBakePivotInVertex;
	};


	class FbxDataConverter
	{
	public:
		static void setJointPostConversionMatrix(FbxAMatrix conversionMatrix) { mJointPostConversionMatrix = conversionMatrix; }

		static float3 convertPos(FbxVector4 vec);

		static float3 convertDir(FbxVector4 dir);
	private:
		static FbxAMatrix mJointPostConversionMatrix;
	};




#define MAX_FBXInstantce 1

	class AFbxImporter
	{
	public:
		~AFbxImporter();

		AFbxImporter();

		Fbx_API static AFbxImporter* getInstance();

		Fbx_API static void returnInstance(AFbxImporter* instance);

		FBXImporterOptions* getImportOptions() const;

		Fbx_API bool importFromFile(const wstring& filename, const wstring& type, bool bPreventMaterialClash = false);

		Fbx_API std::shared_ptr<RStaticMesh> importStaticMeshAsSingle(TArray<FbxNode*>& meshNodeArray, const wstring inName, EObjectFlags flags, FbxStaticMeshImportData* templateImportData, RStaticMesh* inStaticMesh, int LODIndex = 0, void * existMeshDataPtr = nullptr);

		bool openFile(wstring filename, bool bParseStatistics, bool bForSceneInfo = false);

		bool importFile(wstring filename, bool bPreventMaterialNameClash = false);

		void convertScene();

		void releaseScene();

		void cleanUp();

		int32 getImportType(const wstring & inFilename);

		void applyTransformFromSettingsToFbxNode(FbxNode* node, FbxAssetImportData* assetData);

		int32 getFbxMeshCount(FbxNode* node, bool bCountLODs, int32& outNumLODGroups);

		void fillFbxMeshArray(FbxNode* node, TArray<FbxNode*>& outMeshArray, AFbxImporter* fbxImporter);

		void buildFbxMatrixForImportTransform(FbxAMatrix& outMatrix, FbxAssetImportData* assetData);

	protected:
		struct FbxMaterial
		{
			FbxSurfaceMaterial* mFBXMaterial;
			std::shared_ptr<MaterialInterface> mMaterial;

			wstring getName() const { return mFBXMaterial ? ANSI_TO_TCHAR(mFBXMaterial->GetName()) : TEXT("None"); }
		};

		bool buildStaticMeshFromGeometry(FbxNode* node, RStaticMesh* staticMesh, TArray<FbxMaterial>& meshMaterials, int32 LODIndex, RawMesh& rawMesh, EVertexColorImportOption::Type vertexColorImportOption, const TMap<float3, Color>& existingVertexColorData, const Color& vertexOverrideColor);

		FbxAMatrix computeTotalMatrix(FbxNode* node);

		bool isOddNegativeScale(FbxAMatrix& totalMatrix);

		bool getSceneInfo(wstring filename, FbxSceneInfo& sceneInfo, bool bPreventMaterialNameClash);

		FbxNode* recursiveFindParentLodGroup(FbxNode* parentNode);

		FbxNode* findLodGroupNode(FbxNode* nodeLodGroup, int32 lodIndex, FbxNode* nodeToFind = nullptr);

		ANSICHAR* makeName(const ANSICHAR* name);

		FbxNode* recursiveGetFirstMeshNode(FbxNode* node, FbxNode* nodeToFind);

		wstring makeString(const ANSICHAR* name)
		{
			return wstring(ANSI_TO_TCHAR(name));
		}

		void traverseHierarchyNodeRecursively(FbxSceneInfo& sceneInfo, FbxNode* parentNode, FbxNodeInfo& parentInfo);
	
	private:
		void validateAllMeshesAreReferenceByNodeAttribute();

		void fixMaterialClashName();

		void checkSmoothingInfo(FbxMesh* fbxMesh);

	public:
		static std::shared_ptr<AFbxImporter> mInstances[MAX_FBXInstantce];

	public:
		FbxScene * mScene;
		FBXImporterOptions* mImportOptions;
		MD5Hash mMD5Hash;
		FbxDataConverter mDataConverter;
		FbxGeometryConverter* mGeometryConverter;

		bool bFirstMesh;

		FbxImporter* mImporter;

	protected:
		enum IMPORTPHASE
		{
			NOTSTARTED,
			FILEOPENED,
			IMPORTED,
		};
		
		FbxManager * mSDKManager;
		FbxGeometryConverter* mGoemetryConverter;
		wstring mFileBasePath;

		Object* mParent;

		IMPORTPHASE mCurPhase{ NOTSTARTED };
	private:
		bool bIsBusy = false;

		TArray<wstring> mMeshNamesCache;
	};

	
}