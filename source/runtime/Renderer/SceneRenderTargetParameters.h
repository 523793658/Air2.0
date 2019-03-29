#pragma once
#include "CoreMinimal.h"
#include "ShaderParameters.h"
#include "SceneView.h"
namespace Air
{


	namespace ESceneRenderTargetsMode
	{
		enum Type
		{
			SetTextures,
			DontSet,
			DontSetIgnoreBoundByEditorCompositing,
		};
	}

	class SceneTextureShaderParameters
	{
	public:
		void bind(const ShaderParameterMap& parameterMap);
		template<typename ShaderRHIParamRef, typename TRHICmdList>
		void set(
			TRHICmdList& RHICmdList,
			const ShaderRHIParamRef& shaderRHI,
			const SceneView& view,
			ESceneRenderTargetsMode::Type textureMode = ESceneRenderTargetsMode.SetTextures,
			ESamplerFilter colorFilter = SF_Point
		)const;
		friend Archive& operator << (Archive& ar, SceneTextureShaderParameters& p);
	private: 
		ShaderResourceParameter mSceneColorTextureParameter;
		ShaderResourceParameter mSceneColorTextureParameterSampler;

		ShaderResourceParameter mSceneDepthTextureParameter;
		ShaderResourceParameter mSceneDepthTextureParameterSampler;

		ShaderResourceParameter mSceneAlphaCopyTextureParameter;
		ShaderResourceParameter mSceneAlphaCopyTextureParameterSampler;

		ShaderResourceParameter mSceneColorSurfaceParameter;
		ShaderResourceParameter mSceneDepthSurfaceParameter;

		ShaderResourceParameter mSceneDepthTextureNonMS;
		ShaderResourceParameter mDirectionalOcclusionSampler;
		ShaderResourceParameter mDirectionalOcclusionTexture;

		ShaderResourceParameter mMobileCustomStencilTexture;
		ShaderResourceParameter mMobileCustomStencilTextureSampler;
	};

	class DeferredPixelShaderParameters
	{
	public:	 
		void bind(const ShaderParameterMap& parameterMap);
		template<typename ShaderRHIParamRef, typename TRHICmdList>
		void set(TRHICmdList& rhiCmdList,
			const ShaderRHIParamRef shaderRHI,
			const SceneView& view, ESceneRenderTargetsMode::Type textureMode = ESceneRenderTargetsMode::SetTextures) const;

		friend Archive& operator <<(Archive& ar, DeferredPixelShaderParameters& p);
	private:
		SceneTextureShaderParameters mSceneTextureParamters;
		ShaderConstantBufferParameter mGBufferResources;
		ShaderResourceParameter	mDBufferATextureMS;
		ShaderResourceParameter	mDBufferBTextureMS;
		ShaderResourceParameter	mDBufferCTextureMS;
		ShaderResourceParameter	mScreenSpaceAOTextureMS;
		ShaderResourceParameter	mDBufferATextureNonMS;
		ShaderResourceParameter	mDBufferBTextureNonMS;
		ShaderResourceParameter	mDBufferCTextureNonMS;
		ShaderResourceParameter mScreenSpaceAOTextureNonMS;
		ShaderResourceParameter mCustomDepthTextureNonMS;
		ShaderResourceParameter mDBufferATexture;
		ShaderResourceParameter mDBufferRenderMask;
		ShaderResourceParameter mDBufferATextureSampler;
		ShaderResourceParameter mDBufferBTexture;
		ShaderResourceParameter mDBufferBTextureSampler;
		ShaderResourceParameter mDBufferCTexture;
		ShaderResourceParameter mDBufferCTextureSampler;
		ShaderResourceParameter mScreenSpaceAOTexture;
		ShaderResourceParameter mScreenSpaceAOTextureSampler;
		ShaderResourceParameter mCustomDepthTexture;
		ShaderResourceParameter mCustomDepthTextureSampler;
		ShaderResourceParameter mCustomStencilTexture;
	};

}