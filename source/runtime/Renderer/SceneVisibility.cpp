#include "DeferredShadingRenderer.h"
#include "Misc/App.h"
#include "scene.h"
#include "Async/ParallelFor.h"
#include "ScenePrivate.h"
#include "SceneView.h"
#include "PrimitiveSceneInfo.h"
#include "PrimitiveViewRelevance.h"
namespace Air
{
	static int32 GDoInitViewsLightingAfterPrepass = 0;

	float GMinScreenRadiusForLights = 0.03f;

	namespace EMarkMaskBits
	{
		enum Type
		{
			StaticMeshShadowDepthMapMask = 0x1,
			StaticMeshVisibilityMapMask = 0x2,
			StaticMeshVelocityMapMask = 0x4,
			StaticMeshOccluderMapMask = 0x8,
			staticMeshFadeOutDitheredLODMapMask = 0x10,
			staticMeshFadeInDitheredLODMapMask = 0x20,
			StaticMeshEditorSelectedMask = 0x40,
		};
	}

	struct MarkRelevantStaticMeshesForViewData
	{
		float3 mViewOrigin;

		MarkRelevantStaticMeshesForViewData(ViewInfo& view)
		{
			mViewOrigin = view.mViewMatrices.getViewOrigin();
		}
	};

	template<class T, int TAmplifyFactor = 1>
	struct RelevancePrimSet
	{
		enum
		{
			MaxInputPrims = 127,
			MaxOutputPrims = MaxInputPrims * TAmplifyFactor
		};
		int32 mNumPrims;
		T mPrims[MaxOutputPrims];
		FORCEINLINE RelevancePrimSet()
			:mNumPrims(0)
		{}
		FORCEINLINE void addPrim(T prim)
		{
			BOOST_ASSERT(mNumPrims < MaxOutputPrims);
			mPrims[mNumPrims++] = prim;
		}

		FORCEINLINE bool isFull() const
		{
			return mNumPrims >= MaxOutputPrims;
		}

		template<class TARRAY>
		FORCEINLINE void appendTo(TARRAY& destArray)
		{
			destArray.append(mPrims, mNumPrims);
		}
	};

	struct RelevancePacket
	{
		const float mCurrentmWorldTime;
		const float mDeltaWorldTime;
		RHICommandListImmediate& mRHICmdList;
		const Scene* mScene;
		const ViewInfo& mView;
		const uint8 mViewBit;

		bool bUsesGlobalDistanceField{ false };
		bool bUsesLightingChannels{ false };
		bool bUsesSceneDepth{ false };

		RelevancePrimSet<int32> mInput;
		RelevancePrimSet<int32> mRelevantStaticPrimitives;
		RelevancePrimSet<int32> mNotDrawRelevant;
		RelevancePrimSet<PrimitiveSceneInfo*> mLazyUpdatePrimitives;

		uint16 mCombinedShadingModelMask;
		const MarkRelevantStaticMeshesForViewData& mViewData;
		PrimitiveViewMasks& mOutHasDynamicMeshElementsMasks;
		uint8* RESTRICT mMarkMasks;
		RelevancePacket(
			RHICommandListImmediate& inRHICmdList,
			const Scene* inScene,
			const ViewInfo& inView,
			uint8 inViewBit,
			const MarkRelevantStaticMeshesForViewData& inViewData,
			PrimitiveViewMasks& inOutHasDynamicMeshElementsMasks,
			uint8* inMarkMasks
		)
			:mCurrentmWorldTime(inView.mFamily->mCurrentWorldTime),
			mDeltaWorldTime(inView.mFamily->mDeltaWorldTime),
			mRHICmdList(inRHICmdList),
			mScene(inScene),
			mView(inView),
			mViewBit(inViewBit),
			mViewData(inViewData),
			mCombinedShadingModelMask(0),
			mOutHasDynamicMeshElementsMasks(inOutHasDynamicMeshElementsMasks),
			mMarkMasks(inMarkMasks)
		{

		}

		void anyThreadTask()
		{
			computeRelevance();
			markRelevant();
		}

