#pragma once
#include "CoreMinimal.h"
#include "sceneRendering.h"
#include "DepthRendering.h"
#include "DrawingPolicy.h"
namespace Air
{
	class HitProxyConsumer;

	class DeferredShadingSceneRenderer : public SceneRenderer
	{
	public:
		DeferredShadingSceneRenderer(const SceneViewFamily* inViewFamily, HitProxyConsumer* hitProxyConsumer);

		virtual void render(RHICommandListImmediate& RHICmdList) override;


	public:

		EDepthDrawingMode	mEarlyZPassMode;
		bool bEarlyZPassMovable;
	private:
		bool initViews(RHICommandListImmediate& RHICmdList, struct FILCUpdatePrimTaskData& ILCTaskData, GraphEventArray& sortEvents);

		bool renderPrePass(RHICommandListImmediate& RHICmdList, std::function<void()> afterTaskAreStarted);

		bool renderBasePass(RHICommandListImmediate& RHICmdList);

		bool renderBasePassView(RHICommandListImmediate& RHICmdList, ViewInfo& view);

		bool renderBasePassStaticData(RHICommandList& RHICmdList, ViewInfo& view, const DrawingPolicyRenderState& drawRenderState);

		void renderBasePassDynamicData(RHICommandList& RHICmdList, ViewInfo& view, const DrawingPolicyRenderState& drawRenderState, bool &isDrity);

		bool renderBasePassStaticDataType(RHICommandList& RHICmdList, ViewInfo& view, const DrawingPolicyRenderState& drawRenderState, const EBasePassDrawListType drawType);

		void renderLights(RHICommandListImmediate& RHICmdList);

		void renderDynamicSkyLighting(RHICommandListImmediate& RHICmdList, const TRefCountPtr<IPooledRenderTarget>& velocityTexture, TRefCountPtr<IPooledRenderTarget>& dynamicBentNormalAO);

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

		void renderLight(RHICommandList& RHICmdList, const LightSceneInfo* lightSceneInfo, bool bRenderOverlap, bool bIssueDrawEvent);

		bool renderLightFunction(RHICommandListImmediate& RHICmdList, const LightSceneInfo* lightSceneInfo, bool bLightAttenuationCleared, bool bProjectingForForwardShading);
		
		void copySceneCaptureComponentToTarget(RHICommandListImmediate& RHICmdList);

		
	};
}