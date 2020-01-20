#include "MaterialShaderType.h"
namespace Air
{
	Shader* MaterialShaderType::finishCompileShader(const ConstantExpressionSet& constantExpressionSet, const SHAHash& materialShaderMapHash, const ShaderCompileJob& currentJob, const ShaderPipelineType* shaderPipelineType, const wstring& inDebugDescription)
	{
		BOOST_ASSERT(currentJob.bSucceeded);
		ShaderType* specificType = nullptr;
		int32 specificPermutationId = 0;
		if (currentJob.mShaderType->limitShaderResourceToThisType())
		{
			specificType = currentJob.mShaderType;
			specificPermutationId = currentJob.mPermutationId;
		}


		ShaderResource* resource = ShaderResource::findOrCreateShaderResource(currentJob.mOutput, specificType, specificPermutationId);

		if (shaderPipelineType && !shaderPipelineType->shoudlOptimizeUnusedOutputs(currentJob.mInput.mTarget.getPlatform()))
		{
			shaderPipelineType = nullptr;
		}
		Shader* shader = currentJob.mShaderType->findShaderById(ShaderId(materialShaderMapHash, shaderPipelineType, currentJob.mVFType, currentJob.mShaderType, currentJob.mPermutationId, currentJob.mInput.mTarget));
		if (!shader)
		{
			shader = (*mConstructCompiledRef)(CompiledShaderInitializerType(this, currentJob.mPermutationId, currentJob.mOutput, resource, constantExpressionSet, materialShaderMapHash, shaderPipelineType, nullptr, inDebugDescription));
			currentJob.mOutput.mParameterMap.verifyBindingAreComplete(getName(), currentJob.mOutput.mTarget, currentJob.mVFType);
		}
		return shader;
	}

	ShaderCompileJob* MaterialShaderType::beginCompileShader(uint32 shaderMapId, int32 permutationId, const FMaterial* material, ShaderCompilerEnvironment* materialEnvironment, const ShaderPipelineType* shaderPipeline, EShaderPlatform platform, TArray<ShaderCommonCompileJob*>& newJobs)
	{
		ShaderCompileJob* newJob = new ShaderCompileJob(shaderMapId, nullptr, this, permutationId);
		newJob->mInput.mSharedEnvironment = materialEnvironment;
		ShaderCompilerEnvironment& shaderEnvironement = newJob->mInput.mEnvironment;
		setupCompileEnvironment(platform, material, shaderEnvironement);

		globalBeginCompileShader(
			material->getFriendlyName(),
			nullptr,
			this,
			shaderPipeline,
			getShaderFilename(),
			getFunctionName(),
			ShaderTarget(getFrequency(), platform),
			newJob,
			newJobs
		);
		return newJob;
	}
}