		void markRelevant()
		{
			int32 numVisibleStaticMeshElements = 0;
			ViewInfo& writeView = const_cast<ViewInfo&>(mView);

			const SceneViewState* viewState = (SceneViewState*)mView.mState;
			for (int32 staticPrimIndex = 0, num = mRelevantStaticPrimitives.mNumPrims; staticPrimIndex < num; ++staticPrimIndex)
			{
				int32 primitiveIndex = mRelevantStaticPrimitives.mPrims[staticPrimIndex];
				const PrimitiveSceneInfo* RESTRICT primitiveSceneInfo = mScene->mPrimitives[primitiveIndex];
				const PrimitiveBounds& bounds = mScene->mPrimitiveBounds[primitiveIndex];
				const PrimitiveViewRelevance& viewRelevance = mView.mPrimitiveViewRelevanceMap[primitiveIndex];

				const int32 numStaticMeshes = primitiveSceneInfo->mStaticMeshes.size();
				for (int32 meshIndex = 0; meshIndex < numStaticMeshes; meshIndex++)
				{
					const StaticMesh& staticMesh = primitiveSceneInfo->mStaticMeshes[meshIndex];
					if (true)
					{
						uint8 markMask = 0;
						bool bNeedsBatchVisibility = false;
						bool bHiddenByHLODFade = false;
						if (viewRelevance.bDrawRelevance && (staticMesh.bUseForMaterial || staticMesh.bUseAsOccluder) && (viewRelevance.bRenderInMainPass) && true)
						{
							if (staticMesh.bUseForMaterial)
							{
								markMask |= EMarkMaskBits::StaticMeshVisibilityMapMask;
								++numVisibleStaticMeshElements;
							}
							bNeedsBatchVisibility = true;
						}
						if (markMask)
						{
							mMarkMasks[staticMesh.mId] = markMask;
						}
						if (bNeedsBatchVisibility && staticMesh.bRequiresPerElementVisibility)
						{
							writeView.mStaticMeshBatchVisibility[staticMesh.mId] = staticMesh.mVertexFactory->getStaticBatchElementVisibility(mView, &staticMesh);
						}
					}
				}
			}
			static_assert(sizeof(writeView.mNumVisibleStaticMeshElements) == sizeof(int32), "atomic is the wrong size");
			PlatformAtomics::interLockedAdd((volatile int32*)&writeView.mNumVisibleStaticMeshElements, numVisibleStaticMeshElements);
		}

		void renderThreadFinalize()
		{
			ViewInfo& writeView = const_cast<ViewInfo&>(mView);
			for (int32 index = 0; index < mNotDrawRelevant.mNumPrims; index++)
			{
				writeView.mPrimitiveVisibilityMap[mNotDrawRelevant.mPrims[index]] = false;
			}

			for (int32 index = 0; index < mLazyUpdatePrimitives.mNumPrims; index++)
			{
				mLazyUpdatePrimitives.mPrims[index]->conditionalLazyUpdateForRendering(mRHICmdList);
			}
			
		}

		void computeRelevance()
		{
			mCombinedShadingModelMask = 0;
			bUsesGlobalDistanceField = false;
			bUsesLightingChannels = false;
			for (int32 index = 0; index < mInput.mNumPrims; index++)
			{
				int32 bitIndex = mInput.mPrims[index];
				PrimitiveSceneInfo* primitiveSceneInfo = mScene->mPrimitives[bitIndex];
				PrimitiveViewRelevance& viewRelevance = const_cast<PrimitiveViewRelevance&>(mView.mPrimitiveViewRelevanceMap[bitIndex]);
				viewRelevance = primitiveSceneInfo->mProxy->getViewRelevance(&mView);
				viewRelevance.bInitializedThisFrame = true;
				const bool bStaticRelevance = viewRelevance.bStaticRelevance;
				const bool bDrawRelevance = viewRelevance.bDrawRelevance;
				const bool bDynamicRelevance = viewRelevance.bDynamicRelevance;
				const bool bTranslucentRelevance = viewRelevance.hasTranslucency();
				
				if (bStaticRelevance && (bDrawRelevance))
				{
					mRelevantStaticPrimitives.addPrim(bitIndex);
				}

				if (!bDrawRelevance)
				{
					mNotDrawRelevant.addPrim(bitIndex);
					continue;
				}

				if (bDynamicRelevance)
				{

				}
				if (primitiveSceneInfo->needsLazyUpdateForRendering())
				{
					mLazyUpdatePrimitives.addPrim(primitiveSceneInfo);
				}

				mCombinedShadingModelMask |= viewRelevance.mShadingModelMaskRelevance;
				bUsesGlobalDistanceField |= viewRelevance.bUsesGlobalDistanceField;

				if (primitiveSceneInfo->mLastRenderTime < mCurrentmWorldTime - mDeltaWorldTime - DELTA)
				{
					primitiveSceneInfo->mLastVisibilityChangeTime = mCurrentmWorldTime;

				}
				primitiveSceneInfo->mLastRenderTime = mCurrentmWorldTime;
			}
		}
	};

