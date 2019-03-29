#pragma once
#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "ShaderParameterUtils.h"
namespace Air
{
	class HdrCustomResolveVS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(HdrCustomResolveVS, Global)
	public:
		HdrCustomResolveVS() {}
		HdrCustomResolveVS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{}

		virtual bool serialize(Archive& ar) override
		{
			bool bShaderHasOutdatedParameters = GlobalShader::serialize(ar);
			return bShaderHasOutdatedParameters;
		}
		static bool shouldCache(EShaderPlatform platform)
		{
			return isFeatureLevelSupported(platform, ERHIFeatureLevel::ES2);
		}
	};

	class HdrCustomResolve2xPS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(HdrCustomResolve2xPS, Global)
	public:
		HdrCustomResolve2xPS() {}
		HdrCustomResolve2xPS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mTex.bind(initializer.mParameterMap, TEXT("Tex"), SPF_Mandatory);
		}

		virtual bool serialize(Archive& ar) override
		{
			bool bShaderHasOutdatedParameters = GlobalShader::serialize(ar);
			ar << mTex;
			return bShaderHasOutdatedParameters;
		}

		static bool shouldCache(EShaderPlatform platform)
		{
			return isFeatureLevelSupported(platform, ERHIFeatureLevel::ES2);
		}

		void setParameters(RHICommandList& RHICmdList, TextureRHIParamRef texture2DMS)
		{
			PixelShaderRHIParamRef pixelShaderRHI = getPixelShader();
			setTextureParameter(RHICmdList, pixelShaderRHI, mTex, texture2DMS);
		}

		static void modifyCompilationEnvironment(EShaderPlatform platform, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(platform, outEnvironment);
			outEnvironment.setDefine(TEXT("HDR_CUSTOM_RESOLVE_2X"), 1);
		}

	protected:
		ShaderResourceParameter mTex;


	};


	class HdrCustomResolve4xPS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(HdrCustomResolve4xPS, Global)
	public:
		HdrCustomResolve4xPS() {}
		HdrCustomResolve4xPS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mTex.bind(initializer.mParameterMap, TEXT("Tex"), SPF_Mandatory);
		}

		virtual bool serialize(Archive& ar) override
		{
			bool bShaderHasOutdatedParameters = GlobalShader::serialize(ar);
			ar << mTex;
			return bShaderHasOutdatedParameters;
		}

		static bool shouldCache(EShaderPlatform platform)
		{
			return isFeatureLevelSupported(platform, ERHIFeatureLevel::ES2);
		}

		void setParameters(RHICommandList& RHICmdList, TextureRHIParamRef texture2DMS)
		{
			PixelShaderRHIParamRef pixelShaderRHI = getPixelShader();
			setTextureParameter(RHICmdList, pixelShaderRHI, mTex, texture2DMS);
		}

		static void modifyCompilationEnvironment(EShaderPlatform platform, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(platform, outEnvironment);
			outEnvironment.setDefine(TEXT("HDR_CUSTOM_RESOLVE_4X"), 1);
		}

	protected:
		ShaderResourceParameter mTex;


	};


	class HdrCustomResolve8xPS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(HdrCustomResolve8xPS, Global)
	public:
		HdrCustomResolve8xPS() {}
		HdrCustomResolve8xPS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mTex.bind(initializer.mParameterMap, TEXT("Tex"), SPF_Mandatory);
		}

		virtual bool serialize(Archive& ar) override
		{
			bool bShaderHasOutdatedParameters = GlobalShader::serialize(ar);
			ar << mTex;
			return bShaderHasOutdatedParameters;
		}

		static bool shouldCache(EShaderPlatform platform)
		{
			return isFeatureLevelSupported(platform, ERHIFeatureLevel::ES2);
		}

		void setParameters(RHICommandList& RHICmdList, TextureRHIParamRef texture2DMS)
		{
			PixelShaderRHIParamRef pixelShaderRHI = getPixelShader();
			setTextureParameter(RHICmdList, pixelShaderRHI, mTex, texture2DMS);
		}

		static void modifyCompilationEnvironment(EShaderPlatform platform, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(platform, outEnvironment);
			outEnvironment.setDefine(TEXT("HDR_CUSTOM_RESOLVE_8X"), 1);
		}

	protected:
		ShaderResourceParameter mTex;


	};
}