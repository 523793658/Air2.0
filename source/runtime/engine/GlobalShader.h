#pragma once
#include "CoreMinimal.h"
#include "RHI.h"
#include "EngineMininal.h"
#include "ShaderCompiler.h"
#include "Shader.h"
#include "SceneView.h"
namespace Air
{
	class ShaderCompileJob;
	class ShaderCommonCompileJob;

	extern ENGINE_API const int32 mGlobalShaderMapId;

	class GlobalShaderType : public ShaderType
	{
	public:
		typedef Shader::CompiledShaderInitializerType CompiledShaderInitializerType;
		typedef Shader* (*ConstructCompiledType)(const CompiledShaderInitializerType&);
		typedef bool(*ShouldCacheType)(EShaderPlatform);
		typedef void(*ModifyCompilationEvironmentType)(EShaderPlatform, ShaderCompilerEnvironment&);
		GlobalShaderType(
			const TCHAR* inName,
			const TCHAR* inSourceFilename,
			const TCHAR* inFunctionName,
			uint32 inFrequency,
			ConstructSerializedType inConstructSerializedRef,
			ConstructCompiledType inConstructCompiledRef,
			ModifyCompilationEvironmentType inModifyCompilationEnvironmentRef,
			ShouldCacheType inShouldCacheRef,
			GetStreamOutElementsType inGetStreamOutElementsRef
		) :
			ShaderType(EShaderTypeForDynamicCast::Global, inName, inSourceFilename, inFunctionName, inFrequency, inConstructSerializedRef, inGetStreamOutElementsRef),
			mConstructCompiledRef(inConstructCompiledRef),
			mShouldCacheRef(inShouldCacheRef),
			mModifyCompilationEnvironmentRef(inModifyCompilationEnvironmentRef)
		{}

		bool shouldCache(EShaderPlatform platform) const
		{
			return (*mShouldCacheRef)(platform);
		}

		Shader* finishCompileShader(const ShaderCompileJob& compileJob, const ShaderPipelineType* shaderPipelineType);

		ENGINE_API ShaderCompileJob* beginCompileShader(EShaderPlatform platform, const ShaderPipelineType* shaderPipeline, TArray<ShaderCommonCompileJob*>& newJobs);

		ENGINE_API static void beginCompileShaderPipeline(EShaderPlatform platform, const ShaderPipelineType* shaderPipeline, const TArray<GlobalShaderType*>& shaderStages, TArray<ShaderCommonCompileJob*>& newJobs);

		void setCompileEnvironment(EShaderPlatform platform, ShaderCompilerEnvironment& environment)
		{
			(*mModifyCompilationEnvironmentRef)(platform, environment);
		}
	
	private:
		ConstructCompiledType mConstructCompiledRef;
		ShouldCacheType mShouldCacheRef;
		ModifyCompilationEvironmentType mModifyCompilationEnvironmentRef;
	};

	class GlobalShader : public Shader
	{
		DECLARE_SHADER_TYPE(GlobalShader, Global);
	public:
		GlobalShader(): Shader(){}
		ENGINE_API GlobalShader(const ShaderMetaType::CompiledShaderInitializerType& initializer);

		template<typename ShaderRHIParamRef, typename TRHICmdList>
		void setParameters(TRHICmdList& RHICmdList, const ShaderRHIParamRef shaderRHI, const SceneView& view)
		{
			const auto& viewConstantBufferParameter = getConstantBufferParameter<ViewConstantShaderParameters>();
			const auto& builtinSamplersCBParameter = getConstantBufferParameter<BuiltinSamplersParameters>();
			checkShaderIsValid();
			setConstantBufferParameter(RHICmdList, shaderRHI, viewConstantBufferParameter, view.mViewConstantBuffer);
		}

		typedef void(*ModifyCompilationEnvironmentType)(EShaderPlatform, ShaderCompilerEnvironment);

		ENGINE_API static void modifyCompilationEnvironment(EShaderPlatform platform, ShaderCompilerEnvironment& outEnvironment) {}
	};


	class GlobalShaderMapId
	{
	public:
		GlobalShaderMapId(EShaderPlatform platform);

		void appendKeyString(wstring keyString) const;

	private:
		TArray<ShaderTypeDependency> mShaderTypeDependencies;
		TArray<ShaderPipelineTypeDependency> mShaderPipelineTypeDependencies;
	};

	

	extern ENGINE_API TShaderMap<GlobalShaderType>* getGlobalShaderMap(EShaderPlatform platform, bool bRefreshShaderMap = false);

	extern ENGINE_API void verifyGlobalShaders(EShaderPlatform platform, bool bLoadedFromCacheFile);

	extern void processCompiledGlbalShaders(const TArray<ShaderCommonCompileJob*>& compilationResults);

	inline TShaderMap<GlobalShaderType>* getGlobalShaderMap(ERHIFeatureLevel::Type featureLevel, bool bRefreshShaderMap = false)
	{
		return getGlobalShaderMap(GShaderPlatformForFeatureLevel[featureLevel], bRefreshShaderMap);
	}
}