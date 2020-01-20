#include "DemoViewportClient.h"
#include "Classes/Engine/WorldContext.h"
#include "Classes/Engine/Engine.h"
#include "CoreGlobals.h"
#include "SceneView.h"
#include "SceneViewExtension.h"
#include "EngineModule.h"
#include "RendererModule.h"
#include "Math/Matrix.hpp"
#include "Classes/Engine/LocalPlayer.h"
#include "Classes/GameFramework/PlayerController.h"
#include "Classes/Engine/GameInstance.h"
#include "RenderingThread.h"
#include "Framework/Application/SlateApplication.h"
#include "AirEngine.h"

#include "SimpleReflection.h"
namespace Air
{
	DemoViewportClient::DemoViewportClient(const ObjectInitializer& objectInitializer /* = ObjectInitializer::get() */)
		:Object(objectInitializer),
		mNearPlane(0.5f),
		mViewFOV(90.0f)
	{
		mMouseCaptureMode = EMouseCaptureMode::CaptureDuringRightMouseDown;
	}




	ViewportCameraTransform::ViewportCameraTransform()
		:mViewLocation(float3::Zero),
		mViewRotation(Rotator::ZeroRotator),
		mDesiredLocation(float3::Zero),
		mLookAt(float3::Zero),
		mStartLocation(float3::Zero),
		mOrthoZoom(DEFAULT_ORTHOZOOM)
	{

	}

	void DemoViewportClient::init(struct WorldContext& inWorldContext, GameInstance* owningGameInstance)
	{
		inWorldContext.addRef(mWorld);
		mGameInstance = owningGameInstance;
	}

	std::shared_ptr<LocalPlayer> DemoViewportClient::setupInitialLocalPlayer(wstring& outError)
	{
		GameInstance* viewportGameInstance = GEngine->getWorldContextFromViewportChecked(this).mOwningGameInstance.get();
		if (viewportGameInstance == nullptr)
		{
			return std::shared_ptr<LocalPlayer>();
		}
		return viewportGameInstance->createInitialPlayer(outError);
	}

	void DemoViewportClient::draw(Viewport* viewport, Canvas* canvas)
	{
		BOOST_ASSERT(canvas);
		World* myWorld = getWorld();
		SceneViewFamilyContext viewFamily(SceneViewFamily::ConstructionValues(
			viewport,
			myWorld->mScene,
			mEngineShowFlags).setRealtimeUpdates(true));
		if (GEngine->mViewExtensions.size())
		{
			viewFamily.mViewExtensions.append(GEngine->mViewExtensions.getData(), GEngine->mViewExtensions.size());
		}
		for (auto& viewExt : viewFamily.mViewExtensions)
		{
			viewExt->setupViewFamily(viewFamily);
		}
		engineShowFlagOrthographicOverride(true, viewFamily.mEngineShowFlags);

		const bool bEnableStereo = GEngine->isStereoscopic3D(viewport);

		TMap<LocalPlayer*, SceneView*> playerViewMap;

		for (LocalPlayerIterator iterator(GEngine.get(), myWorld); iterator; ++iterator)
		{
			std::shared_ptr<LocalPlayer>& localPlayer = *iterator;
			if (localPlayer)
			{
				APlayerController* playerController = localPlayer->mPlayerComtroller.get();
				const int32 numViews = bEnableStereo ? 2 : 1;
				for (int32 i = 0; i < numViews; i++)
				{
					float3 viewLocation;
					Rotator viewRotation;
					EStereoscopicPass passType;
					if (!bEnableStereo)
					{
						passType = eSSP_FULL;
					}
					else if (i == 0)
					{
						passType = eSSP_LEFT_EYE;
					}
					else if (i == 1)
					{
						passType = eSSP_RIGHT_EYE;
					}
					else
					{
						passType = eSSP_MONOSCROPIC_EYE;
					}

					SceneView* view = localPlayer->calcSceneView(&viewFamily, viewLocation, viewRotation, viewport, nullptr, passType);
					if (view)
					{
						if (i == 0)
						{
							localPlayer->mLastViewLocation = viewLocation;
							playerViewMap.emplace(localPlayer.get(), view);
						}
					}
				}
			}
		}
		finalizeViews(&viewFamily, playerViewMap);

		myWorld->updateLevelStreaming();
		
		if (playerViewMap.size() > 0)
		{
			getRendererModule().beginRenderingViewFamily(canvas, &viewFamily);
		}
		else
		{
			ENQUEUE_RENDER_COMMAND(GameViewportClient_FlushRHIResources)([](RHICommandListBase& cmdList)
				{
					RHICommandListExecutor::getImmediateCommandList().immediateFlush(EImmediateFlushType::FlushRHIThreadFlushResources);
				});
		}
		
	}

