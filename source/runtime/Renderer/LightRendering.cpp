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

	IMPLEMENT_CONSTANT_BUFFER_STRUCT(DeferredLightConstantStruct, TEXT("DeferredLightConstants"));

	IMPLEMENT_SHADER_TYPE(template<>, TDeferredLightVS<false>, TEXT("DeferredLightVertexShaders"), TEXT("DirectionalVertexMain"), SF_Vertex);
	//IMPLEMENT_SHADER_TYPE(template<>, TDeferredLightVS<true>, TEXT("DeferredLightVertexShaders"), TEXT("RadialVertexMain"), SF_Vertex);

	uint32 getShadowQuality()
	{
		return 1;
	}

	template<bool bRadiaAttenuation>
	static VertexDeclarationRHIParamRef getDeferredLightingVertexDeclaration()
	{
		return bRadiaAttenuation ? getVertexDeclarationVector4() : GFilterVertexDeclaration.mVertexDeclarationRHI;
	}

	template<bool bUseIESProfile, bool bRadialAttenuation, bool bInverseSquaredFalloff, bool bVisulaizeLightCulling, bool bUseLightingChannels>
	class TDeferredLightPS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(TDeferredLightPS, Global)
	public:
		static bool shouldCache(EShaderPlatform platform)
		{
			return isFeatureLevelSupported(platform, ERHIFeatureLevel::SM4);
		}
		static void modifyCompilationEnvironment(EShaderPlatform platform, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(platform, outEnvironment);
			outEnvironment.setDefine(TEXT("USE_IES_PROFILE"), (uint32)bUseIESProfile);
			outEnvironment.setDefine(TEXT("RADIAL_ATTENUATION"), (uint32)bRadialAttenuation);
			
			outEnvironment.setDefine(TEXT("INVERSE_SQUARED_FALLOFF"), (uint32)bInverseSquaredFalloff);

			outEnvironment.setDefine(TEXT("LIGHT_SOURCE_SHAPE"), 1);
			
			outEnvironment.setDefine(TEXT("VISUALIZE_LIGHT_CULLING"), (uint32)bVisulaizeLightCulling);
			outEnvironment.setDefine(TEXT("USE_LIGHTING_CHANNELS"), (uint32)bUseLightingChannels);
		}

		TDeferredLightPS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mDeferredParameters.bind(initializer.mParameterMap);
		}
		TDeferredLightPS()
		{}

		void setParameters(RHICommandList& RHICmdList, const SceneView& view, const LightSceneInfo* lightSceneInfo)
		{
			const PixelShaderRHIParamRef shaderRHI = getPixelShader();
			setParametersBase(RHICmdList, shaderRHI, view, nullptr);
			setDeferredLightParameters(RHICmdList, shaderRHI, getConstantBufferParameter<DeferredLightConstantStruct>(), lightSceneInfo, view);
		}

		virtual bool serialize(Archive& ar) override
		{
			bool bShaderHasOutdatedParameters = GlobalShader::serialize(ar);
			ar << mDeferredParameters;
			ar << mLightAttenuationTexture;
			ar << mLightAttenuationTextureSampler;
			ar << mPreIntegratedBRDF;
			ar << mPreIntegratedBRDFSampler;
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

		void setParametersBase(RHICommandList& RHICmdList, const PixelShaderRHIParamRef shaderRHI, const SceneView& view, Texture* IESTextureResource)
		{
			GlobalShader::setParameters(RHICmdList, shaderRHI, view);
			mDeferredParameters.set(RHICmdList, shaderRHI, view);

			SceneRenderTargets& sceneRendertargets = SceneRenderTargets::get(RHICmdList);
			if (mLightAttenuationTexture.isBound())
			{
				setTextureParameter(RHICmdList, shaderRHI, mLightAttenuationTexture, mLightAttenuationTextureSampler, TStaticSamplerState<SF_Point, AM_Wrap, AM_Wrap, AM_Wrap>::getRHI(), sceneRendertargets.getEffectiveLightAttenuationTexture(true));
			}
			
		}


		DeferredPixelShaderParameters mDeferredParameters;
		ShaderResourceParameter	mLightAttenuationTexture;
		ShaderResourceParameter mLightAttenuationTextureSampler;
		ShaderResourceParameter mPreIntegratedBRDF;
		ShaderResourceParameter mPreIntegratedBRDFSampler;
		ShaderResourceParameter mIESTexture;
		ShaderResourceParameter mIESTextureSampler;
		ShaderResourceParameter mLightingChannelsTexture;
		ShaderResourceParameter mLightingChannelsSampler;
	};

