#include "RHICommandList.h"

#include "Materials/MaterialShader.h"
#include "Containers/String.h"
#include "Serialization/MemoryWriter.h"
#include "ScenePrivate.h"
#include "ParameterCollection.h"
#include "HAL/PlatformTime.h"
#include "DerivedDataCacheInterface.h"
#include "Misc/CoreMisc.h"
#include "ShaderParameterUtils.h"
namespace Air
{

	int32 MaterialShader::bAllowCachedConstantExpressions = true;

	MaterialShader::MaterialShader(const MaterialShaderType::CompiledShaderInitializerType& initializer)
		:Shader(initializer)
		, mDebugConstantExpressionCBLayout(RHIConstantBufferLayout::Zero)
	{
		mMaterialConstantBuffer.bind(initializer.mParameterMap, TEXT("Material"));
		for (int32 collectionIndex = 0; collectionIndex < initializer.mConstantExpressionSet.mParameterCollections.size(); collectionIndex++)
		{
			ShaderConstantBufferParameter collectionParameter;
			collectionParameter.bind(initializer.mParameterMap, printf(TEXT("MaterialCollection%u"), collectionIndex).c_str());
			mParameterCollectionConstantBuffers.add(collectionParameter);
		}
		for (int32 index = 0; index < initializer.mConstantExpressionSet.mPerFrameConstantScalarExpressions.size(); index++)
		{
			ShaderParameter parameter;
			parameter.bind(initializer.mParameterMap, printf(TEXT("AIR_Material_PerFrameScalarExpression%u"), index).c_str());
			mPerFrameScalarExpressions.add(parameter);
		}
		for (int32 index = 0; index < initializer.mConstantExpressionSet.mPerFrameConstantVectorExpressions.size(); index++)
		{
			ShaderParameter parameter;
			parameter.bind(initializer.mParameterMap, printf(TEXT("Air_Material_PerframeVectorExpression%u"), index).c_str());
			mPerFrameVectorExpressions.add(parameter);
		}

		for (int32 index = 0; index < initializer.mConstantExpressionSet.mPerFramePrevConstantScalarExpressions.size(); index++)
		{
			ShaderParameter parameter;
			parameter.bind(initializer.mParameterMap, printf(TEXT("Air_Material_PerFramePrevScalarExpression%u"), index).c_str());
			mPerFramePrevScalarExpressions.add(parameter);
		}

		for (int32 index = 0; index < initializer.mConstantExpressionSet.mPerFramePrevConstantVectorExpressions.size(); index++)
		{
			ShaderParameter parameter;
			parameter.bind(initializer.mParameterMap, printf(TEXT("Air_Material_PerFramePrevVectorExpression%u"), index).c_str());
			mPerFramePrevVectorExpressions.add(parameter);
		}
		mSceneColorCopyTexture.bind(initializer.mParameterMap, TEXT("SceneColorCopyTexture"));
		mSceneColorCopyTextureSampler.bind(initializer.mParameterMap, TEXT("SceneColorCopyTextureSampler"));
}
	

#if !(BUILD_TEST || BUILD_SHIPPING || !WITH_EDITOR)
	void MaterialShader::verifyExpressionAndShaderMaps(const MaterialRenderProxy* materialRenderProxy, const FMaterial& material, const ConstantExpressionCache* constantExpressionCache)
	{}
#endif


	static wstring getMaterialShaderMapKeyString(const MaterialShaderMapId& shaderMapId, EShaderPlatform platform)
	{
		wstring format = legacyShaderPlatformToShaderFormat(platform);
		wstring shaderMapKeyString = format + TEXT("_") + boost::lexical_cast<wstring>(getTargetPlatformManagerRef().shaderFormatVersion(format)) + TEXT("_");
		shaderMapAppendKeyString(platform, shaderMapKeyString);
		shaderMapId.appendKeyString(shaderMapKeyString);
		return shaderMapKeyString;
	}

	ConstantBufferRHIParamRef MaterialShader::getParameterCollectionBuffer(const Guid& Id, const SceneInterface* sceneInterface) const
	{
		const Scene* scene = (const Scene*)sceneInterface;
		ConstantBufferRHIParamRef constantBuffer = scene ? scene->getParameterCollectionBuffer(Id) : ConstantBufferRHIParamRef();
		if (!constantBuffer)
		{
			constantBuffer = GDefaultMaterialParameterCollectionInstances.findChecked(Id)->getConstantBuffer();
		}
		return constantBuffer;
	}

	
	void MaterialShaderMap::loadFromDerivedDataCache(const FMaterial* material, const MaterialShaderMapId& shaderMapId, EShaderPlatform platform, TRefCountPtr<MaterialShaderMap>& inOutShaderMap)
	{
		if (inOutShaderMap != nullptr)
		{


		}
		else
		{
			{
				TArray<uint8> cachedData;
				const wstring dataKey = getMaterialShaderMapKeyString(shaderMapId, platform);
				//if(getDerivedDataCacheRef().gets)
			}
		}
	}
	

