#include "MeshMaterialShader.h"
#include "ShaderParameterUtils.h"
#include "Shader.h"
#include "MeshMaterialShaderType.h"
namespace Air
{
	



	static inline bool shouldCacheMeshShader(const MeshMaterialShaderType* shaderType, EShaderPlatform platform, const FMaterial* material, VertexFactoryType* inVertexFactory, int32 permutationId)
	{
		return shaderType->shouldCompilePermutation(platform, material, inVertexFactory, permutationId) && material->shouldCache(platform, shaderType, inVertexFactory) && inVertexFactory->shouldCache(platform, material, shaderType);
	}

	uint32 MeshMaterialShaderMap::beginCompile(uint32 shaderMapId, const MaterialShaderMapId& inShaderMapId, const FMaterial* material, ShaderCompilerEnvironment* materialEnvironment, EShaderPlatform platform, TArray<ShaderCommonCompileJob*>& newJobs)
	{
		if (!mVertexFactoryType)
		{
			return 0;
		}

		uint32 numShadersPerVF = 0;
		TSet<wstring> shaderTypeNames;

		TMap<ShaderType*, ShaderCompileJob*> sharedShaderJobs;
		for (TLinkedList<ShaderType*>::TIterator shaderTypeIt(ShaderType::getTypeList()); shaderTypeIt; shaderTypeIt.next())
		{
			MeshMaterialShaderType* shaderType = shaderTypeIt->getMeshMaterialShaderType();
			if (!shaderType)
			{
				continue;
			}
			for (int32 permutationId = 0; permutationId < shaderType->getPermutationCount(); ++permutationId)
			{


				if (shouldCacheMeshShader(shaderType, platform, material, mVertexFactoryType, permutationId))
				{
					BOOST_ASSERT(inShaderMapId.containsVertexFactoryType(mVertexFactoryType));
					BOOST_ASSERT(inShaderMapId.containsShaderType(shaderType, kUniquePermutationId));
					numShadersPerVF++;
					if (!hasShader(shaderType, permutationId))
					{
						auto* job = shaderType->beginCompileShader(shaderMapId,
							permutationId,
							platform,
							material,
							materialEnvironment,
							mVertexFactoryType,
							nullptr,
							newJobs);
						BOOST_ASSERT(sharedShaderJobs.find(shaderType) == sharedShaderJobs.end());
						sharedShaderJobs.emplace(shaderType, job);
					}
				}
			}
		}
	}

	void MeshMaterialShaderMap::loadMissingShadersFromMemory(const SHAHash& materialShaderMapHash, const FMaterial* material, EShaderPlatform platform)
	{
		for (TLinkedList<ShaderType*>::TIterator shaderTypeIt(ShaderType::getTypeList()); shaderTypeIt; shaderTypeIt.next())
		{
			MeshMaterialShaderType* shaderType = shaderTypeIt->getMeshMaterialShaderType();

			const int32 permutationCount = shaderType ? shaderType->getPermutationCount() : 0;
			for (int32 permutationId = 0; permutationId < permutationCount; ++permutationId)
			{
				if (shouldCacheMeshShader(shaderType, platform, material, mVertexFactoryType, permutationId) && !hasShader((ShaderType*)shaderType, permutationId))
				{
					const ShaderId shaderId(materialShaderMapHash, nullptr, mVertexFactoryType, (ShaderType*)shaderType, permutationId, ShaderTarget(shaderType->getFrequency(), platform));
					Shader* foundShader = ((ShaderType*)shaderType)->findShaderById(shaderId);
					if (foundShader)
					{
						addShader((ShaderType*)shaderType, permutationId, foundShader);
					}
				}
			}
		}
		const bool bHasTessellation = material->getTessellationMode() != MTM_NOTessellation;
		for (TLinkedList<ShaderPipelineType*>::TIterator shaderPipelineIt(ShaderPipelineType::getTypeList()); shaderPipelineIt; shaderPipelineIt.next())
		{
			const ShaderPipelineType* pipelineType = *shaderPipelineIt;
			if (pipelineType && pipelineType->isMeshMaterialTypePipeline() && !hasShaderPipeline(pipelineType) && pipelineType->hasTessellation() == bHasTessellation)
			{
				auto& stages = pipelineType->getStages();
				int32 numshaders = 0;
				for (const ShaderType* shader : stages)
				{
					MeshMaterialShaderType* shaderType = (MeshMaterialShaderType*)shader->getMeshMaterialShaderType();
					if (shaderType && shouldCacheMeshShader(shaderType, platform, material, mVertexFactoryType, kUniquePermutationId))
					{
						++numshaders;
					}
					else
					{
						break;
					}
				}
				if (numshaders == stages.size())
				{
					TArray<Shader*> shaderForPipeline;
					for (auto* shader : stages)
					{
						MeshMaterialShaderType* shaderType = (MeshMaterialShaderType*)shader->getMeshMaterialShaderType();
						if (!hasShader(shaderType, kUniquePermutationId))
						{
							const ShaderId shaderId(materialShaderMapHash, pipelineType->shoudlOptimizeUnusedOutputs(platform) ? pipelineType : nullptr, mVertexFactoryType, shaderType, kUniquePermutationId, ShaderTarget(shaderType->getFrequency(), platform));
							Shader* foundShader = shaderType->findShaderById(shaderId);
							if (foundShader)
							{
								addShader(shaderType, kUniquePermutationId, foundShader);
								shaderForPipeline.add(foundShader);
							}
						}
					}
					if (shaderForPipeline.size() == numshaders && !hasShaderPipeline(pipelineType))
					{
						auto * pipeline = new ShaderPipeline(pipelineType, shaderForPipeline);
						addShaderPipeline(pipelineType, pipeline);
					}
				}
			}
		}
	}

