#include "sceneRendering.h"
#include "DeferredShadingRenderer.h"
#include "SceneView.h"
#include "RenderTarget.h"
#include "RendererModule.h"
#include "scene.h"
#include "LightSceneInfo.h"
#include "ScenePrivate.h"
#include "PostProcess/SceneRenderTargets.h"
namespace Air
{
	SceneRenderer::SceneRenderer(const SceneViewFamily* inViewFamily, HitProxyConsumer* hitProxyConsumer)
		:mScene(inViewFamily->mScene ? inViewFamily->mScene->getRenderScene() : nullptr)
		, mViewFamily(*inViewFamily)
	{
		BOOST_ASSERT(isInGameThread());
		mViewFamily.mFrameNumber = GFrameNumber;
		bool bAnyViewIsLocked = false;
		mViews.reserve(inViewFamily->mViews.size());
		for (int32 viewIndex = 0; viewIndex < inViewFamily->mViews.size(); viewIndex++)
		{
#if AIR_DEBUG

			for (int32 viewIndex2 = 0; viewIndex2 < inViewFamily->mViews.size(); viewIndex2++)
			{
				if (viewIndex != viewIndex2 && inViewFamily->mViews[viewIndex]->mState != nullptr)
				{
					BOOST_ASSERT(inViewFamily->mViews[viewIndex]->mState != inViewFamily->mViews[viewIndex2]->mState);
				}
			}

#endif

			
			ViewInfo* viewInfo = new(mViews) ViewInfo(inViewFamily->mViews[viewIndex]);
			mViewFamily.mViews[viewIndex] = viewInfo;
			viewInfo->mFamily = &mViewFamily;
			bAnyViewIsLocked |= viewInfo->bIsLocked;
			if (viewInfo->mDrawer)
			{
				
			}
		}

		if (bAnyViewIsLocked)
		{
			mViewFamily.mCurrentRealTime = 0.0f;
			mViewFamily.mCurrentWorldTime = 0.0f;
		}
		if (hitProxyConsumer)
		{
			//mViewFamily.mEngineShowFlags.setHit
		}
		if (GCustomCullingImpl)
		{
			for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
			{
				ViewInfo& viewInfo = mViews[viewIndex];
				viewInfo.mCustomVisibilityQuery = GCustomCullingImpl->createQuery(viewInfo);
			}
		}
		mViewFamily.computeFamilySize();
		bHasRequestedToggleFreeze = const_cast<RenderTarget*>(inViewFamily->mRenderTarget)->hasToggleFreezeCommand();
		mFeatureLevel = mScene->getFeatureLevel();

	}



	void SceneRenderer::waitForTasksClearSnapshotsAndDeleteSceneRenderer(RHICommandListImmediate& RHICmdList, SceneRenderer* sceneRenderer)
	{
		{
			RHICmdList.immediateFlush(EImmediateFlushType::WaitForOutstandingTasksOnly);
		}
		SceneRenderTargets::get(RHICmdList).destroyAllSnapshots();

		RHICmdList.immediateFlush(EImmediateFlushType::WaitForDispatchToRHIThread);

		{
			delete sceneRenderer;
		}
	}

	SceneRenderer* SceneRenderer::createSceneRenderer(const SceneViewFamily* inViewFamily)
	{
		EShadingPath shadingPath = inViewFamily->mScene->getShadingPath();
		if (shadingPath == EShadingPath::Deferred)
		{
			return new DeferredShadingSceneRenderer(inViewFamily, nullptr);
		}
		else
		{
			return nullptr;
		}
	}

	void SceneRenderer::onStartFrame(RHICommandListImmediate& RHICmdList)
	{
		SceneRenderTargets& sceneContext = SceneRenderTargets::get(RHICmdList);
		for (int32 viewIndex = 0; viewIndex < mViews.size(); ++viewIndex)
		{
			SceneView& view = mViews[viewIndex];
			SceneViewStateInterface* state = view.mState;
			if (state)
			{
				
			}
		}
	}