	void MaterialShaderMapId::appendKeyString(wstring& keyString) const
	{
		keyString += mBaseMaterialId.toString();
		keyString += TEXT("_");
		wstring qualityLevelName;
		getMaterialQualityLevelName(mQualityLevel, qualityLevelName);
		keyString += qualityLevelName + TEXT("_");
		wstring featureLevelString;
		getFeatureLevelName(mFeatureLevel, featureLevelString);
		keyString += featureLevelString + TEXT("_");
		mParameterSet.appendKeyString(keyString);

		keyString += TEXT("_");
		keyString += boost::lexical_cast<wstring>(mUsage);
		keyString += TEXT("_");
		for (int32 functionIndex = 0; functionIndex < mReferencedFunctions.size(); functionIndex++)
		{
			keyString += mReferencedFunctions[functionIndex].toString();
		}
		keyString += TEXT("_");
		for (int32 collectionIndex = 0; collectionIndex < mReferencedParameterCollections.size(); collectionIndex++)
		{
			keyString += mReferencedParameterCollections[collectionIndex].toString();
		}
		TMap<const TCHAR*, CachedConstantBufferDeclaration> referencedConstantBuffers;
		for (int32 shaderIndex = 0; shaderIndex < mShaderTypeDependencies.size(); shaderIndex++)
		{
			const ShaderTypeDependency& shaderTypeDependency = mShaderTypeDependencies[shaderIndex];
			keyString += TEXT("_");
			keyString += shaderTypeDependency.mShaderType->getName();
			keyString += shaderTypeDependency.mSourceHash.toString();
			shaderTypeDependency.mShaderType->getSerializationHistory().appendKeyString(keyString);
			const TMap<const TCHAR*, CachedConstantBufferDeclaration>& referencedConstantBufferStructsCache = shaderTypeDependency.mShaderType->getReferencedConstantBufferStructsCache();
			for (auto& it : referencedConstantBufferStructsCache)
			{
				referencedConstantBuffers.emplace(it.first, it.second);
			}
		}
		for (int32 typeIndex = 0; typeIndex < mShaderPipelineTypeDependencies.size(); typeIndex++)
		{
			const ShaderPipelineTypeDependency& dependency = mShaderPipelineTypeDependencies[typeIndex];
			keyString += TEXT("_");
			keyString += dependency.mShaderPipelineType->getName();
			keyString += dependency.mStagesSourceHash.toString();
			for (const ShaderType* shaderType : dependency.mShaderPipelineType->getStages())
			{
				const TMap<const TCHAR*, CachedConstantBufferDeclaration>& refrencedConstantStructsCache = shaderType->getReferencedConstantBufferStructsCache();
				for (auto& it : refrencedConstantStructsCache)
				{
					referencedConstantBuffers.emplace(it.first, it.second);
				}
			}
		}
		for (int32 VFIndex = 0; VFIndex < mVertexFactoryTypeDependencies.size(); VFIndex++)
		{
			keyString += TEXT("_");
			const VertexFactoryTypeDependency & vfDependency = mVertexFactoryTypeDependencies[VFIndex];
			keyString += vfDependency.mVertexFactoryType->getName();
			keyString += vfDependency.mVFSourceHash.toString();

			for (int32 frequency = 0; frequency < SF_NumFrequencies; frequency++)
			{
				vfDependency.mVertexFactoryType->getSerializationHistory((EShaderFrequency)frequency);
			}
			const TMap<const TCHAR*, CachedConstantBufferDeclaration>& referenceCosntntBufferStructCache = vfDependency.mVertexFactoryType->getReferencedConstantBufferStructsCache();
			for (auto& it : referenceCosntntBufferStructCache)
			{
				referencedConstantBuffers.emplace(it.first, it.second);
			}
		}
		{
			TArray<uint8> tempData;
			SerializationHistory serializationHistory;
			MemoryWriter ar(tempData, true);
			ShaderSaveArchive saveArchive(ar, serializationHistory); 
			serializeConstantBufferInfo(saveArchive, referencedConstantBuffers);
			serializationHistory.appendKeyString(keyString);
		}
		keyString += bytesToHex(&mTextureReferencesHash.mHash[0], sizeof(mTextureReferencesHash.mHash));
		keyString += bytesToHex(&mBasePropertyOverridesHash.mHash[0], sizeof(mBasePropertyOverridesHash.mHash));
	}
	

