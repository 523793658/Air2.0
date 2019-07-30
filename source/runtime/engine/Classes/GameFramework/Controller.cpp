#include "Classes/GameFramework/PlayerController.h"
#include "Classes/GameFramework/Pawn.h"
#include "Classes/Engine/World.h"
#include "SimpleReflection.h"
namespace Air
{
	AController::AController(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		mPrimaryActorTick.bCanEverTick = true;
		bHidden = true;
		mTransformComponent = createDefaultSubObject<SceneComponent>(TEXT("TransformComponent0"));
		mRootComponent = mTransformComponent;
		bAttachToPawn = false;
		bIsPlayerController = false;
		if (mRootComponent)
		{
			mRootComponent->bAbsoluteRotation = true;
		}
	}

	bool AController::isLocalController() const
	{
		const ENetMode netMode= getNetMode();
		if (netMode == NM_Standalone)
		{
			return true;
		}

		if (netMode == NM_Client && mRole == ROLE_AutonomousProxy)
		{
			return true;
		}

		return false;
	}

	void AController::initPlayerState()
	{

	}

	EStateName AController::getStateName() const
	{
		return mStateName;
	}

	bool AController::isInState(EStateName inStateName) const
	{
		return mStateName == inStateName;
	}

	void AController::possess(APawn* inPawn)
	{
		if (!hasAuthority())
		{
			return;
		}
		if (inPawn != nullptr)
		{
			if (getPawn() && getPawn() != inPawn)
			{
				unPossess();
			}

			if (inPawn->mController != nullptr)
			{
				inPawn->mController->unPossess();
			}
			inPawn->possessedBy(this);
			setPawn(inPawn);
			setControlRotation(mPawn->getActorRotation());
			mPawn->restart();
		}
	}

	void AController::unPossess()
	{
		if (mPawn != nullptr)
		{
			mPawn->unPossessed();
			setPawn(nullptr);
		}
	}

	Rotator AController::getControllerRotation() const
	{
		return mControlRotation;
	}

	void AController::setControlRotation(const Rotator& newRotation)
	{
		if (!mControlRotation.equals(newRotation, 1e-3f))
		{
			mControlRotation = newRotation;
			if (mRootComponent && mRootComponent->bAbsoluteRotation)
			{
				mRootComponent->setWorldRotation(getControllerRotation());
			}
		}
	}

	void AController::setPawn(APawn* inPawn)
	{
		removePawnTickDependency(mPawn);
		mPawn = std::dynamic_pointer_cast<APawn>(inPawn->shared_from_this());
		
		attachToPawn(inPawn);

		addPawnTickDependency(mPawn.get());
	}

	void AController::removePawnTickDependency(std::shared_ptr<APawn> inOldPawn)
	{
		if (inOldPawn != nullptr)
		{
			PawnMovementComponent* pawnMovement = inOldPawn->getMovementComponent();
			if (pawnMovement)
			{
				pawnMovement->mPrimaryComponentTick.removePrerequisite(this, this->mPrimaryActorTick);
			}

			inOldPawn->mPrimaryActorTick.removePrerequisite(this, this->mPrimaryActorTick);
		}
	}

	void AController::addPawnTickDependency(APawn* inPawn)
	{
		if (inPawn)
		{
			bool bNeedsPawnPrereq = true;
			PawnMovementComponent* pawnMovement = inPawn->getMovementComponent();
			if (pawnMovement && pawnMovement->mPrimaryComponentTick.bCanEverTick)
			{
				pawnMovement->mPrimaryComponentTick.addPrerequisite(this, this->mPrimaryActorTick);
				if (pawnMovement->bTickBeforeOwner || inPawn->mPrimaryActorTick.getPrerequisites().contains(TickPrerequisite(pawnMovement, pawnMovement->mPrimaryComponentTick)))
				{
					bNeedsPawnPrereq = false;
				}
			}
			if (bNeedsPawnPrereq)
			{
				inPawn->mPrimaryActorTick.addPrerequisite(this, this->mPrimaryActorTick);
			}
		}
	}

	void AController::attachToPawn(APawn* inPawn)
	{
		if (bAttachToPawn && mRootComponent)
		{
			if (inPawn)
			{
				if (inPawn->getRootComponent() && mRootComponent->getAttachParent() != inPawn->getRootComponent())
				{
					mRootComponent->detachFromComponent(DetachmentTransformRules::keepRelativeTranform);
					mRootComponent->setRelativeLocationAndRotation(float3::Zero, Rotator::ZeroRotator);
					mRootComponent->attachToComponent(inPawn->getRootComponent(), AttachmentTransformRules::keepRelativeTransform);
				}
			}
			else
			{
				detachFromPawn();
			}
		}
	}

	void AController::detachFromPawn()
	{
		if (bAttachToPawn && mRootComponent && mRootComponent->getAttachParent() && dynamic_cast<APawn*>(mRootComponent->getAttachmentRootActor()))
		{
			mRootComponent->detachFromComponent(DetachmentTransformRules::keepWorldTransform);
		}
	}

	bool AController::isLookInputIgnored() const
	{
		return false;
	}
	void AController::beginInactiveState()
	{

	}

	void AController::endInactiveState(){}

	void AController::changeState(EStateName newState)
	{
		if (newState != mStateName)
		{
			if (mStateName == Inactive)
			{
				endInactiveState();
			}
			mStateName = newState;
			if (mStateName == Inactive)
			{
				beginInactiveState();
			}
		}
	}

	void AController::getPlayerViewPoint(float3& outLocation, Rotator& outRotation) const
	{
		if (mPawn != nullptr)
		{
			mPawn->getActorEyesViewPoint(outLocation, outRotation);
		}
	}

	void AController::postInitializeComponents()
	{
		ParentType::postInitializeComponents();
		if (!isPendingKill())
		{
			getWorld()->addController(this);
			if (mRootComponent && mRootComponent->bAbsoluteRotation)
			{
				mRootComponent->setWorldRotation(getControllerRotation());
			}
		}
	}

	DECLARE_SIMPLER_REFLECTION(AController);
}