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




	struct MaterialShaderPermutationParameters
	{
		const EShaderPlatform mPlatform;
		const FMaterial* mMaterial;

		const int32 mPermutationId;

		MaterialShaderPermutationParameters(EShaderPlatform inPlatform, const FMaterial* inMaterial, int32 inPermutationId)
			:mPlatform(inPlatform)
			,mMaterial(inMaterial)
			,mPermutationId(inPermutationId)
		{}
	};


	class MaterialShaderType : public ShaderType
	{

		

	public:
		struct CompiledShaderInitializerType : GlobalShaderType::CompiledShaderInitializerType
		{
			const ConstantExpressionSet& mConstantExpressionSet;
			const wstring mDebugDescription;
			CompiledShaderInitializerType(
				ShaderType* inType,
				int32 inPermutationId,
				const ShaderCompilerOutput& compilerOutput,
				ShaderResource*	inResource,
				const ConstantExpressionSet& inConstantExpressionSet,
				const SHAHash& inMaterialShaderMapHash,
				const ShaderPipelineType* inShaderPipeline,
				VertexFactoryType* inVertexFactoryType,
				const wstring & inDebugDescription
			)
				:GlobalShaderType::CompiledShaderInitializerType(inType, inPermutationId, compilerOutput, inResource, inMaterialShaderMapHash, inShaderPipeline, inVertexFactoryType)
				,mConstantExpressionSet(inConstantExpressionSet)
				,mDebugDescription(inDebugDescription)
			{}

		};

		typedef Shader* (*ConstructCompiledType)(const CompiledShaderInitializerType&);

		typedef bool(*ShouldCompilePermutationType)(const MaterialShaderPermutationParameters&);
		typedef bool(*ModifyCompilationEnvironmentType)(EShaderPlatform, const FMaterial*, ShaderCompilerEnvironment&);

		bool shouldCompilePermutation(EShaderPlatform platform, const FMaterial* material, int32 permuationId) const {
			return (*mShouldCompilePermutationRef)(MaterialShaderPermutationParameters(platform, material, permuationId));
		}

		class ShaderCompileJob* beginCompileShader(uint32 shaderMapId, int32 permutationId, const FMaterial* material, ShaderCompilerEnvironment* materialEnvironment, const ShaderPipelineType* shaderPipeline, EShaderPlatform platform, TArray<ShaderCommonCompileJob*>& newJobs);

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

		ShouldCompilePermutationType	mShouldCompilePermutationRef;

		ModifyCompilationEnvironmentType mModifyCompilationEnvironmentType;
	};
}