	/*bool MaterialShaderMap::processCompilationResults(const TArray<ShaderCommonCompileJob*>& inCompilationResults, int32 & resultIndex, float& timeBudget, TMap<const VertexFactoryType*, TArray<const ShaderPipelineType*>>& sharedPipelines)
	{
		BOOST_ASSERT(resultIndex < inCompilationResults.size());
		double startTime = PlatformTime::seconds();
		SHAHash materialShaderMapHash;
		mShaderMapId.getMaterialHash(materialShaderMapHash);

		do 
		{
			ShaderCompileJob* singleJob = inCompilationResults[resultIndex]->getSingleShaderJob();
			if (singleJob)
			{
				processCompilationResultsForSingleJob(singleJob, nullptr, materialShaderMapHash);
				for (auto pair : singleJob->mSharingPipelines)
				{
					auto& sharedPipelinesPerVF = sharedPipelines.findOrAdd(singleJob->mVFType);
					for (auto* pipeline : pair.second)
					{
						sharedPipelinesPerVF.addUnique(pipeline);
					}
				}
			}
			else
			{
				auto* pipelineJob = inCompilationResults[resultIndex]->getShaderPipelineJob();
				BOOST_ASSERT(pipelineJob);

				const ShaderPipelineCompileJob& currentJob = *pipelineJob;
				BOOST_ASSERT(currentJob.mId == mCompilingId);
				TArray<Shader*> shaderStages;
				VertexFactoryType* vertexFactoryType = currentJob.mStageJobs[0]->getSingleShaderJob()->mVFType;
				for (int32 index = 0; index < currentJob.mStageJobs.size(); ++index)
				{
					singleJob = currentJob.mStageJobs[index]->getSingleShaderJob();
					Shader* shader = processCompilationResultsForSingleJob(singleJob, pipelineJob->mShaderPipeline, materialShaderMapHash);
					shaderStages.add(shader);
					BOOST_ASSERT(vertexFactoryType == currentJob.mStageJobs[index]->getSingleShaderJob()->mVFType);
				}
				ShaderPipeline* shaderpipeline = new ShaderPipeline(pipelineJob->mShaderPipeline, shaderStages);
				if (shaderpipeline)
				{
					if (vertexFactoryType)
					{
						BOOST_ASSERT(vertexFactoryType->isUsedWithMaterial());
						MeshMaterialShaderMap* meshShaderMap = nullptr;
						int32 meshShaderMapIndex = INDEX_NONE;
						for (int32 shaderMapIndex = 0; shaderMapIndex < mMeshShaderMaps.size(); shaderMapIndex++)
						{
							if (mMeshShaderMaps[shaderMapIndex].getVertexFactoryType() == vertexFactoryType)
							{
								meshShaderMap = &mMeshShaderMaps[shaderMapIndex];
								meshShaderMapIndex = shaderMapIndex;
								break;
							}
						}
						BOOST_ASSERT(meshShaderMap);
						BOOST_ASSERT(!meshShaderMap->hasShaderPipeline(shaderpipeline->mPipelineType));
						meshShaderMap->addShaderPipeline(pipelineJob->mShaderPipeline, shaderpipeline);
					}
					else
					{
						BOOST_ASSERT(!hasShaderPipeline(shaderpipeline->mPipelineType));
						addShaderPipeline(pipelineJob->mShaderPipeline, shaderpipeline);
					}
				}
			}
			resultIndex++;
			double newStartTime = PlatformTime::seconds();
			timeBudget -= newStartTime - startTime;
			startTime = newStartTime;
		} while ((timeBudget > 0.0f) && (resultIndex < inCompilationResults.size()));
		if (resultIndex == inCompilationResults.size())
		{
			{
				for (int32 shaderMapIndex = 0; shaderMapIndex < mMeshShaderMaps.size(); shaderMapIndex++)
				{
					auto* meshShaderMap = &mMeshShaderMaps[shaderMapIndex];
					auto* vertexFactory = meshShaderMap->getVertexFactoryType();
					auto foundSharedPipelines = sharedPipelines.find(vertexFactory);
					if (vertexFactory && foundSharedPipelines != sharedPipelines.end())
					{
						for (const ShaderPipelineType* shaderPipelineType : foundSharedPipelines->second)
						{
							if (shaderPipelineType->isMeshMaterialTypePipeline() && !meshShaderMap->hasShaderPipeline(shaderPipelineType))
							{
								auto& stageTypes = shaderPipelineType->getStages();
								TArray<Shader*> shaderStages;
								for (int32 index = 0; index < stageTypes.size(); index++)
								{
									MeshMaterialShaderType* shaderType = ((MeshMaterialShaderType*)(stageTypes[index]))->getMeshMaterialShaderType();
									Shader* shader = meshShaderMap->getShader(shaderType);
									BOOST_ASSERT(shader);
									shaderStages.add(shader);
								}
								BOOST_ASSERT(stageTypes.size() == shaderStages.size());
								ShaderPipeline* shaderPipeline = new ShaderPipeline(shaderPipelineType, shaderStages);
								meshShaderMap->addShaderPipeline(shaderPipelineType, shaderPipeline);
							}
						}
					}
				}
				auto foundSharedPipelines = sharedPipelines.find(nullptr);
				if (foundSharedPipelines != sharedPipelines.end())
				{
					for (const ShaderPipelineType* shaderPipelineType : foundSharedPipelines->second)
					{
						if (shaderPipelineType->isMaterialTypePipeline() && !hasShaderPipeline(shaderPipelineType))
						{
							auto& stageTypes = shaderPipelineType->getStages();
							TArray<Shader*> shaderStages;
							for (int32 index = 0; index < stageTypes.size(); ++index)
							{
								MaterialShaderType* shaderType = ((MaterialShaderType*)(stageTypes[index]))->getMaterialShaderType();
								Shader* shader = getShader(shaderType);
								BOOST_ASSERT(shader);
								shaderStages.add(shader);
							}
							BOOST_ASSERT(stageTypes.size() == shaderStages.size());
							ShaderPipeline* shaderPipeline = new ShaderPipeline(shaderPipelineType, shaderStages);
							addShaderPipeline(shaderPipelineType, shaderPipeline);
						}
					}
				}
			}
			for (int32 shaderMapIndex = mMeshShaderMaps.size() - 1; shaderMapIndex >= 0; shaderMapIndex--)
			{
				if (mMeshShaderMaps[shaderMapIndex].getNumShaders() == 0 && mMeshShaderMaps[shaderMapIndex].getNumShaderPipelines() == 0)
				{
					mMeshShaderMaps.removeAt(shaderMapIndex);
				}
			}
			initOrderedMeshShaderMaps();
			if (bIsPersistent)
			{
				saveToDerivedDataCache();
			}
			bCompilationFinalized = true;
			return true;
		}
		return false;
	}
*/
	

	

