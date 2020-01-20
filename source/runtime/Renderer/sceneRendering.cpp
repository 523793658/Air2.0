#include "sceneRendering.h"
#include "DeferredShadingRenderer.h"
#include "SceneView.h"
#include "RenderTarget.h"
#include "RendererModule.h"
#include "scene.h"
#include "PostProcess/PostProcessing.h"
#include "LightSceneInfo.h"
#include "ScenePrivate.h"
#include "PostProcess/SceneRenderTargets.h"
#include "RHIStaticStates.h"
#include "HdrCustomResolveShaders.h"
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

	class DummySceneColorResolveBuffer : public VertexBuffer
	{
	public:
		virtual void initRHI() override
		{
			const int32 mNumDummyVerts = 3;
			const uint32 mSize = sizeof(float4) * mNumDummyVerts;
			RHIResourceCreateInfo mCreateInfo;
			void* bufferData = nullptr;
			mVertexBufferRHI = RHICreateAndLockVertexBuffer(mSize, BUF_Static, mCreateInfo, bufferData);
			Memory::Memset(bufferData, 0, mSize);
			RHIUnlockVertexBuffer(mVertexBufferRHI);
		}
	};

	TGlobalResource<DummySceneColorResolveBuffer> GResolveDummyVertexBuffer;

	void SceneRenderer::resolveSceneColor(RHICommandList& RHICmdList)
	{
	

		SceneRenderTargets& sceneContext = SceneRenderTargets::get(RHICmdList);

		auto& currentSceneColor = sceneContext.getSceneColor();
		uint32 currentNumSamples = currentSceneColor->getDesc().mNumSamples;

		const EShaderPlatform currentShaderPlatform = GShaderPlatformForFeatureLevel[sceneContext.getCurrentFeatureLevel()];

		if (currentNumSamples <= 1 || !RHISupportsSeparateMSAAAndResolveTextures(currentShaderPlatform))
		{
			RHICmdList.copyToResolveTarget(sceneContext.getSceneColorSurface(), sceneContext.getSceneColorTexture(), ResolveRect(0, 0, mFamilySize.x, mFamilySize.y));
		}
		else
		{
			RHICmdList.transitionResource(EResourceTransitionAccess::EReadable, sceneContext.getSceneColorSurface());

			Texture2DRHIRef maskTexture = GDynamicRHI->RHIGetMaskTexture(sceneContext.getSceneColorSurface());

			RHIRenderPassInfo RPInfo(sceneContext.getSceneColorTexture(), ERenderTargetActions::Load_Store);
			RHICmdList.beginRenderPass(RPInfo, TEXT("ResolveColor"));
			{
				for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
				{
					const ViewInfo& view = mViews[viewIndex];
					GraphicsPipelineStateInitializer graphicsPSOInit;
					RHICmdList.applyCachedRenderTargets(graphicsPSOInit);

					graphicsPSOInit.mBlendState = TStaticBlendState<>::getRHI();
					graphicsPSOInit.mRasterizerState = TStaticRasterizerState<>::getRHI();
					graphicsPSOInit.mDepthStencilState = TStaticDepthStencilState<false, CF_Always>::getRHI();
					RHICmdList.setStreamSource(0, GResolveDummyVertexBuffer.mVertexBufferRHI, 0);

					RHICmdList.setScissorRect(true, view.mViewRect.min.x, view.mViewRect.min.y, view.mViewRect.max.x, view.mViewRect.max.y);

					int32 resolveWith = 0;
					if (currentNumSamples <= 1)
					{
						resolveWith = 0;
					}
					if (resolveWith != 0)
					{
						
					}
					else
					{
						auto shaderMap = getGlobalShaderMap(sceneContext.getCurrentFeatureLevel());
						TShaderMapRef<HdrCustomResolveVS> vertexShader(shaderMap);
						graphicsPSOInit.mBoundShaderState.mVertexDeclarationRHI = getVertexDeclarationVector4();
						graphicsPSOInit.mBoundShaderState.mVertexShaderRHI = GETSAFERHISHADER_VERTEX(*vertexShader);
						graphicsPSOInit.mPrimitiveType = PT_TriangleList;

						if (maskTexture->isValid())
						{
							if (currentNumSamples == 2)
							{
								TShaderMapRef<HdrCustomResolveMask2xPS> pixelShader(shaderMap);
								graphicsPSOInit.mBoundShaderState.mPixelShaderRHI = GETSAFERHISHADER_PIXEL(*pixelShader);

								setGraphicsPipelineState(RHICmdList, graphicsPSOInit);
								pixelShader->setParameters(RHICmdList, currentSceneColor->getRenderTargetItem().mTargetableTexture, maskTexture);

							}
							else if (currentNumSamples == 4)
							{
								TShaderMapRef<HdrCustomResolveMask4xPS> pixelShader(shaderMap);
								graphicsPSOInit.mBoundShaderState.mPixelShaderRHI = GETSAFERHISHADER_PIXEL(*pixelShader);

								setGraphicsPipelineState(RHICmdList, graphicsPSOInit);
								pixelShader->setParameters(RHICmdList, currentSceneColor->getRenderTargetItem().mTargetableTexture, maskTexture);
							}
							else if (currentNumSamples == 8)
							{
								TShaderMapRef<HdrCustomResolveMask8xPS> pixelShader(shaderMap);
								graphicsPSOInit.mBoundShaderState.mPixelShaderRHI = GETSAFERHISHADER_PIXEL(*pixelShader);

								setGraphicsPipelineState(RHICmdList, graphicsPSOInit);
								pixelShader->setParameters(RHICmdList, currentSceneColor->getRenderTargetItem().mTargetableTexture, maskTexture);
							}
							else
							{
								BOOST_ASSERT(false);
								break;
							}
						}
						else
						{
							if (currentNumSamples == 2)
							{
								TShaderMapRef<HdrCustomResolve2xPS> pixelShader(shaderMap);
								graphicsPSOInit.mBoundShaderState.mPixelShaderRHI = GETSAFERHISHADER_PIXEL(*pixelShader);
								setGraphicsPipelineState(RHICmdList, graphicsPSOInit);
								pixelShader->setParameters(RHICmdList, currentSceneColor->getRenderTargetItem().mTargetableTexture);
							}
							if (currentNumSamples == 4)
							{
								TShaderMapRef<HdrCustomResolve4xPS> pixelShader(shaderMap);
								graphicsPSOInit.mBoundShaderState.mPixelShaderRHI = GETSAFERHISHADER_PIXEL(*pixelShader);
								setGraphicsPipelineState(RHICmdList, graphicsPSOInit);
								pixelShader->setParameters(RHICmdList, currentSceneColor->getRenderTargetItem().mTargetableTexture);
							}
							if (currentNumSamples == 8)
							{
								TShaderMapRef<HdrCustomResolve8xPS> pixelShader(shaderMap);
								graphicsPSOInit.mBoundShaderState.mPixelShaderRHI = GETSAFERHISHADER_PIXEL(*pixelShader);
								setGraphicsPipelineState(RHICmdList, graphicsPSOInit);
								pixelShader->setParameters(RHICmdList, currentSceneColor->getRenderTargetItem().mTargetableTexture);
							}
							else
							{
								BOOST_ASSERT(false);
								break;
							}
						}
						RHICmdList.drawPrimitive(0, 1, 1);
					}
				}
				RHICmdList.setScissorRect(false, 0, 0, 0, 0);
			}
			RHICmdList.endRenderPass();
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

	void SceneRenderer::prepareViewRectsForRendering()
	{
		BOOST_ASSERT(isInRenderingThread());

		if (!mViewFamily.supportsScreenPercentage())
		{
			for (ViewInfo& view : mViews)
			{
				view.mViewRect = view.mUnscaledViewRect;
			}
			computeFamilySize();
			return;
		}

		TArray<SceneViewScreenPercentageConfig> viewScreenPercentageConfigs;
		viewScreenPercentageConfigs.reserve(mViews.size());

		for (ViewInfo& view : mViews)
		{
			BOOST_ASSERT(view.mViewRect.area() == 0);

			{
				const bool bWillApplyTemporalAA = GPostProcessing.allowFullPostProcessing(view, mFeatureLevel) || (view.bIsPlanarRefelction && mFeatureLevel >= ERHIFeatureLevel::SM4);
				if (!bWillApplyTemporalAA)
				{
					view.mAntiAliasingMethod = AAM_None;
				}
			}

			SceneViewScreenPercentageConfig config;
			viewScreenPercentageConfigs.add(SceneViewScreenPercentageConfig());
		}

		BOOST_ASSERT(mViewFamily.mScreenPercentageInterface);
		mViewFamily.mScreenPercentageInterface->computePrimaryResolutionFractions_RenderThread(viewScreenPercentageConfigs);
		BOOST_ASSERT(viewScreenPercentageConfigs.size() == mViews.size());

		for (int32 i = 0; i < mViews.size(); i++)
		{
			ViewInfo& view = mViews[i];
			float primaryResolutionFraction = viewScreenPercentageConfigs[i].mPrimaryResolutionFraction;

			if (!mViewFamily.mEngineShowFlags.ScreenPercentage)
			{
				BOOST_ASSERT(primaryResolutionFraction = 1.0f);
				BOOST_ASSERT(primaryResolutionFraction <= mViewFamily.getPrimaryResolutionFractionUpperBound());
				BOOST_ASSERT(SceneViewScreenPercentageConfig::isValidResolutionFraction(primaryResolutionFraction));
				float resolutionFraction = primaryResolutionFraction * mViewFamily.mSecondaryViewFraction;

				int2 viewSize = applyResolutionFraction(mViewFamily, view.mUnscaledViewRect.size(), resolutionFraction);

				int2 viewRectMin = quantizeViewRectMin(int2(Math::ceilToInt(view.mUnscaledViewRect.min.x * resolutionFraction),
					Math::ceilToInt(view.mUnscaledViewRect.min.y * resolutionFraction)));

				view.mViewRect.min = viewRectMin;
				view.mViewRect.max = viewRectMin + viewSize;

				{
					//if(view.primaryscree)
				}
			}
		}
		{
			int2 topLeftShift = mViews[0].mViewRect.min;
			for (int32 i = 1; i < mViews.size(); ++i)
			{
				topLeftShift.x = Math::min(topLeftShift.x, mViews[i].mViewRect.min.x);
				topLeftShift.y = Math::min(topLeftShift.y, mViews[i].mViewRect.min.y);
			}

			for (int32 i = 0; i < mViews.size(); i++)
			{
				mViews[i].mViewRect -= topLeftShift;
			}
		}

		computeFamilySize();
	}

	int2 SceneRenderer::applyResolutionFraction(const SceneViewFamily& viewFamily, const int2& unscaledViewSize, float resolutionFraction)
	{
		int2 viewSize;
		viewSize.x = Math::ceilToInt(unscaledViewSize.x * resolutionFraction);
		viewSize.y = Math::ceilToInt(unscaledViewSize.y * resolutionFraction);
		return viewSize;
	}

	int2 SceneRenderer::quantizeViewRectMin(const int2& viewRectMin)
	{
		int2 out;
		quantizeSceneBufferSize(viewRectMin, out);
		return out;
	}

	void SceneRenderer::computeFamilySize()
	{
		BOOST_ASSERT(mFamilySize.x == 0);
		BOOST_ASSERT(isInRenderingThread());

		bool bInitializeExtents = false;

		float maxFamilyX = 0;
		float maxFamilyY = 0;

		for (const ViewInfo& view : mViews)
		{
			float finalViewMaxX = (float)view.mViewRect.max.x;
			float finalViewMaxY = (float)view.mViewRect.max.y;
			const float xScale = finalViewMaxX / (float)view.mUnscaledViewRect.max.x;
			const float yScale = finalViewMaxY / (float)view.mUnscaledViewRect.max.y;
			if (!bInitializeExtents)
			{
				maxFamilyX = view.mUnconstrainedViewRect.max.x * xScale;
				maxFamilyY = view.mUnconstrainedViewRect.max.y * yScale;
				bInitializeExtents = true;
			}
			else
			{
				maxFamilyX = Math::max(maxFamilyX, view.mUnconstrainedViewRect.max.x * xScale);
				maxFamilyY = Math::max(maxFamilyY, view.mUnconstrainedViewRect.max.y * yScale);
			}
			maxFamilyX = Math::max(maxFamilyX, finalViewMaxX);
			maxFamilyY = Math::max(maxFamilyY, finalViewMaxY);
		}
		mFamilySize.x = Math::truncToInt(maxFamilyX);
		mFamilySize.y = Math::truncToInt(maxFamilyY);

		BOOST_ASSERT(mFamilySize.x != 0);
		BOOST_ASSERT(bInitializeExtents);
	}


	void SceneRenderer::renderFinish(RHICommandListImmediate& RHICmdList)
	{
	}
}