	SceneView* DemoViewportClient::calcSceneView(SceneViewFamily* viewFamily, const EStereoscopicPass stereoPass)
	{
		const bool bStereoRendering = stereoPass != stereoPass;
		SceneViewInitOptions viewInitOptions;
		ViewportCameraTransform& viewTransform = getViewTransform(); 
		viewInitOptions.mViewOrigin = viewTransform.getLocation();
		Rotator viewRotation = viewTransform.getRotation();

		const int2 viewportSizeXY = mViewport->getSizeXY();
		IntRect viewRect = IntRect(0, 0, viewportSizeXY.x, viewportSizeXY.y);
		viewInitOptions.setViewRectangle(viewRect);
		const bool bConstrainAspectRatio = bUseControllingActorViewInfo && false;
		mTimeForForceRedraw = 0.0;

		const EAspectRatioAxisConstraint aspectRatioAxisConstraint = EAspectRatioAxisConstraint::AspectRatio_MaintainXFOV;
		WorldSettings* worldSetting = nullptr;
		if (getScene() != nullptr && getScene()->getWorld() != nullptr)
		{
			worldSetting = getScene()->getWorld()->getWorldSettings();
		}
		if (worldSetting != nullptr)
		{
			viewInitOptions.mWorldToMetersScale = worldSetting->mWorldToMeters;
		}
		if (bUseControllingActorViewInfo)
		{

		}
		else
		{
			if (true)
			{
				float actualFOV = mViewFOV;
				if (bStereoRendering)
				{

				}
				if (bUseingOrbitCamera)
				{

				}
				else
				{
					viewInitOptions.mViewRotationMatrix = InverseRotationMatrix(viewRotation);
				}
				if (bStereoRendering)
				{

				}
				else
				{
					const float minZ = getNearClipPlane();
					const float maxZ = minZ;
					const float matrixFOV = Math::max(0.001f, actualFOV) * (float)PI / 360.0f;
					if (bConstrainAspectRatio)
					{

					}
					else
					{
						float XAxisMultiplier;
						float YAxisMultiplier;
						if (((viewportSizeXY.x > viewportSizeXY.y) && (aspectRatioAxisConstraint == AspectRatio_MajorAxisFOV)) || (aspectRatioAxisConstraint == AspectRatio_MaintainXFOV))
						{
							XAxisMultiplier = 1.0f;
							YAxisMultiplier = viewportSizeXY.x / (float) viewportSizeXY.y;
						}
						else
						{
							XAxisMultiplier = viewportSizeXY.y / (float)viewportSizeXY.x;
							YAxisMultiplier = 1.0f;
						}
						if ((bool)ERHIZBuffer::IsInverted)
						{
							viewInitOptions.mProjectionMatrix = ReversedZPerspectiveMatrix(matrixFOV, matrixFOV, XAxisMultiplier, YAxisMultiplier, minZ, maxZ);
						}
						else
						{
							viewInitOptions.mProjectionMatrix = PerspectiveMatrix(matrixFOV, matrixFOV, XAxisMultiplier, YAxisMultiplier, minZ, maxZ);
						}
					}
				}
			}
			else
			{

			}
			if (bConstrainAspectRatio)
			{
				//viewInitOptions.setConstrainedViewRectangle(mViewport->calculateViewExtents(maspa))
			}
		}
		if (bStereoRendering)
		{

		}
		viewInitOptions.mViewFamily = viewFamily;
		viewInitOptions.mSceneViewStateInterface = ((stereoPass != eSSP_RIGHT_EYE) ? mViewState.getReference() : mStereoViewState.getReference());
		viewInitOptions.mStereoPass = stereoPass;

		//viewInitOptions.mViewElementDrawer = this;

		viewInitOptions.mBackgroundColor = getBackgroundColor();
		viewInitOptions.OverrideFarClippingPlaneDistance = mFarPlane;
		viewInitOptions.mCursorPos = int2::zero();

		SceneView* view = new SceneView(viewInitOptions);
		view->mViewLocation = viewTransform.getLocation();
		view->mViewRotation = viewRotation;
		viewFamily->mViews.add(view);
		//view->startFinalPostprocessSettings(view->mViewLocation);
		if (bUseControllingActorViewInfo)
		{

		}
		else
		{
			
		}
		//view ->endfin
		for (int viewExt = 0; viewExt < viewFamily->mViewExtensions.size(); viewExt++)
		{
			viewFamily->mViewExtensions[viewExt]->setupView(*viewFamily, *view);
		}
		return view;
	}