	void MeshMaterialShaderMap::flushShaderByShaderPipelineType(const ShaderPipelineType* shaderPipelineType)
	{
		if (shaderPipelineType->isMeshMaterialTypePipeline())
		{
			removeShaderPipelineType(shaderPipelineType);
		}
	}

	void MeshMaterialShaderMap::flushShaderByShaderType(ShaderType* shaderType)
	{
		if (shaderType->getMeshMaterialShaderType())
		{
			const int32 permutationCount = shaderType->getPermutationCount();
			for (int permutationId = 0; permutationId < permutationCount; ++permutationId)
			{
				removeShaderTypePermutation(shaderType, permutationId);
			}
		}
	}

	Shader* MeshMaterialShaderType::finishCompileShader(const ConstantExpressionSet& constantExpressionSet, const SHAHash& materialShaderMapHash, const ShaderCompileJob& currentJob, const ShaderPipelineType* shaderPipelineType, const wstring& inDebugDescription)
	{
		BOOST_ASSERT(currentJob.bSucceeded);
		BOOST_ASSERT(currentJob.mVFType);

		ShaderType* specificType = currentJob.mShaderType->limitShaderResourceToThisType() ? currentJob.mShaderType : nullptr;
		ShaderResource* resource = ShaderResource::findOrCreateShaderResource(currentJob.mOutput, specificType, currentJob.mPermutationId);
		if (shaderPipelineType && !shaderPipelineType->shoudlOptimizeUnusedOutputs(currentJob.mInput.mTarget.getPlatform()))
		{
			shaderPipelineType = nullptr;
		}
		Shader* shader = currentJob.mShaderType->findShaderById(ShaderId(materialShaderMapHash, shaderPipelineType, currentJob.mVFType, currentJob.mShaderType, currentJob.mPermutationId, currentJob.mInput.mTarget));
		if (!shader)
		{
			shader = (*mConstructCompiledRef)(CompiledShaderInitializerType(this, currentJob.mPermutationId, currentJob.mOutput, resource, constantExpressionSet, materialShaderMapHash, inDebugDescription, shaderPipelineType, currentJob.mVFType));
			currentJob.mOutput.mParameterMap.verifyBindingAreComplete(getName(), currentJob.mOutput.mTarget, currentJob.mVFType);
		}
		return shader;
	}


