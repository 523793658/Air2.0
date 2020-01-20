#pragma once
#include "CoreMinimal.h"
#include "sceneRendering.h"
#include "DepthRendering.h"
#include "DynamicBufferAllocator.h"
#include "LightSceneInfo.h"
namespace Air
{
	class HitProxyConsumer;

	class DeferredShadingSceneRenderer : public SceneRenderer
	{
	public:
		DeferredShadingSceneRenderer(const SceneViewFamily* inViewFamily, HitProxyConsumer* hitProxyConsumer);

		virtual void render(RHICommandListImmediate& RHICmdList) override;


	
	private:
		bool initViews(RHICommandListImmediate& RHICmdList, FExclusiveDepthStencil::Type basePassDepthStencilAccess, struct FILCUpdatePrimTaskData& ILCTaskData, GraphEventArray& sortEvents);

		bool renderPrePass(RHICommandListImmediate& RHICmdList, std::function<void()> afterTaskAreStarted);

		bool renderBasePass(RHICommandListImmediate& RHICmdList, FExclusiveDepthStencil::Type basePassDepthStencilAccess, IPooledRenderTarget* forwardScreenSpaceShadowMask, bool bParallelBasePass, bool bRenderLightmapDensity);

		bool renderBasePassView(RHICommandListImmediate& RHICmdList, ViewInfo& view, FExclusiveDepthStencil::Type basePassDepthStencilAccess, const MeshPassProcessorRenderState& inDrawRenderState);

	

	

		void renderLights(RHICommandListImmediate& RHICmdList, SortedLightSetSceneInfo &sortedLightSet);

		bool shouldRenderVolumetricFog() const;

		bool shouldUseClusteredDeferredShading() const;

		bool shouldRenderDistanceFieldAO() const;

		void asyncSortBasePassStaticData(const float3 InViewPosition, GraphEventArray &outSortEvents);

		void sortBasePassStaticData(float3 viewPosition);

		void initViewsPossiblyAfterPrepass(RHICommandListImmediate& RHICmdList, struct FILCUpdatePrimTaskData& ILCTaskData, GraphEventArray& sortEvents);

		bool renderPrePassHMD(RHICommandListImmediate& RHICmdList)
		{
			return false;
		}

		bool checkForLightFunction(const LightSceneInfo* lightSceneInfo) const;

		bool canUseTiledDeferred() const;

		void renderSky(RHICommandList& RHICmdList);

		void renderLight(RHICommandList& RHICmdList, const LightSceneInfo* lightSceneInfo, IPooledRenderTarget* screenShadowMaskTexture, bool bRenderOverlap, bool bIssueDrawEvent);

		bool renderLightFunction(RHICommandListImmediate& RHICmdList, const LightSceneInfo* lightSceneInfo, bool bLightAttenuationCleared, bool bProjectingForForwardShading);
		
		void copySceneCaptureComponentToTarget(RHICommandListImmediate& RHICmdList);

		void gatherAndSortLights(SortedLightSetSceneInfo& outSortedLights);

		void computeLightGrid(RHICommandListImmediate& RHICmdList, bool bNeedLightGrid, SortedLightSetSceneInfo& sortedLightSet);


		void clearTranslucentVolumeLightingAsyncCompute(RHICommandListImmediate& RHICmdList);
		void clearTranslucentVolumeLighting(RHICommandListImmediate& RHICmdList, int32 viewIndex);

		void renderDiffuseIndirectAndAmbientOcclusion(RHICommandListImmediate& RHICmdList);

		void renderIndirectCapsuleShadows(RHICommandListImmediate& RHICmdList, RHITexture* indirectLightingTexture, RHITexture* existingIndirectOcclusionTexture) const;

		void injectAmbientCubemapTranslucentVolumeLighting(RHICommandList& RHICmdList, const ViewInfo& view, int32 viewIndex);

		void filterTranslucentVolumeLighting(RHICommandListImmediate& RHICmdList, const ViewInfo& view, const int32 viewIndex);

		virtual void renderFinish(RHICommandListImmediate& RHICmdList) override;
	public:

		EDepthDrawingMode	mEarlyZPassMode;
		bool bEarlyZPassMovable;

	private:
		static GlobalDynamicIndexBuffer mDynamicIndexBufferForInitViews;
		static GlobalDynamicIndexBuffer mDynamicIndexBufferForInitShadows;
		static GlobalDynamicVertexBuffer mDynamicVertexBufferForInitViews;
		static GlobalDynamicVertexBuffer mDynamicVertexBufferForInitShadows;

		static TGlobalResource<GlobalDynamicReadBuffer> mDynamicReadBufferForInitViews;
		static TGlobalResource<GlobalDynamicReadBuffer> mDynamicReadBufferForInitShadows;

		bool bClusteredShadingLightsInLightGrid;

		ComputeFenceRHIRef mTranslucencyLightingVolumeClearEndFence;
	};
}