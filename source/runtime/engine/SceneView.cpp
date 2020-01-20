#include "SceneView.h"
#include "EngineModule.h"
#include "Classes/Engine/World.h"
#include "RendererModule.h"
#include "ShaderParameters.h"
#include "PrimitiveConstantShaderParameters.h"
namespace Air
{

	IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(PrimitiveConstantShaderParameters, "Primitive");

	IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(ViewConstantShaderParameters, "View");


	IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(InstancedViewConstantShaderParameters, "InstancedView");


	float4 createInvDeviceZToWorldZTransform(const Matrix& projMatrix)
	{
		float depthMul = projMatrix.M[2][2];
		float depthAdd = projMatrix.M[3][2];
		if (depthAdd == 0.0f)
		{
			depthAdd = 0.00000001f;
		}
		bool bIsPerspectiveProjection = projMatrix.M[3][3] < 1.0f;
		if (bIsPerspectiveProjection)
		{
			float subtractValue = depthMul / depthAdd;
			subtractValue -= 0.00000001f;
			return float4(0.0f, 0.0f, 1.0f / depthAdd, subtractValue);
		}
		else
		{
			return float4(1.0f / projMatrix.M[2][2], -projMatrix.M[3][2] / projMatrix.M[2][2] + 1.0f, 0.0f, 1.0f);
		}

	}

	SceneViewStateReference::~SceneViewStateReference()
	{
		destroy();
	}

	void SceneViewStateReference::allocate()
	{
		mReference = getRendererModule().allocateViewState();
		mGlobalListLink = TLinkedList<SceneViewStateReference*>(this);
		mGlobalListLink.linkHead(getSceneViewStateList());
	}

	void SceneViewStateReference::destroy()
	{
		mGlobalListLink.unLink();
		if (mReference)
		{
			mReference->destroy();
			mReference = nullptr;
		}
	}

	void SceneViewStateReference::destroyAll()
	{
		for (TLinkedList<SceneViewStateReference*>::TIterator viewStateIt(SceneViewStateReference::getSceneViewStateList()); viewStateIt; viewStateIt.next())
		{
			SceneViewStateReference* viewStateReference = *viewStateIt;
			viewStateReference->mReference->destroy();
			viewStateReference->mReference = nullptr;
		}
	}

	void SceneViewStateReference::allocateAll()
	{
		for (TLinkedList<SceneViewStateReference*>::TIterator viewStateIt(SceneViewStateReference::getSceneViewStateList()); viewStateIt; viewStateIt.next())
		{
			SceneViewStateReference* viewStateReference = *viewStateIt;
			viewStateReference->mReference = getRendererModule().allocateViewState();
		}
	}

	TLinkedList<SceneViewStateReference*>*& SceneViewStateReference::getSceneViewStateList()
	{
		static TLinkedList<SceneViewStateReference*>* List = nullptr;
		return List;
	}


	SceneViewFamilyContext::~SceneViewFamilyContext()
	{
		for (int32 index = 0; index < mViews.size(); ++index)
		{
			delete mViews[index];
		}
	}

	SceneViewFamily::SceneViewFamily(const ConstructionValues& CVS)
		:mRenderTarget(CVS.mRenderTarget),
		mScene(CVS.mScene),
		mEngineShowFlags(CVS.mEngineShowFlags),
		mCurrentWorldTime(CVS.mCurrentWorldTime),
		mDeltaWorldTime(CVS.mDeltaWorldTime),
		mCurrentRealTime(CVS.mCurrentRealTime),
		mFrameNumber(std::numeric_limits<uint32>::max()),
		bRealtimeUpdate(CVS.bRealtimeUpdate),
		bDeferClear(CVS.bDeferClear),
		bResolveScene(CVS.bResolveScene),
		mGammaCorrect(CVS.mGammaCorrection)
	{
		if (isInGameThread() && mScene)
		{
			World* world = mScene->getWorld();
			if (world)
			{
				if (world->isGameWorld())
				{
					mEngineShowFlags.LOD = 1;
				}
				mEngineShowFlags.Rendering = 1;
				bWorldIsPaused = !world->isCameraMoveable();
			}
		}

		const bool bIsMobile = SceneInterface::getShadingPath(getFeatureLevel()) == EShadingPath::Mobile;
	}

