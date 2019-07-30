#include "DeferredShadingRenderer.h"
#include "HitProxies.h"
#include "PostProcess/SceneRenderTargets.h"
#include "PostProcess/RenderTargetPool.h"
#include "SystemTextures.h"
#include "SceneViewExtension.h"
#include "Template/AirTemplate.h"
#include "StaticMeshDrawList.h"
#include "RenderUtils.h"
#include "CoreGlobals.h"
#include "PostProcess/PostProcessing.h"
#include "ScenePrivate.h"
#include "Misc/App.h"
#include "CompositionLighting/CompositionLighting.h"
namespace Air
{

	static FORCEINLINE bool needsPrePass(const DeferredShadingSceneRenderer* renderer)
	{
		return (RHIHasTiledGPU(renderer->mViewFamily.getShaderPlatform()) == false) && (renderer->mEarlyZPassMode != DDM_None || renderer->bEarlyZPassMovable != 0);
	}

	float getSceneColorClearAlpha()
	{
		return 1.0f;
	}

	static void setAndClearViewGBuffer(RHICommandListImmediate& RHICmdList, ViewInfo& view, bool bClearDepth)
	{
		ERenderTargetLoadAction depthLoadAction = bClearDepth ? ERenderTargetLoadAction::EClear : ERenderTargetLoadAction::ELoad;

		const bool bClearBlack = view.mFamily->mEngineShowFlags.ShaderComplexity || view.mFamily->mEngineShowFlags.StationaryLightOverlap;
		const float clearAlpha = getSceneColorClearAlpha();
		const LinearColor clearColor = bClearBlack ? LinearColor(0, 0, 0, clearAlpha) : LinearColor(view.mBackgroundColor.R, view.mBackgroundColor.G, view.mBackgroundColor.B, clearAlpha);

		SceneRenderTargets::get(RHICmdList).beginRenderingGBuffer(RHICmdList, ERenderTargetLoadAction::EClear, depthLoadAction, view.mFamily->mEngineShowFlags.ShaderComplexity, clearColor);
	}

	void serviceLocalQueue()
	{
		TaskGraphInterface::get().processThreadUntilIdle(ENamedThreads::RenderThread_Local);
	}

	DeferredShadingSceneRenderer::DeferredShadingSceneRenderer(const SceneViewFamily* inViewFamily, HitProxyConsumer* hitProxyConsumer)
		:SceneRenderer(inViewFamily, hitProxyConsumer),
		mEarlyZPassMode(DDM_None),
		bEarlyZPassMovable(false)
	{

	}

	void DeferredShadingSceneRenderer::render(RHICommandListImmediate& RHICmdList)
	{
		SceneRenderTargets& sceneContext = SceneRenderTargets::get(RHICmdList);

		GRenderTargetPool.transitionTargetsWritable(RHICmdList);

		sceneContext.releaseSceneColor();

		if (GRHIThread)
		{

		}

		if (!mViewFamily.mEngineShowFlags.Rendering)
		{
			return;
		}
		if (mScene->mPrimitives.size() == 0)
		{
			return;
		}
		{
			GSystemTextures.initializeTextures(RHICmdList, mFeatureLevel);

			sceneContext.allocate(RHICmdList, mViewFamily);
		}

		GraphEventArray sortEvents;
		FILCUpdatePrimTaskData ILCTaskData;

		bool bDoInitViewAftersPrepass = initViews(RHICmdList, ILCTaskData, sortEvents);

		for (size_t viewExt = 0; viewExt < mViewFamily.mViewExtensions.size(); ++viewExt)
		{
			mViewFamily.mViewExtensions[viewExt]->postInitViewFamily_RenderThread(RHICmdList, mViewFamily);
			for (size_t viewIndex = 0; viewIndex < mViewFamily.mViews.size(); ++viewIndex)
			{
				mViewFamily.mViewExtensions[viewExt]->postInitView_RenderThread(RHICmdList, mViews[viewIndex]);
			}
		}



		TGuardValue<bool> lockDrawLists(GDrawListsLocked, true);

		if (GRHIThread)
		{
			RHICommandListExecutor::getImmediateCommandList().immediateFlush(EImmediateFlushType::FlushRHIThreadFlushResources);
		}

		const bool bIsWireframe = false;// mViewFamily.mEngineShowFlags.WireFrame;

		bool bRequiresRHIClear = true;
		bool bRequiresFarZQuadClear = false;

		const bool bUseGBuffer = isUsingGBuffers(getFeatureLevelShaderPlatform(mFeatureLevel));

		bool bLateFXPrerender = false;

		bool bDoFXPrerender = false;

		bool bDidAfterTaskWork = false;
		auto afterTaskAreStarted = [&bDidAfterTaskWork, bDoInitViewAftersPrepass, this, &RHICmdList, &ILCTaskData, &sortEvents, bLateFXPrerender, bDoFXPrerender]()
		{
			if (!bDidAfterTaskWork)
			{

			}
		};
		
		GRenderTargetPool.addPhaseEvent(TEXT("EarlyZPass"));
		const bool bNeedsPrePass = needsPrePass(this);
		bool bDepthWasCleared;

		if (bUseGBuffer || isSimpleForwardShadingEnable(getFeatureLevelShaderPlatform(mFeatureLevel)))
		{
			sceneContext.preallocGBufferTargets();
			sceneContext.allocGBufferTargets(RHICmdList);
		}

		if (bNeedsPrePass)
		{
			bDepthWasCleared = renderPrePass(RHICmdList, afterTaskAreStarted);
		}
		else
		{
			afterTaskAreStarted();
			bDepthWasCleared = renderPrePassHMD(RHICmdList);
		}
		serviceLocalQueue();

		bool bIsGBufferCurrent = false;
		if (bRequiresRHIClear)
		{
			setAndClearViewGBuffer(RHICmdList, mViews[0], true);
		}
		{
			renderBasePass(RHICmdList);
			serviceLocalQueue();

		}

		const bool bUseVelocityGBuffer = false;

		TRefCountPtr<IPooledRenderTarget> velocityRT;
		if (bUseVelocityGBuffer)
		{

		}

		if (mFeatureLevel >= ERHIFeatureLevel::SM4)
		{
			GRenderTargetPool.addPhaseEvent(TEXT("AfterBasePass"));

			for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
			{
				GCompositionLighting.processAfterBasePass(RHICmdList, mViews[viewIndex]);
			}
			serviceLocalQueue();
		}

		{
			renderSky(RHICmdList);
		}
		{
			TRefCountPtr<IPooledRenderTarget> dynamicBentNormalAO;

			renderLights(RHICmdList);
			serviceLocalQueue();

			GRenderTargetPool.addPhaseEvent(TEXT("AfterRenderLights"));

			renderDynamicSkyLighting(RHICmdList, velocityRT, dynamicBentNormalAO);
		}
		

		//前面多个视图都渲染到了同一个renderTarget上的不同区域，后处理之前需要把各自的部分拷贝出来
		sceneContext.resolveSceneColor(RHICmdList, ResolveRect(0, 0, mViewFamily.mFamilySizeX, mViewFamily.mFamilySizeY));

		if (mViewFamily.bResolveScene)
		{
			for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
			{
				GPostProcessing.process(RHICmdList, mViews[viewIndex], TRefCountPtr<IPooledRenderTarget>());
			}
		}
		else
		{
		}


		//RHICmdList.copyToResolveTarget(sceneContext.mGBufferA->getRenderTargetItem().mTargetableTexture, mViewFamily.mRenderTarget->getRenderTargetTexture(), true, ResolveParams());
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
}