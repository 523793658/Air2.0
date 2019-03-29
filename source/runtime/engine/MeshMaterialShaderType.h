#pragma once
#include "MaterialShaderType.h"
namespace Air
{
	class MeshMaterialShaderType : public ShaderType
	{
	public:
		struct CompiledShaderInitializerType : MaterialShaderType::CompiledShaderInitializerType
		{
			VertexFactoryType* mVertexFactoryType;
			CompiledShaderInitializerType(ShaderType* inType,
				const ShaderCompilerOutput& compilerOutput,
				ShaderResource* inResource,
				const ConstantExpressionSet& inConstantExpressionSet,
				const SHAHash& inMaterialShaderMapHash,
				const wstring& inDebugDescription,
				const ShaderPipelineType* inShaderPipeline,
				VertexFactoryType* inVertexFactoryType)
				:MaterialShaderType::CompiledShaderInitializerType(inType, compilerOutput, inResource, inConstantExpressionSet, inMaterialShaderMapHash, inShaderPipeline, inVertexFactoryType, inDebugDescription),
				mVertexFactoryType(inVertexFactoryType)
			{}
		};

		typedef Shader* (*ConstructCompiledType)(const CompiledShaderInitializerType&);
		typedef bool(*ShouldCacheType)(EShaderPlatform, const FMaterial*, const VertexFactoryType* vertexFactoryType);

		typedef void(*ModifyCompilationEnvironmentType)(EShaderPlatform, const FMaterial*, ShaderCompilerEnvironment&);

		MeshMaterialShaderType(
			const TCHAR* inName,
			const TCHAR* InSourceFilename,
			const TCHAR* inFunctionName,
			uint32 inFrequency,
			ConstructSerializedType inConstructSerializedRef,
			ConstructCompiledType inConstructCompiledRef,
			ModifyCompilationEnvironmentType inModifyCompilationEnvironmentRef,
			ShouldCacheType inShouldCacheRef,
			GetStreamOutElementsType inGetStreamOutElementRef
		)
			: ShaderType(EShaderTypeForDynamicCast::MeshMaterial, inName, InSourceFilename, inFunctionName, inFrequency, inConstructSerializedRef, inGetStreamOutElementRef),
			mConstructCompiledRef(inConstructCompiledRef), 
			mShouldCacheRef(inShouldCacheRef),
			mModifyCompilationEnvironmentRef(inModifyCompilationEnvironmentRef)
			{}
		
		bool shouldCache(EShaderPlatform platform, const FMaterial* material, const VertexFactoryType* vertexFactoryType) const
		{
			return (*mShouldCacheRef)(platform, material, vertexFactoryType);
		}

		void setupCompileEnvironment(EShaderPlatform platform, const FMaterial* material, ShaderCompilerEnvironment& environment)
		{
			(*mModifyCompilationEnvironmentRef)(platform, material, environment);
		}

		Shader* finishCompileShader(const ConstantExpressionSet& constantExpressionSet,
			const SHAHash& materialShaderMapHash,
			const ShaderCompileJob& currentJob,
			const ShaderPipelineType* shaderPipelineType,
			const wstring& inDebugDescription);

		class ShaderCompileJob* beginCompileShader(uint32 shaderMapId,
			EShaderPlatform platform,
			const FMaterial* material,
			ShaderCompilerEnvironment* materialEnvironment,
			VertexFactoryType* vertexFactoryType,
			const ShaderPipelineType* shaderPipeline,
			TArray<ShaderCommonCompileJob*>& newJobs);

	private:
		ConstructCompiledType mConstructCompiledRef;
		ShouldCacheType mShouldCacheRef;
		ModifyCompilationEnvironmentType mModifyCompilationEnvironmentRef;


	};
}