	static void computeAndMarkRelevanceForViewParallel(
		RHICommandListImmediate& RHICmdList,
		const Scene* scene,
		ViewInfo& view,
		uint8 viewBit,
		PrimitiveViewMasks& outHasDynamicMeshElementsMasks
	)
	{
		BOOST_ASSERT(outHasDynamicMeshElementsMasks.size() == scene->mPrimitives.size());
		const MarkRelevantStaticMeshesForViewData viewData(view);
		int32 numMesh = view.mStaticMeshVisibilityMap.size();
		BOOST_ASSERT(view.mStaticMeshOccluderMap.size() == numMesh);
		uint8* RESTRICT markMasks = (uint8*)MemStack::get().alloc(numMesh + 31, 8);
		Memory::memzero(markMasks, numMesh + 31);
		int32 estimateOfNumPackets = numMesh / (RelevancePrimSet<int32>::MaxInputPrims * 4);
		TArray<RelevancePacket*, SceneRenderingAllocator> packets;
		packets.reserve(estimateOfNumPackets);

		{
			SceneSetBitIterator bitIt(view.mPrimitiveVisibilityMap);
			if (bitIt)
			{
				RelevancePacket* packet = new(MemStack::get())RelevancePacket(
					RHICmdList,
					scene,
					view,
					viewBit,
					viewData,
					outHasDynamicMeshElementsMasks,
					markMasks
				);
				packets.add(packet);

				while (1)
				{
					packet->mInput.addPrim(bitIt.getIndex());
					++bitIt;
					if (packet->mInput.isFull() || !bitIt)
					{
						if (!bitIt)
						{
							break;
						}
						else
						{
							packet = new(MemStack::get())RelevancePacket(RHICmdList,
								scene,
								view,
								viewBit,
								viewData,
								outHasDynamicMeshElementsMasks,
								markMasks);
							packets.add(packet);
						}
					}
				}
			}
		}
		{
			parallelFor(packets.size(), [&packets](int32 index)
			{
				packets[index]->anyThreadTask();
			}, App::shouldUseThreadingForPerformance());
		}
		{
			for (auto packet : packets)
			{
				packet->renderThreadFinalize();
			}
		}
		BOOST_ASSERT(view.mStaticMeshOccluderMap.size() == numMesh &&
			view.mStaticMeshVisibilityMap.size() == numMesh);

		uint32* RESTRICT staticMeshVisibilityMap_Words = view.mStaticMeshVisibilityMap.getData();

		uint32* RESTRICT staticMeshOccluderMap_Words = view.mStaticMeshOccluderMap.getData();

		const uint64* RESTRICT markMask64 = (const uint64* RESTRICT) markMasks;
		const uint8* RESTRICT markMasks8 = markMasks;

		for (int32 baseIndex = 0; baseIndex < numMesh; baseIndex += 32)
		{
			uint32 staticMeshVisibilityMap_Word = 0;
			uint32 staticMeshOccluderMap_Word = 0;
			uint32 mask = 1;
			bool bAny = false;
			for (int32 qWordIndex = 0; qWordIndex < 4; qWordIndex++)
			{
				if (*markMask64++)
				{
					for (int32 byteIndex = 0; byteIndex < 8; byteIndex++, mask <<= 1, markMasks8++)
					{
						uint8 markMask = *markMasks8;
						staticMeshVisibilityMap_Word |= (markMask & EMarkMaskBits::StaticMeshVisibilityMapMask) ? mask : 0;
						staticMeshOccluderMap_Word |= (markMask & EMarkMaskBits::StaticMeshOccluderMapMask) ? mask : 0;

					}
					bAny = true;
				}
				else
				{
					markMasks8 += 8;
					mask <<= 8;
				}
			}
			if (bAny)
			{
				BOOST_ASSERT(!*staticMeshVisibilityMap_Words && !*staticMeshOccluderMap_Words);
				*staticMeshVisibilityMap_Words = staticMeshVisibilityMap_Word;
				*staticMeshOccluderMap_Words = staticMeshOccluderMap_Word;
			}
			staticMeshVisibilityMap_Words++;
			staticMeshOccluderMap_Words++;
		}
	}