	void MaterialShaderMap::initOrderedMeshShaderMaps()
	{
		mOrderredMeshShaderMaps.empty(VertexFactoryType::getNumVertexFactoryTypes());
		mOrderredMeshShaderMaps.addZeroed(VertexFactoryType::getNumVertexFactoryTypes());
		for (int32 index = 0; index < mMeshShaderMaps.size(); index++)
		{
			BOOST_ASSERT(mMeshShaderMaps[index].getVertexFactoryType());
			const int32 vfIndex = mMeshShaderMaps[index].getVertexFactoryType()->getId();
			mOrderredMeshShaderMaps[vfIndex] = &mMeshShaderMaps[index];
		}
	}

	void MaterialShaderMap::saveToDerivedDataCache()
	{
		TArray<uint8> saveData;
		MemoryWriter ar(saveData, true);
		serialize(ar);
		getDerivedDataCacheRef().put(getMaterialShaderMapKeyString(mShaderMapId, mPlatform).c_str(), saveData);
	}

	struct CompareMeshShaderMaps
	{
		FORCEINLINE bool operator ()(const MeshMaterialShaderMap& A, const MeshMaterialShaderMap& B) const
		{
			return CString::strncmp(A.getVertexFactoryType()->getName(), B.getVertexFactoryType()->getName(), Math::min(CString::strlen(A.getVertexFactoryType()->getName()), CString::strlen(B.getVertexFactoryType()->getName()))) > 0;
		}
	};

