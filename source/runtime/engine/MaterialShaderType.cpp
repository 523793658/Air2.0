#include "MaterialShaderType.h"
namespace Air
{
	Shader* MaterialShaderType::finishCompileShader(const ConstantExpressionSet& constantExpressionSet, const SHAHash& materialShaderMapHash, const ShaderCompileJob& currentJob, const ShaderPipelineType* shaderPipelineType, const wstring& inDebugDescription)
	{
		BOOST_ASSERT(currentJob.bSucceeded);
		ShaderType*specificType = currentJob.mShaderType->limitShaderResourceToThisType() ? currentJob.mShaderType : nullptr;
		ShaderResource* resource = ShaderResource::findOrCreateShaderResource(currentJob.mOutput, specificType);

		if (shaderPipelineType && !shaderPipelineType->shoudlOptimizeUnusedOutputs())
		{
			shaderPipelineType = nullptr;
		}
		Shader* shader = currentJob.mShaderType->findShaderById(ShaderId(materialShaderMapHash, shaderPipelineType, currentJob.mVFType, currentJob.mShaderType, currentJob.mInput.mTarget));
		if (!shader)
		{
			shader = (*mConstructCompiledRef)(CompiledShaderInitializerType(this, currentJob.mOutput, resource, constantExpressionSet, materialShaderMapHash, shaderPipelineType, nullptr, inDebugDescription));
			currentJob.mOutput.mParameterMap.verifyBindingAreComplete(getName(), currentJob.mOutput.mTarget, currentJob.mVFType);
		}
		return shader;
	}

	ShaderCompileJob* MaterialShaderType::beginCompileShader(uint32 shaderMapId, const FMaterial* material, ShaderCompilerEnvironment* materialEnvironment, const ShaderPipelineType* shaderPipeline, EShaderPlatform platform, TArray<ShaderCommonCompileJob*>& newJobs)
	{
		ShaderCompileJob* newJob = new ShaderCompileJob(shaderMapId, nullptr, this);
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