	bool DeferredShadingSceneRenderer::initViews(RHICommandListImmediate& RHICmdList, struct FILCUpdatePrimTaskData& ILCTaskData, GraphEventArray& sortEvents)
	{
		for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
		{
			ViewInfo& view = mViews[viewIndex];
			const bool bWillApplyTemporalAA = false;
			if (!bWillApplyTemporalAA)
			{
				view.mAntiAliasingMethod = AAM_None;
			}
		}
		preVisibilityFrameSetup(RHICmdList);
		computeViewVisibility(RHICmdList);
		postVisibilityFrameSetup(ILCTaskData);

		float3 averageViewPosition(0);
		for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
		{
			ViewInfo& view = mViews[viewIndex];
			averageViewPosition += view.mViewMatrices.getViewOrigin() / mViews.size();
		}
		if (App::shouldUseThreadingForPerformance())
		{
			asyncSortBasePassStaticData(averageViewPosition, sortEvents);
		}
		else
		{
			sortBasePassStaticData(averageViewPosition);
		}
		bool bDoInitViewAftersPrepass = !!GDoInitViewsLightingAfterPrepass;
		if (!bDoInitViewAftersPrepass)
		{
			initViewsPossiblyAfterPrepass(RHICmdList, ILCTaskData, sortEvents);
		}
		{
			for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
			{
				ViewInfo& view = mViews[viewIndex];
				view.initRHIResources();
			}
		}
		{
			onStartFrame(RHICmdList);
		}
		return bDoInitViewAftersPrepass;
	}

	void SceneRenderer::computeViewVisibility(RHICommandListImmediate& RHICmdList)
	{
		if (mScene->mLights.getMaxIndex() > 0)
		{
			mVisibleLightInfos.addZeroed(mScene->mLights.getMaxIndex());
		}

		uint8 viewBit = 0x1;
		int32 numPrimitives = mScene->mPrimitives.size();
		PrimitiveViewMasks hasDynamicMeshElementsMasks;
		hasDynamicMeshElementsMasks.addZeroed(numPrimitives);
		PrimitiveViewMasks hasDynamicEditorMeshElementsMasks;
		for (int32 viewIndex = 0; viewIndex < mViews.size(); ++viewIndex, viewBit <<= 1)
		{
			ViewInfo& view = mViews[viewIndex];
			SceneViewState* viewState = (SceneViewState*)view.mState;
			view.mPrimitiveVisibilityMap.init(false, mScene->mPrimitives.size());
			view.mStaticMeshVisibilityMap.init(false, mScene->mStaticMeshes.getMaxIndex());
			view.mStaticMeshBatchVisibility.addZeroed(mScene->mStaticMeshes.getMaxIndex());
			view.mStaticMeshOccluderMap.init(false, mScene->mStaticMeshes.getMaxIndex());
			view.mVisibleLightInfos.empty(mScene->mLights.size());
			for (int32 lightIndex = 0; lightIndex < mScene->mLights.size(); lightIndex++)
			{
				new(view.mVisibleLightInfos)VisibleLightViewInfo();
			}
			view.mPrimitiveViewRelevanceMap.empty(mScene->mPrimitives.size());
			view.mPrimitiveViewRelevanceMap.addZeroed(mScene->mPrimitives.size());

			const bool bIsParent = viewState && viewState->isViewParent();
			if (bIsParent)
			{

			}
			for (SceneBitArray::Iterator bitIt(view.mPrimitiveVisibilityMap); bitIt; ++bitIt)
			{
				bitIt.getValue() = true;
			}
			{
				computeAndMarkRelevanceForViewParallel(RHICmdList, mScene, view, viewBit, hasDynamicMeshElementsMasks);
			}


		}
	}
	void SceneRenderer::postVisibilityFrameSetup(FILCUpdatePrimTaskData& outILCTaskData)
	{
		for (TSparseArray<LightSceneInfoCompact>::TConstIterator lightIt(mScene->mLights); lightIt; ++lightIt)
		{
			const LightSceneInfoCompact& lightSceneInfoCompact = *lightIt;
			const LightSceneInfo* lightSceneInfo = lightSceneInfoCompact.mLightSceneInfo;
			for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
			{
				const LightSceneProxy* proxy = lightSceneInfo->mProxy;
				ViewInfo& view = mViews[viewIndex];
				VisibleLightViewInfo& visibleLightViewInfo = view.mVisibleLightInfos[lightIt.getIndex()];
				if (proxy->getLightType() == LightType_Point || proxy->getLightType() == LightType_Spot)
				{

				}
				else
				{
					visibleLightViewInfo.bInViewFrustum = true;
				}
			}
		}
	}
	void SceneRenderer::preVisibilityFrameSetup(RHICommandListImmediate& RHICmdList)
	{

	}
}