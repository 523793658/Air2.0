#include "DeferredShadingRenderer.h"
#include "RHIStaticStates.h"
#include "BasePassRendering.h"
#include "SceneManagement.h"
#include "ScenePrivate.h"
#include "Classes/Materials/MaterialShader.h"
#include "PrimitiveSceneInfo.h"
#include "DebugViewModeHelpers.h"
#include "LightMapRendering.h"
#include "SystemTextures.h"
namespace Air
{
	static void setupBasePassView(RHICommandList& cmdList, const ViewInfo& view, const SceneRenderer* sceneRenderer, const bool bIsEditorPrimitivePass = false)
	{
		if (!view.isInstancedStereoPass() || bIsEditorPrimitivePass)
		{
			cmdList.setViewport(view.mViewRect.min.x, view.mViewRect.min.y, 0.0f, view.mViewRect.max.x, view.mViewRect.max.y, 1.0f);
		}
		else
		{
			if (view.bIsMuliViewEnabled)
			{
				const uint32 leftMinX = sceneRenderer->mViews[0].mViewRect.min.x;
				const uint32 leftMaxX = sceneRenderer->mViews[0].mViewRect.max.x;
				const uint32 rightMinX = sceneRenderer->mViews[1].mViewRect.min.x;
				const uint32 rightMaxX = sceneRenderer->mViews[1].mViewRect.max.x;

				const uint32 leftMaxY = sceneRenderer->mViews[0].mViewRect.max.y;
				const uint32 rightMaxY = sceneRenderer->mViews[1].mViewRect.max.y;

				cmdList.setStereoViewport(leftMinX, rightMinX, 0, 0, 0.0f, leftMaxX, rightMaxX, leftMaxY, rightMaxY, 1.0f);
			}
		}
	}



#define IMPLEMENT_BASEPASS_PIXELSHADER_TYPE(LightMapPolicyType, LightMapPolicyName, bEnableSkyLight, SkyLightName)	\
	typedef TBasePassPS<LightMapPolicyType, bEnableSkyLight> TBasePassPS##LightMapPolicyName##SkyLightName;	   \
	IMPLEMENT_MATERIAL_SHADER_TYPE (template<>, TBasePassPS##LightMapPolicyName##SkyLightName, TEXT("BasePassPixelShader"), TEXT("MainPS"), SF_Pixel);

#define IMPLEMENT_BASEPASS_VERTEXSHADER_TYPE(LightMapPolicyType, LightMapPolicyName)	   \
	typedef TBasePassVS<LightMapPolicyType, false> TBasePassVS##LightMapPolicyName; \
	IMPLEMENT_MATERIAL_SHADER_TYPE(template<>, TBasePassVS##LightMapPolicyName, TEXT("BasePassVertexShader"), TEXT("Main"), SF_Vertex); \
	typedef TBasePassHS<LightMapPolicyType, false> TBasePassHS##LightMapPolicyName;\
	IMPLEMENT_MATERIAL_SHADER_TYPE(template<>, TBasePassHS##LightMapPolicyName, TEXT("BasePassTessellationShaders"), TEXT("MainHull"), SF_Hull);\
	typedef TBasePassDS<LightMapPolicyType> TBasePassDS##LightMapPolicyName;\
	IMPLEMENT_MATERIAL_SHADER_TYPE(template<>, TBasePassDS##LightMapPolicyName, TEXT("BasePassTessellationShaders"), TEXT("MainDomain"), SF_Domain);

#define IMPLEMENT_BASEPASS_VERTEXSHADER_ONLY_TYPE(LightMapPolicyType, LightMapPolicyName, AtmosphericFogShaderName) \
	typedef TBasePassVS<LightMapPolicyType, true> TBasePassVS##LightMapPolicyName##AtmosphericFogShaderName;	  \
	IMPLEMENT_MATERIAL_SHADER_TYPE(template<>, TBasePassVS##LightMapPolicyName##AtmosphericFogShaderName, TEXT("BasePassVertexShader"), TEXT("Main"), SF_Vertex)	 \
	typedef TBasePassHS<LightMapPolicyType, true>	TBasePassHS##LightMapPolicyName##AtmosphericFogShaderName;		\
	IMPLEMENT_MATERIAL_SHADER_TYPE(template<>, TBasePassHS##LightMapPolicyName##AtmosphericFogShaderName, TEXT("BasePassTesselationShaders"), TEXT("MainHull"), SF_Hull);



#define IMPLEMENT_BASEPASS_LIGHTMAPPED_SHADER_TYPE(LightMapPolicyType, LightMapPolicyName)	 \
	IMPLEMENT_BASEPASS_VERTEXSHADER_TYPE(LightMapPolicyType, LightMapPolicyName)	   \
	IMPLEMENT_BASEPASS_VERTEXSHADER_ONLY_TYPE(LightMapPolicyType, LightMapPolicyName, mAtmosphericFog)  \
	IMPLEMENT_BASEPASS_PIXELSHADER_TYPE(LightMapPolicyType, LightMapPolicyName, true, Skylight)\
	IMPLEMENT_BASEPASS_PIXELSHADER_TYPE(LightMapPolicyType, LightMapPolicyName, false,);


	IMPLEMENT_BASEPASS_LIGHTMAPPED_SHADER_TYPE(TConstantLightMapPolicy<LMP_NO_LIGHTMAP>, NoLightMapPolicy);

	template<ELightMapPolicyType Policy>
	void getConstantBasePassShaders(const FMaterial& material, VertexFactoryType* vertexFactoryType, bool bNeedsHSDS, bool bEnableAtosphericFog, bool bEnableSkyLight, BaseHS*& hullShader, BaseDS*& domainShader, TBasePassVertexShaderPolicyParamType<ConstantLightMapPolicy>*& vertexShader, TBasePassPixelShaderPolicyParamType<ConstantLightMapPolicy>*& pixelShader)
	{
		if (bNeedsHSDS)
		{
			domainShader = material.getShader<TBasePassDS<TConstantLightMapPolicy<Policy>>>(vertexFactoryType);
			if (bEnableAtosphericFog && domainShader && isMetalPlatform(EShaderPlatform(domainShader->getTarget().mPlatform)))
			{
				hullShader = material.getShader<TBasePassHS<TConstantLightMapPolicy<Policy>, true>>(vertexFactoryType);
			}
			else
			{
				hullShader = material.getShader<TBasePassHS<TConstantLightMapPolicy<Policy>, false>>(vertexFactoryType);
			}
		}
		if (bEnableAtosphericFog)
		{
			vertexShader = (TBasePassVertexShaderPolicyParamType<ConstantLightMapPolicy>*)material.getShader<TBasePassVS<TConstantLightMapPolicy<Policy>, true>>(vertexFactoryType);
		}
		else
		{
			vertexShader = (TBasePassVertexShaderPolicyParamType<ConstantLightMapPolicy>*)material.getShader<TBasePassVS<TConstantLightMapPolicy<Policy>, false>>(vertexFactoryType);
		}
		if (bEnableSkyLight)
		{
			pixelShader = (TBasePassPixelShaderPolicyParamType<ConstantLightMapPolicy>*)material.getShader<TBasePassPS<TConstantLightMapPolicy<Policy>, true>>(vertexFactoryType);
		}
		else
		{
			pixelShader = (TBasePassPixelShaderPolicyParamType<ConstantLightMapPolicy>*)material.getShader<TBasePassPS<TConstantLightMapPolicy<Policy>, false>>(vertexFactoryType);
		}
	}

	

	template<>
	void getBasePassShaders<ConstantLightMapPolicy>(const FMaterial& material, VertexFactoryType* vertexFactoryType, ConstantLightMapPolicy lightMapPolicy, bool bNeedsHSDS, bool bEnableAtmosphericFog, bool bEnableSkyLight, BaseHS*& hullShader, BaseDS*& domainShader, TBasePassVertexShaderPolicyParamType<ConstantLightMapPolicy>*& vertexShader, TBasePassPixelShaderPolicyParamType<ConstantLightMapPolicy>*& pixelShader)
	{
		switch (lightMapPolicy.getIndrectPolicy())
		{
		case LMP_NO_LIGHTMAP:
			getConstantBasePassShaders<LMP_NO_LIGHTMAP>(material, vertexFactoryType, bNeedsHSDS, bEnableAtmosphericFog, bEnableSkyLight, hullShader, domainShader, vertexShader, pixelShader);
			break;
		default:
			break;
		}
	}


	
	bool DeferredShadingSceneRenderer::renderBasePassView(RHICommandListImmediate& RHICmdList, ViewInfo& view, FExclusiveDepthStencil::Type basePassDepthStencilAccess, const MeshPassProcessorRenderState& inDrawRenderState)
	{
		bool bDirty = false;
		MeshPassProcessorRenderState drawRenderState(inDrawRenderState);
		setupBasePassView(RHICmdList, view, this);
		view.mParallelMeshDrawCommandPasses[EMeshPass::BasePass].dispatchDraw(nullptr, RHICmdList);
		return bDirty;
	}

	void setupSharedBasePassParameter(
		RHICommandListImmediate& RHICmdList,
		const ViewInfo& view,
		SceneRenderTargets& sceneRenderTargets,
		SharedBasePassConstantParameters& sharedParameters
	)
	{
		sharedParameters.Forward = view.mForwardLightingResources->mForwardLightData;

		if (view.bIsInstancedStereoEnabled && view.mStereoPass == EStereoscopicPass::eSSP_LEFT_EYE)
		{

		}
		else
		{

		}

		const Scene* scene = view.mFamily->mScene ? view.mFamily->mScene->getRenderScene() : nullptr;

		const IPooledRenderTarget* pooledRT = GSystemTextures.mBlackDummy;

		const SceneRenderTargetItem& item = pooledRT->getRenderTargetItem();
	}

	void createOpaqueBasePassConstantBuffer(
		RHICommandListImmediate& RHICmdList,
		const ViewInfo& view,
		IPooledRenderTarget* forwardScreenSpaceShadowMask,
		TConstantBufferRef<OpaqueBasePassConstantParameters>& basePassConstantBuffer)
	{
		SceneRenderTargets& sceneRenderTargets = SceneRenderTargets::get(RHICmdList);
		OpaqueBasePassConstantParameters basePassParameters;
		setupSharedBasePassParameter(RHICmdList, view, sceneRenderTargets, basePassParameters.Shared);
		{

		}
		Scene* scene = view.mFamily->mScene ? view.mFamily->mScene->getRenderScene() : nullptr;

		if (scene)
		{
			scene->mConstantBuffers.mOpaqueBasePassConstantBuffer.updateConstantBufferImmediate(basePassParameters);
			basePassConstantBuffer = scene->mConstantBuffers.mOpaqueBasePassConstantBuffer;
		}
		else
		{
			basePassConstantBuffer = TConstantBufferRef<OpaqueBasePassConstantParameters>::createConstantBufferImmediate(basePassParameters, ConstantBuffer_SingleFrame);
		}
	}

	void setupBasePassState(FExclusiveDepthStencil::Type basePassDepthStencilAccess, const bool bShaderComplexity, MeshPassProcessorRenderState& drawRenderState)
	{
		drawRenderState.setDepthStencilAccess(basePassDepthStencilAccess);
		if (bShaderComplexity)
		{
			drawRenderState.setBlendState(TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_One, BO_Add, BF_Zero, BF_One>::getRHI());
			drawRenderState.setDepthStencilState(TStaticDepthStencilState<false, CF_DepthNearOrEqual>::getRHI());
		}
		else
		{
			drawRenderState.setBlendState(TStaticBlendStateWriteMask<CW_RGBA, CW_RGBA, CW_RGBA, CW_RGBA, CW_RGBA, CW_RGBA, CW_NONE>::getRHI());
			if (drawRenderState.getDepthStencilAccess() & FExclusiveDepthStencil::DepthWrite)
			{
				drawRenderState.setDepthStencilState(TStaticDepthStencilState<true, CF_DepthNearOrEqual>::getRHI());
			}
			else
			{
				drawRenderState.setDepthStencilState(TStaticDepthStencilState<false, CF_DepthNearOrEqual>::getRHI());
			}
		}

	}


	bool DeferredShadingSceneRenderer::renderBasePass(RHICommandListImmediate& RHICmdList, FExclusiveDepthStencil::Type basePassDepthStencilAccess, IPooledRenderTarget* forwardScreenSpaceShadowMask, bool bParallelBasePass, bool bRenderLightmapDensity)
	{
		bool bDirty = false;

		RHICmdList.automaticCacheFlushAfterComputeShader(false);
		if (false)
		{

		}
		else
		{
			if (false)
			{
				//并行渲染分支 

			}
			else
			{
				for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
				{
					ViewInfo& view = mViews[viewIndex];

					TConstantBufferRef<OpaqueBasePassConstantParameters> basePassConstantBuffer;
					createOpaqueBasePassConstantBuffer(RHICmdList, view, forwardScreenSpaceShadowMask, basePassConstantBuffer);

					MeshPassProcessorRenderState drawRenderState(view, basePassConstantBuffer);

					setupBasePassState(basePassDepthStencilAccess, mViewFamily.mEngineShowFlags.ShaderComplexity, drawRenderState);

					if (view.shouldRenderView())
					{
						mScene->mConstantBuffers.updateViewConstantBuffer(view);

						bDirty |= renderBasePassView(RHICmdList, view, basePassDepthStencilAccess, drawRenderState);
					}
					

					//渲染编辑器辅助物件
				}
			}
		}
		RHICmdList.automaticCacheFlushAfterComputeShader(true);
		RHICmdList.flushComputeShaderCache();
		return bDirty;
	}

	

	static void setDepthStencilStateForBasePass(MeshPassProcessorRenderState& drawRenderState, ERHIFeatureLevel::Type featureLevel, const MeshBatch& mesh, const PrimitiveSceneProxy* primitiveSceneProxy, bool bEnableReceiveDecalOutput, bool bUseDebugViewPS, RHIDepthStencilState* lodFadeOverrideDepthStencilState)
	{
		bool bMaskInEarlyPass = true;

		if (bEnableReceiveDecalOutput && !bUseDebugViewPS)
		{

		}
		else if(bMaskInEarlyPass)
		{
			drawRenderState.setDepthStencilState(TStaticDepthStencilState<false, CF_Equal>::getRHI());
		}
	}



	
	

	
	
}