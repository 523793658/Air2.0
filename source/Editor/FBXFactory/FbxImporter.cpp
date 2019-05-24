#include "FbxImporter.h"
#include "HAL/PlatformProcess.h"
#include "FBX/2019.2/include/fbxsdk.h"
#include "boost/algorithm/string.hpp"
#include "Misc/Paths.h"
#include "Containers/Set.h"
#include "boost/lexical_cast.hpp"
#include "Classes/Engine/StaticMesh.h"
#include "FbxStaticMeshImportData.h"
#include "StaticMeshResources.h"
#include "Misc/FileHelper.h"

namespace AirFbx
{
	static const int32 LARGE_MESH_MATERIAL_INDEX_THRESHOLD = 64;

	std::shared_ptr<AFbxImporter> AFbxImporter::mInstances[MAX_FBXInstantce];

	FbxAMatrix FbxDataConverter::mJointPostConversionMatrix;

	struct FFBXUVs
	{
		FFBXUVs(FbxMesh* mesh)
			:mUniqueUVCount(0)
		{
			BOOST_ASSERT(mesh);
			int32 layerCount = mesh->GetLayerCount();
			if (layerCount > 0)
			{
				int32 uvLayerIndex;
				for (uvLayerIndex = 0; uvLayerIndex < layerCount; uvLayerIndex++)
				{
					FbxLayer* layer = mesh->GetLayer(uvLayerIndex);
					int uvSetCount = layer->GetUVSetCount();
					if (uvSetCount)
					{
						FbxArray<FbxLayerElementUV const*> eleUVs = layer->GetUVSets();
						for (int32 uvIndex = 0; uvIndex < uvSetCount; uvIndex++)
						{
							FbxLayerElementUV const * elementUV = eleUVs[uvIndex];
							if (elementUV)
							{
								const char* uvSetName = elementUV->GetName();
								wstring localUVSetName = UTF8_TO_TCHAR(uvSetName);
								if (localUVSetName.empty())
								{
									localUVSetName = TEXT("UVmap_") + boost::lexical_cast<wstring>(uvLayerIndex);
								}
								mUVSets.addUnique(localUVSetName);
							}
						}
					}
				}
			}

			if (mUVSets.size())
			{
				for (int32 channelNumIdx = 0; channelNumIdx < 4; channelNumIdx++)
				{
					wstring channelName = printf(TEXT("UVChannel_%d"), channelNumIdx + 1);
					int32 setIdx = mUVSets.find(channelName);
					if (setIdx != INDEX_NONE && setIdx != channelNumIdx)
					{
						for (int32 arrSize = mUVSets.size(); arrSize < channelNumIdx + 1; arrSize++)
						{
							mUVSets.add(TEXT(""));
						}
						mUVSets.swap(setIdx, channelNumIdx);
					}
				}
			}
		}

		void phase2(FbxMesh* mesh)
		{
			mUniqueUVCount = mUVSets.size();
			if (mUniqueUVCount > 0)
			{
				mLayerElementUV.addZeroed(mUniqueUVCount);
				mUVReferenceNode.addZeroed(mUniqueUVCount);
				mUVMappingMode.addZeroed(mUniqueUVCount);
			}
			for (int32 uvIndex = 0; uvIndex < mUniqueUVCount; uvIndex++)
			{
				mLayerElementUV[uvIndex] = nullptr;
				for (int32 uvLayerIndex = 0, layerCount = mesh->GetLayerCount(); uvLayerIndex < layerCount; uvLayerIndex++)
				{
					FbxLayer* layer = mesh->GetLayer(uvLayerIndex);
					int uvSetCount = layer->GetUVSetCount();
					if (uvSetCount)
					{
						FbxArray<FbxLayerElementUV const*> eleUVs = layer->GetUVSets();
						for (int32 fbxUVIndex = 0; fbxUVIndex < uvSetCount; fbxUVIndex++)
						{
							FbxLayerElementUV const* elementUV = eleUVs[fbxUVIndex];
							if (elementUV)
							{
								const char* uvSetName = elementUV->GetName();
								wstring localUVSetName = UTF8_TO_TCHAR(uvSetName);
								if (localUVSetName.empty())
								{
									localUVSetName = TEXT("UVmap_") + boost::lexical_cast<wstring>(uvLayerIndex);
								}
								if (localUVSetName == mUVSets[uvIndex])
								{
									mLayerElementUV[uvIndex] = elementUV;
									mUVReferenceNode[uvIndex] = elementUV->GetReferenceMode();
									mUVMappingMode[uvIndex] = elementUV->GetMappingMode();
									break;
								}
							}
						}
					}
				}
			}
			mUniqueUVCount = Math::min<int32>(mUniqueUVCount, MAX_MESH_TEXTURE_COORDS);
		}

		int32 findLightUVIndex() const {
			for (int32 uvSetIndex = 0; uvSetIndex < mUVSets.size(); uvSetIndex++)
			{
				if (mUVSets[uvSetIndex] == TEXT("LightMapUV"))
				{
					return uvSetIndex;
				}
			}
			return INDEX_NONE;
		}

		int32 computeUVIndex(int32 uvLayerIndex, int32 controlPointIndex, int32 faceCornerIndex) const
		{
			int32 uvMapIndex = (mUVMappingMode[uvLayerIndex] == FbxLayerElement::eByControlPoint) ? controlPointIndex : faceCornerIndex;
			int32 ret;

			if (mUVReferenceNode[uvLayerIndex] == FbxLayerElement::eDirect)
			{
				ret = uvMapIndex;
			}
			else
			{
				FbxLayerElementArrayTemplate<int>& arr = mLayerElementUV[uvLayerIndex]->GetIndexArray();
				ret = arr.GetAt(uvMapIndex);
			}
			return ret;
		}

		void cleanup()
		{
			mLayerElementUV.empty();
			mUVReferenceNode.empty();
			mUVMappingMode.empty();
		}


		TArray<wstring> mUVSets;

		TArray<FbxLayerElementUV const*> mLayerElementUV;
		TArray<FbxLayerElement::EReferenceMode> mUVReferenceNode;
		TArray<FbxLayerElement::EMappingMode> mUVMappingMode;
		int32 mUniqueUVCount;
	};

	AFbxImporter::AFbxImporter()
		:mScene(nullptr)
		, mImportOptions(nullptr)
		, mGoemetryConverter(nullptr)
		, mSDKManager(nullptr)
		, mImporter(nullptr)
		, bFirstMesh(true)
	{
		mSDKManager = FbxManager::Create();
		FbxIOSettings *ios = FbxIOSettings::Create(mSDKManager, IOSROOT);
		mSDKManager->SetIOSettings(ios);
		mGoemetryConverter = new FbxGeometryConverter(mSDKManager);
		mScene = nullptr;

		mImportOptions = new FBXImporterOptions();

		Memory::memzero(*mImportOptions);
		mImportOptions->bConvertScene = true;
		mImportOptions->bTransformVertexToAbsolute = true;
	}




	AFbxImporter* AFbxImporter::getInstance()
	{
		static std::mutex mutex;
		std::lock_guard<std::mutex> lock(mutex);
		while (true)
		{
			for (int i = 0; i < MAX_FBXInstantce; i++)
			{
				if (!mInstances[i])
				{
					mInstances[i] = MakeSharedPtr<AFbxImporter>();
					mInstances[i]->bIsBusy = true;
					return mInstances[i].get();
				}
				if (!mInstances[i]->bIsBusy)
				{
					return mInstances[i].get();
				}
			}
			PlatformProcess::sleep(0.1);
		}
	}

	void AFbxImporter::returnInstance(AFbxImporter* instance)
	{
		instance->bIsBusy = false;
	}

	FBXImporterOptions* AFbxImporter::getImportOptions() const
	{
		return mImportOptions;
	}

	bool AFbxImporter::importFromFile(const wstring& filename, const wstring& type, bool bPreventMaterialClash /* = false */)
	{
		bool result = true;

		switch (mCurPhase)
		{
		case NOTSTARTED:
			if (!openFile(filename, false))
			{
				result = false;
				break;
			}
		case FILEOPENED:
			if (!importFile(filename, bPreventMaterialClash))
			{
				result = false;
				mCurPhase = NOTSTARTED;
			}
		case IMPORTED:
		{
			static const wstring obj(TEXT("obj"));
			if (!boost::iequals(obj, type))
			{
				convertScene();

				FbxDocumentInfo* docInfo = mScene->GetSceneInfo();
				if (docInfo)
				{
				}

			}
			validateAllMeshesAreReferenceByNodeAttribute();
			mMeshNamesCache.empty();
		}
			break;
		default:
			break;
		}

		return result;
	}

