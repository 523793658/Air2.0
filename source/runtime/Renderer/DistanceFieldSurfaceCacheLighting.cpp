#include "DeferredShadingRenderer.h"
#include "ScenePrivate.h"
#include "DistanceFieldSurfaceCacheLighting.h"
#include "RHIStaticStates.h"
#include "StaticBoundShaderState.h"
#include "PostProcess/PostProcessing.h"
#include "PostProcess/SceneFilterRendering.h"
#include "GlobalDistanceField.h"
#include "sceneRendering.h"
namespace Air
{

	int32 GDistanceFieldAOApplyToStaticIndirect = 0;

	int32 GAOOverwriteSceneColor = 0;

	const int32 GAODownsampleFactor = 2;

	int32 GAOGlobalDistanceField = 1;

	int32 GAOUseSurfaceCache = 0;


	DistanceFieldAOParameters::DistanceFieldAOParameters(float inOcclusionMaxDistance, float inContrast /* = 0 */)
	{
		Contrast = Math::clamp(inContrast, 0.01f, 2.0f);
		inOcclusionMaxDistance = Math::clamp(inOcclusionMaxDistance, 2.0f, 3000.0f);
		if (GAOGlobalDistanceField != 0 && GAOUseSurfaceCache == 0)
		{
			extern float GAOGlobalDFStartDistance;
			ObjectMaxOcclusionDistance = Math::min(inOcclusionMaxDistance, GAOGlobalDFStartDistance);
			GlobalMaxOcclusionDistance = inOcclusionMaxDistance >= GAOGlobalDFStartDistance ? inOcclusionMaxDistance : 0;
		}
		else
		{
			ObjectMaxOcclusionDistance = inOcclusionMaxDistance;
			GlobalMaxOcclusionDistance = 0;
		}
	}

	bool DeferredShadingSceneRenderer::shouldRenderDistanceFieldAO() const
	{
		return mViewFamily.mEngineShowFlags.DistanceFieldAO
			&& !mViewFamily.mEngineShowFlags.VisualizeDistanceFieldAO
			&& !mViewFamily.mEngineShowFlags.VisualizeDistanceFieldGI
			&& !mViewFamily.mEngineShowFlags.VisualizeMeshDistanceFields;
	}

	bool shouldRenderDeferredDynamicSkyLight(const Scene* scene, const SceneViewFamily& viewFamily)
	{
		return scene->mSkyLight
			&& scene->mSkyLight->mProcessedTexture
			&& !scene->mSkyLight->bWantsStaticShadowing
			&& !scene->mSkyLight->bHasStaticLighting
			&& viewFamily.mEngineShowFlags.SkyLighting
			&& scene->getFeatureLevel() >= ERHIFeatureLevel::SM4
			&& !isAnyForwardShadingEnabled(scene->getShaderPlatform())
			&& !viewFamily.mEngineShowFlags.VisualizeLightCulling;
	}

	template<bool bApplyShadowing, bool bSupportIrradiance>
	class TDynamicSkyLightDiffusePS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(TDynamicSkyLightDiffusePS, Global);
	public:
		static bool shouldCache(EShaderPlatform platform)
		{
			return isFeatureLevelSupported(platform, ERHIFeatureLevel::SM4);
		}

		static void modifyCompilationEnvironment(EShaderPlatform platform, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(platform, outEnvironment);
			outEnvironment.setDefine(TEXT("DOWNSAMPLE_FACTOR"), GAODownsampleFactor);
			outEnvironment.setDefine(TEXT("APPLY_SHADOWING"), bApplyShadowing);
			outEnvironment.setDefine(TEXT("SUPPORT_IRRADIANCE"), bSupportIrradiance);
		}

		TDynamicSkyLightDiffusePS() {}

		TDynamicSkyLightDiffusePS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mDeferredParameters.bind(initializer.mParameterMap);
			mDynamicBentNormalAOTexture.bind(initializer.mParameterMap, TEXT("BentNormalAOTexture"));
			mDynamicBentNormalAOSampler.bind(initializer.mParameterMap, TEXT("BentNormalAOSampler"));
			mDynamicIrradianceTexture.bind(initializer.mParameterMap, TEXT("IrradianceTexture"));
			mDynamicIrradianceSampler.bind(initializer.mParameterMap, TEXT("IrradianceSampler"));
			mContrastAndNormalizeMulAdd.bind(initializer.mParameterMap, TEXT("ContrastAndNormalizeMulAdd"));
			mOcclusionTintAndMinOcclusion.bind(initializer.mParameterMap, TEXT("OcclusionTintAndMinOcclusion"));
		}

		void setParameters(RHICommandList& RHICmdList, const SceneView& view, TextureRHIParamRef dynamicBentNormalAO, IPooledRenderTarget* dynamicIrradiance, const DistanceFieldAOParameters& parameters, const SkyLightSceneProxy* skyLight)
		{
			const PixelShaderRHIParamRef shaderRHI = getPixelShader();
			GlobalShader::setParameters(RHICmdList, shaderRHI, view);
			mDeferredParameters.set(RHICmdList, shaderRHI, view);
			setTextureParameter(RHICmdList, shaderRHI, mDynamicBentNormalAOTexture, mDynamicBentNormalAOSampler, TStaticSamplerState<SF_Point>::getRHI(), dynamicBentNormalAO);
			if (mDynamicIrradianceTexture.isBound())
			{
				setTextureParameter(RHICmdList, shaderRHI, mDynamicIrradianceTexture, mDynamicIrradianceSampler, TStaticSamplerState<SF_Point>::getRHI(), dynamicIrradiance->getRenderTargetItem().mShaderResourceTexture);

			}

			const float min = 1 / (1 + Math::exp(-parameters.Contrast * (0 * 10 - 5)));
			const float max = 1 / (1 + Math::exp(-parameters.Contrast * (1 * 10 - 5)));
			const float mul = 1.0f / (max - min);
			const float add = -min / (max - min);

			setShaderValue(RHICmdList, shaderRHI, mContrastAndNormalizeMulAdd, float3(parameters.Contrast, mul, add));

			float4 occlusionTintAndMinOccusionValue = float4(skyLight->mOcclusionTint);
			occlusionTintAndMinOccusionValue.w = skyLight->mMinOcclusion;
			setShaderValue(RHICmdList, shaderRHI, mOcclusionTintAndMinOcclusion, occlusionTintAndMinOccusionValue);
		}

		virtual bool serialize(Archive& ar) override
		{
			bool bShaderHasOutdatedParameters = GlobalShader::serialize(ar);
			ar << mDeferredParameters;
			ar << mDynamicBentNormalAOTexture;
			ar << mDynamicBentNormalAOSampler;
			ar << mDynamicIrradianceTexture;
			ar << mDynamicIrradianceSampler;
			ar << mContrastAndNormalizeMulAdd;
			ar << mOcclusionTintAndMinOcclusion;
			return bShaderHasOutdatedParameters;
		}
	private:
		DeferredPixelShaderParameters mDeferredParameters;
		ShaderResourceParameter mDynamicBentNormalAOTexture;
		ShaderResourceParameter mDynamicBentNormalAOSampler;
		ShaderResourceParameter mDynamicIrradianceTexture;
		ShaderResourceParameter mDynamicIrradianceSampler;
		ShaderParameter mContrastAndNormalizeMulAdd;
		ShaderParameter mOcclusionTintAndMinOcclusion;
	};

