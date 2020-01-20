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
		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			return isFeatureLevelSupported(parameters.mPlatform, ERHIFeatureLevel::ES2);
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

		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			return isFeatureLevelSupported(parameters.mPlatform, ERHIFeatureLevel::ES2);
		}

		void setParameters(RHICommandList& RHICmdList, RHITexture* texture2DMS)
		{
			RHIPixelShader* pixelShaderRHI = getPixelShader();
			setTextureParameter(RHICmdList, pixelShaderRHI, mTex, texture2DMS);
		}

		static void modifyCompilationEnvironment(const GlobalShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(parameters, outEnvironment);
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

		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			return isFeatureLevelSupported(parameters.mPlatform, ERHIFeatureLevel::ES2);
		}

		void setParameters(RHICommandList& RHICmdList, RHITexture* texture2DMS)
		{
			RHIPixelShader* pixelShaderRHI = getPixelShader();
			setTextureParameter(RHICmdList, pixelShaderRHI, mTex, texture2DMS);
		}

		static void modifyCompilationEnvironment(const GlobalShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(parameters, outEnvironment);
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

		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			return isFeatureLevelSupported(parameters.mPlatform, ERHIFeatureLevel::ES2);
		}

		void setParameters(RHICommandList& RHICmdList, RHITexture* texture2DMS)
		{
			RHIPixelShader* pixelShaderRHI = getPixelShader();
			setTextureParameter(RHICmdList, pixelShaderRHI, mTex, texture2DMS);
		}

		static void modifyCompilationEnvironment(const GlobalShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(parameters, outEnvironment);
			outEnvironment.setDefine(TEXT("HDR_CUSTOM_RESOLVE_8X"), 1);
		}

	protected:
		ShaderResourceParameter mTex;


	};


	class HdrCustomResolveMask2xPS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(HdrCustomResolveMask2xPS, Global);
	public:
		HdrCustomResolveMask2xPS() {}

		HdrCustomResolveMask2xPS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mTex.bind(initializer.mParameterMap, TEXT("Tex"), SPF_Mandatory);
			mMaskTex.bind(initializer.mParameterMap, TEXT("MaskTex"), SPF_Optional);
		}

		virtual bool serialize(Archive& ar) override
		{
			bool bShaderHasOutdataParameters = GlobalShader::serialize(ar);
			ar << mTex;
			ar << mMaskTex;
			return bShaderHasOutdataParameters;
		}

		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			return isFeatureLevelSupported(parameters.mPlatform, ERHIFeatureLevel::ES2);
		}

		void setParameters(RHICommandList& RHICmdList, RHITexture* texture2DMS, RHITexture* maskTexture2D)
		{
			RHIPixelShader* pixelShaderRHi = getPixelShader();
			setTextureParameter(RHICmdList, pixelShaderRHi, mTex, texture2DMS);
			setTextureParameter(RHICmdList, pixelShaderRHi, mMaskTex, maskTexture2D);
		}

		static void modifyCompilationEnvironment(const GlobalShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(parameters, outEnvironment);
			outEnvironment.setDefine(TEXT("HDR_RESOLVE_NUM_SAMPLES"), 2);
			outEnvironment.setDefine(TEXT("HDR_CUSTOM_RESOLVE_USES_MASK"), 1);
		}

	protected:
		ShaderResourceParameter mTex;
		ShaderResourceParameter mMaskTex;
	};

	class HdrCustomResolveMask4xPS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(HdrCustomResolveMask4xPS, Global);
	public:
		HdrCustomResolveMask4xPS() {}

		HdrCustomResolveMask4xPS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mTex.bind(initializer.mParameterMap, TEXT("Tex"), SPF_Mandatory);
			mMaskTex.bind(initializer.mParameterMap, TEXT("MaskTex"), SPF_Optional);
		}

		virtual bool serialize(Archive& ar) override
		{
			bool bShaderHasOutdataParameters = GlobalShader::serialize(ar);
			ar << mTex;
			ar << mMaskTex;
			return bShaderHasOutdataParameters;
		}

		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			return isFeatureLevelSupported(parameters.mPlatform, ERHIFeatureLevel::ES2);
		}

		void setParameters(RHICommandList& RHICmdList, RHITexture* texture2DMS, RHITexture* maskTexture2D)
		{
			RHIPixelShader* pixelShaderRHi = getPixelShader();
			setTextureParameter(RHICmdList, pixelShaderRHi, mTex, texture2DMS);
			setTextureParameter(RHICmdList, pixelShaderRHi, mMaskTex, maskTexture2D);
		}

		static void modifyCompilationEnvironment(const GlobalShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(parameters, outEnvironment);
			outEnvironment.setDefine(TEXT("HDR_RESOLVE_NUM_SAMPLES"), 4);
			outEnvironment.setDefine(TEXT("HDR_CUSTOM_RESOLVE_USES_MASK"), 1);
		}

	protected:
		ShaderResourceParameter mTex;
		ShaderResourceParameter mMaskTex;
	};


	class HdrCustomResolveMask8xPS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(HdrCustomResolveMask8xPS, Global);
	public:
		HdrCustomResolveMask8xPS() {}

		HdrCustomResolveMask8xPS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mTex.bind(initializer.mParameterMap, TEXT("Tex"), SPF_Mandatory);
			mMaskTex.bind(initializer.mParameterMap, TEXT("MaskTex"), SPF_Optional);
		}

		virtual bool serialize(Archive& ar) override
		{
			bool bShaderHasOutdataParameters = GlobalShader::serialize(ar);
			ar << mTex;
			ar << mMaskTex;
			return bShaderHasOutdataParameters;
		}

		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			return isFeatureLevelSupported(parameters.mPlatform, ERHIFeatureLevel::ES2);
		}

		void setParameters(RHICommandList& RHICmdList, RHITexture* texture2DMS, RHITexture* maskTexture2D)
		{
			RHIPixelShader* pixelShaderRHi = getPixelShader();
			setTextureParameter(RHICmdList, pixelShaderRHi, mTex, texture2DMS);
			setTextureParameter(RHICmdList, pixelShaderRHi, mMaskTex, maskTexture2D);
		}

		static void modifyCompilationEnvironment(const GlobalShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(parameters, outEnvironment);
			outEnvironment.setDefine(TEXT("HDR_RESOLVE_NUM_SAMPLES"), 8);
			outEnvironment.setDefine(TEXT("HDR_CUSTOM_RESOLVE_USES_MASK"), 1);
		}

	protected:
		ShaderResourceParameter mTex;
		ShaderResourceParameter mMaskTex;
	};
}