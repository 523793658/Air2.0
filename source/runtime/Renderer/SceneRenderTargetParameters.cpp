#include "PostProcess/SceneRenderTargets.h"
#include "SceneRenderTargetParameters.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"
namespace Air
{
	void SceneTextureShaderParameters::bind(const ShaderParameterMap& parameterMap)
	{
		mSceneColorTextureParameter.bind(parameterMap, TEXT("SceneColorTexture"));
		mSceneColorTextureParameterSampler.bind(parameterMap, TEXT("SceneColorTextureSampler"));

		mSceneDepthTextureParameter.bind(parameterMap, TEXT("SceneDepthTexture"));
		mSceneDepthTextureParameterSampler.bind(parameterMap, TEXT("SceneDepthTextureSampler"));

		mSceneAlphaCopyTextureParameter.bind(parameterMap, TEXT("SceneAlphaCopyTexture"));
		mSceneAlphaCopyTextureParameterSampler.bind(parameterMap, TEXT("SceneAlphaCopyTextureSampler"));

		mSceneDepthTextureNonMS.bind(parameterMap, TEXT("SceneDepthTextureNonMS"));
		mSceneColorSurfaceParameter.bind(parameterMap, TEXT("SceneColorSurface"));
		mSceneDepthSurfaceParameter.bind(parameterMap, TEXT("SceneDepthSurface"));

		mDirectionalOcclusionSampler.bind(parameterMap, TEXT("DirectionalOcclusionSampler"));
		mDirectionalOcclusionTexture.bind(parameterMap, TEXT("DirectionalOcclusionTexture"));

		mMobileCustomStencilTexture.bind(parameterMap, TEXT("MobileCustomStencilTexture"));
		mMobileCustomStencilTextureSampler.bind(parameterMap, TEXT("MobileCustomStencilTextureSampler"));
	}

	void DeferredPixelShaderParameters::bind(const ShaderParameterMap& parameterMap)
	{
		mSceneTextureParamters.bind(parameterMap);
		mGBufferResources.bind(parameterMap, TEXT("GBuffers"));
		mDBufferATextureMS.bind(parameterMap, TEXT("DBufferATextureMS"));
		mDBufferBTextureMS.bind(parameterMap, TEXT("DBufferBTextureMS"));
		mDBufferCTextureMS.bind(parameterMap, TEXT("DBufferCTextureMS"));
		mScreenSpaceAOTextureMS.bind(parameterMap, TEXT("ScreenSpaceAOTexture"));
		mDBufferATextureNonMS.bind(parameterMap, TEXT("DBufferATextureNonMS"));
		mDBufferBTextureNonMS.bind(parameterMap, TEXT("DBufferBTextureNonMS"));
		mDBufferCTextureNonMS.bind(parameterMap, TEXT("DBufferCTextureNonMS"));
		mScreenSpaceAOTextureMS.bind(parameterMap, TEXT("ScreenSpaceAOTextureMS"));
		mCustomDepthTextureNonMS.bind(parameterMap, TEXT("CustomDepthTextureNonMS"));
		mDBufferATexture.bind(parameterMap, TEXT("DBufferATexture"));
		mDBufferBTexture.bind(parameterMap, TEXT("DBufferBTexture"));
		mDBufferCTexture.bind(parameterMap, TEXT("DBufferCTexture"));
		mDBufferATextureSampler.bind(parameterMap, TEXT("DBufferATextureSampler"));
		mDBufferBTextureSampler.bind(parameterMap, TEXT("DBufferBTextureSampler"));
		mDBufferCTextureSampler.bind(parameterMap, TEXT("DBufferCTextureSampler"));
		mScreenSpaceAOTexture.bind(parameterMap, TEXT("ScreenSpaceAOTexture"));
		mScreenSpaceAOTextureSampler.bind(parameterMap, TEXT("ScreenSpaceAOTextureSampler"));
		mCustomDepthTexture.bind(parameterMap, TEXT("CustomDepthTexture"));
		mCustomDepthTextureSampler.bind(parameterMap, TEXT("CustomDepthTextureSampler"));
		mCustomStencilTexture.bind(parameterMap, TEXT("CustomStencilTexture"));
		mDBufferRenderMask.bind(parameterMap, TEXT("DBufferMask"));

	}

