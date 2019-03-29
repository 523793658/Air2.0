#include "PlayerCameraManager.h"
#include "Classes/GameFramework/PlayerController.h"
#include "Classes/Engine/World.h"
#include "Classes/GameFramework/Pawn.h"
#include "Classes/Camera/CameraActor.h"
#include "SimpleReflection.h"
namespace Air
{
	bool TViewTarget::equal(const TViewTarget& otherTarget) const
	{
		return (mTarget == otherTarget.mTarget) && (mPlayerState == otherTarget.mPlayerState) && mPOV.equals(otherTarget.mPOV);
	}

	void TViewTarget::setNewTarget(AActor* newTarget)
	{
		mTarget = newTarget;
	}



	PlayerCameraManager::PlayerCameraManager(const ObjectInitializer& objectInitializer)
		: ParentType(objectInitializer)
		, bGameCameraCutThisFrame(false)
		, mDefaultFOV(60.0f)
		, mDefaultAspectRatio(1.333333f)
	{
		static wstring name_default(TEXT("Default"));
		bHidden = true;
		mTransformComponent = createDefaultSubObject<SceneComponent>(TEXT("TransformComponent0"));
		mRootComponent = mTransformComponent;
		mViewPitchMin = -89.9f;
		mViewPitchMax = 89.9f;
		mViewYawMin = 0.f;
		mViewYawMax = 359.999f;
		mViewRollMin = -89.9f;
		mViewRollMax = 89.9f;

	}

	AActor* PlayerCameraManager::getViewTarget() const
	{
		PlayerCameraManager* const nonConstThis = const_cast<PlayerCameraManager*>(this);
		return mViewTarget.mTarget;
	}

	void PlayerCameraManager::setViewTarget(class AActor* newTarget)
	{
		if (newTarget == nullptr)
		{
			newTarget = mPCOwner;
		}
		mViewTarget.checkViewTarget(mPCOwner);

		if (newTarget != mViewTarget.mTarget)
		{
			assignViewTarget(newTarget, mViewTarget);
			mViewTarget.checkViewTarget(mPCOwner);
		}
	}

	void PlayerCameraManager::assignViewTarget(AActor* newTarget, TViewTarget& vt)
	{
		if (!newTarget || (newTarget == vt.mTarget))
		{
			return;
		}

		AActor* oldViewTarget = vt.mTarget;
		vt.mTarget = newTarget;
		vt.mPOV.mAspectRatio = mDefaultAspectRatio;
		vt.mPOV.mFOV = mDefaultFOV;

		if (oldViewTarget)
		{
			oldViewTarget->endViewTarget(mPCOwner);
		}
		vt.mTarget->becomeViewTarget(mPCOwner);
		//if(!mPCOwner->bIsLocalPlayerController && (metmo))
	}

	void PlayerCameraManager::initializeFor(class APlayerController* pc)
	{
		mCameraCache.mPOV.mFOV = mDefaultFOV;
		mPCOwner = pc;
		setViewTarget(pc);
		updateCamera(0.f);
	}

	void PlayerCameraManager::udpateViewTarget(TViewTarget& outVT, float deltaTime)
	{
		MinimalViewInfo origPOV = outVT.mPOV;
		outVT.mPOV.mFOV = mDefaultFOV;
		outVT.mPOV.mOrthoWidth = mDefaultOrthoWidth;
		outVT.mPOV.bConstrainAspectRatio = false;
		outVT.mPOV.bUseFieldOfViewForLOD = true;
		outVT.mPOV.mProjectonMode = bIsOrthographic ? ECameraProjectionMode::Orthographic : ECameraProjectionMode::Perspective;
		bool bDoNotApplyModifiers = false;
		if (ACameraActor* camActor = dynamic_cast<ACameraActor*>(outVT.mTarget))
		{

		}
		else
		{
			static const wstring NAME_Fixed = TEXT("Fixed");
			static const wstring NAME_ThirdPerson = TEXT("ThirdPerson");
			static const wstring NAME_FreeCam = TEXT("FreeCam");
			static const wstring NAME_FreeCam_Default = TEXT("FreeCam_Default");
			static const wstring NAME_FirstPerson = TEXT("FirstPerson");

			if (mCameraStyle == NAME_Fixed)
			{
				outVT.mPOV = origPOV;
				bDoNotApplyModifiers = true;
			}
			else
			{
				updateViewTargetInternal(outVT, deltaTime);
			}
		}
		if (!bDoNotApplyModifiers)
		{

		}

		setActorLocationAndRotation(outVT.mPOV.mLocation, outVT.mPOV.mRotation, false);
		
	}

	void PlayerCameraManager::doUpdateCamera(float deltaTime)
	{
		MinimalViewInfo newPOV = mViewTarget.mPOV;
		mViewTarget.checkViewTarget(mPCOwner);
		udpateViewTarget(mViewTarget, deltaTime);
		
		newPOV = mViewTarget.mPOV;

		
		fillCameraCache(newPOV);

	}

