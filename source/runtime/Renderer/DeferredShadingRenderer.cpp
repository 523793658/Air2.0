#include "DeferredShadingRenderer.h"
#include "HitProxies.h"
#include "PostProcess/SceneRenderTargets.h"
#include "PostProcess/RenderTargetPool.h"
#include "SystemTextures.h"
#include "SceneViewExtension.h"
#include "Template/AirTemplate.h"
#include "RenderUtils.h"
#include "CoreGlobals.h"
#include "PostProcess/PostProcessing.h"
#include "ScenePrivate.h"
#include "Misc/App.h"
#include "CompositionLighting/CompositionLighting.h"
#include "RayTracing/RaytracingOptions.h"
#include "EngineModule.h"
namespace Air
{
	int32 GbEnableAsyncComputeTranslucencyLightingVolumeClear = 1;


	bool canOverlayRayTracingOutput(const ViewInfo& view)
	{
		return (view.mRayTracingRenderMode != ERayTracingRenderMode::Pathtracing) && (view.mRayTracingRenderMode != ERayTracingRenderMode::RayTracingdebug);
	}

	static FORCEINLINE bool needsPrePass(const DeferredShadingSceneRenderer* renderer)
	{
		return (RHIHasTiledGPU(renderer->mViewFamily.getShaderPlatform()) == false) && (renderer->mEarlyZPassMode != DDM_None || renderer->bEarlyZPassMovable != 0);
	}

	float getSceneColorClearAlpha()
	{
		return 1.0f;
	}



	void serviceLocalQueue()
	{
		TaskGraphInterface::get().processThreadUntilIdle(ENamedThreads::getRenderThread_Local());
	}


	GlobalDynamicIndexBuffer DeferredShadingSceneRenderer::mDynamicIndexBufferForInitViews;
	GlobalDynamicIndexBuffer DeferredShadingSceneRenderer::mDynamicIndexBufferForInitShadows;
	GlobalDynamicVertexBuffer DeferredShadingSceneRenderer::mDynamicVertexBufferForInitViews;
	GlobalDynamicVertexBuffer DeferredShadingSceneRenderer::mDynamicVertexBufferForInitShadows;
	TGlobalResource<GlobalDynamicReadBuffer> DeferredShadingSceneRenderer::mDynamicReadBufferForInitViews;
	TGlobalResource<GlobalDynamicReadBuffer> DeferredShadingSceneRenderer::mDynamicReadBufferForInitShadows;

	DeferredShadingSceneRenderer::DeferredShadingSceneRenderer(const SceneViewFamily* inViewFamily, HitProxyConsumer* hitProxyConsumer)
		:SceneRenderer(inViewFamily, hitProxyConsumer),
		mEarlyZPassMode(DDM_None),
		bEarlyZPassMovable(false)
	{

	}