	template<typename ShaderRHIParamRef, typename TRHICmdList>
	void DeferredPixelShaderParameters::set(TRHICmdList& rhiCmdList, const ShaderRHIParamRef shaderRHI, const SceneView& view, ESceneRenderTargetsMode::Type textureMode /* = ESceneRenderTargetsMode::SetTextures */) const
	{
		SceneRenderTargets& sceneContext = SceneRenderTargets::get(rhiCmdList);
		mSceneTextureParamters.set(rhiCmdList, shaderRHI, view, textureMode, SF_Point);
		{
		}

		const auto featureLevel = view.getFeatureLevel();
		if (featureLevel >= ERHIFeatureLevel::SM4)
		{
			if (mGBufferResources.isBound())
			{
				setConstantBufferParameter(rhiCmdList, shaderRHI, mGBufferResources, sceneContext.getGBufferResourcesConstantBuffer());
			}
			//setTextureParameter(rhiCmdList, shaderRHI, mScreenSpaceAOTexture, mScreenSpaceAOTextureSampler, TStaticSamplerState<>::getRHI(), screspa)


		}
		else if (textureMode == ESceneRenderTargetsMode::DontSet || textureMode == ESceneRenderTargetsMode::DontSetIgnoreBoundByEditorCompositing)
		{
			BOOST_ASSERT(!mGBufferResources.isBound());
		}
	}
#define IMPLEMENT_DEFERRED_PARAMETERS_SET(ShaderRHIParamRef, TRHICmdList) \
 template void DeferredPixelShaderParameters::set<ShaderRHIParamRef, TRHICmdList>(TRHICmdList & RHICmdList, const ShaderRHIParamRef shaderRHI, const SceneView& view, ESceneRenderTargetsMode::Type textureMode) const ;

	IMPLEMENT_DEFERRED_PARAMETERS_SET(VertexShaderRHIParamRef, RHICommandList)
	IMPLEMENT_DEFERRED_PARAMETERS_SET(HullShaderRHIParamRef, RHICommandList)
	IMPLEMENT_DEFERRED_PARAMETERS_SET(DomainShaderRHIParamRef, RHICommandList)
	IMPLEMENT_DEFERRED_PARAMETERS_SET(GeometryShaderRHIParamRef, RHICommandList)
	IMPLEMENT_DEFERRED_PARAMETERS_SET(PixelShaderRHIParamRef, RHICommandList)
	IMPLEMENT_DEFERRED_PARAMETERS_SET(ComputeShaderRHIParamRef, RHICommandList)



	template<typename ShaderRHIParamRef, typename TRHICmdList>
	void SceneTextureShaderParameters::set(TRHICmdList& RHICmdList, const ShaderRHIParamRef& shaderRHI, const SceneView& view, ESceneRenderTargetsMode::Type textureMode /* = ESceneRenderTargetsMode.SetTextures */, ESamplerFilter colorFilter /* = SF_Point */) const
	{
		if (textureMode == ESceneRenderTargetsMode::SetTextures)
		{
			SceneRenderTargets& sceneContext = SceneRenderTargets::get(RHICmdList);
			if (mSceneColorTextureParameter.isBound())
			{
				SamplerStateRHIRef filter;
				switch (colorFilter)
				{
				case SF_Bilinear:
					filter = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::getRHI();
					break;
				case SF_Trilinear:
					filter = TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::getRHI();
					break;
				case SF_AnisotropicPoint:
					filter = TStaticSamplerState<SF_AnisotropicPoint, AM_Clamp, AM_Clamp, AM_Clamp>::getRHI();
					break;
				case SF_AnisotropicLinear:
					filter = TStaticSamplerState<SF_AnisotropicLinear, AM_Clamp, AM_Clamp, AM_Clamp>::getRHI();
					break;
				case SF_Point:
				default:
					filter = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::getRHI();
					break;
				}
				setTextureParameter(RHICmdList, shaderRHI, mSceneColorTextureParameter,
					mSceneColorTextureParameterSampler, filter, sceneContext.getSceneColorTexture());
			}
			if (mSceneDepthTextureParameter.isBound() || mSceneDepthTextureParameterSampler.isBound())
			{
				const Texture2DRHIRef* depthTexture = sceneContext.getActualDepthTexture();
				if (sceneContext.isSeparateTranslucencyPass() && sceneContext.isSeparateTranslucencyDepthValid())
				{
					int2 outScaledSize;
					float outScale;
					sceneContext.getSeparateTranslucencyDimensions(outScaledSize, outScale);
					if (outScale < 1.0f)
					{
						depthTexture = &sceneContext.getSeparateTranslucencyDepthSurface();
					}
				}
				setTextureParameter(RHICmdList, shaderRHI, mSceneDepthTextureParameter, mSceneDepthTextureParameterSampler, TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::getRHI(), *depthTexture);

			}
			const auto featureLevel = view.getFeatureLevel();
			if (featureLevel >= ERHIFeatureLevel::SM5)
			{
				setTextureParameter(RHICmdList, shaderRHI, mSceneColorSurfaceParameter, sceneContext.getSceneColorSurface());
			}
			if (featureLevel >= ERHIFeatureLevel::SM4)
			{
				if (GSupportsDepthFetchDuringDepthTest)
				{
					if (mSceneDepthSurfaceParameter.isBound())
					{
						setTextureParameter(RHICmdList, shaderRHI, mSceneDepthTextureParameter, sceneContext.getSceneDepthSurface());
					}
					if (mSceneDepthTextureNonMS.isBound())
					{
						setTextureParameter(RHICmdList, shaderRHI, mSceneDepthTextureNonMS, sceneContext.getSceneDepthTexture());
					}
				}
				else
				{
					if (mSceneDepthSurfaceParameter.isBound())
					{
						setTextureParameter(RHICmdList, shaderRHI, mSceneDepthSurfaceParameter, sceneContext.getAuxiliarySceneDepthSurface());
					}
					if (mSceneDepthTextureNonMS.isBound())
					{
						setTextureParameter(RHICmdList, shaderRHI, mSceneDepthTextureNonMS, sceneContext.getAuxiliarySceneDepthSurface());
					}
				}
			}
			if (featureLevel <= ERHIFeatureLevel::ES3_1)
			{
				
			}
		}
		else if (textureMode == ESceneRenderTargetsMode::DontSet)
		{
			BOOST_ASSERT(!mSceneDepthTextureParameter.isBound() &&
				!mSceneDepthTextureParameter.isBound() &&
				!mSceneColorSurfaceParameter.isBound() &&
				!mSceneDepthSurfaceParameter.isBound() &&
				!mSceneDepthTextureNonMS.isBound());
		}
		else if (textureMode == ESceneRenderTargetsMode::DontSetIgnoreBoundByEditorCompositing)
		{
			BOOST_ASSERT(!mSceneColorTextureParameter.isBound()
				&& !mSceneDepthTextureParameter.isBound()
				&& !mSceneColorSurfaceParameter.isBound()
				&& !mSceneDepthSurfaceParameter.isBound());
		}
		if (mDirectionalOcclusionTexture.isBound())
		{

		}
	}

#define IMPLEMENT_SCENE_TEXTURE_PARAM_SET(ShaderRHIParamRef)	\
template void SceneTextureShaderParameters::set<ShaderRHIParamRef>(\
	RHICommandList& RHICmdList,	\
	const ShaderRHIParamRef& shaderRHI,	\
	const SceneView&	view,	\
	ESceneRenderTargetsMode::Type textureMode,	\
	ESamplerFilter colorFilter	\
	) const;