	ShaderCompileJob* MeshMaterialShaderType::beginCompileShader(uint32 shaderMapId, uint32 permutationId, EShaderPlatform platform, const FMaterial* material, ShaderCompilerEnvironment* materialEnvironment, VertexFactoryType* vertexFactoryType, const ShaderPipelineType* shaderPipeline, TArray<ShaderCommonCompileJob*>& newJobs)
	{
		ShaderCompileJob* newJob = new ShaderCompileJob(shaderMapId, vertexFactoryType, this, permutationId);
		newJob->mInput.mSharedEnvironment = materialEnvironment;
		ShaderCompilerEnvironment& shaderEnvironment = newJob->mInput.mEnvironment;
		BOOST_ASSERT(vertexFactoryType);
		vertexFactoryType->modifyCompilationEnvironment(platform, material, shaderEnvironment);
		setupCompileEnvironment(platform, material, permutationId, shaderEnvironment);

		bool bAllowDevelopmentShaderCompile = material->getAllowDevelopmentShaderCompile();
		globalBeginCompileShader(
			material->getFriendlyName(),
			vertexFactoryType,
			this,
			shaderPipeline,
			getShaderFilename(),
			getFunctionName(),
			ShaderTarget(getFrequency(), platform),
			newJob,
			newJobs,
			bAllowDevelopmentShaderCompile);
		return newJob;
	}

	bool MeshMaterialShader::serialize(Archive& ar)
	{
		bool bShaderHasOutdatedParameters = MaterialShader::serialize(ar);
		ar << mPassConstantBuffer;
		bShaderHasOutdatedParameters |= ar << mVertexFactoryParameters;
		return bShaderHasOutdatedParameters;
	}

	uint32 MeshMaterialShader::getAllocatedSize() const
	{
		return MaterialShader::getAllocatedSize() + mVertexFactoryParameters.getAllocatedSize();
	}

	inline bool MeshMaterialShaderMap::isMeshShaderComplete(const MeshMaterialShaderMap* meshShaderMap, EShaderPlatform platform, const FMaterial* material, const MeshMaterialShaderType* shaderType, const ShaderPipelineType* pipeline, VertexFactoryType* inVertexFactoryType, int32 permutationId, bool bSilent)
	{
		if (!shouldCacheMeshShader(shaderType, platform, material, inVertexFactoryType, permutationId))
		{
			return true;
		}
		if (!meshShaderMap || (pipeline && !meshShaderMap->hasShaderPipeline(pipeline)) || (!pipeline && !meshShaderMap->hasShader((ShaderType*)shaderType, permutationId)))
		{
			if (!bSilent)
			{
				if (pipeline)
				{

				}
				else
				{

				}
			}
			return false;
		}
		return true;
	}

	bool MeshMaterialShaderMap::isComplete(const MeshMaterialShaderMap* meshShaderMap, EShaderPlatform platform, const FMaterial* material, VertexFactoryType* inVertexFactoryType, bool bSilent)
	{
		for (TLinkedList<ShaderType*>::TIterator shaderTypeIt(ShaderType::getTypeList()); shaderTypeIt; shaderTypeIt.next())
		{
			MeshMaterialShaderType* shaderType = shaderTypeIt->getMeshMaterialShaderType();
			const int32 permutationCount = shaderType ? shaderType->getPermutationCount() : 0;
			for (int32 permutationId = 0; permutationId < permutationCount; ++permutationId)
			{

				if (shaderType && !isMeshShaderComplete(meshShaderMap, platform, material, shaderType, nullptr, inVertexFactoryType, permutationId, bSilent))
				{
					return false;
				}
			}
		}

		const bool bHasTessellation = material->getTessellationMode() != MTM_NOTessellation;
		for (TLinkedList<ShaderPipelineType*>::TIterator shaderPipelineIt(ShaderPipelineType::getTypeList()); shaderPipelineIt; shaderPipelineIt.next())
		{
			const ShaderPipelineType* shaderPipelineType = *shaderPipelineIt;
			if (shaderPipelineType->isMeshMaterialTypePipeline() && shaderPipelineType->hasTessellation() == bHasTessellation)
			{
				auto& stages = shaderPipelineType->getStages();
				int32 numShouldCache = 0;
				for (int32 index = 0; index < stages.size(); ++index)
				{
					auto* shaderType = stages[index]->getMeshMaterialShaderType();
					if (shouldCacheMeshShader(shaderType, platform, material, inVertexFactoryType, kUniquePermutationId))
					{
						++numShouldCache;
					}
					else
					{
						break;
					}
				}
				if (numShouldCache == stages.size())
				{
					for (int32 index = 0; index < stages.size(); ++index)
					{
						auto* shaderType = stages[index]->getMeshMaterialShaderType();
						if (shaderType && !isMeshShaderComplete(meshShaderMap, platform, material, shaderType, shaderPipelineType, inVertexFactoryType, kUniquePermutationId, bSilent))
						{
							return false;
						}
					}
				}
			}
		}
		return false;
	}

}