	void MaterialShaderMap::serialize(Archive& ar, bool bInlineShaderResources /* = true */)
	{
		mShaderMapId.serialize(ar);
		int32 tempPlatform = (int32)mPlatform;
		ar << tempPlatform;
		mPlatform = (EShaderPlatform)tempPlatform;
		ar << mFriendlyName;

		mMaterialCompilationOutput.serialize(ar);

		if (ar.isSaving())
		{
			TShaderMap<MaterialShaderType>::serializeInline(ar, bInlineShaderResources, false);
			registerSerializedShaders();
			int32 numMeshShaderMaps = 0;
			for (int32 vfindex = 0; vfindex < mOrderredMeshShaderMaps.size(); vfindex++)
			{
				MeshMaterialShaderMap* meshShaderMap = mOrderredMeshShaderMaps[vfindex];
				if (meshShaderMap)
				{
					numMeshShaderMaps++;
				}
			}
			ar << numMeshShaderMaps;
			TArray<MeshMaterialShaderMap*> sortedMeshShaderMaps;
			sortedMeshShaderMaps.empty(mMeshShaderMaps.size());
			for (int32 mapIndex = 0; mapIndex < mMeshShaderMaps.size(); mapIndex++)
			{
				sortedMeshShaderMaps.add(&mMeshShaderMaps[mapIndex]);
			}

			sortedMeshShaderMaps.sort(CompareMeshShaderMaps());
			for (int32 mapindex = 0; mapindex < sortedMeshShaderMaps.size(); mapindex)
			{
				MeshMaterialShaderMap* meshShaderMap = sortedMeshShaderMaps[mapindex];
				if (meshShaderMap)
				{
					VertexFactoryType* vfType = meshShaderMap->getVertexFactoryType();
					BOOST_ASSERT(vfType);
					ar << vfType;
					meshShaderMap->serializeInline(ar, bInlineShaderResources, false);
					meshShaderMap->registerSerializedShaders();
				}
			}
		}
		if (ar.isLoading())
		{
			mMeshShaderMaps.empty();
			for (TLinkedList<VertexFactoryType*>::TIterator vertexFactoryTypeIt(VertexFactoryType::getTypeList()); vertexFactoryTypeIt; vertexFactoryTypeIt.next())
			{
				VertexFactoryType* vertexFactoryType = *vertexFactoryTypeIt;
				BOOST_ASSERT(vertexFactoryType);
				if (vertexFactoryType->isUsedWithMaterial())
				{
					new(mMeshShaderMaps)MeshMaterialShaderMap(vertexFactoryType);
				}
			}
			initOrderedMeshShaderMaps();

			TShaderMap<MaterialShaderType>::serializeInline(ar, bInlineShaderResources, false);
			int32 numMeshShaderMaps = 0;
			ar << numMeshShaderMaps;
			for (int32 vfIndex = 0; vfIndex < numMeshShaderMaps; vfIndex++)
			{
				VertexFactoryType* vfType = nullptr;
				ar << vfType;
				BOOST_ASSERT(vfType);
				MeshMaterialShaderMap* meshShaderMap = mOrderredMeshShaderMaps[vfType->getId()];
				BOOST_ASSERT(meshShaderMap);
				meshShaderMap->serializeInline(ar, bInlineShaderResources, false);
			}
		}
	}

	void MaterialShaderMapId::serialize(Archive& ar)
	{
		uint32 usageInt = mUsage;
		ar << usageInt;
		mUsage = (EMaterialShaderMapUsage::Type)usageInt;
		ar << mBaseMaterialId;
		ar << (int32&)mQualityLevel;
		ar << (int32&)mFeatureLevel;
		mParameterSet.serialize(ar);
		ar << mReferencedFunctions;

		ar << mReferencedParameterCollections;

		ar << mShaderTypeDependencies;

		ar << mShaderPipelineTypeDependencies;

		ar << mVertexFactoryTypeDependencies;

		ar << mTextureReferencesHash;

		ar << mBasePropertyOverridesHash;
	}

	void MaterialShaderType::beginCompileShaderPipeline(uint32 shaderMapId, EShaderPlatform platform, const FMaterial* material, ShaderCompilerEnvironment* materialEnvironment, const ShaderPipelineType* shaderPipeline, const TArray<MaterialShaderType*>& shaderStages, TArray<ShaderCommonCompileJob*>& newJobs)
	{
		BOOST_ASSERT(shaderStages.size() > 0);
		BOOST_ASSERT(shaderPipeline);
		auto* newPipelineJob = new ShaderPipelineCompileJob(shaderMapId, shaderPipeline, shaderStages.size());
		for (int32 index = 0; index < shaderStages.size(); ++index)
		{
			auto* shaderStage = shaderStages[index];
			shaderStage->beginCompileShader(shaderMapId, material, materialEnvironment, shaderPipeline, platform, newPipelineJob->mStageJobs);
		}
		newJobs.add(newPipelineJob);
	}

	
}