	void SceneRenderer::clearPrimitiveSingleFramePrecomputedLightingBuffers()
	{
		for (int32 viewIndex = 0; viewIndex < mViews.size(); ++viewIndex)
		{
			ViewInfo& view = mViews[viewIndex];
		}
	}

	SceneRenderer::~SceneRenderer()
	{
		clearPrimitiveSingleFramePrecomputedLightingBuffers();
		if (mScene)
		{
			//for(TSparseArray<LightSceneInfoCompact)
		}
	}

	ViewInfo::ViewInfo(const SceneView* inView)
		:SceneView(*inView)
	{
		init();
	}

	ViewInfo::ViewInfo(ViewInfo&& inInfo)
		:SceneView(std::move(inInfo))
	{

	}

	ViewInfo::~ViewInfo()
	{
		for (int32 resourceIndex = 0; resourceIndex < mDynamicResources.size(); resourceIndex++)
		{
			mDynamicResources[resourceIndex]->releasePrimitiveResource();
		}
		if (mCustomVisibilityQuery)
		{
			mCustomVisibilityQuery->Release();
		}
	}

	void ViewInfo::init()
	{
		bIsViewInfo = true;
		bUseLightingChannels = false;
		mShaderMap = getGlobalShaderMap(mFeatureLevel);
		
	}

	void ViewInfo::destroyAllSnapshots()
	{
		BOOST_ASSERT(isInRenderingThread());
		//int32 numToRemove = 
	}

	void ViewInfo::setupConstantBufferParameters(SceneRenderTargets& sceneContext,
		const ViewMatrices & inViewMatrices,
		const ViewMatrices& inPreViewMatrices,
		Box* outTranslucentCascadeBoundsArray,
		int32 numTranslucentCascades,
		ViewConstantShaderParameters& viewConstantShaderParameters) const
	{
		BOOST_ASSERT(mFamily);
		const IntRect effectiveViewRect = mViewRect;
		setupCommonViewConstantBufferParameters(
			viewConstantShaderParameters, sceneContext.getBufferSize(),
			sceneContext.getMSAACount(),
			effectiveViewRect,
			inViewMatrices,
			inPreViewMatrices
		);

		viewConstantShaderParameters.SkyBoxTexture = mSkyTexture ? mSkyTexture.getReference() : GWhiteTexture->mTextureRHI.getReference();

		const bool bCheckerBoardSubsurfaceRendering = false;
		Scene* scene = nullptr;
		if (mFamily->mScene)
		{
			scene = mFamily->mScene->getRenderScene();
		}
		if (scene)
		{
			if (scene->mSimpleDirectionalLight)
			{
				viewConstantShaderParameters.DirectionalLightColor = scene->mSimpleDirectionalLight->mProxy->getColor() / PI;
				viewConstantShaderParameters.DirectionalLightDirection = -scene->mSimpleDirectionalLight->mProxy->getDirection();
			}
			else
			{
				viewConstantShaderParameters.DirectionalLightColor = LinearColor::Black;
				viewConstantShaderParameters.DirectionalLightDirection = float3::Zero;
			}
		}
	}

	void ViewInfo::initRHIResources()
	{
		Box volumeBounds[TVC_Max];
		BOOST_ASSERT(isInRenderingThread());
		mCachedViewConstantShaderParameters = std::make_unique<ViewConstantShaderParameters>();
		SceneRenderTargets& sceneContext = SceneRenderTargets::get(RHICommandListExecutor::getImmediateCommandList());
		setupConstantBufferParameters(sceneContext, volumeBounds, TVC_Max, *mCachedViewConstantShaderParameters);
		mViewConstantBuffer = TConstantBufferRef<ViewConstantShaderParameters>::createConstantBufferImmediate(*mCachedViewConstantShaderParameters, ConstantBuffer_SingleFrame);
		for (int32 cascadeIndex = 0; cascadeIndex < TVC_Max; ++cascadeIndex)
		{

		}
		for (int32 resourceIndex = 0; resourceIndex < mDynamicResources.size(); resourceIndex++)
		{
			mDynamicResources[resourceIndex]->initPrimitiveResource();
		}
	}
}