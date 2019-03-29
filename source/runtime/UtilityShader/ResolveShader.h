#pragma once
#include "CoreMinimal.h"
#include "ShaderParameters.h"
#include "Shader.h"
#include "GlobalShader.h"
#include "UtilityShaderConfig.h"
namespace Air
{
	struct DummyResolveParameter {};

	class ResolveDepthPS : public GlobalShader
	{
		DECLARE_EXPORTED_SHADER_TYPE(ResolveDepthPS, Global, UTILITY_SHADER_API);
	public:
		typedef DummyResolveParameter Parameter;
		static bool shouldCache(EShaderPlatform platform) { return platform == SP_PCD3D_SM5; }

		ResolveDepthPS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mUnresolvedSurface.bind(initializer.mParameterMap, TEXT("UnresolvedSurface"), SPF_Mandatory);
		}
		ResolveDepthPS() {}

		void setParameters(RHICommandList& RHICmdList, Parameter)
		{

		}

		virtual bool serialize(Archive& ar) override
		{
			bool bshaderHasOutdatedParameters = GlobalShader::serialize(ar);
			ar << mUnresolvedSurface;
			return bshaderHasOutdatedParameters;
		}

		ShaderResourceParameter mUnresolvedSurface;

	};

	class ResolveDepthNonMSPS : public GlobalShader
	{
		DECLARE_EXPORTED_SHADER_TYPE(ResolveDepthNonMSPS, Global, UTILITY_SHADER_API);
	public:
		typedef DummyResolveParameter Parameter;


		static bool shouldCache(EShaderPlatform platform) { return getMaxSupportedFeatureLevel(platform) <= ERHIFeatureLevel::SM4; }

		ResolveDepthNonMSPS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mUnresolvedSurface.bind(initializer.mParameterMap, TEXT("UnresolvedSurfaceNonMS"), SPF_Mandatory);
		}

		ResolveDepthNonMSPS() {}
		void setParameters(RHICommandList& RHICmdList, Parameter)
		{}

		virtual bool serialize(Archive& ar) override
		{
			bool bShaderHasOutDatedParameters = GlobalShader::serialize(ar);
			ar << mUnresolvedSurface;
			return bShaderHasOutDatedParameters;
		}
		ShaderResourceParameter mUnresolvedSurface;
	};

	class ResolveSingleSamplePS : public GlobalShader
	{
		DECLARE_EXPORTED_SHADER_TYPE(ResolveSingleSamplePS, Global, UTILITY_SHADER_API);
	public:
		typedef uint32 Parameter;
		static bool shouldCache(EShaderPlatform platform) { return platform == SP_PCD3D_SM5; }

		ResolveSingleSamplePS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mUnresolvedSurface.bind(initializer.mParameterMap, TEXT("UnresolvedSurface"), SPF_Mandatory);
			mSingleSampleIndex.bind(initializer.mParameterMap, TEXT("SingleSampleIndex"), SPF_Mandatory);
		}

		ResolveSingleSamplePS() {}
		UTILITY_SHADER_API void setParameters(RHICommandList& RHICmdList, uint32 singleSampleIndexValue);

		virtual bool serialize(Archive& ar) override
		{
			bool b = GlobalShader::serialize(ar);
			ar << mUnresolvedSurface;
			ar << mSingleSampleIndex;
			return b;
		}

		ShaderResourceParameter mUnresolvedSurface;
		ShaderParameter mSingleSampleIndex;
	};

	class ResolveVS : public GlobalShader
	{
		DECLARE_EXPORTED_SHADER_TYPE(ResolveVS, Global, UTILITY_SHADER_API);
	public:
		static bool shouldCache(EShaderPlatform platform) { return true; }

		ResolveVS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{

		}

		ResolveVS() {}
	};
}