#define IMPLEMENT_DEFERREDLIGHT_PIXELSHADER_TYPE(A, B, C, D, E, EntryName)	\
	typedef TDeferredLightPS<A, B, C, D, E> TDeferredLightPS##A##B##C##D##E;	\
	IMPLEMENT_SHADER_TYPE(template<>, TDeferredLightPS##A##B##C##D##E, TEXT("DeferredLightPixelShaders"), EntryName, SF_Pixel);
	IMPLEMENT_DEFERREDLIGHT_PIXELSHADER_TYPE(false, false, false, false, false, TEXT("DirectionalPixelMain"));

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



	void DeferredShadingSceneRenderer::renderLights(RHICommandListImmediate& RHICmdList)
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
						SortedLightSceneInfo* sortedLightInfo = new (sortedLights)SortedLightSceneInfo(lightSceneInfoCompact);
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
						const LightSceneInfoCompact& lightSceneInfoCompact = sortedLightInfo.mSceneInfo;
						const LightSceneInfo* const lightSceneInfo = lightSceneInfoCompact.mLightSceneInfo;

						renderLight(RHICmdList, lightSceneInfo, false, false);
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
				for (int32 lightIndex = attenuationLightStart; lightIndex < sortedLights.size(); lightIndex)
				{
					const SortedLightSceneInfo& sortedLightInfo = sortedLights[lightIndex];
					const LightSceneInfoCompact& lightSceneInfoCompact = sortedLightInfo.mSceneInfo;
					const LightSceneInfo& lightSceneInfo = *lightSceneInfoCompact.mLightSceneInfo;
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
						renderLight(RHICmdList, &lightSceneInfo, false, true);
					}
				}
				sceneContext.setLightAttenuationMode(true);
			}
		}
	}

	void DeferredShadingSceneRenderer::renderLight(RHICommandList& RHICmdList, const LightSceneInfo* lightSceneInfo, bool bRenderOverlap, bool bIssueDrawEvent)
	{
		RHICmdList.setBlendState(TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_One, BO_Add, BF_One, BF_One>::getRHI());
		bool bStencilDirty = false;
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
				TShaderMapRef<TDeferredLightVS<false>> vertexShader(view.mShaderMap);
				RHICmdList.setRasterizerState(TStaticRasterizerState<FM_Solid, CM_None>::getRHI());
				RHICmdList.setDepthStencilState(TStaticDepthStencilState<false
					, CF_Always>::getRHI());
				if (bRenderOverlap)
				{
				
				}
				else
				{
					if (bUseIESTexture)
					{

					}
					else
					{
						setShaderTemplLighting<false, false, false>(RHICmdList, view, *vertexShader, lightSceneInfo);
					}
				}
				vertexShader->setParameters(RHICmdList, view, lightSceneInfo);

				drawRectangle(RHICmdList, 0, 0, view.mViewRect.width(), view.mViewRect.height(), view.mViewRect.min.x, view.mViewRect.min.y, view.mViewRect.width(), view.mViewRect.height(), view.mViewRect.size(), SceneRenderTargets::get(RHICmdList).getBufferSizeXY(), *vertexShader, EDRF_UseTriangleOptimization);
			}
			else
			{

			}
		}
		if (bStencilDirty)
		{
			RHICmdList.clearDepthStencilTexture(SceneRenderTargets::get(RHICmdList).getSceneDepthTexture(), EClearDepthStencil::Stencil, (float)ERHIZBuffer::FarPlane, 0, IntRect());
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
}