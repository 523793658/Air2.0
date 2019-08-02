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

		static void modifyCompilationEnvironment(EShaderPlatform platform, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(platform, outEnvironment);
		}

		static bool shouldCache(EShaderPlatform platform)
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

		static bool shouldCache(EShaderPlatform platform)
		{
			return true;
		}
	};
}