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
#include "MeshDrawCommands.h"
#include "RHIUtilities.h"
#include "PrimitiveConstantShaderParameters.h"
namespace Air
{
	class Scene;
	struct FILCUpdatePrimTaskData;
	class DynamicPrimitiveResource;
	class PrimitiveSceneInfo;
	class SceneRenderTargets;
	class SceneViewState;

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
		TArray<PrimitiveConstantShaderParameters> mDynamicPrimitiveShaderData;
		SceneBitArray mPrimitiveVisibilityMap;
		SceneBitArray mStaticMeshVisibilityMap;
		SceneBitArray mStaticMeshOccluderMap;
		RWBufferStructured mOneFramePrimitiveShaderDataBuffer;
		TRefCountPtr<IPooledRenderTarget> mHZB;

		TArray<uint64> mStaticMeshBatchVisibility;
		TArray<PrimitiveViewRelevance, SceneRenderingAllocator> mPrimitiveViewRelevanceMap;
		TArray<VisibleLightViewInfo, SceneRenderingAllocator> mVisibleLightInfos;
		
		TShaderMap<GlobalShaderType>* mShaderMap;

		TStaticArray<ParallelMeshDrawCommandPass, EMeshPass::Num> mParallelMeshDrawCommandPasses;

		bool bAllowStencilDither;


		int32 mNumVisibleStaticMeshElements;

		uint32 bUseLightingChannels : 1;
		uint32 bTranslucentSurfaceLighting : 1;
		EStereoscopicPass mStereoPass;

		SceneViewState* mViewState;

#if RHI_RAYTRACING
		void setupRayTracedRendering();

		ERayTracingRenderMode mRayTracingRenderMode = ERayTracingRenderMode::Disabled;


#endif