	void DemoViewportClient::setupViewForRendering(SceneViewFamily& viewFamily, SceneView& view)
	{
		if (viewFamily.mEngineShowFlags.Wireframe)
		{
			view.mDiffuseOverrideParameter = float4(0.f, 0.f, 0.f, 0.f);
			view.mSpecularOverrideParameter = float4(0.f, 0.f, 0.f, 0.f);
		}

	}
	SceneInterface* DemoViewportClient::getScene() const
	{
		World* world = getWorld();
		if (world)
		{
			return world->mScene;
		}
		return nullptr;
	}
	float DemoViewportClient::getNearClipPlane() const
	{
		return (mNearPlane < 0.0f) ? GNearClippingPlane : mNearPlane;
	}

	LinearColor DemoViewportClient::getBackgroundColor() const
	{
		LinearColor backgroundColor = Color(55, 55, 55);
		return backgroundColor;
	}

	bool DemoViewportClient::inputAxis(Viewport* viewport, int32 controllerId, Key key, float delta, float deltaTime, int32 numSamples /* = 1 */, bool bGamepad /* = false */)
	{
		if (ignoreInput())
		{
			return false;
		}
		bool bResult = false;

		LocalPlayer* const targetPlayer = GEngine->getLocalPlayerFromControllerId(this, controllerId);
		if (targetPlayer && targetPlayer->mPlayerComtroller)
		{
			bResult = targetPlayer->mPlayerComtroller->inputAxis(key, delta, deltaTime, numSamples, bGamepad);
		}
		return bResult;
	}

	bool DemoViewportClient::inputKey(Viewport* viewport, int32 controllerId, Key key, EInputEvent e, float amountDepressed /* = 1.0f */, bool bGamepad /* = false */)
	{
		bool result = false;
		LocalPlayer* const targetPlayer = GEngine->getLocalPlayerFromControllerId(this, controllerId);
		if (targetPlayer && targetPlayer->mPlayerComtroller)
		{
			result = targetPlayer->mPlayerComtroller->inputKey(key, e, amountDepressed, bGamepad);
		}
		return result;
	}

	EMouseCursor::Type DemoViewportClient::getCursor(Viewport* viewport, int32 x, int32 y)
	{
		if (!SlateApplication::get().isActive() || (!viewport->hasMouseCapture() && !viewport->hasFocus()))
		{
			return EMouseCursor::Default;
		}
		else if (mGameInstance && mGameInstance->getFirstLocalPlayerController())
		{
			return mGameInstance->getFirstLocalPlayerController()->getMouseCursor();
		}
		return ViewportClient::getCursor(viewport, x, y);
	}

	DECLARE_SIMPLER_REFLECTION(DemoViewportClient)
}