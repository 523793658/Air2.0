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

			if (scene->mSkyLight)
			{
				SkyLightSceneProxy* skyLight = scene->mSkyLight;
				viewConstantShaderParameters.SkyLightColor = skyLight->mLightColor;
				bool bApplyPrecomputedBentNormalShadowing = skyLight->bCastShadows && skyLight->bWantsStaticShadowing;

				viewConstantShaderParameters.SkyLightParameters = bApplyPrecomputedBentNormalShadowing ? 1 : 0;
			}
			else
			{
				viewConstantShaderParameters.SkyLightParameters = 0;
				viewConstantShaderParameters.SkyLightColor = LinearColor::Black;
			}
		}

		BOOST_ASSERT(sizeof(viewConstantShaderParameters.SkyIrradianceEnvironmentMap) == sizeof(float4) * 7);
		setupSkyIrradianceEnvironmentMapConstants((float4*)& viewConstantShaderParameters.SkyIrradianceEnvironmentMap);
	}

	void ViewInfo::setupSkyIrradianceEnvironmentMapConstants(float4* outSkyIrradianceEnvironmentMap) const
	{
		Scene* scene = nullptr;
		if (mFamily->mScene)
		{
			scene = mFamily->mScene->getRenderScene();
		}

		if (scene && scene->mSkyLight && !scene->mSkyLight->bHasStaticLighting
			&& mFamily->mEngineShowFlags.SkyLighting)
		{
			const SHVectorRGB3& skyIrradiance = scene->mSkyLight->mIrradianceEnvironmentMap;
			const float sqrtPI = Math::sqrt(PI);
			const float coefficient0 = 1.0f / (2 * sqrtPI);
			const float coefficient1 = Math::sqrt(3) / (3 * sqrtPI);
			const float coefficient2 = Math::sqrt(15) / (8 * sqrtPI);
			const float coefficient3 = Math::sqrt(5) / (16 * sqrtPI);
			const float coefficient4 = 0.5f * coefficient2;

			outSkyIrradianceEnvironmentMap[0].x = -coefficient1 * skyIrradiance.r.V[3];
			outSkyIrradianceEnvironmentMap[0].y = -coefficient1 * skyIrradiance.r.V[1];
			outSkyIrradianceEnvironmentMap[0].z = coefficient1 * skyIrradiance.r.V[2];
			outSkyIrradianceEnvironmentMap[0].w = coefficient0 * skyIrradiance.r.V[0] -coefficient3 * skyIrradiance.r.V[6];


			outSkyIrradianceEnvironmentMap[1].x = -coefficient1 * skyIrradiance.g.V[3];
			outSkyIrradianceEnvironmentMap[1].y = -coefficient1 * skyIrradiance.g.V[1];
			outSkyIrradianceEnvironmentMap[1].z = coefficient1 * skyIrradiance.g.V[2];
			outSkyIrradianceEnvironmentMap[1].w = coefficient0 * skyIrradiance.g.V[0] - coefficient3 * skyIrradiance.b.V[6];


			outSkyIrradianceEnvironmentMap[2].x = -coefficient1 * skyIrradiance.b.V[3];
			outSkyIrradianceEnvironmentMap[2].y = -coefficient1 * skyIrradiance.b.V[1];
			outSkyIrradianceEnvironmentMap[2].z = coefficient1 * skyIrradiance.b.V[2];
			outSkyIrradianceEnvironmentMap[2].w = coefficient0 * skyIrradiance.b.V[0] - coefficient3 * skyIrradiance.b.V[6];


			outSkyIrradianceEnvironmentMap[3].x = coefficient2 * skyIrradiance.r.V[4];
			outSkyIrradianceEnvironmentMap[3].y = -coefficient2 * skyIrradiance.r.V[5];
			outSkyIrradianceEnvironmentMap[3].z = 3 * coefficient3 * skyIrradiance.r.V[6];
			outSkyIrradianceEnvironmentMap[3].w = -coefficient2 * skyIrradiance.r.V[7];


			outSkyIrradianceEnvironmentMap[4].x = coefficient2 * skyIrradiance.g.V[4];
			outSkyIrradianceEnvironmentMap[4].y = -coefficient2 * skyIrradiance.g.V[5];
			outSkyIrradianceEnvironmentMap[4].z = 3 * coefficient3 * skyIrradiance.g.V[6];
			outSkyIrradianceEnvironmentMap[4].w = -coefficient2 * skyIrradiance.g.V[7];


			outSkyIrradianceEnvironmentMap[5].x = coefficient2 * skyIrradiance.b.V[4];
			outSkyIrradianceEnvironmentMap[5].y = -coefficient2 * skyIrradiance.b.V[5];
			outSkyIrradianceEnvironmentMap[5].z = 3 * coefficient3 * skyIrradiance.b.V[6];
			outSkyIrradianceEnvironmentMap[5].w = -coefficient2 * skyIrradiance.b.V[7];

			outSkyIrradianceEnvironmentMap[6].x = coefficient4 * skyIrradiance.r.V[8];
			outSkyIrradianceEnvironmentMap[6].y = coefficient4 * skyIrradiance.g.V[8];
			outSkyIrradianceEnvironmentMap[6].z = coefficient4 * skyIrradiance.b.V[8];
			outSkyIrradianceEnvironmentMap[6].w = 1;
		}
		else
		{
			Memory::memzero(outSkyIrradianceEnvironmentMap, sizeof(float4) * 7);
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