#define IMPLEMENT_SKYLIGHT_PS_TYPE(bApplyShadowing, bSupportIrradiance) \
	typedef TDynamicSkyLightDiffusePS<bApplyShadowing, bSupportIrradiance> TDynamicSkyLightDiffusePS##bApplyShadowing##bSupportIrradiance; \
	IMPLEMENT_SHADER_TYPE(template<>, TDynamicSkyLightDiffusePS##bApplyShadowing##bSupportIrradiance, TEXT("SkyLighting"), TEXT("SkyLightDiffusePS"), SF_Pixel);

	IMPLEMENT_SKYLIGHT_PS_TYPE(true, true)
	IMPLEMENT_SKYLIGHT_PS_TYPE(true, false)
	IMPLEMENT_SKYLIGHT_PS_TYPE(false, true)
	IMPLEMENT_SKYLIGHT_PS_TYPE(false, false)




	void DeferredShadingSceneRenderer::renderDynamicSkyLighting(RHICommandListImmediate& RHICmdList, const TRefCountPtr<IPooledRenderTarget>& velocityTexture, TRefCountPtr<IPooledRenderTarget>& dynamicBentNormalAO)
	{
		if (shouldRenderDeferredDynamicSkyLight(mScene, mViewFamily))
		{
			bool bApplyShadowing = false;
			DistanceFieldAOParameters parameters(mScene->mSkyLight->mOcclusionMaxDistance, mScene->mSkyLight->mContrast);
			TRefCountPtr<IPooledRenderTarget> dynamicIrradiance;
			if (mScene->mSkyLight->bCastShadows && !GDistanceFieldAOApplyToStaticIndirect
				&& shouldRenderDistanceFieldAO()
				&& mViewFamily.mEngineShowFlags.AmbientOcclusion)
			{
				bApplyShadowing = false;
			}

			SceneRenderTargets& sceneContext = SceneRenderTargets::get(RHICmdList);

			sceneContext.beginRenderingSceneColor(RHICmdList, ESimpleRenderTargetMode::EExistingColorAndDepth, FExclusiveDepthStencil::DepthRead_StencilRead);

			for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
			{
				const ViewInfo& view = mViews[viewIndex];
				RHICmdList.setViewport(view.mViewRect.min.x, view.mViewRect.min.y, 0.0f, view.mViewRect.max.x, view.mViewRect.max.y, 1.0f);
				RHICmdList.setRasterizerState(TStaticRasterizerState<FM_Solid, CM_None>::getRHI());
				RHICmdList.setDepthStencilState(TStaticDepthStencilState<false, CF_Always>::getRHI());

				if (GAOOverwriteSceneColor)
				{
					RHICmdList.setBlendState(TStaticBlendState<>::getRHI());
				}
				else
				{
					RHICmdList.setBlendState(TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_One, BO_Add, BF_One, BF_One>::getRHI());
				}

				const bool bUseDistanceFieldGI = false;
				TShaderMapRef<PostProcessVS> vertexShader(view.mShaderMap);

				if (bApplyShadowing)
				{

				}
				else
				{
					if (bUseDistanceFieldGI)
					{

					}
					else
					{
						TShaderMapRef<TDynamicSkyLightDiffusePS<false, false>> pixelShader(view.mShaderMap);
						static GlobalBoundShaderState boundShaderState;
						setGlobalBoundShaderState(RHICmdList, view.getFeatureLevel(), boundShaderState, GFilterVertexDeclaration.mVertexDeclarationRHI, *vertexShader, *pixelShader);
						pixelShader->setParameters(RHICmdList, view, GWhiteTexture->mTextureRHI, nullptr, parameters, mScene->mSkyLight);
					}
				}

				drawRectangle(RHICmdList,
					0, 0,
					view.mViewRect.width(), view.mViewRect.height(),
					view.mViewRect.min.x, view.mViewRect.min.y,
					view.mViewRect.width(), view.mViewRect.height(),
					int2(view.mViewRect.width(), view.mViewRect.height()),
					sceneContext.getBufferSizeXY(),
					*vertexShader);
			}
		}
	}
}