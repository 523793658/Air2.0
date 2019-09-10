#pragma once
#include "RendererMininal.h"
#include "SceneView.h"
#include "RHICommandList.h"
#include "RHI.h"
#include "RendererInterface.h"
#include "ScenePrivateBase.h"
#include "PrimitiveViewRelevance.h"
#include "PostProcess/SceneRenderTargets.h"
#include "GlobalShader.h"
namespace Air
{
	class Scene;
	struct FILCUpdatePrimTaskData;
	class DynamicPrimitiveResource;
	class PrimitiveSceneInfo;
	class SceneRenderTargets;

	class VisibleLightViewInfo
	{
	public:
		TArray<PrimitiveSceneInfo*, SceneRenderingAllocator> mVisibleDynamicLitPrimitives;
		SceneBitArray mProjectShadowVisibilityMap;
		TArray<PrimitiveViewRelevance, SceneRenderingAllocator> mProjectShadowViewRelevanceMap;
		uint32 bInViewFrustum : 1;

		VisibleLightViewInfo()
			:bInViewFrustum(false)
		{}
	};

	class ViewInfo : public SceneView
	{
	public:
		explicit ViewInfo(const SceneView* inView);

		ViewInfo(ViewInfo&& inInfo);
			
		~ViewInfo();
		void init();

		void initRHIResources();

		void setupConstantBufferParameters(
			SceneRenderTargets& sceneContext,
			const ViewMatrices & inViewMatrices,
			const ViewMatrices& inPreViewMatrices,
			Box* outTranslucentCascadeBoundsArray,
			int32 numTranslucentCascades,
			ViewConstantShaderParameters& viewConstantShaderParameters) const;
		inline void setupConstantBufferParameters(SceneRenderTargets& sceneContext, Box* outTranslucentCascadeBoundsArray, int32 numTranslucentCascades, ViewConstantShaderParameters& viewConstantShaderParameters)
		{
			setupConstantBufferParameters(sceneContext, mViewMatrices, mPrevViewMatrices, outTranslucentCascadeBoundsArray, numTranslucentCascades, viewConstantShaderParameters);
		}

		void destroyAllSnapshots();

		bool shouldRenderView() const
		{
			if (!bIsInstancedStereoEnabled && !bIsMobileMultiViewEnabled)
			{
				return true;
			}
			else if (bIsInstancedStereoEnabled && mStereoPass != eSSP_RIGHT_EYE)
			{
				return true;
			}
			else if (bIsMobileMultiViewEnabled && mStereoPass != eSSP_RIGHT_EYE && mFamily && mFamily->mViews.size() > 1)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		ERHIFeatureLevel::Type getFeatureLevel() const { return mFeatureLevel; }

		void setupSkyIrradianceEnvironmentMapConstants(float4* outSkyIrradianceEnvironmentMap) const;
	public:
		ICustomVisibilityQuery* mCustomVisibilityQuery{ nullptr };
		std::unique_ptr<ViewConstantShaderParameters> mCachedViewConstantShaderParameters;

		TArray<DynamicPrimitiveResource*> mDynamicResources;
		TArray<MeshBatchAndRelevance> mDynamicMeshElements;

		SceneBitArray mPrimitiveVisibilityMap;
		SceneBitArray mStaticMeshVisibilityMap;
		SceneBitArray mStaticMeshOccluderMap;

		TArray<uint64> mStaticMeshBatchVisibility;
		TArray<PrimitiveViewRelevance, SceneRenderingAllocator> mPrimitiveViewRelevanceMap;
		TArray<VisibleLightViewInfo, SceneRenderingAllocator> mVisibleLightInfos;
		
		TShaderMap<GlobalShaderType>* mShaderMap;

		bool bAllowStencilDither;


		int32 mNumVisibleStaticMeshElements;

		uint32 bUseLightingChannels : 1;
		EStereoscopicPass mStereoPass;

	private:
		bool bIsInstancedStereoEnabled{ false };
		bool bIsMobileMultiViewEnabled{ false };
	};

	class HitProxyConsumer;

	class SceneRenderer
	{
	public:
		SceneRenderer(const SceneViewFamily* inViewFamily, HitProxyConsumer* hitProxyConsumer);

		virtual ~SceneRenderer();

		static SceneRenderer* createSceneRenderer(const SceneViewFamily* inViewFamily);

		static void waitForTasksClearSnapshotsAndDeleteSceneRenderer(RHICommandListImmediate& RHICmdList, SceneRenderer* sceneRenderer);

		virtual void render(RHICommandListImmediate& RHICmdList) = 0;

		void onStartFrame(RHICommandListImmediate& RHICmdList);


	protected:
		void preVisibilityFrameSetup(RHICommandListImmediate& RHICmdList);

		void computeViewVisibility(RHICommandListImmediate& RHICmdList);

		void postVisibilityFrameSetup(FILCUpdatePrimTaskData& outILCTaskData);

		bool checkForProjectedShadows(const LightSceneInfo* lightSceneInfo) const;

		void clearPrimitiveSingleFramePrecomputedLightingBuffers();
	public:
		SceneViewFamily mViewFamily;

		TArray<ViewInfo> mViews;

		Scene* mScene;
		bool bHasRequestedToggleFreeze;

		bool bUsedPrecomputedVisibility{ false };

		ERHIFeatureLevel::Type mFeatureLevel;

		TArray<VisibleLightViewInfo, SceneRenderingAllocator> mVisibleLightInfos;
	};

	typedef TArray<uint8, SceneRenderingAllocator> PrimitiveViewMasks;
}