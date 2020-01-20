#include "LightRendering.h"
#include "DeferredShadingRenderer.h"
#include "PrimitiveSceneProxy.h"
#include "ScenePrivate.h"
#include "LightSceneInfo.h"
#include "RHIStaticStates.h"
#include "ShadowRendering.h"
#include "PostProcess/SceneFilterRendering.h"
#include "ShaderParameterUtils.h"
#include "StaticBoundShaderState.h"
namespace Air
{
	static int32 bAllowSimpleLight = 0;

	IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(DeferredLightConstantStruct, "DeferredLightConstants");

	

	uint32 getShadowQuality()
	{
		return 1;
	}

	template<bool bRadiaAttenuation>
	static RHIVertexDeclaration* getDeferredLightingVertexDeclaration()
	{
		return bRadiaAttenuation ? getVertexDeclarationVector4() : GFilterVertexDeclaration.mVertexDeclarationRHI;
	}

	enum class ELightSourceShape
	{
		Directional,
		Capsule,
		Rect,
		MAX
	};


	class DeferredLightPS : public GlobalShader
	{
		DECLARE_GLOBAL_SHADER(DeferredLightPS)

		SHADER_PERMUTATION_ENUM_CLASS(SourceShapeDim, "LIGHT_00SOURCE_SHAPE", ELightSourceShape);
		SHADER_PERMUTATION_BOOL(SourceTextureDim, "USE_SOURCE_TEXTURE");


		using PermutationDomain = TShaderPermutationDomain<SourceShapeDim, SourceTextureDim>;


	public:
		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			PermutationDomain permutationVector(parameters.mPermutationId);

			if (permutationVector.get<SourceShapeDim>() == ELightSourceShape::Rect)
			{

			}
			else
			{
				if (permutationVector.get<SourceTextureDim>())
				{
					return false;
				}
			}


			return isFeatureLevelSupported(parameters.mPlatform, ERHIFeatureLevel::SM4);
		}
		
