#include "RHICommandList.h"

#include "MaterialShader.h"
#include "Containers/FString.h"
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
	{
		mMaterialConstantBuffer.bind(initializer.mParameterMap, TEXT("Material"));
		for (int32 collectionIndex = 0; collectionIndex < initializer.mConstantExpressionSet.mParameterCollections.size(); collectionIndex++)
		{
			ShaderConstantBufferParameter collectionParameter;
			collectionParameter.bind(initializer.mParameterMap, printf(TEXT("MaterialCollection%u"), collectionIndex).c_str());
			mParameterCollectionConstantBuffers.add(collectionParameter);
		}
		mSceneTextureParameters.bind(initializer);

	}
	

	template<typename TRHIShader>
	void MaterialShader::setParameters(RHICommandList& RHICmdList, const TRHIShader* shaderRHI, const MaterialRenderProxy* materialRenderProxy, const FMaterial& material, const SceneView& view, const TConstantBufferRef<ViewConstantShaderParameters>& viewConstantBuffer, ESceneTextureSetupMode sceneTextureSetupMode)
	{
		setViewParameters(RHICmdList, shaderRHI, view, viewConstantBuffer);
		MaterialShader::setParametersInner(RHICmdList, shaderRHI, materialRenderProxy, material, view);
		mSceneTextureParameters.set(RHICmdList, shaderRHI, view.mFeatureLevel, sceneTextureSetupMode);
	}



#if !(BUILD_TEST || BUILD_SHIPPING || !WITH_EDITOR)
	void MaterialShader::verifyExpressionAndShaderMaps(const MaterialRenderProxy* materialRenderProxy, const FMaterial& material, const ConstantExpressionCache* constantExpressionCache)
	{}
#endif


	

	RHIConstantBuffer* MaterialShader::getParameterCollectionBuffer(const Guid& Id, const SceneInterface* sceneInterface) const
	{
		const Scene* scene = (const Scene*)sceneInterface;
		RHIConstantBuffer* constantBuffer = scene ? scene->getParameterCollectionBuffer(Id) : nullptr;
		if (!constantBuffer)
		{
			constantBuffer = GDefaultMaterialParameterCollectionInstances.findChecked(Id)->getConstantBuffer();
		}
		return constantBuffer;
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
	

	

	

	

	
	void MaterialShaderType::beginCompileShaderPipeline(uint32 shaderMapId, EShaderPlatform platform, const FMaterial* material, ShaderCompilerEnvironment* materialEnvironment, const ShaderPipelineType* shaderPipeline, const TArray<MaterialShaderType*>& shaderStages, TArray<ShaderCommonCompileJob*>& newJobs)
	{
		BOOST_ASSERT(shaderStages.size() > 0);
		BOOST_ASSERT(shaderPipeline);
		auto* newPipelineJob = new ShaderPipelineCompileJob(shaderMapId, shaderPipeline, shaderStages.size());
		for (int32 index = 0; index < shaderStages.size(); ++index)
		{
			auto* shaderStage = shaderStages[index];
			shaderStage->beginCompileShader(shaderMapId, kUniquePermutationId, material, materialEnvironment, shaderPipeline, platform, newPipelineJob->mStageJobs);
		}
		newJobs.add(newPipelineJob);
	}

	
}