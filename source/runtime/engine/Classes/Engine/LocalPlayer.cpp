#include "Classes/Engine/LocalPlayer.h"
#include "SceneView.h"
#include "Engine.h"
#include "Classes/Engine/EngineBaseTypes.h"
#include "SceneViewExtension.h"
#include "Classes/GameFramework/PlayerController.h"
#include "Classes/Camera/PlayerCameraManager.h"
#include "Containers/Map.h"
#include "SimpleReflection.h"
#include "Texture.h"
namespace Air
{
	LocalPlayer::LocalPlayer(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
		,mSize(1, 1)
	{
	}
	bool LocalPlayer::calcSceneViewInitOptions(struct SceneViewInitOptions& outInitOptions, Viewport* viewport, class ViewElementDrawer* viewDrawer /* = nullptr */, EStereoscopicPass stereopass /* = eSSP_FULL */)
	{
		if ((mPlayerComtroller == nullptr) || (mSize.x <= 0.0f) || (mSize.y <= 0.0f) || (viewport == nullptr))
		{
			return false;
		}

		if (getProjectionData(viewport, stereopass, outInitOptions) == false)
		{
			return false;
		}

		if (!outInitOptions.isValidViewRectangle())
		{
			return false;
		}

		if (mPlayerComtroller->mPlayerCameraManager != nullptr)
		{
			//if(mPlayerComtroller->mPlayerCameraManager->)
			//if(mPlayerComtroller->mPlayerCameraManager)
			outInitOptions.mInCameraCut = mPlayerComtroller->mPlayerCameraManager->bGameCameraCutThisFrame;
		}
		switch (stereopass)
		{
		case Air::eSSP_FULL:
		case Air::eSSP_LEFT_EYE:
			outInitOptions.mSceneViewStateInterface = mViewState.getReference();
			break;
		case Air::eSSP_RIGHT_EYE:
			outInitOptions.mSceneViewStateInterface = mStereoViewState.getReference();
			break;

		case Air::eSSP_MONOSCROPIC_EYE:
			outInitOptions.mSceneViewStateInterface = mMonoViewState.getReference();
			break;
		}

		outInitOptions.mViewActor = mPlayerComtroller->getViewTarget();
		outInitOptions.mViewElementDrawer = viewDrawer;
		outInitOptions.mBackgroundColor = LinearColor::Black;
		outInitOptions.mLodDistanceFactor = mPlayerComtroller->mLocalPlayerCachedLODDistanceFactor;

		outInitOptions.mStereoPass = stereopass;
		outInitOptions.mWorldToMetersScale = mPlayerComtroller->getWorldSettings()->mWorldToMeters;
		outInitOptions.mCursorPos = int2(-1, -1);
		outInitOptions.mOriginOffsetThisFrame = mPlayerComtroller->getWorld()->mOriginOffsetThisFrame;
		return true;
	}

	bool LocalPlayer::spawnPlayActor(const wstring& url, wstring& outError, World* inWorld)
	{
		BOOST_ASSERT(inWorld);
		if (inWorld->isServer())
		{
			URL playerURL;// (nullptr, url, )
			mPlayerComtroller = inWorld->spawnPlayActor(this, ROLE_SimulatedProxy);
		}
		else
		{
			ActorSpawnParameters spawnInfo;
			mPlayerComtroller = inWorld->spawnActor<APlayerController>(spawnInfo);
			auto players = GEngine->getGamePlayers(inWorld);
			const int32 playerIndex = players.find(this);
			mPlayerComtroller->mNetPlayerIndex = playerIndex;
		}
		return mPlayerComtroller != nullptr;
	}

	SceneView* LocalPlayer::calcSceneView(class SceneViewFamily* viewFamily, float3& outViewLocation, Rotator& outViewRotation, Viewport* viewport, class ViewElementDrawer* viewDrawer /* = nullptr */, EStereoscopicPass stereoPass /* = eSSP_FULL */)
	{
		SceneViewInitOptions viewInitOptions;
		if (!calcSceneViewInitOptions(viewInitOptions, viewport, viewDrawer, stereoPass))
		{
			return nullptr;
		}
		MinimalViewInfo viewInfo;
		getViewPoint(viewInfo, stereoPass);
		outViewLocation = viewInfo.mLocation;
		outViewRotation = viewInfo.mRotation;
		viewInitOptions.mUseFieldOfViewForLOD = viewInfo.bUseFieldOfViewForLOD;

		viewInitOptions.mViewFamily = viewFamily;

		engineShowFlagOrthographicOverride(viewInitOptions.isPerspectiveProjection(), viewFamily->mEngineShowFlags);

		SceneView* const view = new SceneView(viewInitOptions);
		view->mViewLocation = outViewLocation;
		view->mViewRotation = outViewRotation;
		if (mPlayerComtroller->getWorld()->mSkyTexture && mPlayerComtroller->getWorld()->mSkyTexture->mResource)
		{
			view->mSkyTexture = mPlayerComtroller->getWorld()->mSkyTexture->mResource->mTextureRHI;
		}
		
		viewFamily->mViews.push_back(view);

		{
			/*view->startFinalPostprocessSettings(outViewLocation);

			if (mPlayerComtroller->mPlayerCameraManager)
			{
				TArray<
			}*/

		}
		for (int viewExt = 0; viewExt < viewFamily->mViewExtensions.size(); viewExt++)
		{
			viewFamily->mViewExtensions[viewExt]->setupView(*viewFamily, *view);
		}
		return view;
	}


	bool LocalPlayer::getProjectionData(Viewport* viewport, EStereoscopicPass stereoPass, SceneViewProjectionData& projectionData) const
	{
		int2 sizeXY = viewport->getSizeXY();

		if ((viewport == nullptr) || (sizeXY.x == 0) || (sizeXY.y == 0))
		{
			return false;
		}
		int32 x = trunc(mOrigin.x * sizeXY.x);
		int32 y = trunc(mOrigin.y * sizeXY.y);

		uint32 sizeX = trunc(mSize.x * sizeXY.x);
		uint32 sizeY = trunc(mSize.y * sizeXY.y);

		IntRect unconstrainedRectangle = IntRect(x, y, x + sizeX, y + sizeY);
		projectionData.setViewRectangle(unconstrainedRectangle);

		MinimalViewInfo viewInfo;
		getViewPoint(viewInfo, stereoPass);

		const bool bNeedStereo = (stereoPass != eSSP_FULL) && GEngine->isStereoscopic3D();
		if (bNeedStereo)
		{

		}
		float3 stereoViewLocation = viewInfo.mLocation;
		if (bNeedStereo)
		{

		}

		projectionData.mViewOrigin = stereoViewLocation;
		projectionData.mViewRotationMatrix = InverseRotationMatrix(viewInfo.mRotation);
		if (!bNeedStereo)
		{
			MinimalViewInfo::calculateProjectionMatrixGivenView(viewInfo, mAspectRatioAxisConstraint, mViewportClient->mViewport, projectionData);
		}
		else
		{

		}
		return true;
	}

	class LockedViewState
	{
	public:
		static LockedViewState& get()
		{
			static LockedViewState state;
			return state;
		}

		bool getViewPoint(const LocalPlayer * player, float3 & outViewLocation, Rotator& outViewRotation, float& outFOV)
		{
			PlayerState playerState = mPlayerStates[const_cast<LocalPlayer*>(player)];
			if (playerState.bLocked)
			{
				outViewLocation = playerState.mViewPoint.mLocation;
				outViewRotation = playerState.mViewPoint.mRoation;
				outFOV = playerState.mViewPoint.mFOV;
				return true;
			}
			return false;
		}


	private:
		struct ViewPoint
		{
			float3 mLocation;
			float mFOV;
			Rotator mRoation;
		};

		struct PlayerState
		{
			ViewPoint mViewPoint;
			bool bLocked;

			PlayerState()
			{
				mViewPoint.mLocation = float3::Zero;
				mViewPoint.mFOV = 60.0f;
				mViewPoint.mRoation = Rotator::ZeroRotator;
				bLocked = false;
			}

			bool isDefault() const
			{
				return bLocked == false &&
					mViewPoint.mLocation == float3::Zero &&
					mViewPoint.mFOV == 60.0f &&
					mViewPoint.mRoation == Rotator::ZeroRotator;
			}
		};

		TMap<LocalPlayer*, PlayerState> mPlayerStates;

	};


	void LocalPlayer::getViewPoint(MinimalViewInfo& outViewInfo, EStereoscopicPass stereoPass) const
	{
		if (LockedViewState::get().getViewPoint(this, outViewInfo.mLocation, outViewInfo.mRotation, outViewInfo.mFOV) == false && mPlayerComtroller != nullptr)
		{
			if (mPlayerComtroller->mPlayerCameraManager != nullptr)
			{
				outViewInfo = mPlayerComtroller->mPlayerCameraManager->mCameraCache.mPOV;
				outViewInfo.mFOV = mPlayerComtroller->mPlayerCameraManager->getFOVAngle();
				mPlayerComtroller->getPlayerViewPoint(outViewInfo.mLocation, outViewInfo.mRotation);
			}
			else
			{
				mPlayerComtroller->getPlayerViewPoint(outViewInfo.mLocation, outViewInfo.mRotation);
			}
		}

		for (int viewExt = 0; viewExt < GEngine->mViewExtensions.size(); viewExt++)
		{
			GEngine->mViewExtensions[viewExt]->setViewPoint(mPlayerComtroller, outViewInfo);
		}
	}

	DECLARE_SIMPLER_REFLECTION(LocalPlayer);
}