	void DeferredShadingSceneRenderer::render(RHICommandListImmediate& RHICmdList)
	{
		BOOST_ASSERT(RHICmdList.isOutsideRenderPass());

		prepareViewRectsForRendering();

		if (mScene->mSunLight && mScene->hasAtmosphericFog())
		{
			
		}


		SceneRenderTargets& sceneContext = SceneRenderTargets::get(RHICmdList);
		GRenderTargetPool.transitionTargetsWritable(RHICmdList);

		sceneContext.releaseSceneColor();

		const bool bDBuffer = false;

		if (!mViewFamily.mEngineShowFlags.Rendering)
		{
			return;
		}

		{
			GSystemTextures.initializeTextures(RHICmdList, mFeatureLevel);
			sceneContext.allocate(RHICmdList, this);
		}

		const bool bUseVirtualTexturing = false;

		const bool bIsWireframe = mViewFamily.mEngineShowFlags.Wireframe;

		const bool bAllowReadonlyDepthBasePass = mEarlyZPassMode == DDM_ALLOpaque && !mViewFamily.mEngineShowFlags.ShaderComplexity && !mViewFamily.useDebugViewPS() && !bIsWireframe;

		const FExclusiveDepthStencil::Type basePassDepthStencilAccess = bAllowReadonlyDepthBasePass ? FExclusiveDepthStencil::DepthRead_StencilWrite : FExclusiveDepthStencil::DepthWrite_StencilWrite;

		GraphEventArray updateViewCustomDataEvents;

		FILCUpdatePrimTaskData ILCTaskData;
		RHICmdList.immediateFlush(EImmediateFlushType::DispatchToRHIThread);

		bool bDoInitViewAftersPrepass = false;
		{
			bDoInitViewAftersPrepass = initViews(RHICmdList, basePassDepthStencilAccess, ILCTaskData, updateViewCustomDataEvents);
		}

		updateGPUScene(RHICmdList, *mScene);
	
		if (bUseVirtualTexturing)
		{

		}
		for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
		{
			uploadDynamicPrimitiveShaderDataForView(RHICmdList, *mScene, mViews[viewIndex]);
		}

		if (!bDoInitViewAftersPrepass)
		{

		}

		//if(!gdoprepare)
	
		bool bRequiresRHIClear = true;
		bool bRequiresFarZQuadClear = false;

		const bool bUseGBuffer = isUsingGBuffers(mShaderPlatform);
		bool bCanOverlayRayTracingOutput = canOverlayRayTracingOutput(mViews[0]);

		const bool bRenderDeferredLighting = mViewFamily.mEngineShowFlags.Lighting
			&& mFeatureLevel >= ERHIFeatureLevel::SM4
			&& mViewFamily.mEngineShowFlags.DeferredLighting
			&& bUseGBuffer
			&& bCanOverlayRayTracingOutput;

		bool bComputeLightGrid = false;

		if (!isSimpleForwardShadingEnable(mShaderPlatform))
		{
			if (bUseGBuffer)
			{
				bComputeLightGrid = bRenderDeferredLighting;
			}
			else
			{
				bComputeLightGrid = mViewFamily.mEngineShowFlags.Lighting;
			}
		}

		BOOST_ASSERT(mViews.size());

		const bool bIsOcclusionTesting = false;

		//GEngine->mgetpre
		mDynamicIndexBufferForInitViews.commit();
		mDynamicVertexBufferForInitViews.commit();
		mDynamicReadBufferForInitViews.commit();

		if (!bDoInitViewAftersPrepass)
		{
			mDynamicVertexBufferForInitShadows.commit();
			mDynamicIndexBufferForInitShadows.commit();
			mDynamicReadBufferForInitShadows.commit();
		}

		bool bDidAfterTaskWork = false;
		auto afterTasksAreStarted = [&bDidAfterTaskWork, bDoInitViewAftersPrepass, this, &RHICmdList, &ILCTaskData, &updateViewCustomDataEvents]()
		{
			if (!bDidAfterTaskWork)
			{
				bDidAfterTaskWork = true;
				if (bDoInitViewAftersPrepass)
				{
					initViewsPossiblyAfterPrepass(RHICmdList, ILCTaskData, updateViewCustomDataEvents);
					{
						mDynamicVertexBufferForInitShadows.commit();
						mDynamicIndexBufferForInitShadows.commit();
						mDynamicReadBufferForInitShadows.commit();
					}

					serviceLocalQueue();
				}
			}
		};

		BOOST_ASSERT(RHICmdList.isOutsideRenderPass());

		GRenderTargetPool.addPhaseEvent(TEXT("EarlyZPass"));
		const bool bNeedsPrePass = needsPrePass(this);
		bool bDepthWasCleared;
		if (bNeedsPrePass)
		{
			bDepthWasCleared = renderPrePass(RHICmdList, afterTasksAreStarted);
		}
		else
		{
			afterTasksAreStarted();

			bDepthWasCleared = renderPrePassHMD(RHICmdList);
		}

		BOOST_ASSERT(bDidAfterTaskWork);
		serviceLocalQueue();

#if RHI_RAYTRACING
		
#endif

		BOOST_ASSERT(RHICmdList.isOutsideRenderPass());

		const bool bShouldRenderVelocities = false;
		const bool bBasePassCanOutputVelocity = false;
		const bool bUseSelectiveBasePassOutputs = isUsingSelectiveBasePassOutputs(mShaderPlatform);

		sceneContext.resolveSceneDepthTexture(RHICmdList, ResolveRect(0, 0, mFamilySize.x, mFamilySize.y));
		sceneContext.resolveSceneDepthToAuxiliaryTexture(RHICmdList);

		SortedLightSetSceneInfo sortedLightSet;

		{
			gatherAndSortLights(sortedLightSet);
			computeLightGrid(RHICmdList, bComputeLightGrid, sortedLightSet);
		}

		{
			sceneContext.preallocGBufferTargets();
			sceneContext.allocGBufferTargets(RHICmdList);
		}

		const bool bOcclustionBeforeBasePass = (mEarlyZPassMode == EDepthDrawingMode::DDM_AllOccluders) || (mEarlyZPassMode == EDepthDrawingMode::DDM_ALLOpaque);

		if (bOcclustionBeforeBasePass)
		{
			if (bIsOcclusionTesting)
			{

			}

			bool bUseHzbOcclusion = false;
			if (bUseHzbOcclusion || bIsOcclusionTesting)
			{

			}

			if (bIsOcclusionTesting)
			{

			}
		}

		serviceLocalQueue();

		BOOST_ASSERT(RHICmdList.isOutsideRenderPass());

		if (bOcclustionBeforeBasePass)
		{
			if (bDoInitViewAftersPrepass && updateViewCustomDataEvents.size() > 0)
			{
				TaskGraphInterface::get().waitUntilTaskComplete(updateViewCustomDataEvents, ENamedThreads::getRenderThread());
			}

			renderShadowDepthMaps(RHICmdList);
			serviceLocalQueue();
		}

		BOOST_ASSERT(RHICmdList.isOutsideRenderPass());

		if (bOcclustionBeforeBasePass)
		{
			
		}

		TRefCountPtr<IPooledRenderTarget> forwardScreenSpaceShadowMask;

		BOOST_ASSERT(RHICmdList.isOutsideRenderPass());

		if (bRenderDeferredLighting)
		{
			bool bShouldAllocateDeferredShadingPathRenderTargets = false;
			const char* str = sceneContext.mScreenSpaceAO ? "Allocated" : "Unallocated";
			for (int32 index = 0; index < (NumTranslucentVolumeRenderTargetSets * mViews.size()); ++index)
			{
				if (!sceneContext.mTranslucencyLightingVolumeAmbient[index] || !sceneContext.mTranslucencyLightingVolumeDirectional[index])
				{
					BOOST_ASSERT(sceneContext.mTranslucencyLightingVolumeAmbient[index]);
					BOOST_ASSERT(sceneContext.mTranslucencyLightingVolumeDirectional[index]);
					bShouldAllocateDeferredShadingPathRenderTargets = true;
					break;
				}
			}

			if (bShouldAllocateDeferredShadingPathRenderTargets)
			{
				sceneContext.allocateDeferredShadingPathRenderTarget(RHICmdList);
			}

			if (GbEnableAsyncComputeTranslucencyLightingVolumeClear && GSupportsEfficientAsyncCompute)
			{
				clearTranslucentVolumeLightingAsyncCompute(RHICmdList);
			}
		}

		BOOST_ASSERT(RHICmdList.isOutsideRenderPass());

		bool bIsWireframeRenderpass = bIsWireframe;
		bool bDoParallelBasePass = GRHICommandList.useParallelAlgorithms();
		bool bRenderLightmapDensity = false;
		bool bIsGBufferCurrent = false;

		if (bRequiresRHIClear)
		{
			bool bClearDepth = !bDepthWasCleared;
			ERenderTargetLoadAction colorLoadAction = ERenderTargetLoadAction::ELoad;
			ERenderTargetLoadAction depthLoadAction = bClearDepth ? ERenderTargetLoadAction::EClear : (!isMetalPlatform(mShaderPlatform) ? ERenderTargetLoadAction::ENoAction : ERenderTargetLoadAction::ELoad);

			const bool bClearBlack = mViewFamily.mEngineShowFlags.ShaderComplexity || mViewFamily.mEngineShowFlags.StationaryLightOverlap;
			const float clearAlpha = getSceneColorClearAlpha();
			LinearColor clearColor = bClearBlack ? LinearColor(0, 0, 0, clearAlpha) : LinearColor(mViews[0].mBackgroundColor.R, mViews[0].mBackgroundColor.G, mViews[0].mBackgroundColor.B, clearAlpha);
			colorLoadAction = ERenderTargetLoadAction::EClear;

			sceneContext.beginRenderingGBuffer(RHICmdList, colorLoadAction, depthLoadAction, basePassDepthStencilAccess, mViewFamily.mEngineShowFlags.ShaderComplexity, true, clearColor);

			if (bIsWireframeRenderpass)
			{
				RHICmdList.endRenderPass();
			}
			else
			{
				bIsGBufferCurrent = true;
			}
			serviceLocalQueue();
		}

		if (bIsWireframeRenderpass)
		{
			BOOST_ASSERT(bRequiresRHIClear);
		}
		else if (!bIsGBufferCurrent)
		{

		}

		if (isForwardShadingEnabled(mShaderPlatform))
		{

		}

		GRenderTargetPool.addPhaseEvent(TEXT("BasePass"));

		renderBasePass(RHICmdList, basePassDepthStencilAccess, forwardScreenSpaceShadowMask.getReference(), bDoParallelBasePass, bRenderLightmapDensity);

		if (isForwardShadingEnabled(mShaderPlatform))
		{
			forwardScreenSpaceShadowMask.safeRelease();
		}

		serviceLocalQueue();

		if (bDoParallelBasePass && !bRenderLightmapDensity)
		{
			sceneContext.beginRenderingGBuffer(RHICmdList, ERenderTargetLoadAction::ELoad, ERenderTargetLoadAction::ELoad, basePassDepthStencilAccess, mViewFamily.mEngineShowFlags.ShaderComplexity);
		}

		{
			for (int32 viewExt = 0; viewExt < mViewFamily.mViewExtensions.size(); ++viewExt)
			{
				for (int32 viewIndex = 0; viewIndex < mViewFamily.mViews.size(); ++viewIndex)
				{
					mViewFamily.mViewExtensions[viewExt]->postRenderBasePass_RenderThread(RHICmdList, mViews[viewIndex]);
				}
			}
		}

		{
			sceneContext.finishGBufferPassAndResolve(RHICmdList);
		}

		if (!bAllowReadonlyDepthBasePass)
		{
			sceneContext.resolveSceneDepthTexture(RHICmdList, ResolveRect(0, 0, mFamilySize.x, mFamilySize.y));
			sceneContext.resolveSceneDepthToAuxiliaryTexture(RHICmdList);
		}

		if (mViewFamily.mEngineShowFlags.VisualizeLightCulling)
		{

		}

		BOOST_ASSERT(RHICmdList.isOutsideRenderPass());

		sceneContext.resolveSceneDepthToAuxiliaryTexture(RHICmdList);

		serviceLocalQueue();

		BOOST_ASSERT(RHICmdList.isOutsideRenderPass());

		if (!bUseGBuffer)
		{
			resolveSceneColor(RHICmdList);
		}

		if (!bOcclustionBeforeBasePass)
		{
			if (bDoInitViewAftersPrepass && updateViewCustomDataEvents.size() > 0)
			{
				TaskGraphInterface::get().waitUntilTaskComplete(updateViewCustomDataEvents, ENamedThreads::getRenderThread());
			}

			renderShadowDepthMaps(RHICmdList);
			BOOST_ASSERT(RHICmdList.isOutsideRenderPass());

			serviceLocalQueue();
		}

		BOOST_ASSERT(RHICmdList.isOutsideRenderPass());

		if (!isForwardShadingEnabled(mShaderPlatform))
		{
			GCompositionLighting.gfxWaitForAsyncSSAO(RHICmdList);
		}
		else
		{
			sceneContext.mScreenSpaceAO.safeRelease();
			sceneContext.bScreenSpaceAOIsValid = false;

			for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
			{
				ViewInfo& view = mViews[viewIndex];
				view.mHZB.safeRelease();
			}
		}

		BOOST_ASSERT(RHICmdList.isOutsideRenderPass());

		if (mFeatureLevel >= ERHIFeatureLevel::SM4)
		{
			GRenderTargetPool.addPhaseEvent(TEXT("AfterBasePass"));
			if (!isForwardShadingEnabled(mShaderPlatform))
			{
				sceneContext.resolveSceneDepthTexture(RHICmdList, ResolveRect(0, 0, mFamilySize.x, mFamilySize.y));
				sceneContext.resolveSceneDepthToAuxiliaryTexture(RHICmdList);
			}

			for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
			{
				mScene->mConstantBuffers.updateViewConstantBuffer(mViews[viewIndex]);
				GCompositionLighting.processAfterBasePass(RHICmdList, mViews[viewIndex]);
			}

			serviceLocalQueue();
		}

		if (!isForwardShadingEnabled(mShaderPlatform))
		{
			RHIRenderPassInfo RPInfo(sceneContext.getSceneDepthSurface(), EDepthStencilTargetActions::ClearStencilDontLoadDepth_StoreStencilNotDepth);
			RPInfo.mDepthStencilRenderTarget.mExculusiveDepthStencil = FExclusiveDepthStencil::DepthNop_StencilWrite;
			RHICmdList.beginRenderPass(RPInfo, TEXT("ClearStencilFromBasePass"));
			RHICmdList.endRenderPass();

			RHICmdList.transitionResource(EResourceTransitionAccess::EReadable, sceneContext.getSceneDepthSurface());
		}
		BOOST_ASSERT(RHICmdList.isOutsideRenderPass());

		if (bRenderDeferredLighting)
		{
			GRenderTargetPool.addPhaseEvent(TEXT("Lighting"));
			renderDiffuseIndirectAndAmbientOcclusion(RHICmdList);

			renderIndirectCapsuleShadows(RHICmdList, sceneContext.getSceneColorSurface(), sceneContext.bScreenSpaceAOIsValid ? sceneContext.mScreenSpaceAO->getRenderTargetItem().mTargetableTexture : nullptr);
			
			TRefCountPtr<IPooledRenderTarget> dynamicBentNormalAO;

			if ((GbEnableAsyncComputeTranslucencyLightingVolumeClear && GSupportsEfficientAsyncCompute) == false)
			{
				for (int32 viewIndex = 0; viewIndex < mViews.size(); ++viewIndex)
				{
					clearTranslucentVolumeLighting(RHICmdList, viewIndex);
				}
			}

			renderLights(RHICmdList, sortedLightSet);
			serviceLocalQueue();

			BOOST_ASSERT(RHICmdList.isOutsideRenderPass());

			GRenderTargetPool.addPhaseEvent(TEXT("AfterRenderLights"));

			for(int32 viewIndex = 0; viewIndex < mViews.size(); ++viewIndex)
			{
				injectAmbientCubemapTranslucentVolumeLighting(RHICmdList, mViews[viewIndex], viewIndex);
			}

			serviceLocalQueue();

			for (int32 viewIndex = 0; viewIndex < mViews.size(); ++viewIndex)
			{
				filterTranslucentVolumeLighting(RHICmdList, mViews[viewIndex], viewIndex);
			}
			serviceLocalQueue();

		}

		IRendererModule& rendererModule = getRendererModule();

		resolveSceneColor(RHICmdList);

		copySceneCaptureComponentToTarget(RHICmdList);

		if (mViewFamily.bResolveScene)
		{
			for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
			{
				if (mViewFamily.useDebugViewPS())
				{

				}
				else
				{
					GPostProcessing.process(RHICmdList, mViews[viewIndex], sceneContext.mSceneVelocity);
				}
			}
		}
		else
		{
			sceneContext.adjustGBufferRefCount(RHICmdList, -1);
		}

		for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
		{
			
		}

		sceneContext.mSceneVelocity.safeRelease();

		{
			renderFinish(RHICmdList);
		}
		serviceLocalQueue();
	}

	bool DeferredShadingSceneRenderer::renderPrePass(RHICommandListImmediate& RHICmdList, std::function<void()> afterTaskAreStarted)
	{
		return true;
	}
	

	void DeferredShadingSceneRenderer::asyncSortBasePassStaticData(const float3 InViewPosition, GraphEventArray &outSortEvents)
	{

	}
	void DeferredShadingSceneRenderer::initViewsPossiblyAfterPrepass(RHICommandListImmediate& RHICmdList, struct FILCUpdatePrimTaskData& ILCTaskData, GraphEventArray& sortEvents)
	{

	}
	void DeferredShadingSceneRenderer::sortBasePassStaticData(float3 viewPosition)
	{

	}

	void DeferredShadingSceneRenderer::renderFinish(RHICommandListImmediate& RHICmdList)
	{
		SceneRenderer::renderFinish(RHICmdList);

		SceneRenderTargets::get(RHICmdList).setLightAttenuation(0);
	}

	
}