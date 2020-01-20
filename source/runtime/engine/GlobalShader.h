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

#define DECLARE_GLOBAL_SHADER(ShaderClass) \
	public:\
	\
	using ShaderMetaType = GlobalShaderType;\
	\
	static ShaderMetaType mStaticType;\
	\
	static Shader* constructSerializedInstance() {return new ShaderClass();}\
	\
	static Shader* constructCompiledInstance(const ShaderMetaType::CompiledShaderInitializerType& initializer)\
	{\
		return new ShaderClass(initializer);\
	}\
	virtual uint32 getTypeSize() const override{return sizeof(*this);}\
	static void modifyCompilationEnvironmentImpl(\
		const GlobalShaderPermutationParameters& parameters,\
		ShaderCompilerEnvironment& outEnvironment)\
	{\
		PermutationDomain permutationVector(parameters.mPermutationId);\
		permutationVector.modifyCompilationEnvironment(outEnvironment);\
		ShaderClass::modifyCompilationEnvironment(parameters, outEnvironment);\
	}

#define IMPLEMENT_GLOBAL_SHADER(ShaderClass, SourceFilename, FunctionName, Frequency)\
	ShaderClass::ShaderMetaType ShaderClass::mStaticType(\
		TEXT(#ShaderClass),\
		TEXT(SourceFilename),\
		TEXT(FunctionName),\
		Frequency,\
		ShaderClass::PermutationDomain::mPermutationCount,\
		ShaderClass::constructSerializedInstance,\
		ShaderClass::constructCompiledInstance,\
		ShaderClass::modifyCompilationEnvironmentImpl,\
		ShaderClass::shouldCompilePermutation,\
		ShaderClass::validateCompiledResult,\
		ShaderClass::getStreamOutElements,\
		ShaderClass::getRootParametersMetadata()\
	)


	struct GlobalShaderPermutationParameters
	{
		const EShaderPlatform mPlatform;
		const int32 mPermutationId;
		GlobalShaderPermutationParameters(EShaderPlatform inPlatform, int32 inPermutationId)
			:mPlatform(inPlatform)
			,mPermutationId(inPermutationId)
		{}
	};

	class GlobalShaderType : public ShaderType
	{
		friend class GlobalShaderTypeCompiler;

	public:
		typedef Shader::CompiledShaderInitializerType CompiledShaderInitializerType;
		typedef Shader* (*ConstructCompiledType)(const CompiledShaderInitializerType&);
		typedef bool (*ShouldCompilePermutationType)(const GlobalShaderPermutationParameters&);
		typedef bool (*ValidateCompileResultType)(EShaderPlatform, const ShaderParameterMap&, TArray<wstring>&);
		typedef void(*ModifyCompilationEvironmentType)(const GlobalShaderPermutationParameters&, ShaderCompilerEnvironment&);
		GlobalShaderType(
			const TCHAR* inName,
			const TCHAR* inSourceFilename,
			const TCHAR* inFunctionName,
			uint32 inFrequency,
			int32 inTotalPermutationCount,
			ConstructSerializedType inConstructSerializedRef,
			ConstructCompiledType inConstructCompiledRef,
			ModifyCompilationEvironmentType inModifyCompilationEnvironmentRef,
			ShouldCompilePermutationType inShouldCoompilePermutationRef,
			ValidateCompileResultType inValidateCompileResultRef,
			GetStreamOutElementsType inGetStreamOutElementsRef,
			const ShaderParametersMetadata* inRootParametersMetadata = nullptr
		) :
			ShaderType(EShaderTypeForDynamicCast::Global, inName, inSourceFilename, inFunctionName, inFrequency, inTotalPermutationCount, inConstructSerializedRef, inGetStreamOutElementsRef),
			mConstructCompiledRef(inConstructCompiledRef),
			mShouldCompilePermutationRef(inShouldCoompilePermutationRef),
			mValidateCompiledResultRef(inValidateCompileResultRef),
			mModifyCompilationEnvironmentRef(inModifyCompilationEnvironmentRef)
		{}

		bool shouldCompilePermutation(EShaderPlatform platform, int32 permutationId) const
		{
			return (*mShouldCompilePermutationRef)(GlobalShaderPermutationParameters(platform, permutationId));
		}



		void setupCompileEnvironment(EShaderPlatform platform, int32 permutationId, ShaderCompilerEnvironment& environment)
		{
			(*mModifyCompilationEnvironmentRef)(GlobalShaderPermutationParameters(platform, permutationId), environment);
		}

		
		bool validateCompiledResult(EShaderPlatform platform, const ShaderParameterMap& parameterMap, TArray<wstring>& outError) const
		{
			return (*mValidateCompiledResultRef)(platform, parameterMap, outError);
		}
	private:
		ConstructCompiledType mConstructCompiledRef;
		ShouldCompilePermutationType mShouldCompilePermutationRef;
		ValidateCompileResultType mValidateCompiledResultRef;
		ModifyCompilationEvironmentType mModifyCompilationEnvironmentRef;
	};

	class GlobalShader : public Shader
	{
		DECLARE_SHADER_TYPE(GlobalShader, Global);
	public:
		GlobalShader(): Shader(){}
		ENGINE_API GlobalShader(const ShaderMetaType::CompiledShaderInitializerType& initializer);

		template<typename TViewConstantShaderParameters, typename ShaderRHIParamRef, typename TRHICmdList>
		void setParameters(TRHICmdList& RHICmdList, const ShaderRHIParamRef shaderRHI, RHIConstantBuffer* viewConstantBuffer)
		{
			const auto& viewConstantBufferParameter = getConstantBufferParameter<TViewConstantShaderParameters>();
			
			setConstantBufferParameter(RHICmdList, shaderRHI, viewConstantBufferParameter, viewConstantBuffer);
		}

		typedef void(*ModifyCompilationEnvironmentType)(EShaderPlatform, ShaderCompilerEnvironment);

		ENGINE_API static void modifyCompilationEnvironment(const GlobalShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment) {}

		ENGINE_API static bool validateCompiledResult(EShaderPlatform platform, const ShaderParameterMap& parameterMap, TArray<wstring>& outErrors) { return true; }
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

	

	extern ENGINE_API TShaderMap<GlobalShaderType>* getGlobalShaderMap(EShaderPlatform platform);

	extern ENGINE_API void verifyGlobalShaders(EShaderPlatform platform, bool bLoadedFromCacheFile);

	extern void processCompiledGlbalShaders(const TArray<ShaderCommonCompileJob*>& compilationResults);

	inline TShaderMap<GlobalShaderType>* getGlobalShaderMap(ERHIFeatureLevel::Type featureLevel)
	{
		return getGlobalShaderMap(GShaderPlatformForFeatureLevel[featureLevel]);
	}

	extern RENDER_CORE_API TShaderMap<GlobalShaderType>* GGlobalShaderMap[SP_NumPlatforms];
}