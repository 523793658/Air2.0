#pragma once
#include "CoreMinimal.h"
#include "ShaderParameters.h"
#include "SceneView.h"
#include "ShaderParameterMacros.h"
#include "Shader.h"
namespace Air
{
	BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(SceneTextureConstantParameters, ENGINE_API)
		SHADER_PARAMETER_TEXTURE(Texture2D, SceneColorTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, SceneColorTextureSampler)
		SHADER_PARAMETER_TEXTURE(Texture2D, SceneDepthTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, SceneDepthTextureSampler)
		SHADER_PARAMETER_TEXTURE(Texture2D<float>, SceneDepthTextureNonMS)

		//GBuffer
		SHADER_PARAMETER_TEXTURE(Texture2D, GBufferATexture)
		SHADER_PARAMETER_TEXTURE(Texture2D, GBufferBTexture)
		SHADER_PARAMETER_TEXTURE(Texture2D, GBufferCTexture)
		SHADER_PARAMETER_TEXTURE(Texture2D, GBufferDTexture)
		SHADER_PARAMETER_TEXTURE(Texture2D, GBufferETexture)
		SHADER_PARAMETER_TEXTURE(Texture2D, GBufferVelocityTexture)
		SHADER_PARAMETER_TEXTURE(Texture2D<float4>, GBufferATextureNonMS)
		SHADER_PARAMETER_TEXTURE(Texture2D<float4>, GBufferBTextureNonMS)
		SHADER_PARAMETER_TEXTURE(Texture2D<float4>, GBufferCTextureNonMS)
		SHADER_PARAMETER_TEXTURE(Texture2D<float4>, GBufferDTextureNonMS)
		SHADER_PARAMETER_TEXTURE(Texture2D<float4>, GBufferETextureNonMS)
		SHADER_PARAMETER_TEXTURE(Texture2D<float4>, GBufferVelocityTextureNonMS)
		SHADER_PARAMETER_SAMPLER(SamplerState, GBufferATextureSampler)
		SHADER_PARAMETER_SAMPLER(SamplerState, GBufferBTextureSampler)
		SHADER_PARAMETER_SAMPLER(SamplerState, GBufferCTextureSampler)
		SHADER_PARAMETER_SAMPLER(SamplerState, GBufferDTextureSampler)
		SHADER_PARAMETER_SAMPLER(SamplerState, GBufferETextureSampler)
		SHADER_PARAMETER_SAMPLER(SamplerState, GBufferVelocityTextureSampler)

		//Custom Depth / Stencil
	END_GLOBAL_SHADER_PARAMETER_STRUCT()

	BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(MobileSceneTextureConstantParameters, ENGINE_API)
		SHADER_PARAMETER_TEXTURE(Texture2D, SceneColorTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, SceneColorTextureSampler)
		SHADER_PARAMETER_TEXTURE(Texture2D, SceneDepthTexture)
		SHADER_PARAMETER_TEXTURE(SamplerState, SceneDepthTextureSampler)
		SHADER_PARAMETER_TEXTURE(Texture2D, SceneAlphaCopyTexture)
		SHADER_PARAMETER_TEXTURE(SamplerState, SceneAlphaCopyTextureSampler)
		SHADER_PARAMETER_TEXTURE(Texture2D, CustomDepthTexture)
		SHADER_PARAMETER_TEXTURE(SamplerState, CustomDepthTextureSampler)
		SHADER_PARAMETER_TEXTURE(Texture2D, MobileCustomStencilTexture)
		SHADER_PARAMETER_TEXTURE(SamplerState, MobileCustomStencilTextureSampler)
	END_GLOBAL_SHADER_PARAMETER_STRUCT()



	namespace ESceneRenderTargetsMode
	{
		enum Type
		{
			SetTextures,
			DontSet,
			DontSetIgnoreBoundByEditorCompositing,
		};
	}

	enum class ESceneTextureSetupMode : uint32
	{
		None = 0,
		SceneDepth = 1,
		GBuffers = 2,
		SSAO = 4,
		CustomDepth = 8,
		All = SceneDepth | GBuffers | SSAO | CustomDepth
	};