	const SceneView& SceneViewFamily::getSteredEyeView(const EStereoscopicPass eye) const
	{
		const int32 eyeIndex = static_cast<int32>(eye);
		BOOST_ASSERT(mViews.size() > 0 && mViews.size() >= eyeIndex);
		if (eyeIndex <= 1)
		{
			return *mViews[0];
		}
		else if (eyeIndex == 2)
		{
			return *mViews[1];
		}
		else
		{
			return *mViews[eyeIndex - eSSP_RIGHT_EYE + 1];
		}
	}

	bool SceneViewFamily::supportsScreenPercentage() const
	{
		EShadingPath shadingPath = mScene->getShadingPath();

		if (mScene->getShadingPath() == EShadingPath::Deferred)
		{
			return true;
		}
		if (getFeatureLevel() <= ERHIFeatureLevel::ES3_1)
		{
			return false;
		}

		return true;
	}

	void SceneViewFamily::computeFamilySize()
	{
		bool bInitializedExtens = false;
		float maxFamilyX = 0;
		float maxFamilyY = 0;
		for (size_t viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
		{
			const SceneView* view = mViews[viewIndex];
			//if(view->mresolution)
			if (false)
			{

			}
			else
			{
				float finalViewMaxX = (float)view->mViewRect.max.x;
				float finalViewMaxY = (float)view->mViewRect.max.y;

				const float xScale = finalViewMaxX / (float)view->mUnscaledViewRect.max.x;
				const float yScale = finalViewMaxY / (float)view->mUnscaledViewRect.max.y;
				if (!bInitializedExtens)
				{
					maxFamilyX = view->mUnconstrainedViewRect.max.x * xScale;
					maxFamilyY = view->mUnconstrainedViewRect.max.y * yScale;
					bInitializedExtens = true;
				}
				else
				{
					maxFamilyX = std::max<float>(maxFamilyX, view->mUnconstrainedViewRect.max.x * xScale);
					maxFamilyY = std::max<float>(maxFamilyY, view->mUnconstrainedViewRect.max.y * yScale);
				}

				maxFamilyX = std::max<float>(maxFamilyX, finalViewMaxX);
				maxFamilyY = std::max<float>(maxFamilyY, finalViewMaxY);
			}
			mInstancedStereoWidth = std::max<uint32>(mInstancedStereoWidth, static_cast<uint32>(mViews[viewIndex]->mViewRect.max.x));
		}
		mFamilySizeX = std::truncl(maxFamilyX);
		mFamilySizeY = std::truncl(maxFamilyY);
		BOOST_ASSERT(bInitializedExtens);
	}

	ViewMatrices::ViewMatrices(const SceneViewInitOptions& inOptions)
	{
		float3 localViewOrigin = inOptions.mViewOrigin;
		Matrix viewRotationMatrix = inOptions.mViewRotationMatrix;
		if (!viewRotationMatrix.getOrigin().isNearlyZero(0.0f))
		{
			localViewOrigin += viewRotationMatrix.inverseTransformPosition(float3::Zero);
			viewRotationMatrix = viewRotationMatrix.removeTranslation();
		}
		mViewMatrix = TranslationMatrix(-localViewOrigin) * viewRotationMatrix;
		mHMDViewMatrixNoRoll = inOptions.mViewRotationMatrix;

		mProjectionMatrix = adjustProjectionMatrixForRHI(inOptions.mProjectionMatrix);
		mInvProjectionMatrix = invertProjectionMatrix(mProjectionMatrix);

		mViewProjectionMatrix = getViewMatrix() * getProjectionMatrix();
		mInvViewMatrix = viewRotationMatrix.getTransposed() * TranslationMatrix(localViewOrigin);
		mInvViewProjectionMatrix = mInvProjectionMatrix * mInvViewMatrix;

		bool bApplyPreViewTranslation = true;
		bool bViewOriginIsFudged = false;
		if (isPerspectiveProjection())
		{
			this->mViewOrigin = localViewOrigin;
		}
		else
		{
			this->mViewOrigin = float4(mInvViewMatrix.transformVector(float3(0, 0, -1)).getSafeNormal(), 0);
			bApplyPreViewTranslation = false;
		}

		Matrix localtranslatedViewMatrix = viewRotationMatrix;
		Matrix localInvTranslatedViewMatrix = localtranslatedViewMatrix.getTransposed();

		if (bApplyPreViewTranslation)
		{
			mPreviewTranslation = -float3(localViewOrigin);
			{

			}
		}
		else
		{
			localtranslatedViewMatrix = mViewMatrix;
			localInvTranslatedViewMatrix = mInvViewMatrix;
		}
		if (bViewOriginIsFudged)
		{
			localtranslatedViewMatrix = TranslationMatrix(-mPreviewTranslation);
			localInvTranslatedViewMatrix = localtranslatedViewMatrix.inverse();
		}

		mTranslatedViewMatrix = localtranslatedViewMatrix;
		mInvTranslatedViewMatrix = localInvTranslatedViewMatrix;
		mOverriddenTranslatedViewMatrix = TranslationMatrix(-getPreviewTranslation()) * getViewMatrix();
		mOverriddenInvTranslatedViewMatrix = getInvViewMatrix() * TranslationMatrix(getPreviewTranslation());

		mTranslatedViewProjectionMatrix = localtranslatedViewMatrix * mProjectionMatrix;
		mInvTranslatedViewProjectionMatrix = mInvProjectionMatrix* localInvTranslatedViewMatrix;

		const bool bStereo = inOptions.mStereoPass != eSSP_FULL;
		const float screenXScale = bStereo ? 2.0f : 1.0f;
		mProjectionScale.x = screenXScale * Math::abs(mProjectionMatrix.M[0][0]);

		mProjectionScale.y = Math::abs(mProjectionMatrix.M[1][1]);
		mScreenScale = std::max<float>(inOptions.getConstrainedViewRect().width() * 0.5f * mProjectionScale.x, inOptions.getConstrainedViewRect().height() * 0.5f * mProjectionScale.y);




	}

	SceneView::SceneView(const SceneViewInitOptions& initOptions)
		:mFamily(initOptions.mViewFamily)
		, mState(initOptions.mSceneViewStateInterface)
		, mDrawer(initOptions.mViewElementDrawer)
		, mViewActor(initOptions.mViewActor)
		, mViewRect(initOptions.getConstrainedViewRect())
		, mUnscaledViewRect(initOptions.getConstrainedViewRect())
		, mUnconstrainedViewRect(initOptions.getViewRect())
		, mViewMatrices(initOptions)
		, mWorldToMetersScale(initOptions.mWorldToMetersScale)
		, mShadowViewMatrices(initOptions)
		, mProjectionMatrixUnadjustedForRHI(initOptions.mProjectionMatrix)
		, mBackgroundColor(initOptions.mBackgroundColor)
		, mOverlayColor(initOptions.mOverlayColor)
		, mColorScale(initOptions.mColorScale)
		, mStereoPass(initOptions.mStereoPass)
		, mFeatureLevel(initOptions.mViewFamily ? initOptions.mViewFamily->getFeatureLevel() : GMaxRHIFeatureLevel)
	{
		mShadowViewMatrices = mViewMatrices;
		{
			int32 value = 0;
			static ViewMatrices backup = mShadowViewMatrices;
			if (value)
			{
				mShadowViewMatrices = backup;
			}
			else
			{
				backup = mShadowViewMatrices;
			}
		}

		if (initOptions.OverrideFarClippingPlaneDistance > 0.0f)
		{
			const Plane farPlane(mViewMatrices.getViewOrigin() + getViewDirection() * initOptions.OverrideFarClippingPlaneDistance, getViewDirection());

			getViewFrustumBounds(mViewFrustum, mViewMatrices.getViewProjectionMatrix(), farPlane, true, false);
		}
		else
		{
			getViewFrustumBounds(mViewFrustum, mViewMatrices.getViewProjectionMatrix(), false);
		}

		bHasNearClippingPlane = mViewMatrices.getViewProjectionMatrix().getFrustumFarPlane(mNearClippingPlane);
		if (mViewMatrices.getProjectionMatrix().M[2][3] > DELTA)
		{
			mNearClippingDistance = mViewMatrices.getProjectionMatrix().M[3][2];
		}
		else
		{
			mNearClippingDistance = (1.0f - mViewMatrices.getProjectionMatrix().M[3][2] / mViewMatrices.getProjectionMatrix().M[2][2]);
		}
		bReverseCulling = (mFamily && mFamily->mScene) ? Math::
			isNegativeFloat(mViewMatrices.getViewMatrix().determinant()) : false;

		auto shaderPlatform = GShaderPlatformForFeatureLevel[mFeatureLevel];

		bool bUsingMobileRenderer = SceneInterface::getShadingPath(mFeatureLevel) == EShadingPath::Mobile;
		bool bPlantformRequiresReversCulling = (isOpenGLPlatform(shaderPlatform) && bUsingMobileRenderer && !isPCPlatform(shaderPlatform) && !isVulkanMobilePlatform(shaderPlatform));
		bReverseCulling = (bPlantformRequiresReversCulling) ? !bReverseCulling : bReverseCulling;
		mInvDeviceZToWorldZTransform = createInvDeviceZToWorldZTransform(mProjectionMatrixUnadjustedForRHI);
		
		if (isInGameThread())
		{
			bIsGameView = (mFamily && mFamily->mScene && mFamily->mScene->getWorld()) ? mFamily->mScene->getWorld()->isGameWorld() : false;
		}
		bUseFieldOfViewForLOD = initOptions.mUseFieldOfViewForLOD;
		mDrawDynamicFlags = EDrawDynamicFlags::None;
		bAllowTemporalJitter = true;
		mTemporalJitterPixelsX = 0.0f;
		mTemporalJitterPixelsY = 0.0f;
		bIsMobileMultiViewEnable = false;
		bShouldBindInstancedViewUB = bIsInstancedStereoEnabled || bIsMobileMultiViewEnable;

		bIsMobileMultiViewEnable = false;
	}

	void SceneView::setupCommonViewConstantBufferParameters(ViewConstantShaderParameters& viewConstantShaderParameters, const int2& bufferSize, int32 numMSAASamplers, const IntRect& effectiveViewRect, const ViewMatrices& inViewMatrices, const ViewMatrices& inPreViewMatrices) const
	{
		viewConstantShaderParameters.TranslatedWorldToClip = inViewMatrices.getTranslatedViewProjectionMatrix();
		viewConstantShaderParameters.WorldToClip = inViewMatrices.getViewProjectionMatrix();
		viewConstantShaderParameters.TranslatedWorldToView = inViewMatrices.getOverriddenTranlatedViewMatrix();
		viewConstantShaderParameters.ViewToTranslatedWorld = inViewMatrices.getOverriddenInvTranslatedViewMatrix();
		viewConstantShaderParameters.TranslatedWorldToCameraView = inViewMatrices.getTranslatedViewMatrix();
		viewConstantShaderParameters.CameraViewToTranslatedWorld = inViewMatrices.getInvTranslatedViewMatrix();
		viewConstantShaderParameters.ViewToClip = inViewMatrices.getProjectionMatrix();
		viewConstantShaderParameters.ClipToView = inViewMatrices.getInvProjectionMatrix();
		viewConstantShaderParameters.ClipToTranslatedWorld = inViewMatrices.getInvTranslatedViewProjectionMatrix();
		viewConstantShaderParameters.ViewForward = inViewMatrices.getOverriddenTranlatedViewMatrix().getColomn(2);
		viewConstantShaderParameters.ViewUp = inViewMatrices.getOverriddenTranlatedViewMatrix().getColomn(1);
		viewConstantShaderParameters.ViewRight = inViewMatrices.getOverriddenTranlatedViewMatrix().getColomn(0);
		viewConstantShaderParameters.InvDeviceZToWorldZTransform = mInvDeviceZToWorldZTransform;
		viewConstantShaderParameters.WorldViewOrigin = inViewMatrices.getOverriddenInvTranslatedViewMatrix().transformPosition(float3(0)) - inViewMatrices.getPreviewTranslation();
		viewConstantShaderParameters.WorldCameraOrigin = inViewMatrices.getViewOrigin();

		viewConstantShaderParameters.TranslatedWorldCameraOrigin = inViewMatrices.getViewOrigin() + inViewMatrices.getPreviewTranslation();

		viewConstantShaderParameters.PreViewTranslation = inViewMatrices.getPreviewTranslation();

		viewConstantShaderParameters.Random = Math::rand();
		viewConstantShaderParameters.GameTime = mFamily->mCurrentWorldTime;
		viewConstantShaderParameters.RealTime = mFamily->mCurrentRealTime;
		viewConstantShaderParameters.ScreenToTranslatedWorld = Matrix(
			Plane(1, 0, 0, 0),
			Plane(0, 1, 0, 0),
			Plane(0, 0, mProjectionMatrixUnadjustedForRHI.M[2][2], 1),
			Plane(0, 0, mProjectionMatrixUnadjustedForRHI.M[3][2], 0)) * inViewMatrices.getInvTranslatedViewProjectionMatrix();
		setupViewRectConstantBufferParameters(viewConstantShaderParameters, bufferSize, effectiveViewRect, inViewMatrices, inPreViewMatrices);
	}

	void SceneView::setupViewRectConstantBufferParameters(ViewConstantShaderParameters& viewConstantShaderParameters, const int2& inBufferSize, const IntRect& inEffectiveViewRect, const ViewMatrices& inViewMatrices, const ViewMatrices& inPrevViewMatrice) const
	{
		BOOST_ASSERT(inEffectiveViewRect.area() > 0);
		const float invBufferSizeX = 1.0f / inBufferSize.x;
		const float invBufferSizeY = 1.0f / inBufferSize.y;

		const float4 screenPositionScaleBias(
			inEffectiveViewRect.width() * invBufferSizeX / 2.0f,
			inEffectiveViewRect.height() * invBufferSizeY / (-2.0f * GProjectionSignY),
			(inEffectiveViewRect.height() / 2.0f + inEffectiveViewRect.min.y) * invBufferSizeY,
			(inEffectiveViewRect.width() / 2.0f + inEffectiveViewRect.min.x) * invBufferSizeX
		);
		viewConstantShaderParameters.ScreenPositionScaleBias = screenPositionScaleBias;
		viewConstantShaderParameters.ViewRectMin = float4(inEffectiveViewRect.min.x, inEffectiveViewRect.min.y, 0.0f, 0.0f);
		viewConstantShaderParameters.ViewSizeAndInvSize = float4(inEffectiveViewRect.width(), inEffectiveViewRect.height(), 1.0f / inEffectiveViewRect.width(), 1.0f / inEffectiveViewRect.height());
		viewConstantShaderParameters.BufferSizeAndInvSize = float4(inBufferSize.x, inBufferSize.y, invBufferSizeX, invBufferSizeY);

		float2 oneScenePixelUVSize = float2(1.0f / inBufferSize.x, 1.0f / inBufferSize.y);
		float4 sceneTexMinMax(((float)inEffectiveViewRect.min.x / inBufferSize.x), ((float)inEffectiveViewRect.min.y / inBufferSize.y), (((float)inEffectiveViewRect.max.x / inBufferSize.x) - oneScenePixelUVSize.x), (((float)inEffectiveViewRect.max.y / inBufferSize.y) - oneScenePixelUVSize.y));
		viewConstantShaderParameters.SceneTextureMinMax = sceneTexMinMax;
		//viewConstantShaderParameters.MotionBlurNormalizedToPixel = f
		{
			float mx = 2.0f * viewConstantShaderParameters.ViewSizeAndInvSize.z;
			float my = -2.0f * viewConstantShaderParameters.ViewSizeAndInvSize.w;
			float ax = -1.0f - 2.0f * inEffectiveViewRect.min.x * viewConstantShaderParameters.ViewSizeAndInvSize.z;
			float ay = 1.0f + 2.0f * inEffectiveViewRect.min.y * viewConstantShaderParameters.ViewSizeAndInvSize.w;

			viewConstantShaderParameters.SVPositionToTranslatedWorld = Matrix(Plane(mx, 0, 0, 0),
				Plane(0, my, 0, 0),
				Plane(0, 0, 1, 0),
				Plane(ax, ay, 0, 1)) * inViewMatrices.getInvTranslatedViewProjectionMatrix();

			viewConstantShaderParameters.ScreenToWorld = Matrix(
				Plane(1, 0, 0, 0),
				Plane(0, 1, 0, 0),
				Plane(0, 0, mProjectionMatrixUnadjustedForRHI.M[2][2], 1),
				Plane(0, 0, mProjectionMatrixUnadjustedForRHI.M[3][2], 0)) * inViewMatrices.getInvViewProjectionMatrix();
		}

	}

	EShaderPlatform SceneView::getShaderPlatform() const
	{
		return GShaderPlatformForFeatureLevel[getFeatureLevel()];
	}


}