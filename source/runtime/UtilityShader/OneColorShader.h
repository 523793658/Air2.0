#pragma once
#include "UtilityShaderConfig.h"
#include "GlobalShader.h"
#include "Shader.h"
namespace Air
{
	template<bool bUsingNDCPositions = true, bool bUsingVertexLayers = false>
	class TOneColorVS : public GlobalShader
	{
		DECLARE_EXPORTED_SHADER_TYPE(TOneColorVS, Global, UTILITY_SHADER_API);
		TOneColorVS(){}

	public:
		TOneColorVS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{}

		static void modifyCompilationEnvironment(const GlobalShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(parameters, outEnvironment);
		}

		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			return true;
		}

		static const TCHAR*	getSourceFilename()
		{
			return TEXT("OneColorShader");
		}

		static const TCHAR* getFunctionName()
		{
			return TEXT("MainVertexShader");
		}
	};
	enum class ECapturePreAnimatedState : uint8
	{
		None,
		Global,
		Entity,
	};

	class OneColorPS : public GlobalShader
	{
		DECLARE_EXPORTED_SHADER_TYPE(OneColorPS, Global, UTILITY_SHADER_API);
	public:
		OneColorPS() {}
		OneColorPS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{

		}

		virtual void setColors(RHICommandList& RHICmdList, const LinearColor* colors, int32 numColors);

		virtual bool serialize(Archive& ar) override
		{
			return GlobalShader::serialize(ar);
		}

		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			return true;
		}
	};

	template<int32 NumOutputs>
	class TOneColorPixelShaderMRT : public OneColorPS
	{
		DECLARE_EXPORTED_SHADER_TYPE(TOneColorPixelShaderMRT, Global, UTILITY_SHADER_API);

	public:
		TOneColorPixelShaderMRT() {}
		TOneColorPixelShaderMRT(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:OneColorPS(initializer)
		{}

		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			if (NumOutputs > 1)
			{
				return isFeatureLevelSupported(parameters.mPlatform, ERHIFeatureLevel::ES3_1);
			}
			return true;
		}

		static void modifyCompilationEnvironment(const GlobalShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			OneColorPS::modifyCompilationEnvironment(parameters, outEnvironment);
			outEnvironment.setDefine(TEXT("NUM_OUTPUTS"), NumOutputs);
		}
	};
}