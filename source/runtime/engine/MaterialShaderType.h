#pragma once
#include "Shader.h"
#include "GlobalShader.h"
#include "MaterialShared.h"


namespace Air
{
#define IMPLEMENT_MATERIAL_SHADER_TYPE(TemplatePrefix, ShaderClass, SourceFilename, FunctionName, Frequency) \
	IMPLEMENT_SHADER_TYPE(\
		TemplatePrefix,	\
		ShaderClass,	\
		SourceFilename,	\
		FunctionName,	\
		Frequency\
	);







	class MaterialShaderType : public ShaderType
	{

		

	public:
		struct CompiledShaderInitializerType : GlobalShaderType::CompiledShaderInitializerType
		{
			const ConstantExpressionSet& mConstantExpressionSet;
			const wstring mDebugDescription;
			CompiledShaderInitializerType(
				ShaderType* inType,
				const ShaderCompilerOutput& compilerOutput,
				ShaderResource*	inResource,
				const ConstantExpressionSet& inConstantExpressionSet,
				const SHAHash& inMaterialShaderMapHash,
				const ShaderPipelineType* inShaderPipeline,
				VertexFactoryType* inVertexFactoryType,
				const wstring & inDebugDescription
			)
				:GlobalShaderType::CompiledShaderInitializerType(inType, compilerOutput, inResource, inMaterialShaderMapHash, inShaderPipeline, inVertexFactoryType)
				,mConstantExpressionSet(inConstantExpressionSet)
				,mDebugDescription(inDebugDescription)
			{}

		};

		typedef Shader* (*ConstructCompiledType)(const CompiledShaderInitializerType&);

		typedef bool(*ShouldCacheType)(EShaderPlatform, const FMaterial*);
		typedef bool(*ModifyCompilationEnvironmentType)(EShaderPlatform, const FMaterial*, ShaderCompilerEnvironment&);

		bool shouldCache(EShaderPlatform platform, const FMaterial* material) const {
			return (*mShouldCacheType)(platform, material);
		}

		class ShaderCompileJob* beginCompileShader(uint32 shaderMapId, const FMaterial* material, ShaderCompilerEnvironment* materialEnvironment, const ShaderPipelineType* shaderPipeline, EShaderPlatform platform, TArray<ShaderCommonCompileJob*>& newJobs);

		static void beginCompileShaderPipeline(uint32 shaderMapId, EShaderPlatform platform, const FMaterial* material, ShaderCompilerEnvironment* materialEnvironment,
			const ShaderPipelineType* shaderPipeline, const TArray<MaterialShaderType*>& shaderStages, TArray<ShaderCommonCompileJob*>& newJobs);

		Shader* finishCompileShader(
			const ConstantExpressionSet& constantExpressionSet,
			const SHAHash& materialShaderMapHash,
			const ShaderCompileJob& currentJob,
			const ShaderPipelineType* shaderPipeline,
			const wstring& inDebugDescription
		);

	protected:
		void setupCompileEnvironment(EShaderPlatform platform, const FMaterial* material, ShaderCompilerEnvironment& environment)
		{
			(*mModifyCompilationEnvironmentType)(platform, material, environment);
		}

	private:
		ConstructCompiledType mConstructCompiledRef;

		ShouldCacheType	mShouldCacheType;

		ModifyCompilationEnvironmentType mModifyCompilationEnvironmentType;
	};
}