	void PlayerCameraManager::updateViewTargetInternal(TViewTarget& outVT, float deltaTime)
	{
		if (outVT.mTarget)
		{
			const bool bK2Camera = false;
			if (!bK2Camera)
			{
				outVT.mTarget->calcCamera(deltaTime, outVT.mPOV);
			}
		}
	}

	void PlayerCameraManager::fillCameraCache(const MinimalViewInfo& newInfo)
	{
		if (mCameraCache.mTimeStamp != getWorld()->mTimeSeconds)
		{
			mLastCameraCache = mCameraCache;
		}
		mCameraCache.mTimeStamp = getWorld()->mTimeSeconds;
		mCameraCache.mPOV = newInfo;
	}

	void PlayerCameraManager::updateCamera(float deltaTime)
	{
		if (mPCOwner->mPlayer && mPCOwner->isLocalPlayerController())
		{
			doUpdateCamera(deltaTime);
		}
	}

	class APawn* PlayerCameraManager::getViewTargetPawn() const
	{
		PlayerCameraManager* const nonConstThis = const_cast<PlayerCameraManager*>(this);
		if (mPendingViewTarget.mTarget)
		{
			nonConstThis->mPendingViewTarget.checkViewTarget(nonConstThis->mPCOwner);
			if (mPendingViewTarget.mTarget)
			{
				return mPendingViewTarget.getTargetPawn();
			}
		}
		nonConstThis->mViewTarget.checkViewTarget(nonConstThis->mPCOwner);
		return mViewTarget.getTargetPawn();
	}


	void TViewTarget::checkViewTarget(APlayerController* owningController)
	{
		BOOST_ASSERT(owningController);
		if (mTarget == nullptr)
		{
			mTarget = owningController;
		}

		if (mTarget == owningController)
		{
			mPlayerState = nullptr;
		}
		if ((mTarget == nullptr) || mTarget->isPendingKill())
		{
			if (owningController->getPawn() && !owningController->getPawn()->isPendingKill())
			{
				owningController->mPlayerCameraManager->assignViewTarget(owningController->getPawn(), *this);
			}
			else
			{
				owningController->mPlayerCameraManager->assignViewTarget(owningController, *this);
			}
		}
	}

	APawn* TViewTarget::getTargetPawn() const
	{
		if (APawn* pawn = dynamic_cast<APawn*>(mTarget))
		{
			return pawn;
		}
		else if (AController* controller = dynamic_cast<AController*>(mTarget))
		{
			return controller->getPawn();
		}
		else
		{
			return nullptr;
		}
	}

	float PlayerCameraManager::getFOVAngle() const
	{
		return (mLockedFOV > 0.f) ? mLockedFOV : mCameraCache.mPOV.mFOV;
	}

	void PlayerCameraManager::getCameraViewPoint(float3 & outCamLoc, Rotator& outCamRot) const
	{
		outCamLoc = mCameraCache.mPOV.mLocation;
		outCamRot = mCameraCache.mPOV.mRotation;
	}

	void PlayerCameraManager::limitViewPitch(Rotator& viewRotation, float inViewPitchMin, float inViewPitchMax)
	{
		viewRotation.mPitch = Math::clampAngle(viewRotation.mPitch, inViewPitchMin, inViewPitchMax);
		viewRotation.mPitch = Rotator::clampAxis(viewRotation.mPitch);
	}

	void PlayerCameraManager::limitViewYaw(Rotator& viewRotation, float inViewYawMin, float inViewYawMax)
	{
		viewRotation.mYaw = Math::clampAngle(viewRotation.mYaw, inViewYawMin, inViewYawMax);
		viewRotation.mYaw = Rotator::clampAxis(viewRotation.mYaw);
	}

	void PlayerCameraManager::limitViewRoll(Rotator& viewRotation, float inViewRollMin, float inViewRollMax)
	{
		viewRotation.mRoll = Math::clampAngle(viewRotation.mRoll, inViewRollMin, inViewRollMax);
		viewRotation.mRoll = Rotator::clampAxis(viewRotation.mRoll);
	}

	void PlayerCameraManager::processViewRotation(float deltaTime, Rotator& outViewRotation, Rotator& outDeltaRot)
	{
		outViewRotation += outDeltaRot;
		outDeltaRot = Rotator::ZeroRotator;
		limitViewPitch(outViewRotation, mViewPitchMin, mViewPitchMax);
		limitViewRoll(outViewRotation, mViewRollMin, mViewRollMax);
		limitViewYaw(outViewRotation, mViewYawMin, mViewYawMax);
	}

	DECLARE_SIMPLER_REFLECTION(PlayerCameraManager);
}