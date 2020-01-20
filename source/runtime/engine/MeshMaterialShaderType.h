#pragma once
#include "MaterialShaderType.h"
namespace Air
{
	struct MeshMaterialShaderPermutationParameters
	{
		const EShaderPlatform mPlatform;
		const FMaterial* mMaterial;
		const VertexFactoryType* mVertexFactoryType;

		const int32 mPermutationId;

		MeshMaterialShaderPermutationParameters(EShaderPlatform inPlatform, const FMaterial* inMaterial, const VertexFactoryType* inVertexFactoryType, const int32 inPermutationId)
			:mPlatform(inPlatform)
			, mMaterial(inMaterial)
			, mVertexFactoryType(inVertexFactoryType)
			, mPermutationId(inPermutationId)
		{}
	};

	
	class MeshMaterialShaderType : public ShaderType
	{
	public:
		struct CompiledShaderInitializerType : MaterialShaderType::CompiledShaderInitializerType
		{
			VertexFactoryType* mVertexFactoryType;
			CompiledShaderInitializerType(ShaderType* inType,
				int32 inPermutationId,
				const ShaderCompilerOutput& compilerOutput,
				ShaderResource* inResource,
				const ConstantExpressionSet& inConstantExpressionSet,
				const SHAHash& inMaterialShaderMapHash,
				const wstring& inDebugDescription,
				const ShaderPipelineType* inShaderPipeline,
				VertexFactoryType* inVertexFactoryType)
				:MaterialShaderType::CompiledShaderInitializerType(inType, inPermutationId, compilerOutput, inResource, inConstantExpressionSet, inMaterialShaderMapHash, inShaderPipeline, inVertexFactoryType, inDebugDescription),
				mVertexFactoryType(inVertexFactoryType)
			{}
		};

		typedef Shader* (*ConstructCompiledType)(const CompiledShaderInitializerType&);
		typedef bool(*ShouldCompilePermutationType)(const MeshMaterialShaderPermutationParameters&);

		typedef bool(*ValidateCompiledResultType)(EShaderPlatform, const TArray<FMaterial*>&, const VertexFactoryType*, const ShaderParameterMap&, TArray<wstring>&);

		typedef void(*ModifyCompilationEnvironmentType)(const MaterialShaderPermutationParameters&, ShaderCompilerEnvironment&);

		MeshMaterialShaderType(
			const TCHAR* inName,
			const TCHAR* InSourceFilename,
			const TCHAR* inFunctionName,
			uint32 inFrequency,
			int32 inTotalPermutationCount,
			ConstructSerializedType inConstructSerializedRef,
			ConstructCompiledType inConstructCompiledRef,
			ModifyCompilationEnvironmentType inModifyCompilationEnvironmentRef,
			ShouldCompilePermutationType inShouldCacheRef,
			ValidateCompiledResultType inValidateCompiledResultRef,
			GetStreamOutElementsType inGetStreamOutElementRef
		)
			: ShaderType(EShaderTypeForDynamicCast::MeshMaterial, inName, InSourceFilename, inFunctionName, inFrequency, inTotalPermutationCount, inConstructSerializedRef, inGetStreamOutElementRef),
			mConstructCompiledRef(inConstructCompiledRef), 
			mShouldCacheRef(inShouldCacheRef),
			mModifyCompilationEnvironmentRef(inModifyCompilationEnvironmentRef),
			mValidateCompiledResultRef(inValidateCompiledResultRef)
			{}
		
		bool shouldCompilePermutation(EShaderPlatform platform, const FMaterial* material, VertexFactoryType* vertexFactoryType, int32 permutationId) const
		{
			return (*mShouldCacheRef)(MeshMaterialShaderPermutationParameters(platform, material, vertexFactoryType, permutationId));
		}

		void setupCompileEnvironment(EShaderPlatform platform, const FMaterial* material, int32 permutationId, ShaderCompilerEnvironment& environment)
		{
			(*mModifyCompilationEnvironmentRef)(MaterialShaderPermutationParameters(platform, material, permutationId), environment);
		}

		Shader* finishCompileShader(const ConstantExpressionSet& constantExpressionSet,
			const SHAHash& materialShaderMapHash,
			const ShaderCompileJob& currentJob,
			const ShaderPipelineType* shaderPipelineType,
			const wstring& inDebugDescription);

		class ShaderCompileJob* beginCompileShader(uint32 shaderMapId,
			uint32 permutationId,
			EShaderPlatform platform,
			const FMaterial* material,
			ShaderCompilerEnvironment* materialEnvironment,
			VertexFactoryType* vertexFactoryType,
			const ShaderPipelineType* shaderPipeline,
			TArray<ShaderCommonCompileJob*>& newJobs);

	private:
		ConstructCompiledType mConstructCompiledRef;
		ShouldCompilePermutationType mShouldCacheRef;
		ModifyCompilationEnvironmentType mModifyCompilationEnvironmentRef;
		ValidateCompiledResultType mValidateCompiledResultRef;

	};
}