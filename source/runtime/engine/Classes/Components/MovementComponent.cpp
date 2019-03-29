#include "Classes/Components/MovementComponent.h"
#include "Classes/GameFramework/Actor.h"
#include "Classes/Components/PrimitiveComponent.h"
#include "Classes/Engine/World.h"
#include "SimpleReflection.h"
namespace Air
{
	MovementComponent::MovementComponent(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		mPrimaryComponentTick.mTickGroup = TG_PrePhysics;
		mPrimaryComponentTick.bCanEverTick = true;
		bAutoUpdateTickRegistration = true;
		bTickBeforeOwner = true;
		bAutoRegisterUpdatedComponent = true;

		bWantsInitializeComponent = true;

		bAutoActivate = true;
	}

	void MovementComponent::updateComponentVelocity()
	{
		if (mUpdatedComponent)
		{
			mUpdatedComponent->mComponentVelocity = mVelocity;
		}
	}

	bool MovementComponent::isExceedingMaxSpeed(float maxSpeed) const
	{
		maxSpeed = Math::max(0.f, maxSpeed);
		const float maxSpeedSquared = Math::square(maxSpeed);
		const float overVelocityPercent = 1.01f;
		return (mVelocity.sizeSquared() > maxSpeedSquared * overVelocityPercent);
	}

	bool MovementComponent::shouldSkipUpdate(float deltaTime) const
	{
		if (mUpdatedComponent == nullptr)
		{
			return true;
		}

		if (mUpdatedComponent->mMobility != EComponentMobility::Movable)
		{
			return true;
		}
		return false;
	}

	bool MovementComponent::safeMoveUpdatedComponent(const float3& delta, const Quaternion& newRotation, bool bSweep, ETeleportType teleport /* = ETeleportType::None */)
	{
		if (mUpdatedComponent == nullptr)
		{
			return false;
		}

		bool bMoveResult = false;

		{
			bMoveResult = moveUpdatedComponent(delta, newRotation, bSweep, teleport);
		}

		return bMoveResult;
	}

	bool MovementComponent::moveUpdatedComponentImpl(const float3& delta, const Quaternion& newRotation, bool bSweep, ETeleportType teleport /* = ETeleportType::None */)
	{
		if (mUpdatedComponent)
		{
			return mUpdatedComponent->moveComponent(delta, newRotation, bSweep);
		}
		return false;
	}

	void MovementComponent::registerComponentTickFunctions(bool bRegister)
	{
		ParentType::registerComponentTickFunctions(bRegister);
		updateTickRegistration();
		AActor* owner = getOwner();
		if (bTickBeforeOwner && bRegister && mPrimaryComponentTick.bCanEverTick && owner && owner->canEverTick())
		{
			owner->mPrimaryActorTick.addPrerequisite(this, mPrimaryComponentTick);
		}
	}

	void MovementComponent::updateTickRegistration()
	{
		if (bAutoUpdateTickRegistration)
		{
			const bool bHasUpdateComponent = mUpdatedComponent != nullptr;
			setComponentTickEnabled(bHasUpdateComponent && bAutoActivate);
		}
	}

	void MovementComponent::tickComponent(float deltaTime, enum ELevelTick tickType, ActorComponentTickFunction* thisTickFunction)
	{
		ParentType::tickComponent(deltaTime, tickType, thisTickFunction);
		if (mUpdatedComponent != nullptr && mUpdatedComponent->isPendingKill())
		{
			setUpdatedComponent(nullptr);
		}
	}

	void MovementComponent::setUpdatedComponent(SceneComponent* newUpdatedComponent)
	{
		if (mUpdatedComponent && mUpdatedComponent != newUpdatedComponent)
		{
			if (!mUpdatedComponent->isPendingKill())
			{

			}
			mUpdatedComponent->mPrimaryComponentTick.removePrerequisite(this, mPrimaryComponentTick);
		}
		mUpdatedComponent = isValid(newUpdatedComponent) ? newUpdatedComponent : nullptr;
		mUpdatedPrimitive = dynamic_cast<PrimitiveComponent*>(mUpdatedComponent);
		if (mUpdatedComponent && !mUpdatedComponent->isPendingKill())
		{
			mUpdatedComponent->mPrimaryComponentTick.addPrerequisite(this, mPrimaryComponentTick);
		}
		updateTickRegistration();
	}

	void MovementComponent::onRegister()
	{
		mUpdatedPrimitive = dynamic_cast<PrimitiveComponent*>(mUpdatedComponent);
		ParentType::onRegister();
		const World* myWorld = getWorld();
		if (myWorld && myWorld->isGameWorld())
		{
			SceneComponent* newUpdatedComponent = mUpdatedComponent;
			if (!mUpdatedComponent && bAutoRegisterUpdatedComponent)
			{
				AActor* myActor = getOwner();
				if (myActor)
				{
					newUpdatedComponent = myActor->getRootComponent();
				}
			}
			setUpdatedComponent(newUpdatedComponent);
		}
	}

	DECLARE_SIMPLER_REFLECTION(MovementComponent);
}