		DeferredLightPS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mSceneTextureParameters.bind(initializer);
		}
		DeferredLightPS()
		{}

		void setParameters(RHICommandList& RHICmdList, const SceneView& view, const LightSceneInfo* lightSceneInfo, IPooledRenderTarget* screenShadowMaskTexture)
		{
			RHIPixelShader* shaderRHI = getPixelShader();
			setParametersBase(RHICmdList, shaderRHI, view, screenShadowMaskTexture, nullptr);
			setDeferredLightParameters(RHICmdList, shaderRHI, getConstantBufferParameter<DeferredLightConstantStruct>(), lightSceneInfo, view);
		}

		virtual bool serialize(Archive& ar) override
		{
			bool bShaderHasOutdatedParameters = GlobalShader::serialize(ar);
			ar << mSceneTextureParameters;
			ar << mLightAttenuationTexture;
			ar << mLightAttenuationTextureSampler;
			ar << mIESTexture;
			ar << mIESTextureSampler;
			ar << mLightingChannelsTexture;
			ar << mLightingChannelsSampler;
			return bShaderHasOutdatedParameters;
		}


		GlobalBoundShaderState& getBoundShaderState()
		{
			static GlobalBoundShaderState state;
			return state;
		}

	private:

		void setParametersBase(RHICommandList& RHICmdList, RHIPixelShader* shaderRHI, const SceneView& view, IPooledRenderTarget* screenShadowMaskTexture, Texture* IESTextureResource)
		{
			GlobalShader::setParameters<ViewConstantShaderParameters>(RHICmdList, shaderRHI, view.mViewConstantBuffer);
			mSceneTextureParameters.set(RHICmdList, shaderRHI, view.mFeatureLevel, ESceneTextureSetupMode::All);

			SceneRenderTargets& sceneRendertargets = SceneRenderTargets::get(RHICmdList);
			if (mLightAttenuationTexture.isBound())
			{
				setTextureParameter(RHICmdList, shaderRHI, mLightAttenuationTexture, mLightAttenuationTextureSampler, TStaticSamplerState<SF_Point, AM_Wrap, AM_Wrap, AM_Wrap>::getRHI(), screenShadowMaskTexture ? screenShadowMaskTexture->getRenderTargetItem().mShaderResourceTexture : GWhiteTexture->mTextureRHI);
			}


			
		}

		SceneTextureShaderParameters mSceneTextureParameters;
		ShaderResourceParameter	mLightAttenuationTexture;
		ShaderResourceParameter mLightAttenuationTextureSampler;
		ShaderResourceParameter mIESTexture;
		ShaderResourceParameter mIESTextureSampler;
		ShaderResourceParameter mLightingChannelsTexture;
		ShaderResourceParameter mLightingChannelsSampler;
	};
	
	IMPLEMENT_GLOBAL_SHADER(DeferredLightPS, "DeferredLightPixelShaders.hlsl", "DeferredLightPixelMain", SF_Pixel);

	template<bool bUseIESProfile, bool bRadialAttenuation, bool bInverseSquaredFalloff>
	static void setShaderTemplLighting(
		RHICommandList& RHICmdList,
		const ViewInfo& view,
		Shader* vertexShader,
		const LightSceneInfo* lightSceneInfo)
	{
		if (view.mFamily->mEngineShowFlags.VisualizeLightCulling)
		{

		}
		else
		{
			if (view.bUseLightingChannels)
			{

			}
			else
			{
				TShaderMapRef<TDeferredLightPS<false, false, false, false, false>> pixelShader(view.mShaderMap);
				setGlobalBoundShaderState(RHICmdList, view.getFeatureLevel(), pixelShader->getBoundShaderState(), getDeferredLightingVertexDeclaration<bRadialAttenuation>(), vertexShader, *pixelShader);
				pixelShader->setParameters(RHICmdList, view, lightSceneInfo);
			}
		}
	}



	void DeferredShadingSceneRenderer::renderLights(RHICommandListImmediate& RHICmdList, SortedLightSetSceneInfo& sortedLightSet)
	{
		bool bStencilBufferDirty = false;

		SimpleLightArray simpleLights;
		if (bAllowSimpleLight)
		{

		}

		TArray<SortedLightSceneInfo, SceneRenderingAllocator> sortedLights;
		sortedLights.empty(mScene->mLights.size());
		bool dynamicShadows = mViewFamily.mEngineShowFlags.DynamicShadows && getShadowQuality() > 0;

		for (TSparseArray<LightSceneInfoCompact>::TConstIterator lightIt(mScene->mLights); lightIt; ++lightIt)
		{
			const LightSceneInfoCompact& lightSceneInfoCompact = *lightIt;
			const LightSceneInfo* const lightSceneInfo = lightSceneInfoCompact.mLightSceneInfo;
			if (lightSceneInfo->shouldRenderLightViewIndependent() && !mViewFamily.mEngineShowFlags.ReflectionOverride)
			{
				for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
				{
					if (lightSceneInfo->shouldRenderLight(mViews[viewIndex]))
					{
						SortedLightSceneInfo* sortedLightInfo = new (sortedLights)SortedLightSceneInfo(lightSceneInfo);
						sortedLightInfo->mSortKey.mFields.mLightType = lightSceneInfoCompact.mLightType;
						sortedLightInfo->mSortKey.mFields.bTextureProfile = false;
						sortedLightInfo->mSortKey.mFields.bShadowed = dynamicShadows && checkForProjectedShadows(lightSceneInfo);
						sortedLightInfo->mSortKey.mFields.bLightFunction = mViewFamily.mEngineShowFlags.LightFunctions && checkForLightFunction(lightSceneInfo);
						break;
					}
				}
			}
		}

		struct CompareFSortedLightSceneInfo
		{
			FORCEINLINE bool operator()(const SortedLightSceneInfo& a, const SortedLightSceneInfo& b) const
			{
				return a.mSortKey.mPacked < b.mSortKey.mPacked;
			}
		};
		sortedLights.sort(CompareFSortedLightSceneInfo());
		{
			SceneRenderTargets& sceneContext = SceneRenderTargets::get(RHICmdList);

			int32 attenuationLightStart = sortedLights.size();
			int32 supportedByTiledDeferredLightEnd = sortedLights.size();
			bool bAnyUnsupportedByTiledDeferred = false;
			for (int32 lightIndex = 0; lightIndex < sortedLights.size(); lightIndex++)
			{
				const SortedLightSceneInfo& sortedLightInfo = sortedLights[lightIndex];
				bool bDrawShadows = sortedLightInfo.mSortKey.mFields.bShadowed;
				bool bDrawLightFunction = sortedLightInfo.mSortKey.mFields.bLightFunction;
				bool bTextureLightProfile = sortedLightInfo.mSortKey.mFields.bTextureProfile;

				if (bTextureLightProfile && supportedByTiledDeferredLightEnd == sortedLights.size())
				{
					supportedByTiledDeferredLightEnd = lightIndex;
				}

				if (bDrawShadows || bDrawLightFunction)
				{
					attenuationLightStart = lightIndex;
					if (supportedByTiledDeferredLightEnd == sortedLights.size())
					{
						supportedByTiledDeferredLightEnd = lightIndex;
					}
					break;
				}
				if (lightIndex < supportedByTiledDeferredLightEnd)
				{
					bAnyUnsupportedByTiledDeferred = bAnyUnsupportedByTiledDeferred || (sortedLightInfo.mSortKey.mFields.mLightType != LightType_Point && sortedLightInfo.mSortKey.mFields.mLightType != LightType_Spot);
				}
			}
			if (mViewFamily.mEngineShowFlags.DirectLighting)
			{
				sceneContext.setLightAttenuationMode(false);
				int32 standardDeferredStart = 0;
				
				//tiled后续实现
				if (canUseTiledDeferred())
				{

				}
				else if(simpleLights.mInstanceData.size() > 0)
				{

				}

				{

					sceneContext.beginRenderingSceneColor(RHICmdList, ESimpleRenderTargetMode::EExistingColorAndDepth, FExclusiveDepthStencil::DepthRead_StencilWrite, true);

					for (int32 lightIndex = standardDeferredStart; lightIndex < attenuationLightStart; lightIndex++)
					{
						const SortedLightSceneInfo& sortedLightInfo = sortedLights[lightIndex];
					
						const LightSceneInfo* const lightSceneInfo = sortedLightInfo.mLightSceneInfo;

						renderLight(RHICmdList, lightSceneInfo, nullptr, false, false);
					}
				}
			}
			EShaderPlatform shaderPlatform = GShaderPlatformForFeatureLevel[mFeatureLevel];

			if (isFeatureLevelSupported(shaderPlatform, ERHIFeatureLevel::SM5))
			{
				bool bRenderedRSM = false;
			}
			{
				bool bDirectLighting = mViewFamily.mEngineShowFlags.DirectLighting;

				TRefCountPtr<IPooledRenderTarget> screenShadowMaskTexture;

				for (int32 lightIndex = attenuationLightStart; lightIndex < sortedLights.size(); lightIndex)
				{
					const SortedLightSceneInfo& sortedLightInfo = sortedLights[lightIndex];

					const LightSceneInfo& lightSceneInfo = *sortedLightInfo.mLightSceneInfo;
					bool bDrawShadows = sortedLightInfo.mSortKey.mFields.bShadowed;
					bool bDrawLightFunction = sortedLightInfo.mSortKey.mFields.bLightFunction;
					bool bUseLightAttenuation = false;
					if (bDrawShadows)
					{

					}
					for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
					{
						const ViewInfo& view = mViews[viewIndex];
					}

					if (bDirectLighting)
					{
						const bool bLightFunctionRendered = renderLightFunction(RHICmdList, &lightSceneInfo, bDrawShadows, false);
						bUseLightAttenuation |= bLightFunctionRendered;
					}
					if (bUseLightAttenuation)
					{
						sceneContext.finishRenderingLightAttenuation(RHICmdList);
					}
					if (bDirectLighting)
					{

					}

					sceneContext.setLightAttenuationMode(bUseLightAttenuation);
					sceneContext.beginRenderingSceneColor(RHICmdList, ESimpleRenderTargetMode::EExistingColorAndDepth, FExclusiveDepthStencil::DepthRead_StencilWrite);
					if (bDirectLighting)
					{
						renderLight(RHICmdList, &lightSceneInfo, screenShadowMaskTexture, false, true);
					}
				}
				sceneContext.setLightAttenuationMode(true);
			}
		}
	}

	void DeferredShadingSceneRenderer::renderLight(RHICommandList& RHICmdList, const LightSceneInfo* lightSceneInfo, IPooledRenderTarget* screenShadowMaskTexture, bool bRenderOverlap, bool bIssueDrawEvent)
	{
		GraphicsPipelineStateInitializer graphicsPSOInit;
		RHICmdList.applyCachedRenderTargets(graphicsPSOInit);
		graphicsPSOInit.mBlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_One, BO_Add, BF_One, BF_One>::getRHI();
		graphicsPSOInit.mPrimitiveType = PT_TriangleList;

		for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
		{
			ViewInfo& view = mViews[viewIndex];
			if (!lightSceneInfo->shouldRenderLight(view))
			{
				continue;
			}

			bool bUseIESTexture = false;
			if (view.mFamily->mEngineShowFlags.TexturedLightProfiles)
			{
				
			}
			RHICmdList.setViewport(view.mViewRect.min.x, view.mViewRect.min.y, 0.0f, view.mViewRect.max.x, view.mViewRect.max.y, 1.0f);

			if (lightSceneInfo->mProxy->getLightType() == LightType_Directional)
			{
				graphicsPSOInit.bDepthBounds = false;
				TShaderMapRef<TDeferredLightVS<false>> vertexShader(view.mShaderMap);

				graphicsPSOInit.mRasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::getRHI();
				graphicsPSOInit.mDepthStencilState = TStaticDepthStencilState<false, CF_Always>::getRHI();


				if (bRenderOverlap)
				{
				
				}
				else
				{
					DeferredLightPS::PermutationDomain permutationVector;
					permutationVector.set<DeferredLightPS::SourceShapeDim>(ELightSourceShape::Directional);

					TShaderMapRef<DeferredLightPS> pixelShader(view.mShaderMap, permutationVector);
					graphicsPSOInit.mBoundShaderState.mVertexDeclarationRHI = GFilterVertexDeclaration.mVertexDeclarationRHI;
					graphicsPSOInit.mBoundShaderState.mVertexShaderRHI = GETSAFERHISHADER_VERTEX(*vertexShader);
					graphicsPSOInit.mBoundShaderState.mPixelShaderRHI = GETSAFERHISHADER_PIXEL(*pixelShader);

					setGraphicsPipelineState(RHICmdList, graphicsPSOInit);
					pixelShader->setParameters(RHICmdList, view, lightSceneInfo, screenShadowMaskTexture);
				}
				vertexShader->setParameters(RHICmdList, view, lightSceneInfo);

				drawRectangle(RHICmdList, 0, 0, view.mViewRect.width(), view.mViewRect.height(), view.mViewRect.min.x, view.mViewRect.min.y, view.mViewRect.width(), view.mViewRect.height(), view.mViewRect.size(), SceneRenderTargets::get(RHICmdList).getBufferSizeXY(), *vertexShader, EDRF_UseTriangleOptimization);
			}
			else
			{

			}
		}
		
	}

	bool DeferredShadingSceneRenderer::renderLightFunction(RHICommandListImmediate& RHICmdList, const LightSceneInfo* lightSceneInfo, bool bLightAttenuationCleared, bool bProjectingForForwardShading)
	{
		if (mViewFamily.mEngineShowFlags.LightFunctions)
		{
			//渲染光照方程
			
		}

		return false;
	}

	int32 bAllowSimpleLights = 0;

	void DeferredShadingSceneRenderer::gatherAndSortLights(SortedLightSetSceneInfo& outSortedLights)
	{
		if (bAllowSimpleLights)
		{
			
		}

		SimpleLightArray& simpleLights = outSortedLights.mSimpleLights;

		TArray<SortedLightSceneInfo, SceneRenderingAllocator>& sortedLights = outSortedLights.mSortedLights;
		sortedLights.empty(mScene->mLights.size() + simpleLights.mInstanceData.size());

		bool bDynamicShadows = mViewFamily.mEngineShowFlags.DynamicShadows && getShadowQuality() > 0;

		for (TSparseArray<LightSceneInfoCompact>::TConstIterator lightIt(mScene->mLights); lightIt; ++lightIt)
		{
			const LightSceneInfoCompact& lightSceneInfoCompact = *lightIt;
			const LightSceneInfo* const lightSceneInfo = lightSceneInfoCompact.mLightSceneInfo;
			if (lightSceneInfo->shouldRenderLightViewIndependent() && !mViewFamily.mEngineShowFlags.ReflectionOverride)
			{
				for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
				{
					if (lightSceneInfo->shouldRenderLight(mViews[viewIndex]))
					{

						SortedLightSceneInfo* sortedLightInfo = new (sortedLights)SortedLightSceneInfo(lightSceneInfo);

						sortedLightInfo->mSortKey.mFields.mLightType = lightSceneInfoCompact.mLightType;
						sortedLightInfo->mSortKey.mFields.bTextureProfile = mViewFamily.mEngineShowFlags.TexturedLightProfiles && lightSceneInfo->mProxy->getIESTextureResource();
						sortedLightInfo->mSortKey.mFields.bShadowed = bDynamicShadows && checkForProjectedShadows(lightSceneInfo);
						sortedLightInfo->mSortKey.mFields.bLightFunction = mViewFamily.mEngineShowFlags.LightFunctions && checkForLightFunction(lightSceneInfo);
						sortedLightInfo->mSortKey.mFields.bUsesLightingChannels = mViews[viewIndex].bUseLightingChannels && lightSceneInfo->mProxy->getLightingChannelMask() != getDefaultLightingChannelMask();
						sortedLightInfo->mSortKey.mFields.bIsNotSimpleLight = 1;


						const bool bTiledOrClusteredDeferredSupported = !sortedLightInfo->mSortKey.mFields.bTextureProfile &&
							!sortedLightInfo->mSortKey.mFields.bShadowed &&
							!sortedLightInfo->mSortKey.mFields.bLightFunction &&
							!sortedLightInfo->mSortKey.mFields.bUsesLightingChannels
							&& lightSceneInfoCompact.mLightType != LightType_Directional
							&& lightSceneInfoCompact.mLightType != LightType_Rect;

						sortedLightInfo->mSortKey.mFields.bTiledDeferredNotSupported = !(bTiledOrClusteredDeferredSupported && lightSceneInfo->mProxy->isTitledDferredLightingSupported());
						sortedLightInfo->mSortKey.mFields.bClusteredDeferredNotSupported = !bTiledOrClusteredDeferredSupported;
						break;
					}
				}
			}
		}
		for (int32 simpleLightIndex = 0; simpleLightIndex < simpleLights.mInstanceData.size(); simpleLightIndex++)
		{
			SortedLightSceneInfo* sortedLightInfo = new(sortedLights)SortedLightSceneInfo(simpleLightIndex);
			sortedLightInfo->mSortKey.mFields.mLightType = LightType_Point;
			sortedLightInfo->mSortKey.mFields.bTextureProfile = 0;
			sortedLightInfo->mSortKey.mFields.bShadowed = 0;
			sortedLightInfo->mSortKey.mFields.bLightFunction = 0;
			sortedLightInfo->mSortKey.mFields.bUsesLightingChannels = 0;
			sortedLightInfo->mSortKey.mFields.bIsNotSimpleLight = 0;
			sortedLightInfo->mSortKey.mFields.bTiledDeferredNotSupported = 0;
			sortedLightInfo->mSortKey.mFields.bClusteredDeferredNotSupported = 0;
		}

		struct CompareSortedLightSceneInfo
		{
			FORCEINLINE bool operator()(const SortedLightSceneInfo& a, const SortedLightSceneInfo& b) const
			{
				return a.mSortKey.mPacked < b.mSortKey.mPacked;
			}
		};

		sortedLights.sort(CompareSortedLightSceneInfo());

		outSortedLights.mSimpleLightsEnd = sortedLights.size();
		outSortedLights.mTiledSupportedEnd = sortedLights.size();
		outSortedLights.mClusteredSupportedEnd = sortedLights.size();
		outSortedLights.mAttenuationLightStart = sortedLights.size();

		for (int32 lightIndex = 0; lightIndex < sortedLights.size(); lightIndex++)
		{
			const SortedLightSceneInfo& sortedLightInfo = sortedLights[lightIndex];
			const bool bDrawShadows = sortedLightInfo.mSortKey.mFields.bShadowed;
			const bool bDrawLightFunction = sortedLightInfo.mSortKey.mFields.bLightFunction;
			const bool bTextureLightProfile = sortedLightInfo.mSortKey.mFields.bTextureProfile;
			const bool bLightingChannels = sortedLightInfo.mSortKey.mFields.bUsesLightingChannels;

			if (sortedLightInfo.mSortKey.mFields.bIsNotSimpleLight && outSortedLights.mSimpleLightsEnd == sortedLights.size())
			{
				outSortedLights.mSimpleLightsEnd = lightIndex;
			}

			if (sortedLightInfo.mSortKey.mFields.bTiledDeferredNotSupported && outSortedLights.mTiledSupportedEnd == sortedLights.size())
			{
				outSortedLights.mTiledSupportedEnd = lightIndex;
			}

			if (sortedLightInfo.mSortKey.mFields.bClusteredDeferredNotSupported && outSortedLights.mClusteredSupportedEnd == sortedLights.size())
			{
				outSortedLights.mClusteredSupportedEnd = lightIndex;
			}

			if (bDrawShadows || bDrawLightFunction || bLightingChannels)
			{
				BOOST_ASSERT(sortedLightInfo.mSortKey.mFields.bTiledDeferredNotSupported);
				outSortedLights.mAttenuationLightStart = lightIndex;
				break;
			}
		}
		BOOST_ASSERT(outSortedLights.mTiledSupportedEnd >= outSortedLights.mSimpleLightsEnd);
		BOOST_ASSERT(outSortedLights.mClusteredSupportedEnd >= outSortedLights.mTiledSupportedEnd);
		BOOST_ASSERT(outSortedLights.mAttenuationLightStart >= outSortedLights.mClusteredSupportedEnd);
	}
}