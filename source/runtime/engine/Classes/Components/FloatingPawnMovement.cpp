#include "Classes/Components/FloatingPawnMovement.h"
#include "Classes/Components/PawnMovementComponent.h"
#include "Classes/GameFramework/Controller.h"
#include "Classes/GameFramework/Pawn.h"
#include "Classes/GameFramework/WorldSettings.h"
#include "SimpleReflection.h"

namespace Air
{
	FloatingPawnMovement::FloatingPawnMovement(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		mMaxSpeed = 12.f;
		mAcceleration = 40.f;
		mDeceleration = 80.f;
		mTurningBoost = 8.0f;
		bPositionCorrected = false;
	}

	void FloatingPawnMovement::tickComponent(float deltaTime, enum ELevelTick tickType, ActorComponentTickFunction* thisTickFunction)
	{
		if (shouldSkipUpdate(deltaTime))
		{
			return;
		}

		ParentType::tickComponent(deltaTime, tickType, thisTickFunction);

		if (!mPawnOwner || !mUpdatedComponent)
		{
			return;
		}

		const AController* controller = mPawnOwner->getController();

		if (controller && controller->isLocalController())
		{
			if (controller->isLocalPlayerController() == true)
			{
				applyControlInputToVelocity(deltaTime);
			}
			limitWorldBounds();
			bPositionCorrected = false;
			float3 delta = mVelocity * deltaTime;

			if (!delta.isNearlyZero(1e-6f))
			{
				const float3 oldLocation = mUpdatedComponent->getComponentLocation();
				const Quaternion rotation = mUpdatedComponent->getComponentQuat();

				safeMoveUpdatedComponent(delta, rotation, true);

				if (!bPositionCorrected)
				{
					const float3 newLocation = mUpdatedComponent->getComponentLocation();
					mVelocity = ((newLocation - oldLocation) / deltaTime);
				}
			}

			updateComponentVelocity();
		}
	}

	bool FloatingPawnMovement::limitWorldBounds()
	{
		WorldSettings* worldSettings = mPawnOwner ? mPawnOwner->getWorldSettings() : nullptr;
		if (!worldSettings || !worldSettings->bEnableWorldBoundsCheck || !mUpdatedComponent)
		{
			return false;
		}

		float3 currentLocation = mUpdatedComponent->getComponentLocation();
		if (currentLocation.y < worldSettings->mKillY)
		{
			mVelocity.y = Math::min(getMaxSpeed(), worldSettings->mKillY - currentLocation.y + 0.002f);
			return true;
		}
		return false;
	}

	void FloatingPawnMovement::applyControlInputToVelocity(float deltaTime)
	{
		const float3 controlAcceleration = getPendingInputVector().getClampedToMaxSize(1.f);
		const float analogInputModifier = (controlAcceleration.sizeSquared() > 0.f ? controlAcceleration.size() : 0.f);
		const float maxPawnSpeed = getMaxSpeed() * analogInputModifier;
		const bool bExceedingMaxSpeed = isExceedingMaxSpeed(maxPawnSpeed);
		if (analogInputModifier > 0.f && !bExceedingMaxSpeed)
		{
			if (mVelocity.sizeSquared() > 0.f)
			{
				const float timeScale = Math::clamp(deltaTime * mTurningBoost, 0.f, 1.0f);
				mVelocity = mVelocity + (controlAcceleration * mVelocity.size() - mVelocity) * timeScale;
			}
		}
		else
		{
			if (mVelocity.sizeSquared() > 0.f)
			{
				const float3 oldVelocity = mVelocity;
				const float velSize = Math::max(mVelocity.size() - Math::abs(mDeceleration) * deltaTime, 0.f);
				mVelocity = mVelocity.getSafeNormal() * velSize;

				if (bExceedingMaxSpeed && mVelocity.sizeSquared() < Math::square(maxPawnSpeed))
				{
					mVelocity = oldVelocity.getSafeNormal() * maxPawnSpeed;
				}
			}
		}
		const float newMaxSpeed = (isExceedingMaxSpeed(maxPawnSpeed)) ? mVelocity.size() : maxPawnSpeed;
		mVelocity += controlAcceleration * Math::abs(mAcceleration) * deltaTime;
		mVelocity = mVelocity.getClampedToMaxSize(newMaxSpeed);
		consumeInputVector();
	}

	DECLARE_SIMPLER_REFLECTION(FloatingPawnMovement);
}