	IMPLEMENT_SCENE_TEXTURE_PARAM_SET(VertexShaderRHIParamRef);
	IMPLEMENT_SCENE_TEXTURE_PARAM_SET(HullShaderRHIParamRef);
	IMPLEMENT_SCENE_TEXTURE_PARAM_SET(DomainShaderRHIParamRef);
	IMPLEMENT_SCENE_TEXTURE_PARAM_SET(GeometryShaderRHIParamRef);
	IMPLEMENT_SCENE_TEXTURE_PARAM_SET(PixelShaderRHIParamRef);
	IMPLEMENT_SCENE_TEXTURE_PARAM_SET(ComputeShaderRHIParamRef);


	Archive& operator << (Archive& ar, DeferredPixelShaderParameters& parameters)
	{
		ar << parameters.mSceneTextureParamters;
		ar << parameters.mDBufferATextureMS;
		ar << parameters.mDBufferBTextureMS;
		ar << parameters.mDBufferCTextureMS;
		ar << parameters.mScreenSpaceAOTextureMS;

		ar << parameters.mDBufferATextureNonMS;
		ar << parameters.mDBufferBTextureNonMS;
		ar << parameters.mDBufferCTextureNonMS;
		ar << parameters.mScreenSpaceAOTextureNonMS;

		ar << parameters.mCustomDepthTextureNonMS;

		ar << parameters.mDBufferATexture;
		ar << parameters.mDBufferRenderMask;
		ar << parameters.mDBufferATextureSampler;
		ar << parameters.mDBufferBTexture;
		ar << parameters.mDBufferBTextureSampler;
		ar << parameters.mDBufferCTexture;
		ar << parameters.mDBufferCTextureSampler;
		ar << parameters.mScreenSpaceAOTexture;
		ar << parameters.mScreenSpaceAOTextureSampler;
		ar << parameters.mCustomDepthTexture;
		ar << parameters.mCustomDepthTextureSampler;
		ar << parameters.mCustomStencilTexture;
		return ar;
	}


	Archive& operator<<(Archive& ar, SceneTextureShaderParameters& parameters)
	{
		ar << parameters.mSceneColorTextureParameter;
		ar << parameters.mSceneColorTextureParameterSampler;
		ar << parameters.mSceneAlphaCopyTextureParameter;
		ar << parameters.mSceneAlphaCopyTextureParameterSampler;
		ar << parameters.mSceneColorSurfaceParameter;
		ar << parameters.mSceneDepthTextureParameter;
		ar << parameters.mSceneDepthTextureParameterSampler;
		ar << parameters.mSceneDepthSurfaceParameter;
		ar << parameters.mSceneDepthTextureNonMS;
		ar << parameters.mDirectionalOcclusionTexture;
		ar << parameters.mDirectionalOcclusionSampler;
		ar << parameters.mMobileCustomStencilTexture;
		ar << parameters.mMobileCustomStencilTextureSampler;
		return ar;
	}
}