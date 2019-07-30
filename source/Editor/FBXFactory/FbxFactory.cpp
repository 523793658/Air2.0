#include "FbxFactory.h"
#include "Modules/ModuleManager.h"
#include "FbxImporter.h"
#include "boost/algorithm/string.hpp"
#include "FbxImportConfig.h"
#include "Misc/Paths.h"
#include "FbxStaticMeshImportData.h"
#include "Classes/Engine/StaticMesh.h"
#include "FbxImportConfig.h"
#include "StaticMeshResources.h"

namespace Air
{
	class FbxFactoryModule : public IFactoryModule
	{
	public:
		virtual Factory* createFactory() override
		{
			static Factory* instance;
			if (instance == nullptr)
			{
				instance = new FbxFactory();
			}
			return instance;
		}
	};

	IMPLEMENT_MODULE(FbxFactoryModule, FbxFactory);

	void FbxFactory::getExtensions(TCHAR**& extensions, uint32& num)
	{
		static TCHAR* Supported[] = { TEXT("fbx") };
		extensions = Supported;
		num = 1;
	}


	std::shared_ptr<Object> FbxFactory::createFromFileInner(Object* inObject, wstring filename, wstring name, EObjectFlags flags)
	{
		FbxImportConfig* importConfig = new FbxImportConfig();


		Object* newObject;
		if (inObject == nullptr)
		{
		}
		else
		{
			newObject = inObject;
		}
		if (newObject->isA(RStaticMesh::StaticClass()))
		{
			importConfig->mMeshTypeToImport = FBXIT_StaticMesh;
		}

		if (importConfig->mMeshTypeToImport == FBXIT_StaticMesh)
		{

		}

		AirFbx::AFbxImporter* fbxImporter = AirFbx::AFbxImporter::getInstance();
		wstring type = Paths::getExtension(filename);
		if (!fbxImporter->importFromFile(filename, type))
		{
		}
		else
		{
			FbxNode* rootNodeToImport = nullptr;
			rootNodeToImport = fbxImporter->mScene->GetRootNode();

			int32 interestingNodeCount = 1;
			TArray<TArray<FbxNode*>*> skelMeshArray;
			bool bImportStaticMeshLODs = importConfig->mStaticMeshImportData->bImportMeshLODs;
			bool bCombineMeshes = importConfig->mStaticMeshImportData->bCombineMeshes;
			bool bCombineMeshesLOD = false;

			if (importConfig->mMeshTypeToImport == FBXIT_StaticMesh)
			{
				fbxImporter->applyTransformFromSettingsToFbxNode(rootNodeToImport, importConfig->mStaticMeshImportData);
				if (bCombineMeshes && !bImportStaticMeshLODs)
				{
					interestingNodeCount = 1;
				}
				else
				{
					bool bCountLODGroupMeshes = !bImportStaticMeshLODs;
					int32 numLODGroups = 0;

					interestingNodeCount = fbxImporter->getFbxMeshCount(rootNodeToImport, bCountLODGroupMeshes, numLODGroups);

					if (bImportStaticMeshLODs && bCombineMeshes && numLODGroups > 0)
					{
						bCombineMeshes = false;
						bCombineMeshesLOD = true;
					}
				}
			}
			if (interestingNodeCount > 1)
			{
				importConfig->bUsedAsFullName = false;
			}
			if (rootNodeToImport && interestingNodeCount > 0)
			{
				int32 nodeIndex = 0; 
				int32 importedMeshCount = 0;
				if (importConfig->mMeshTypeToImport == FBXIT_StaticMesh)
				{
					std::shared_ptr<RStaticMesh> newStaticMesh = nullptr;
					if (bCombineMeshes)
					{
						TArray<FbxNode*> fbxMeshArray;
						fbxImporter->fillFbxMeshArray(rootNodeToImport, fbxMeshArray, fbxImporter);
						if (fbxMeshArray.size() > 0)
						{
							newStaticMesh = fbxImporter->importStaticMeshAsSingle(fbxMeshArray, name, flags, importConfig->mStaticMeshImportData, static_cast<RStaticMesh*>(newObject), 0);
							if (newStaticMesh != nullptr)
							{
							}
						}
						importedMeshCount = newStaticMesh ? 1 : 0;
					}
					//todo µº»Îπ“µ„
				}
			}
		}
		return std::dynamic_pointer_cast<Object>(newObject->shared_from_this());
	}

}