	public:
		bool bIsInstancedStereoEnabled{ false };
		bool bIsMobileMultiViewEnabled{ false };
	};

	class HitProxyConsumer;

	class ShadowMapRenderTargetsRefCounted
	{
	public:
		TArray<TRefCountPtr<IPooledRenderTarget>, SceneRenderingAllocator> mColorTargets;
		TRefCountPtr<IPooledRenderTarget> mDepthTarget;

		bool isValid() const
		{
			if (mDepthTarget)
			{
				return true;
			}
			else
			{
				return mColorTargets.size() > 0;
			}
		}

		int2 getSize() const
		{
			const PooledRenderTargetDesc* desc = nullptr;
			if (mDepthTarget)
			{
				desc = &mDepthTarget->getDesc();
			}
			else
			{
				BOOST_ASSERT(mColorTargets.size() > 0);
				desc = &mColorTargets[0]->getDesc();
			}

			return desc->mExtent;
		}

		int64 computeMemorySize() const
		{
			int64 memorySize = 0;
			for (uint32 i = 0; i < mColorTargets.size(); i++)
			{
				memorySize += mColorTargets[i]->computeMemorySize();
			}

			if (mDepthTarget)
			{
				memorySize += mDepthTarget->computeMemorySize();
			}

			return memorySize;
		}

		void release()
		{
			for (int32 i = 0; i < mColorTargets.size(); ++i)
			{
				mColorTargets[i] = nullptr;
			}
			mColorTargets.empty();

			mDepthTarget = nullptr;
		}

	};
	class ProjectedShadowInfo;
	struct SortedShadowMapAtlas
	{
		ShadowMapRenderTargetsRefCounted mRenderTargets;
		TArray<ProjectedShadowInfo*, SceneRenderingAllocator> mShadows;
	};

	class SceneRenderer
	{
	public:
		SceneRenderer(const SceneViewFamily* inViewFamily, HitProxyConsumer* hitProxyConsumer);

		virtual ~SceneRenderer();

		static SceneRenderer* createSceneRenderer(const SceneViewFamily* inViewFamily);

		static void waitForTasksClearSnapshotsAndDeleteSceneRenderer(RHICommandListImmediate& RHICmdList, SceneRenderer* sceneRenderer);

		virtual void render(RHICommandListImmediate& RHICmdList) = 0;

		void onStartFrame(RHICommandListImmediate& RHICmdList);

		static int2 applyResolutionFraction(const SceneViewFamily& viewFamily, const int2& unscaledViewSize, float resolutionFraction);

		static int2 quantizeViewRectMin(const int2& viewRectMin);
	protected:
		void preVisibilityFrameSetup(RHICommandListImmediate& RHICmdList);

		void computeViewVisibility(RHICommandListImmediate& RHICmdList);

		void postVisibilityFrameSetup(FILCUpdatePrimTaskData& outILCTaskData);

		bool checkForProjectedShadows(const LightSceneInfo* lightSceneInfo) const;

		void clearPrimitiveSingleFramePrecomputedLightingBuffers();

		void resolveSceneColor(RHICommandList& RHICmdList);

		void prepareViewRectsForRendering();

		void renderShadowDepthMaps(RHICommandListImmediate& RHICmdList);

		void renderShadowDepthMapAtlases(RHICommandListImmediate& RHICmdList);

		virtual void renderFinish(RHICommandListImmediate& RHICmdList);
	private:
		void computeFamilySize();
	public:
		int2 mFamilySize;

		SceneViewFamily mViewFamily;

		TArray<ViewInfo> mViews;

		Scene* mScene;
		bool bHasRequestedToggleFreeze;

		bool bUsedPrecomputedVisibility{ false };

		ERHIFeatureLevel::Type mFeatureLevel;
		EShaderPlatform mShaderPlatform;

		TArray<VisibleLightViewInfo, SceneRenderingAllocator> mVisibleLightInfos;
	};

	typedef TArray<uint8, SceneRenderingAllocator> PrimitiveViewMasks;

	class ParallelCommandListSet
	{
	public:
		const ViewInfo& mView;
		const SceneRenderer* mSceneRenderer;
		MeshPassProcessorRenderState mDrawRenderState;
		RHICommandListImmediate& mParentCmdList;
		SceneRenderTargets* mSnapshot;
		int32 mWidth;
		int32 mNumAlloc;
		int32 mMinDrawPerCommandList;

		bool bBalanceCommands;
		bool bSpewBalance;

	public:
		TArray<RHICommandList*, SceneRenderingAllocator> mCommandLists;
		TArray<GraphEventRef, SceneRenderingAllocator> mEvents;

		TArray<int32, SceneRenderingAllocator> mNumDrawsIfKnown;

	protected:
		void dispatch(bool bHighPriority = false);
		RHICommandList* allocCommandList();

		bool bParallelExecute;
		bool bCreateSeneContext;
	public:
		ParallelCommandListSet(
			const ViewInfo& inView,
			const SceneRenderer* inSceneRenderer,
			RHICommandListImmediate& inParentCmdList,
			bool bInParalletExecute,
			bool bInCreateSceneContext,
			const MeshPassProcessorRenderState& inDrawRenderState
		);

		virtual ~ParallelCommandListSet();

		int32 numParallelCommandLists() const
		{
			return mCommandLists.size();
		}

		RHICommandList* newParallelCommandList();

		FORCEINLINE GraphEventArray* getPrereqs()
		{
			return nullptr;
		}

		void addParallelCommandList(RHICommandList* cmdList, GraphEventRef& completionEvent, int32 inNuDrawsIfKnown = -1);

		virtual void setStateOnCommandList(RHICommandList& cmdList)
		{}

		static void waitForTasks();

	private:
		void waitForTasksInternal();
	};

#define FORWARD_GLOBAL_LIGHT_DATA_CONSTANT_BUFFER_MEMBER_TABLE \
	SHADER_PARAMETER(uint32, NumLocalLights)\
	SHADER_PARAMETER_SRV(StrongTypedBuffer<float4>, ForwardLocalLightBuffer) \
	SHADER_PARAMETER_SRV(StrongTypedBuffer<uint>, NumCulledLightsGrid) \
	SHADER_PARAMETER_SRV(StrongTypedBuffer<uint>, CulledLightDataGrid)
	

	BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT_WITH_CONSTRUCTOR(ForwardLightData,)
		FORWARD_GLOBAL_LIGHT_DATA_CONSTANT_BUFFER_MEMBER_TABLE
	END_GLOBAL_SHADER_PARAMETER_STRUCT()


	class ForwardLightingViewResources
	{
	public:
		ForwardLightData mForwardLightData;
		TConstantBufferRef<ForwardLightData> mForwardLightDataConstantBuffer;
		DynamicReadBuffer mForwardLocalLightBuffer;
		RWBuffer mNumCulledLightsGrid;
		RWBuffer mCulledLightDataGrid;

		void release()
		{
			mForwardLightDataConstantBuffer.safeRelease();
			mForwardLocalLightBuffer.release();
			mNumCulledLightsGrid.release();
			mCulledLightDataGrid.release();
		}
	};
}