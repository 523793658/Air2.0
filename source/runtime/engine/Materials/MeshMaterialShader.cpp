#include "Materials/MeshMaterialShader.h"
#include "ShaderParameterUtils.h"
#include "Shader.h"
#include "MeshMaterialShaderType.h"
namespace Air
{
	template<typename ShaderRHIParamRef>
	void MeshMaterialShader::setMesh(RHICommandList& RHICmdList, const ShaderRHIParamRef shaderRHI, const VertexFactory* vertexFactory, const SceneView& view, const PrimitiveSceneProxy* proxy, const MeshBatchElement& batchElement, const DrawingPolicyRenderState& drawRenderState, uint32 dataFlags /* = 0 */)
	{
		mVertexFactoryParameters.setMesh(RHICmdList, this, vertexFactory, view, batchElement, dataFlags);
		if (isValidRef(batchElement.mPrimitiveConstantBuffer))
		{
			setConstantBufferParameter(RHICmdList, shaderRHI, getConstantBufferParameter<PrimitiveConstantShaderParameters>(), batchElement.mPrimitiveConstantBuffer);
		}
		else
		{
			BOOST_ASSERT(batchElement.mPrimitiveConstantBufferResource);
			setConstantBufferParameter(RHICmdList, shaderRHI, getConstantBufferParameter<PrimitiveConstantShaderParameters>(), *batchElement.mPrimitiveConstantBufferResource);
		}
	}
#define IMPLEMENT_MESH_MATERIAL_SHADER_SetMesh(ShaderRHIParamRef)	\
	template ENGINE_API void MeshMaterialShader::setMesh<ShaderRHIParamRef>(\
		RHICommandList& RHICmdList, \
		const ShaderRHIParamRef shaderRHI,\
		const VertexFactory* vertexFactory,  \
		const SceneView& view,	\
		const PrimitiveSceneProxy* proxy,	\
		const MeshBatchElement& batchElement,\
		const DrawingPolicyRenderState& drawRenderState,	\
		uint32 dataFlags	\
);

	IMPLEMENT_MESH_MATERIAL_SHADER_SetMesh(VertexShaderRHIParamRef);
	IMPLEMENT_MESH_MATERIAL_SHADER_SetMesh(HullShaderRHIParamRef);
	IMPLEMENT_MESH_MATERIAL_SHADER_SetMesh(DomainShaderRHIParamRef);
	IMPLEMENT_MESH_MATERIAL_SHADER_SetMesh(GeometryShaderRHIParamRef);
	IMPLEMENT_MESH_MATERIAL_SHADER_SetMesh(PixelShaderRHIParamRef);
	IMPLEMENT_MESH_MATERIAL_SHADER_SetMesh(ComputeShaderRHIParamRef);

	static inline bool shouldCacheMeshShader(const MeshMaterialShaderType* shaderType, EShaderPlatform platform, const FMaterial* material, VertexFactoryType* inVertexFactory)
	{
		return shaderType->shouldCache(platform, material, inVertexFactory) && material->shouldCache(platform, shaderType, inVertexFactory) && inVertexFactory->shouldCache(platform, material, shaderType);
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
			if (shaderType && shouldCacheMeshShader(shaderType, platform, material, mVertexFactoryType))
			{
				BOOST_ASSERT(inShaderMapId.containsVertexFactoryType(mVertexFactoryType));
				BOOST_ASSERT(inShaderMapId.containsShaderType(shaderType));
				numShadersPerVF++;
				if (!hasShader(shaderType))
				{
					auto* job = shaderType->beginCompileShader(shaderMapId,
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

	void MeshMaterialShaderMap::loadMissingShadersFromMemory(const SHAHash& materialShaderMapHash, const FMaterial* material, EShaderPlatform platform)
	{
		for (TLinkedList<ShaderType*>::TIterator shaderTypeIt(ShaderType::getTypeList()); shaderTypeIt; shaderTypeIt.next())
		{
			MeshMaterialShaderType* shaderType = shaderTypeIt->getMeshMaterialShaderType();
			if (shaderType && shouldCacheMeshShader(shaderType, platform, material, mVertexFactoryType) && !hasShader((ShaderType*)shaderType))
			{
				const ShaderId shaderId(materialShaderMapHash, nullptr, mVertexFactoryType, (ShaderType*)shaderType, ShaderTarget(shaderType->getFrequency(), platform));
				Shader* foundShader = ((ShaderType*)shaderType)->findShaderById(shaderId);
				if (foundShader)
				{
					addShader((ShaderType*)shaderType, foundShader);
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
					if (shaderType && shouldCacheMeshShader(shaderType, platform, material, mVertexFactoryType))
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
						if (!hasShader(shaderType))
						{
							const ShaderId shaderId(materialShaderMapHash, pipelineType->shoudlOptimizeUnusedOutputs() ? pipelineType : nullptr, mVertexFactoryType, shaderType, ShaderTarget(shaderType->getFrequency(), platform));
							Shader* foundShader = shaderType->findShaderById(shaderId);
							if (foundShader)
							{
								addShader(shaderType, foundShader);
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
			removeShaderType(shaderType);
		}
	}

	Shader* MeshMaterialShaderType::finishCompileShader(const ConstantExpressionSet& constantExpressionSet, const SHAHash& materialShaderMapHash, const ShaderCompileJob& currentJob, const ShaderPipelineType* shaderPipelineType, const wstring& inDebugDescription)
	{
		BOOST_ASSERT(currentJob.bSucceeded);
		BOOST_ASSERT(currentJob.mVFType);

		ShaderType* specificType = currentJob.mShaderType->limitShaderResourceToThisType() ? currentJob.mShaderType : nullptr;
		ShaderResource* resource = ShaderResource::findOrCreateShaderResource(currentJob.mOutput, specificType);
		if (shaderPipelineType && !shaderPipelineType->shoudlOptimizeUnusedOutputs())
		{
			shaderPipelineType = nullptr;
		}
		Shader* shader = currentJob.mShaderType->findShaderById(ShaderId(materialShaderMapHash, shaderPipelineType, currentJob.mVFType, currentJob.mShaderType, currentJob.mInput.mTarget));
		if (!shader)
		{
			shader = (*mConstructCompiledRef)(CompiledShaderInitializerType(this, currentJob.mOutput, resource, constantExpressionSet, materialShaderMapHash, inDebugDescription, shaderPipelineType, currentJob.mVFType));
			currentJob.mOutput.mParameterMap.verifyBindingAreComplete(getName(), currentJob.mOutput.mTarget, currentJob.mVFType);
		}
		return shader;
	}


	ShaderCompileJob* MeshMaterialShaderType::beginCompileShader(uint32 shaderMapId, EShaderPlatform platform, const FMaterial* material, ShaderCompilerEnvironment* materialEnvironment, VertexFactoryType* vertexFactoryType, const ShaderPipelineType* shaderPipeline, TArray<ShaderCommonCompileJob*>& newJobs)
	{
		ShaderCompileJob* newJob = new ShaderCompileJob(shaderMapId, vertexFactoryType, this);
		newJob->mInput.mSharedEnvironment = materialEnvironment;
		ShaderCompilerEnvironment& shaderEnvironment = newJob->mInput.mEnvironment;
		BOOST_ASSERT(vertexFactoryType);
		vertexFactoryType->modifyCompilationEnvironment(platform, material, shaderEnvironment);
		setupCompileEnvironment(platform, material, shaderEnvironment);

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
}