	bool AFbxImporter::openFile(wstring filename, bool bParseStatistics, bool bForSceneInfo)
	{
		bool result = true;
		int32 sdkMajor, sdkMinor, sdkRevision;

		mImporter = FbxImporter::Create(mSDKManager, "");

		FbxManager::GetFileFormatVersion(sdkMajor, sdkMinor, sdkRevision);

		if (bParseStatistics)
		{
			mImporter->ParseForStatistics(true);
		}
		wstring abs = Paths::getAbsolutePath(filename);
		const bool bImportStatus = mImporter->Initialize(TCHAR_TO_UTF8(abs.c_str()));
		if (!bImportStatus)
		{
			AIR_LOG(logFBX, Error, TEXT("Call to FbxImporter::Initialize() failed."));

			AIR_LOG(LogFBX, Warning, TEXT("Error returned: %s"), UTF8_TO_TCHAR(mImporter->GetStatus().GetErrorString()));
			if (mImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
			{
				AIR_LOG(LogFBX, Warning, TEXT("FBX version number for this fbx sdk is %d.%d.%d"), sdkMajor, sdkMinor, sdkRevision);
			}
			return false;
		}

		if (!bParseStatistics && !bForSceneInfo)
		{
			int32 fileMajor, fileMinor, fileRevision;
			mImporter->GetFileVersion(fileMajor, fileMinor, fileRevision);

			int32 fileVersion = (fileMajor << 16 | fileMinor << 8 | fileRevision);
			int32 sdkVersion = (sdkMajor << 16 | sdkMinor << 8 | sdkRevision);
			if (fileVersion != sdkVersion)
			{
				wstring configStr = printf(TEXT("Warning_OutOfDataFBX_%d"), sdkVersion);
			}
		}
		mMD5Hash = MD5Hash::hashFile(filename.c_str());
		
		mCurPhase = FILEOPENED;

		return result;
	}

#ifdef IOS_REF
#undef IOS_REF
#define IOS_REF (*(mSDKManager->GetIOSettings()))
#endif

	bool AFbxImporter::importFile(wstring filename, bool bPreventMaterialNameClash)
	{
		bool result = true;
		bool bStatus;
		mFileBasePath = Paths::getPath(filename);
		mScene = FbxScene::Create(mSDKManager, "");
		AIR_LOG(LogFBX, Log, TEXT("Loading FBX Scene from %s"), filename.c_str());

		int32 fileMajor, fileMinor, fileRevision;

		IOS_REF.SetBoolProp(IMP_FBX_MATERIAL, true);
		IOS_REF.SetBoolProp(IMP_FBX_TEXTURE, true);
		IOS_REF.SetBoolProp(IMP_FBX_LINK, true);
		IOS_REF.SetBoolProp(IMP_FBX_SHAPE, true);
		IOS_REF.SetBoolProp(IMP_FBX_GOBO, true);
		IOS_REF.SetBoolProp(IMP_FBX_ANIMATION, true);
		IOS_REF.SetBoolProp(IMP_SKINS, true);
		bStatus = mImporter->Import(mScene);
		if (bPreventMaterialNameClash)
		{
			fixMaterialClashName();
		}

		mImporter->GetFileVersion(fileMajor, fileMinor, fileRevision);

		if (bStatus)
		{
			mCurPhase = IMPORTED;

			mImporter->Destroy();
			mImporter = nullptr;
		}
		else
		{
			releaseScene();
			result = false;
			mCurPhase = NOTSTARTED;
		}
		return result;
	}

	void AFbxImporter::convertScene()
	{
		if (getImportOptions()->bConvertScene)
		{
			FbxAxisSystem::ECoordSystem coordSystem = FbxAxisSystem::eLeftHanded;
			FbxAxisSystem::EUpVector upVector = FbxAxisSystem::eYAxis;
			FbxAxisSystem::EFrontVector frontVector = (FbxAxisSystem::EFrontVector) - FbxAxisSystem::eParityOdd;
			FbxAxisSystem airImportAxis(upVector, frontVector, coordSystem);
			FbxAxisSystem sourceSetup = mScene->GetGlobalSettings().GetAxisSystem();
			if (sourceSetup != airImportAxis)
			{
				FbxRootNodeUtility::RemoveAllFbxRoots(mScene);
				airImportAxis.ConvertScene(mScene);
				FbxAMatrix jointOrientationMatrix;
				jointOrientationMatrix.SetIdentity();
				FbxDataConverter::setJointPostConversionMatrix(jointOrientationMatrix);
			}
		}

		if (getImportOptions()->bConvertScene && mScene->GetGlobalSettings().GetSystemUnit() != FbxSystemUnit::m)
		{
			FbxSystemUnit::m.ConvertScene(mScene);
		}
	}

	AFbxImporter::~AFbxImporter()
	{
		cleanUp();
	}

	void AFbxImporter::cleanUp()
	{
		releaseScene();
		delete mGoemetryConverter;
		mGoemetryConverter = nullptr;
		delete mImportOptions;
		mImportOptions = nullptr;
		if (mSDKManager)
		{
			mSDKManager->Destroy();
		}
		mSDKManager = nullptr;
	}

	void AFbxImporter::validateAllMeshesAreReferenceByNodeAttribute()
	{
		for (int geoIndex = 0; geoIndex < mScene->GetGeometryCount(); ++geoIndex)
		{
			bool bFoundOneGeometryLinkToANode = false;
			FbxGeometry* geometry = mScene->GetGeometry(geoIndex);
			for (int nodeIndex = 0; nodeIndex < mScene->GetNodeCount(); ++nodeIndex)
			{
				FbxNode* sceneNode = mScene->GetNode(nodeIndex);
				FbxGeometry* nodeGeometry = static_cast<FbxGeometry*>(sceneNode->GetMesh());
				if (nodeGeometry && nodeGeometry->GetUniqueID() == geometry->GetUniqueID())
				{
					bFoundOneGeometryLinkToANode = true;
					break;
				}
			}
		}
	}

	void AFbxImporter::fixMaterialClashName()
	{
		FbxArray<FbxSurfaceMaterial*> materialArray;
		mScene->FillMaterialArray(materialArray);
		TSet<wstring> allMaterialName;
		for (int32 materialIndex = 0; materialIndex < materialArray.Size(); ++materialIndex)
		{
			FbxSurfaceMaterial* material = materialArray[materialIndex];
			wstring materialName = UTF8_TO_TCHAR(material->GetName());
			if (allMaterialName.contains(materialName))
			{
				wstring originalMaterialName = materialName;
				wstring MaterialBaseName = materialName + TEXT(NAMECLASH1_KEY);
				int32 nameIndex = 1;
				materialName = MaterialBaseName + boost::lexical_cast<wstring>(nameIndex++);
				while (allMaterialName.contains(materialName))
				{
					materialName = MaterialBaseName + boost::lexical_cast<wstring>(nameIndex++);
				}
				material->SetName(TCHAR_TO_UTF8(materialName.c_str()));
			}
			allMaterialName.add(materialName);
		}
	}

	void AFbxImporter::releaseScene()
	{
		if (mImporter)
		{
			mImporter->Destroy();
			mImporter = nullptr;
		}
		if (mScene)
		{
			mScene->Destroy();
			mScene = nullptr;
		}
		bFirstMesh = true;

	}

	RStaticMesh* AFbxImporter::importStaticMeshAsSingle(TArray<FbxNode *>& meshNodeArray, const wstring inName, EObjectFlags flags, FbxStaticMeshImportData* templateImportData, RStaticMesh* inStaticMesh, int LODIndex, void* existMeshDataPtr /* = nullptr */)
	{
		bool bBuildStatus = true;
		if (meshNodeArray.size() == 0)
		{
			return nullptr;
		}

		int32 numVerts = 0;
		int32 meshIndex = 0; 
		for (meshIndex = 0; meshIndex < meshNodeArray.size(); meshIndex++)
		{
			FbxNode* node = meshNodeArray[meshIndex];
			FbxMesh* fbxMesh = node->GetMesh();
			if (fbxMesh)
			{
				numVerts += fbxMesh->GetControlPointsCount();
				if (!mImportOptions->bCombineToSingle)
				{
					numVerts = 0;
				}
			}
		}

		wstring meshName = inName;

		checkSmoothingInfo(meshNodeArray[0]->GetMesh());

		RStaticMesh* staticMesh = nullptr;
		TMap<float3, Color> existingVertexColorData;

		EVertexColorImportOption::Type vertexColorImportOption = mImportOptions->mVertexColorImportOption;

		if (vertexColorImportOption == EVertexColorImportOption::Ignore)
		{
			vertexColorImportOption = EVertexColorImportOption::Replace;
		}

		if (inStaticMesh != nullptr)
		{
			staticMesh = inStaticMesh;
		}
		else
		{
			staticMesh = newObject<RStaticMesh>(nullptr);
		}

		if (staticMesh->mSourceModels.size() < LODIndex + 1)
		{
			new(staticMesh->mSourceModels)StaticMeshSourceModel();
			if (staticMesh->mSourceModels.size() < LODIndex + 1)
			{
				LODIndex = staticMesh->mSourceModels.size() - 1;
			}
		}
		TArray<int32> oldMaterialIndex;

		StaticMeshSourceModel& srcModel = staticMesh->mSourceModels[LODIndex];
		if (inStaticMesh != nullptr && LODIndex > 0 && !srcModel.mRawMeshBulkData->isEmpty())
		{
			for (const StaticMeshSection & section : staticMesh->mRenderData->mLODResources[LODIndex].mSections)
			{
				bool canReuseSlotIndex = true;

				for (int32 lodREsourceIndex = 0; lodREsourceIndex < staticMesh->mRenderData->mLODResources.size(); ++lodREsourceIndex)
				{
					if (lodREsourceIndex == LODIndex)
					{
						continue;
					}

					for (const StaticMeshSection& lodSection : staticMesh->mRenderData->mLODResources[lodREsourceIndex].mSections)
					{
						if (lodSection.mMaterialIndex == section.mMaterialIndex)
						{
							canReuseSlotIndex = false;
							break;
						}
					}
					if (!canReuseSlotIndex)
					{
						break;
					}
				}
				if (canReuseSlotIndex)
				{
					oldMaterialIndex.add(section.mMaxVertexIndex);
				}
			}
			RawMesh emptyRawMesh;
			srcModel.mRawMeshBulkData->saveRawMesh(emptyRawMesh);
		}

		staticMesh->mLightingGuid = Guid::newGuid();

		RawMesh newRawMesh;
		TArray<FbxMaterial> meshMaterials;
		for (meshIndex = 0; meshIndex < meshNodeArray.size(); meshIndex++)
		{
			FbxNode* node = meshNodeArray[meshIndex];
			if (node->GetMesh())
			{
				if (!buildStaticMeshFromGeometry(node, staticMesh, meshMaterials, LODIndex, newRawMesh, vertexColorImportOption, existingVertexColorData, mImportOptions->mVertexOverrideColor))
				{
					bBuildStatus = false;
					break;
				}
			}
		}

		srcModel.mRawMeshBulkData->saveRawMesh(newRawMesh);
		if (bBuildStatus)
		{
			AIR_LOG(LogFbx, Verbose, TEXT("== Initial material list:"));
			for (int32 materialIndex = 0; materialIndex < meshMaterials.size(); ++materialIndex)
			{
				AIR_LOG(LogFbx, Verbose, TEXT("%d: %s"), materialIndex, meshMaterials[materialIndex].getName().c_str());

			}

			bool bDoRemap = false;
			TArray<int32> materialMap;
			TArray<FbxMaterial> uniqueMaterials;

			for (int32 materialIndex = 0; materialIndex < meshMaterials.size(); ++materialIndex)
			{
				bool bUnique = true;
				for (int32 otherMaterialIndex = materialIndex - 1; otherMaterialIndex >= 0; --otherMaterialIndex)
				{
					if (meshMaterials[materialIndex].mFBXMaterial == meshMaterials[otherMaterialIndex].mFBXMaterial)
					{

						int32 uniqueIndex = materialMap[otherMaterialIndex];
						materialMap.add(uniqueIndex);
						bDoRemap = true;
						bUnique = false;
						break;
					}
				}
				if (bUnique)
				{
					int32 uniqueIndex = uniqueMaterials.add(meshMaterials[materialIndex]);
					materialMap.add(uniqueIndex);
				}
				else
				{
					AIR_LOG(logFbx, Verbose, TEXT("   remap %d -> %d"), materialIndex, materialMap[materialIndex]);
				}
			}
			if (uniqueMaterials.size() > LARGE_MESH_MATERIAL_INDEX_THRESHOLD)
			{
				BOOST_ASSERT(false);
			}
			TArray<uint32> sortedMaterialIndex;
			TArray<int32> usedMaterials;
			for (int32 faceMaterialIndex = 0; faceMaterialIndex < newRawMesh.mFaceMaterialIndices.size(); ++faceMaterialIndex)
			{
				int32 materialIndex = newRawMesh.mFaceMaterialIndices[faceMaterialIndex];
				if (!usedMaterials.contains(materialIndex))
				{
					int32 newIndex = usedMaterials.add(materialIndex);
					if (newIndex != materialIndex)
					{
						bDoRemap = true;
					}
				}
			}

			for (int32 materialIndex = 0; materialIndex < meshMaterials.size(); ++materialIndex)
			{
				int32 skinIndex = 0xFFFF;
				if (bDoRemap)
				{
					int32 usedIndex = 0; 
					if (usedMaterials.find(materialIndex, usedIndex))
					{
						skinIndex = usedIndex;
					}
				}
				int32 remappedIndex = materialMap[materialIndex];
				uint32 sortedMaterialKey = ((uint32)skinIndex << 16) | ((uint32)remappedIndex & 0xffff);
				if (!sortedMaterialIndex.isValidIndex(sortedMaterialKey))
				{
					sortedMaterialIndex.add(sortedMaterialKey);
				}
			}
			sortedMaterialIndex.sort();

			TArray<FbxMaterial> sortedMaterials;
			for (int32 sortedIndex = 0; sortedIndex < sortedMaterialIndex.size(); ++sortedIndex)
			{
				int32 remappedIndex = sortedMaterialIndex[sortedIndex] & 0xffff;
				sortedMaterials.add(uniqueMaterials[remappedIndex]);
			}

			for (int32 materialIndex = 0; materialIndex < materialMap.size(); ++materialIndex)
			{
				for (int32 sortedIndex = 0; sortedIndex < sortedMaterialIndex.size(); ++sortedIndex)
				{
					int32 remappedIndex = sortedMaterialIndex[sortedIndex] & 0xffff;
					if (materialMap[materialIndex] == remappedIndex)
					{
						materialMap[materialIndex] = sortedIndex;
						break;
					}
				}
			}

			int32 maxMaterialIndex = 0; 
			int32 firstOpenUVChannel = 1;
			{
				RawMesh localRawMesh;
				srcModel.mRawMeshBulkData->loadRawMesh(localRawMesh);
				if (bDoRemap)
				{
					for (int32 triIndex = 0; triIndex < localRawMesh.mFaceMaterialIndices.size(); ++triIndex)
					{
						localRawMesh.mFaceMaterialIndices[triIndex] = materialMap[localRawMesh.mFaceMaterialIndices[triIndex]];
					}
				}
				localRawMesh.compactMaterialIndices();

				if (localRawMesh.mMaterialIndexToImportIndex.size() > 0)
				{
					TArray<FbxMaterial> oldSortedMaterials;
					exchange(oldSortedMaterials, sortedMaterials);
					sortedMaterials.empty(localRawMesh.mMaterialIndexToImportIndex.size());
					for (int32 materialIndex = 0; materialIndex < localRawMesh.mMaterialIndexToImportIndex.size(); ++materialIndex)
					{
						FbxMaterial material;
						int32 importIndex = localRawMesh.mMaterialIndexToImportIndex[materialIndex];
						if (oldSortedMaterials.isValidIndex(importIndex))
						{
							material = oldSortedMaterials[importIndex];
						}
						sortedMaterials.add(material);
					}
				}

				for (int32 triIndex = 0; triIndex < localRawMesh.mFaceMaterialIndices.size(); ++triIndex)
				{
					maxMaterialIndex = Math::max<int32>(maxMaterialIndex, localRawMesh.mFaceMaterialIndices[triIndex]);
				}

				for (int32 i = 0; i < MAX_MESH_TEXTURE_COORDS; i++)
				{
					if (localRawMesh.mWedgeTexCoords[i].size() == 0)
					{
						firstOpenUVChannel = i;
						break;
					}
				}
				srcModel.mRawMeshBulkData->saveRawMesh(localRawMesh);
			}
			if (LODIndex == 0)
			{
				staticMesh->mStaticMaterials.empty();
			}

			int32 numMaterials = Math::min(sortedMaterials.size(), maxMaterialIndex + 1);

			for (int32 materialIndex = 0; materialIndex < numMaterials; ++materialIndex)
			{
				MeshSectionInfo info = staticMesh->mSectionInfoMap.get(LODIndex, materialIndex);
				int32 index = 0; 
				wstring materialName = sortedMaterials[materialIndex].getName();

				wstring cleanMaterialSlotName = materialName;

				int32 skinOffset = cleanMaterialSlotName.find(TEXT("_skin"));
				if (skinOffset != INDEX_NONE)
				{
					cleanMaterialSlotName = cleanMaterialSlotName.substr(0, skinOffset);
				}

				if (oldMaterialIndex.size() > 0)
				{
					index = oldMaterialIndex[0];
					staticMesh->mStaticMaterials[index].mMaterialInterface = sortedMaterials[materialIndex].mMaterial;
					oldMaterialIndex.removeAt(0);
				}
				else if(inStaticMesh)
				{
					index = INDEX_NONE;
					StaticMaterial staticMaterialImported(sortedMaterials[materialIndex].mMaterial, cleanMaterialSlotName, materialName);
					for (int32 originalMaterialIndex = 0; originalMaterialIndex < inStaticMesh->mStaticMaterials.size(); ++originalMaterialIndex)
					{
						if (inStaticMesh->mStaticMaterials[originalMaterialIndex] == staticMaterialImported)
						{
							index = originalMaterialIndex;
							break;
						}
					}
					if (index == INDEX_NONE || (index >= numMaterials && index >= inStaticMesh->mStaticMaterials.size()))
					{
						index = staticMesh->mStaticMaterials.add(StaticMaterial(sortedMaterials[materialIndex].mMaterial, cleanMaterialSlotName, materialName));

					}
				}
				else
				{
					index = staticMesh->mStaticMaterials.add(StaticMaterial(sortedMaterials[materialIndex].mMaterial, cleanMaterialSlotName, materialName));
				}

				info.mMaterialIndex = index;
				staticMesh->mSectionInfoMap.remove(LODIndex, materialIndex);
				staticMesh->mSectionInfoMap.set(LODIndex, materialIndex, info);
			}
			RawMesh localMeshData;
			srcModel.mRawMeshBulkData->loadRawMesh(localMeshData);

			if (LODIndex == 0)
			{
				int32 numLODs = 1;
				while (staticMesh->mSourceModels.size() < numLODs)
				{
					new(staticMesh->mSourceModels)StaticMeshSourceModel();
				}
				for (int32 modelLODIndex = 0; modelLODIndex < numLODs; ++modelLODIndex)
				{
				}
			}
			FbxStaticMeshImportData* importData = FbxStaticMeshImportData::getImportDataForStaticMesh(staticMesh, templateImportData);
			
		}

		if (staticMesh)
		{
			
		}
		return staticMesh;
	}

	bool AFbxImporter::buildStaticMeshFromGeometry(FbxNode* node, RStaticMesh* staticMesh, TArray<FbxMaterial>& meshMaterials, int32 LODIndex, RawMesh& rawMesh, EVertexColorImportOption::Type vertexColorImportOption, const TMap<float3, Color>& existingVertexColorData, const Color& vertexOverrideColor)
	{
		BOOST_ASSERT(staticMesh->mSourceModels.isValidIndex(LODIndex));

		FbxMesh* mesh = node->GetMesh();
		StaticMeshSourceModel& srcModel = staticMesh->mSourceModels[LODIndex];

		mesh->RemoveBadPolygons();

		FbxLayer* baseLayer = mesh->GetLayer(0);
		if (baseLayer == nullptr)
		{
			return false;
		}

		FFBXUVs fbxUVs(mesh);
		int32 fbxNamedLightMapCoordinateIndex = fbxUVs.findLightUVIndex();
		if (fbxNamedLightMapCoordinateIndex != INDEX_NONE)
		{

		}
		int32 materialCount = 0;
		TArray<MaterialInterface*> materials;
		materialCount = node->GetMaterialCount();

		int32 materialIndexOffset = meshMaterials.size();
		for (int32 materialIndex = 0; materialIndex < materialCount; materialIndex++)
		{

		}
		if (materialCount == 0)
		{
			RMaterial* defaultMaterial = RMaterial::getDefaultMaterial(MD_Surface);
			BOOST_ASSERT(defaultMaterial);
			FbxMaterial* newMaterial = new (meshMaterials)FbxMaterial;
			newMaterial->mMaterial = defaultMaterial;
			newMaterial->mFBXMaterial = nullptr;
			materialCount = 1;
		}

		int32 layerSmoothingCount = mesh->GetLayerCount(FbxLayerElement::eSmoothing);
		for (int32 i = 0; i < layerSmoothingCount; i++)
		{
			FbxLayerElementSmoothing const* smoothingInfo = mesh->GetLayer(0)->GetSmoothing();
			if (smoothingInfo && smoothingInfo->GetMappingMode() != FbxLayerElement::eByPolygon)
			{
				mGeometryConverter->ComputePolygonSmoothingFromEdgeSmoothing(mesh, i);
			}
		}
		if (!mesh->IsTriangleMesh())
		{
			const bool bReplace = true;
			FbxNodeAttribute* convertedNode = mGeometryConverter->Triangulate(mesh, bReplace);
			if (convertedNode != nullptr && convertedNode->GetAttributeType() == FbxNodeAttribute::eMesh)
			{
				mesh = (FbxMesh*)convertedNode;
			}
			else
			{
				return false;
			}
		}
		baseLayer = mesh->GetLayer(0);

		FbxLayerElementMaterial* layerElementMaterial = baseLayer->GetMaterials();
		FbxLayerElement::EMappingMode materialMappingMode = layerElementMaterial ? layerElementMaterial->GetMappingMode() : FbxLayerElement::eByPolygon;

		fbxUVs.phase2(mesh);

		bool bSmoothingAvailable = false;

		FbxLayerElementSmoothing const * smoothingInfo = baseLayer->GetSmoothing();
		FbxLayerElement::EReferenceMode smoothingReferenceMode(FbxLayerElement::eDirect);
		FbxLayerElement::EMappingMode smoothingMappingMode(FbxLayerElement::eByEdge);
		if (smoothingInfo)
		{
			if (smoothingInfo->GetMappingMode() == FbxLayerElement::eByPolygon)
			{
				bSmoothingAvailable = true;
			}
			smoothingMappingMode = smoothingInfo->GetMappingMode();
			smoothingReferenceMode = smoothingInfo->GetReferenceMode();
		}

		FbxLayerElementVertexColor* layerElementVertexColor = baseLayer->GetVertexColors();
		FbxLayerElement::EReferenceMode vertexColorReferenceMode(FbxLayerElement::eDirect);
		FbxLayerElement::EMappingMode vertexColorMappingMode(FbxLayerElement::eByControlPoint);
		if (layerElementVertexColor)
		{
			vertexColorMappingMode = layerElementVertexColor->GetMappingMode();
			vertexColorReferenceMode = layerElementVertexColor->GetReferenceMode();
		}

		FbxLayerElementNormal* layerElementNormal = baseLayer->GetNormals();
		FbxLayerElementTangent* layerElementTangent = baseLayer->GetTangents();
		FbxLayerElementBinormal* layerElementBinormal = baseLayer->GetBinormals();

		bool bHasNTBInformation = layerElementNormal && layerElementTangent && layerElementBinormal;

		FbxLayerElement::EReferenceMode normalReferenceMode(FbxLayerElement::eDirect);
		FbxLayerElement::EMappingMode normalMappingMode(FbxLayerElement::eByControlPoint);
		if (layerElementNormal)
		{
			normalReferenceMode = layerElementNormal->GetReferenceMode();
			normalMappingMode = layerElementNormal->GetMappingMode();
		}


		FbxLayerElement::EReferenceMode tangentReferenceMode(FbxLayerElement::eDirect);
		FbxLayerElement::EMappingMode tangentMappingMode(FbxLayerElement::eByControlPoint);
		if (layerElementTangent)
		{
			tangentReferenceMode = layerElementTangent->GetReferenceMode();
			tangentMappingMode = layerElementTangent->GetMappingMode();
		}

		FbxLayerElement::EReferenceMode binormalReferenceMode(FbxLayerElement::eDirect);
		FbxLayerElement::EMappingMode binormalMappingMode(FbxLayerElement::eByControlPoint);
		if (layerElementBinormal)
		{
			binormalMappingMode = layerElementBinormal->GetMappingMode();
			binormalReferenceMode = layerElementBinormal->GetReferenceMode();
		}

		FbxAMatrix totalMatrix;
		FbxAMatrix totalMatrixForNormal;
		totalMatrix = computeTotalMatrix(node);

		totalMatrixForNormal = totalMatrix.Inverse();
		totalMatrixForNormal = totalMatrixForNormal.Transpose();
		int32 triangleCount = mesh->GetPolygonCount();
		if (triangleCount == 0)
		{
			return false;
		}

		int32 vertexCount = mesh->GetControlPointsCount();
		int32 wedgeCount = triangleCount * 3;
		bool oldNegativeScale = isOddNegativeScale(totalMatrix);

		int32 vertexOffset = rawMesh.mVertexPositions.size();
		int32 wedgeOffset = rawMesh.mWedgeIndices.size();
		int32 triangleOffset = rawMesh.mFaceMaterialIndices.size();

		int32 maxMaterialIndex = 0; 
		rawMesh.mFaceMaterialIndices.addZeroed(triangleCount);
		rawMesh.mFaceSmoothingMasks.addZeroed(triangleCount);
		rawMesh.mWedgeIndices.addZeroed(wedgeCount);

		if (bHasNTBInformation || rawMesh.mWedgeTangentX.size() > 0 || rawMesh.mWedgeTangentY.size() > 0)
		{
			rawMesh.mWedgeTangentX.addZeroed(wedgeOffset + wedgeCount - rawMesh.mWedgeTangentX.size());
			rawMesh.mWedgeTangentY.addZeroed(wedgeOffset + wedgeCount - rawMesh.mWedgeTangentY.size());
		}

		if (layerElementNormal || rawMesh.mWedgeTangentZ.size() > 0)
		{
			rawMesh.mWedgeTangentZ.addZeroed(wedgeOffset + wedgeCount - rawMesh.mWedgeTangentZ.size());
		}

		if (layerElementVertexColor || vertexColorImportOption != EVertexColorImportOption::Replace || rawMesh.mWedgeColors.size())
		{
			int32 numNewColors = wedgeOffset + wedgeCount - rawMesh.mWedgeColors.size();
			int32 firstNewColor = rawMesh.mWedgeColors.size();
			rawMesh.mWedgeColors.addUninitialized(numNewColors);
			for (int32 wedgetIndex = firstNewColor; wedgetIndex < firstNewColor + numNewColors; ++wedgetIndex)
			{
				rawMesh.mWedgeColors[wedgetIndex] = Color::White;
			}
		}

		int32 existingUVCount = 0;
		for (int32 existingUVIndex = 0; existingUVIndex < MAX_MESH_TEXTURE_COORDS; ++existingUVIndex)
		{
			if (rawMesh.mWedgeTexCoords[existingUVIndex].size() > 0)
			{
				++existingUVCount;
			}
			else
			{
				break;
			}
		}

		int32 uvcount = Math::max(fbxUVs.mUniqueUVCount, existingUVCount);
		uvcount = Math::max(1, uvcount);
		for (int32 uvLayerIndex = 0; uvLayerIndex < uvcount; uvLayerIndex++)
		{
			rawMesh.mWedgeTexCoords[uvLayerIndex].addZeroed(wedgeOffset + wedgeCount - rawMesh.mWedgeTexCoords[uvLayerIndex].size());
		}

		int32 triangleIndex;
		TMap<int32, int32> indexMap;
		bool bHasNonDegenerateTriangles = false;
		for (triangleIndex = 0; triangleIndex < triangleCount; triangleIndex++)
		{
			int32 destTriangleIndex = triangleOffset + triangleIndex;

			float3 cornerPositions[3];

			for (int32 cornerIndex = 0; cornerIndex < 3; cornerIndex++)
			{
				int32 wedgeIndex = wedgeOffset + triangleIndex * 3 + (oldNegativeScale ? 2 - cornerIndex : cornerIndex);
				int32 controlPointIndex = mesh->GetPolygonVertex(triangleIndex, cornerIndex);
				auto& existingIndex = indexMap.find(controlPointIndex);
				if (existingIndex != indexMap.end())
				{
					rawMesh.mWedgeIndices[wedgeIndex] = existingIndex->second;
					cornerPositions[cornerIndex] = rawMesh.mVertexPositions[existingIndex->second];
				}
				else
				{
					FbxVector4 fbxPosition = mesh->GetControlPoints()[controlPointIndex];
					FbxVector4 finalPositoin = totalMatrix.MultT(fbxPosition);
					int32 vertexIndex = rawMesh.mVertexPositions.add(mDataConverter.convertPos(finalPositoin));
					rawMesh.mWedgeIndices[wedgeIndex] = vertexIndex;
					indexMap.emplace(controlPointIndex, vertexIndex);
					cornerPositions[cornerIndex] = rawMesh.mVertexPositions[vertexIndex];
				}

				if (layerElementNormal)
				{
					int triangleConnerIndex = triangleIndex * 3 + cornerIndex;
					int normalMapIndex = (normalMappingMode == FbxLayerElement::eByControlPoint) ? controlPointIndex : triangleConnerIndex;
					int normalValueIndex = (normalReferenceMode == FbxLayerElement::eDirect) ? normalMapIndex : layerElementNormal->GetIndexArray().GetAt(normalMapIndex);

					if (bHasNTBInformation)
					{
						int tangentMapIndex = (tangentMappingMode == FbxLayerElement::eByControlPoint) ? controlPointIndex : triangleConnerIndex;
						int tangentValueIndex = (tangentReferenceMode == FbxLayerElement::eDirect) ? tangentMapIndex : layerElementTangent->GetIndexArray().GetAt(tangentMapIndex);

						FbxVector4 tempValue = layerElementTangent->GetDirectArray().GetAt(tangentValueIndex);
						tempValue = totalMatrixForNormal.MultT(tempValue);
						float3 tangentX = mDataConverter.convertDir(tempValue);
						rawMesh.mWedgeTangentX[wedgeIndex] = tangentX.getSafeNormal();



						int binormalMapIndex = (binormalMappingMode == FbxLayerElement::eByControlPoint) ? controlPointIndex : triangleConnerIndex;
						int binormalValueIndex = (binormalReferenceMode == FbxLayerElement::eDirect) ? binormalMapIndex : layerElementBinormal->GetIndexArray().GetAt(binormalMapIndex);

						tempValue = layerElementBinormal->GetDirectArray().GetAt(binormalValueIndex);
						tempValue = totalMatrixForNormal.MultT(tempValue);
						float3 tangentZ = -mDataConverter.convertDir(tempValue);
						rawMesh.mWedgeTangentZ[wedgeIndex] = tangentZ.getSafeNormal();
					}
					FbxVector4 tempValue = layerElementNormal->GetDirectArray().GetAt(normalValueIndex);
					tempValue = totalMatrixForNormal.MultT(tempValue);
					float3 tangentY = mDataConverter.convertDir(tempValue);
					rawMesh.mWedgeTangentY[wedgeIndex] = tangentY.getSafeNormal();

				}


				if (vertexColorImportOption == EVertexColorImportOption::Replace)
				{
					if (layerElementVertexColor)
					{
						int32 vertexColorMappingIndex = (vertexColorMappingMode == FbxLayerElement::eByControlPoint) ? mesh->GetPolygonVertex(triangleIndex, cornerIndex) : (triangleIndex * 3 + cornerIndex);
						int32 vertexColorIndex = (vertexColorReferenceMode == FbxLayerElement::eDirect) ? vertexColorMappingIndex : layerElementVertexColor->GetIndexArray().GetAt(vertexColorMappingIndex);

						FbxColor vertexColor = layerElementVertexColor->GetDirectArray().GetAt(vertexColorIndex);
						rawMesh.mWedgeColors[wedgeIndex] = Color(
							uint8(255.f * vertexColor.mRed),
							uint8(255.f * vertexColor.mGreen),
							uint8(255.f * vertexColor.mBlue),
							uint8(255.f * vertexColor.mAlpha)
						);
					}
				}
				else if (vertexColorImportOption == EVertexColorImportOption::Ignore)
				{
					float3 position = rawMesh.mVertexPositions[rawMesh.mWedgeIndices[wedgeIndex]];
					const auto& paintedColor = existingVertexColorData.find(position);
					if (paintedColor != existingVertexColorData.end())
					{
						rawMesh.mWedgeColors[wedgeIndex] = paintedColor->second;
					}
				}
				else
				{
					rawMesh.mWedgeColors[wedgeIndex] = vertexOverrideColor;
				}
			}

			if (!bHasNonDegenerateTriangles)
			{
				float comparisonThreshold = 0.00002f;
				if (!(cornerPositions[0].equals(cornerPositions[1], comparisonThreshold))
					|| (cornerPositions[0].equals(cornerPositions[2], comparisonThreshold))
					|| (cornerPositions[1].equals(cornerPositions[2], comparisonThreshold)))
				{
					bHasNonDegenerateTriangles = true;
				}
			}

			if (bSmoothingAvailable && smoothingInfo)
			{
				if (smoothingMappingMode == FbxLayerElement::eByPolygon)
				{
					int lSmoothingIndex = (smoothingReferenceMode == FbxLayerElement::eDirect) ? triangleIndex : smoothingInfo->GetIndexArray().GetAt(triangleIndex);
					rawMesh.mFaceSmoothingMasks[destTriangleIndex] = smoothingInfo->GetDirectArray().GetAt(lSmoothingIndex);
				}
				else
				{

				}
			}

			int32 uvLayerIndex;
			for (uvLayerIndex = 0; uvLayerIndex < fbxUVs.mUniqueUVCount; uvLayerIndex++)
			{
				if (fbxUVs.mLayerElementUV[uvLayerIndex] != nullptr)
				{
					for (int32 cornerIndex = 0; cornerIndex < 3; cornerIndex++)
					{
						int32 wedgeIndex = wedgeOffset + triangleIndex * 3 + (oldNegativeScale ? 2 - cornerIndex : cornerIndex);
						int lControlPointIndex = mesh->GetPolygonVertex(triangleIndex, cornerIndex);
						int uvMapIndex = (fbxUVs.mUVMappingMode[uvLayerIndex] == FbxLayerElement::eByControlPoint) ? lControlPointIndex : triangleIndex * 3 + cornerIndex;
						int32 uvIndex = (fbxUVs.mUVReferenceNode[uvLayerIndex] == FbxLayerElement::eDirect) ? uvMapIndex : fbxUVs.mLayerElementUV[uvLayerIndex]->GetIndexArray().GetAt(uvMapIndex);
						FbxVector2 uvVector = fbxUVs.mLayerElementUV[uvLayerIndex]->GetDirectArray().GetAt(uvIndex);
						rawMesh.mWedgeTexCoords[uvLayerIndex][wedgeIndex].x = static_cast<float>(uvVector[0]);
						rawMesh.mWedgeTexCoords[uvLayerIndex][wedgeIndex].y = 1.f - static_cast<float>(uvVector[1]);
					}
				}
			}

			int32 materialIndex = 0;
			if (materialCount > 0)
			{
				if (layerElementMaterial)
				{
					switch (materialMappingMode)
					{
					case FbxLayerElement::eAllSame:
					{
						materialIndex = layerElementMaterial->GetIndexArray().GetAt(0);
					}
					break;
					case FbxLayerElement::eByPolygon:
					{
						materialIndex = layerElementMaterial->GetIndexArray().GetAt(triangleIndex);
					}
					break;
					}
				}
			}
			materialIndex += materialIndexOffset;
			if (materialIndex >= materialCount + materialIndexOffset || materialIndex < 0)
			{
				materialIndex = 0;
			}

			rawMesh.mFaceMaterialIndices[destTriangleIndex] = materialIndex;
		}

		fbxUVs.cleanup();
		{

		}
		bool bIsValidMesh = bHasNonDegenerateTriangles;
		return bIsValidMesh;

	}

	float3 FbxDataConverter::convertPos(FbxVector4 vec)
	{
		float3 out;
		out[0] = vec[0];
		out[1] = vec[1];
		out[2] = vec[2];
		return out;
	}

	float3 FbxDataConverter::convertDir(FbxVector4 dir)
	{
		float3 out;
		out[0] = dir[0];
		out[1] = dir[1];
		out[2] = dir[2];
		return out;
	}

	int32 AFbxImporter::getImportType(const wstring & inFilename)
	{
		int32 result = -1;
		wstring filename = inFilename;

		if (openFile(filename, true))
		{
			FbxStatistics statistics;
			mImporter->GetStatistics(&statistics);
			int32 itemIndex;
			FbxString itemName;
			int32 itemCount;
			bool bHasAnimation = false;

			for (itemIndex = 0; itemIndex < statistics.GetNbItems(); itemIndex++)
			{
				statistics.GetItemPair(itemIndex, itemName, itemCount);
				const string nameBuffer(itemName.Buffer());

			}
			FbxSceneInfo sceneInfo;
			if (getSceneInfo(filename, sceneInfo, true))
			{
				if (sceneInfo.SkinnedMeshNum > 0)
				{
					result = 1;
				}
				else if (sceneInfo.totalGeometryNum > 0)
				{
					result = 0;
				}
				bHasAnimation = sceneInfo.bHasAnimation;
			}
			if (mImporter)
			{
				mImporter->Destroy();
			}
			mImporter = nullptr;
			mCurPhase = NOTSTARTED;

			if (bHasAnimation)
			{
				if (result == -1)
				{
					result = 2;
				}
				else if (result == 0)
				{
					result = 1;
				}
			}
		}
		return result;
	}

	void AFbxImporter::applyTransformFromSettingsToFbxNode(FbxNode* node, FbxAssetImportData* assetData)
	{
		BOOST_ASSERT(node);
		BOOST_ASSERT(assetData);

		FbxAMatrix transformMatrix;
		buildFbxMatrixForImportTransform(transformMatrix, assetData);
		FbxDouble3 existingTranslation = node->LclTranslation.Get();
		FbxDouble3 existingRotation = node->LclRotation.Get();
		FbxDouble3 existingScaling = node->LclScaling.Get();

		FbxVector4 addedTranslation = transformMatrix.GetT();
		FbxVector4 addedRotation = transformMatrix.GetR();
		FbxVector4 addedScaling = transformMatrix.GetS();

		FbxDouble3 newTranslation = FbxDouble3(existingTranslation[0] + addedTranslation[0], existingTranslation[1] + addedTranslation[1], existingTranslation[2] + addedTranslation[2]);

		FbxDouble3 newRotation = FbxDouble3(existingRotation[0] + addedRotation[0], existingRotation[1] + addedRotation[1], existingRotation[2] + addedRotation[2]);

		FbxDouble3 newScaling = FbxDouble3(existingScaling[0] * addedScaling[0], existingScaling[1] * addedScaling[1], existingScaling[2] * addedScaling[2]);

		node->LclTranslation.Set(newTranslation);
		node->LclRotation.Set(newRotation);
		node->LclScaling.Set(newScaling);

		mScene->GetAnimationEvaluator()->Reset();
	}

	void AFbxImporter::buildFbxMatrixForImportTransform(FbxAMatrix& outMatrix, FbxAssetImportData* assetData)
	{
		if (!assetData)
		{
			outMatrix.SetIdentity();
			return;
		}
		outMatrix.SetIdentity();
		return;
	}

	void AFbxImporter::fillFbxMeshArray(FbxNode* node, TArray<FbxNode *>& outMeshArray, AFbxImporter* fbxImporter)
	{
		if (node->GetMesh())
		{
			if (node->GetMesh()->GetPolygonVertexCount() > 0)
			{
				outMeshArray.add(node);
			}
		}
		int32 childIndex;
		for (childIndex = 0; childIndex < node->GetChildCount(); ++childIndex)
		{
			fillFbxMeshArray(node->GetChild(childIndex), outMeshArray, fbxImporter);
		}
	}

	int32 AFbxImporter::getFbxMeshCount(FbxNode* node, bool bCountLODs, int32& outNumLODGroups)
	{
		bool bLODGroup = node->GetNodeAttribute() && node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eLODGroup;

		if (bLODGroup)
		{
			++outNumLODGroups;
		}

		int32 meshCount = 0;
		if (!bLODGroup || bCountLODs)
		{
			if (node->GetMesh())
			{
				meshCount = 1;
			}

			for (int32 childIndex = 0; childIndex < node->GetChildCount(); ++childIndex)
			{
				meshCount += getFbxMeshCount(node->GetChild(childIndex), bCountLODs, outNumLODGroups);
			}
		}
		else
		{
			meshCount = 1;
		}
		return meshCount;
	}

	FbxAMatrix AFbxImporter::computeTotalMatrix(FbxNode* node)
	{
		FbxAMatrix geometry;
		FbxVector4 translation, rotation, scaling;
		translation = node->GetGeometricTranslation(FbxNode::eSourcePivot);
		rotation = node->GetGeometricRotation(FbxNode::eSourcePivot);
		scaling = node->GetGeometricScaling(FbxNode::eSourcePivot);
		geometry.SetT(translation);
		geometry.SetR(rotation);
		geometry.SetS(scaling);

		FbxAMatrix& globalTransform = mScene->GetAnimationEvaluator()->GetNodeGlobalTransform(node);
		if (!mImportOptions->bTransformVertexToAbsolute)
		{
			if (mImportOptions->bBakePivotInVertex)
			{
				FbxAMatrix pivotGeometry;
				FbxVector4 rotationPivot = node->GetRotationPivot(FbxNode::eSourcePivot);
				FbxVector4 fullPivot;
				fullPivot[0] = -rotationPivot[0];
				fullPivot[1] = -rotationPivot[1];
				fullPivot[2] = -rotationPivot[2];
				pivotGeometry.SetT(fullPivot);
				geometry = geometry * pivotGeometry;
			}
			else
			{
				geometry.SetIdentity();
			}
		}

		FbxAMatrix totalMatrix = mImportOptions->bTransformVertexToAbsolute ? globalTransform * geometry : geometry;
		return totalMatrix;
	}

	bool AFbxImporter::isOddNegativeScale(FbxAMatrix& totalMatrix)
	{
		FbxVector4 scale = totalMatrix.GetS();
		int32 negativeNum = 0;
		if (scale[0] < 0)negativeNum++;
		if (scale[1] < 0)negativeNum++;
		if (scale[2] < 0)negativeNum++;
		return negativeNum == 1 || negativeNum == 3;
	}

	bool AFbxImporter::getSceneInfo(wstring filename, FbxSceneInfo& sceneInfo, bool bPreventMaterialNameClash)
	{
		bool result = true;
		bool bSceneInfo = true;
		switch (mCurPhase)
		{
		case AirFbx::AFbxImporter::NOTSTARTED:
			if (!openFile(filename, false, bSceneInfo))
			{
				result = false;
				break;
			}
		case AirFbx::AFbxImporter::FILEOPENED:
			if (!importFile(filename, bPreventMaterialNameClash))
			{
				result = false;
				break;
			}
		case AirFbx::AFbxImporter::IMPORTED:
		default:
			break;
		}

		if (result)
		{
			FbxTimeSpan globalTimeSpan(FBXSDK_TIME_INFINITE, FBXSDK_TIME_MINUS_INFINITE);
			sceneInfo.totalMaterialNum = mScene->GetMaterialCount();
			sceneInfo.totalTextureNum = mScene->GetTextureCount();
			sceneInfo.totalGeometryNum = 0;
			sceneInfo.mNonSkinnedMeshNum = 0;
			sceneInfo.SkinnedMeshNum = 0;
			for (int32 geometryIndex = 0; geometryIndex < mScene->GetGeometryCount(); geometryIndex++)
			{
				FbxGeometry* geometry = mScene->GetGeometry(geometryIndex);
				if (geometry->GetAttributeType() == FbxNodeAttribute::eMesh)
				{
					FbxNode* geoNode = geometry->GetNode();
					FbxMesh* mesh = (FbxMesh*)geometry;
					if (geoNode && mesh->GetDeformerCount(FbxDeformer::eSkin) < 0)
					{
						FbxNode* parentNode = recursiveFindParentLodGroup(geoNode->GetParent());
						if (parentNode != nullptr && parentNode->GetNodeAttribute() && parentNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eLODGroup)
						{
							bool isLodRoot = false;
							for (int32 childindex = 0; childindex < parentNode->GetChildCount(); ++childindex)
							{
								FbxNode* meshNode = findLodGroupNode(parentNode, childindex);
								if (geoNode == meshNode)
								{
									isLodRoot = true;
									break;
								}
							}
							if (!isLodRoot)
							{
								continue;
							}
						}
					}
					sceneInfo.totalGeometryNum++;

					sceneInfo.meshInfo.addZeroed(1);

					FbxMeshInfo& meshInfo = sceneInfo.meshInfo.last();
					if (geometry->GetName()[0] != '\0')
					{
						meshInfo.name = ANSI_TO_TCHAR(makeName(geometry->GetName()));
					}
					else
					{
						meshInfo.name = makeString(geoNode ? geoNode->GetName() : "None");
					}
					meshInfo.bTriangulated = mesh->IsTriangleMesh();
					meshInfo.mateiralNum = geoNode ? geoNode->GetMaterialCount() : 0;
					meshInfo.faceNum = mesh->GetPolygonCount();
					meshInfo.vertexNum = mesh->GetControlPointsCount();

					meshInfo.LODGroup = nullptr;
					if (geoNode)
					{
						FbxNode* parentNode = recursiveFindParentLodGroup(geoNode->GetParent());
						if (parentNode != nullptr && parentNode->GetNodeAttribute() && parentNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eLODGroup)
						{
							meshInfo.LODGroup = makeString(parentNode->GetName());
							for (int32 lodIndex = 0; lodIndex < parentNode->GetChildCount(); lodIndex++)
							{
								FbxNode* meshNode = findLodGroupNode(parentNode, lodIndex, geoNode);
								if (geoNode == meshNode)
								{
									meshInfo.LODLevel = lodIndex;
									break;
								}
							}
						}
					}
					if (mesh->GetDeformerCount(FbxDeformer::eSkin) > 0)
					{
						sceneInfo.SkinnedMeshNum++;
						meshInfo.bIsSkelMesh = true;
						meshInfo.norphNum = mesh->GetShapeCount();
						FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(0, FbxDeformer::eSkin);
						int32 clusterCount = skin->GetClusterCount();
						FbxNode* link = nullptr;
						for (int32 clusterId = 0; clusterId < clusterCount; clusterId++)
						{
							FbxCluster* cluster = skin->GetCluster(clusterId);
							link = cluster->GetLink();
							while (link && link->GetParent() && link->GetParent()->GetSkeleton())
							{
								link = link->GetParent();
							}

							if (link != nullptr)
							{
								break;
							}
						}
						meshInfo.skeletonRoot = makeString(link ? link->GetName() : ("None"));
						meshInfo.skeletonElemeNum = link ? link->GetChildCount(true) : 0;
						if (link)
						{
							FbxTimeSpan animTimeSpan(FBXSDK_TIME_INFINITE, FBXSDK_TIME_MINUS_INFINITE);
							link->GetAnimationInterval(animTimeSpan);
							globalTimeSpan.UnionAssignment(animTimeSpan);
						}
					}
					else
					{
						sceneInfo.mNonSkinnedMeshNum++;
						meshInfo.bIsSkelMesh = false;
						meshInfo.skeletonRoot = nullptr;
					}
					meshInfo.uniqueId = mesh->GetUniqueID();

				}
			}
			sceneInfo.bHasAnimation = false;
			int32 animCurveNodeCount = mScene->GetSrcObjectCount<FbxAnimCurveNode>();
			for (int32 animCurveNodeIndex = 2; animCurveNodeIndex < animCurveNodeCount; animCurveNodeIndex++)
			{
				FbxAnimCurveNode* curAnimCurveNode = mScene->GetSrcObject<FbxAnimCurveNode>(animCurveNodeIndex);
				if (curAnimCurveNode->IsAnimated(true))
				{
					sceneInfo.bHasAnimation = true;
					break;
				}
			}
			sceneInfo.frameRate = FbxTime::GetFrameRate(mScene->GetGlobalSettings().GetTimeMode());
			if (globalTimeSpan.GetDirection() == FBXSDK_TIME_FORWARD)
			{
				sceneInfo.totalTime = (globalTimeSpan.GetDuration().GetMilliSeconds()) / 1000.f * sceneInfo.frameRate;
			}
			else
			{
				sceneInfo.totalTime = 0;
			}

			FbxNode* rootNode = mScene->GetRootNode();
			FbxNodeInfo rootInfo;
			rootInfo.objectName = makeName(rootNode->GetName());
			rootInfo.uniqueId = rootNode->GetUniqueID();
			rootInfo.transform = rootNode->EvaluateGlobalTransform();
			rootInfo.attributeName = nullptr;
			rootInfo.attributeUniqueId = 0;
			rootInfo.attributeType = nullptr;
			rootInfo.parentName = nullptr;
			rootInfo.parentUniqueId = 0;
			sceneInfo.hierarchyInfo.add(rootInfo);
			traverseHierarchyNodeRecursively(sceneInfo, rootNode, rootInfo);
		}
		return result;
	}

	void AFbxImporter::traverseHierarchyNodeRecursively(FbxSceneInfo& sceneInfo, FbxNode* parentNode, FbxNodeInfo& parentInfo)
	{
		int32 nodeCount = parentNode->GetChildCount();
		for (int32 nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
		{
			FbxNode* childNode = parentNode->GetChild(nodeIndex);
			FbxNodeInfo childInfo;
			childInfo.objectName = makeName(childNode->GetName());
			childInfo.uniqueId = childNode->GetUniqueID();
			childInfo.parentName = parentInfo.objectName;
			childInfo.parentUniqueId = parentInfo.uniqueId;
			childInfo.rotationPivot = childNode->RotationPivot.Get();
			childInfo.scalePivot = childNode->ScalingPivot.Get();
			childInfo.transform = childNode->EvaluateLocalTransform();
			if (childNode->GetNodeAttribute())
			{
				FbxNodeAttribute* childAttribute = childNode->GetNodeAttribute();
				childInfo.attributeUniqueId = childAttribute->GetUniqueID();
				if (childAttribute->GetName()[0] != '\0')
				{
					childInfo.attributeName = makeName(childAttribute->GetName());
				}
				else
				{
					childInfo.attributeName = makeName(childAttribute->GetNode()->GetName());
				}
				switch (childAttribute->GetAttributeType())
				{
				case FbxNodeAttribute::eUnknown:
					childInfo.attributeType = "eUnknown";
					break;
				case FbxNodeAttribute::eNull:
					childInfo.attributeType = "eNull";
					break;
				case FbxNodeAttribute::eMarker:
					childInfo.attributeType = "eMarker";
					break;
				case FbxNodeAttribute::eSkeleton:
					childInfo.attributeType = "eSkeleton";
					break;
				case FbxNodeAttribute::eMesh:
					childInfo.attributeType = "eMesh";
					break;
				case FbxNodeAttribute::eNurbs:
					childInfo.attributeType = "eNurbs";
					break;
				case FbxNodeAttribute::ePatch:
					childInfo.attributeType = "ePatch";
					break;
				case FbxNodeAttribute::eCamera:
					childInfo.attributeType = "eCamera";
					break;
				case FbxNodeAttribute::eCameraStereo:
					childInfo.attributeType = "eCameraStereo";
					break;
				case FbxNodeAttribute::eCameraSwitcher:
					childInfo.attributeType = "eCameraSwitcher";
					break;
				case FbxNodeAttribute::eLight:
					childInfo.attributeType = "eLight";
break;
				case FbxNodeAttribute::eOpticalReference:
					childInfo.attributeType = "eOpticalReference";
					break;
				case FbxNodeAttribute::eOpticalMarker:
					childInfo.attributeType = "eOpticalMarker";
					break;
				case FbxNodeAttribute::eNurbsCurve:
					childInfo.attributeType = "eNurbsCurve";
					break;;
				case FbxNodeAttribute::eTrimNurbsSurface:
					childInfo.attributeType = "eTrimNurbsSurface";
					break;
				case FbxNodeAttribute::eBoundary:
					childInfo.attributeType = "eBoundary";
					break;
				case FbxNodeAttribute::eNurbsSurface:
					childInfo.attributeType = "eNurbsSurface";
					break;
				case FbxNodeAttribute::eShape:
					childInfo.attributeType = "eShape";
					break;
				case FbxNodeAttribute::eLODGroup:
					childInfo.attributeType = "eLODGroup";
					break;
				case FbxNodeAttribute::eSubDiv:
					childInfo.attributeType = "eSubDiv";
					break;
				case FbxNodeAttribute::eCachedEffect:
					childInfo.attributeType = "eCachedEffect";
					break;
				case FbxNodeAttribute::eLine:
					childInfo.attributeType = "eLine";
					break;
				}
			}
			else
			{
				childInfo.attributeType = "eNull";
				childInfo.attributeName = nullptr;
			}
			sceneInfo.hierarchyInfo.add(childInfo);
			traverseHierarchyNodeRecursively(sceneInfo, childNode, childInfo);
		}
	}

	FbxNode* AFbxImporter::findLodGroupNode(FbxNode* nodeLodGroup, int32 lodIndex, FbxNode* nodeToFind /* = nullptr */)
	{
		BOOST_ASSERT(nodeLodGroup->GetChildCount() >= lodIndex);
		FbxNode* childNode = nodeLodGroup->GetChild(lodIndex);
		return recursiveGetFirstMeshNode(childNode, nodeToFind);
	}

	FbxNode* AFbxImporter::recursiveGetFirstMeshNode(FbxNode* node, FbxNode* nodeToFind)
	{
		if (node->GetMesh() != nullptr)
		{
			return node;
		}
		for (int32 childIndex = 0; childIndex < node->GetChildCount(); childIndex++)
		{
			FbxNode* meshNode = recursiveGetFirstMeshNode(node->GetChild(childIndex), nodeToFind);
			if (nodeToFind == nullptr)
			{
				if (meshNode != nullptr)
				{
					return meshNode;
				}
			}
			else if (meshNode == nodeToFind)
			{
				return meshNode;
			}
		}
		return nullptr;
	}

	FbxNode* AFbxImporter::recursiveFindParentLodGroup(FbxNode* parentNode)
	{
		if (parentNode == nullptr)
		{
			return nullptr;
		}
		if (parentNode->GetNodeAttribute() && parentNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eLODGroup)
		{
			return parentNode;
		}
		return recursiveFindParentLodGroup(parentNode->GetParent());
	}

	ANSICHAR* AFbxImporter::makeName(const ANSICHAR* name)
	{
		const int specialChars[] = { '.', ',', '/', '`', '%' };
		const int len = CStringAnsi::strlen(name);
		ANSICHAR* tmpName = new ANSICHAR[len + 1];
		CStringAnsi::strcpy(tmpName, len + 1, name);
		for (int32 i = 0; i < ARRAY_COUNT(specialChars); i++)
		{
			ANSICHAR* charPtr = tmpName;
			while ((charPtr = CStringAnsi::strchr(charPtr, specialChars[i])) != nullptr)
			{
				charPtr[0] = '_';
			}
		}

		ANSICHAR * newName;
		newName = CStringAnsi::strchr(tmpName, ':');
		while (newName && CStringAnsi::strchr(newName + 1, ':'))
		{
			newName = CStringAnsi::strchr(newName + 1, ':');
		}
		if (newName)
		{
			return newName + 1;
		}
		return newName;
	}

	void AFbxImporter::checkSmoothingInfo(FbxMesh* fbxMesh)
	{
		if (fbxMesh && bFirstMesh)
		{
			bFirstMesh = false;

		}
	}
}