	inline ESceneTextureSetupMode operator | (ESceneTextureSetupMode lhs, ESceneTextureSetupMode rhs)
	{
		return static_cast<ESceneTextureSetupMode>(static_cast<uint32>(lhs) | static_cast<uint32>(rhs));
	}

	inline ESceneTextureSetupMode operator & (ESceneTextureSetupMode lhs, ESceneTextureSetupMode rhs)
	{
		return static_cast<ESceneTextureSetupMode>(static_cast<uint32>(lhs) & static_cast<uint32>(rhs));
	}


	extern ENGINE_API void bindSceneTextureConstantBufferDependentOnShadingPath(
		const Shader::CompiledShaderInitializerType& initializer,
		ShaderConstantBufferParameter& sceneTexturesConstantBuffer,
		ShaderConstantBufferParameter& mobileSceneTexturesConstantBuffer
	);

	template<typename TRHICmdList>
	ENGINE_API TConstantBufferRef<SceneTextureConstantParameters> createSceneTextureConstantBufferSingleDraw(TRHICmdList& RHICmdList, ESceneTextureSetupMode sceneTextureSetupMode, ERHIFeatureLevel::Type featureLevel);

	template<typename TRHICmdList>
	ENGINE_API TConstantBufferRef<MobileSceneTextureConstantParameters> createMobileSceneTextureConstantBufferSingleDraw(TRHICmdList& RHICmdList, ERHIFeatureLevel::Type featureLevel);

	class ENGINE_API SceneTextureShaderParameters
	{
	public:
		void bind(const Shader::CompiledShaderInitializerType& initializer)
		{
			bindSceneTextureConstantBufferDependentOnShadingPath(initializer, mSceneTexturesConstantBuffer, mMobileSceneTexturesConstantBuffer);
		}

		template<typename ShaderRHIParamRef, typename TRHICmdList>
		void set(TRHICmdList& RHICmdList, const ShaderRHIParamRef& shaderRHI, ERHIFeatureLevel::Type featureLevel, ESceneTextureSetupMode setupMode) const
		{
			if (SceneInterface::getShadingPath(featureLevel) == EShadingPath::Deferred && mSceneTexturesConstantBuffer.isBound())
			{
				TConstantBufferRef<SceneTextureConstantParameters> constantBuffer = createSceneTextureConstantBufferSingleDraw(RHICmdList, setupMode, featureLevel);
				setConstantBufferParameter(RHICmdList, shaderRHI, mSceneTexturesConstantBuffer, constantBuffer);
			}

			/*if (SceneInterface::getShadingPath(featureLevel) == EShadingPath::Mobile && mMobileSceneTexturesConstantBuffer.isBound())
			{
				TConstantBufferRef<MobileSceneTextureConstantParameters> constantBuffer = createMobileSceneTextureConstantBufferSingleDraw(RHICmdList, featureLevel);
				setConstantBufferParameter(RHICmdList, shaderRHI, mMobileSceneTexturesConstantBuffer);
			}*/
		}

		friend Archive& operator << (Archive& ar, SceneTextureShaderParameters& p)
		{
			ar << p.mSceneTexturesConstantBuffer;
			ar << p.mMobileSceneTexturesConstantBuffer;
			return ar;
		}

		inline bool isBound() const
		{
			return mSceneTexturesConstantBuffer.isBound() || mMobileSceneTexturesConstantBuffer.isBound();
		}

		bool isSameConstantParameters(const ShaderConstantBufferParameter& parameter)
		{
			if (parameter.isBound())
			{
				if (mSceneTexturesConstantBuffer.isBound() && mSceneTexturesConstantBuffer.getBaseIndex() == parameter.getBaseIndex())
				{
					return true;
				}

				if (mMobileSceneTexturesConstantBuffer.isBound() && mMobileSceneTexturesConstantBuffer.getBaseIndex() == parameter.getBaseIndex())
				{
					return true;
				}
			}
			return false;
		}
		
	private: 
		TShaderConstantBufferParameter<SceneTextureConstantParameters> mSceneTexturesConstantBuffer;
		TShaderConstantBufferParameter<MobileSceneTextureConstantParameters> mMobileSceneTexturesConstantBuffer;
	};

}