#include "Classes/GameFramework/Pawn.h"
#include "Classes/GameFramework/Controller.h"
#include "Classes/GameFramework/PlayerController.h"
#include "Classes/Components/InputComponent.h"
#include "Classes/Engine/World.h"
#include "Classes/GameFramework/PlayerController.h"
#include "Classes/Camera/PlayerCameraManager.h"
#include "SimpleReflection.h"
namespace Air
{
	APawn::APawn(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		mPrimaryActorTick.bCanEverTick = true;
		mPrimaryActorTick.mTickGroup = TG_PrePhysics;
		bInputEnabled = true;
	}

	PawnMovementComponent* APawn::getMovementComponent() const
	{
		return findComponentByClass<PawnMovementComponent>();
	}

	void APawn::possessedBy(AController* newController)
	{
		std::shared_ptr<AController> const oldController = mController;
		mController = std::dynamic_pointer_cast<AController>(newController->shared_from_this());
		if (mController->mPlayerState != nullptr)
		{
			mPlayerState = mController->mPlayerState;
		}

		if (std::shared_ptr<APlayerController> playerController = std::dynamic_pointer_cast<APlayerController>(mController))
		{

		}
		if (oldController.get() != newController)
		{
			
		}
	}

	void APawn::unPossessed()
	{
		std::shared_ptr<AController> const oldController = mController;
		mPlayerState = nullptr;
		setOwner(nullptr);
		mController = nullptr;
		destroyPlayerInputComponent();
		if (oldController)
		{

		}
		consumeMovementInputVector();
	}

	void APawn::pawnClientRestart()
	{
		restart();
		std::shared_ptr<APlayerController>& pc = std::dynamic_pointer_cast<APlayerController>(mController);
		if (pc && pc->isLocalController())
		{
			if (pc->bAutoManagerActiveCameraTarget)
			{
				pc->autoManageActiveCameraTarget(this);
			}
			if (mInputComponent == nullptr)
			{
				mInputComponent = createPlayerInputComponent();
				if (mInputComponent)
				{
					setupPlayerInputComponent(mInputComponent.get());
					mInputComponent->registerComponent();

				}
			}
		}
	}

	void APawn::restart()
	{
		PawnMovementComponent* movementComponent = getMovementComponent();
		if (movementComponent)
		{
			movementComponent->stopMovementImmediately();
		}
		consumeMovementInputVector();
		mBaseEyeHeight = 0.0f;
	}

	std::shared_ptr<InputComponent> APawn::createPlayerInputComponent()
	{
		static const wstring inputComponentName(TEXT("PawnInputComponent0"));
		return newObject<InputComponent>(this, inputComponentName);
	}

	float3 APawn::consumeMovementInputVector()
	{
		PawnMovementComponent* movementComponent = getMovementComponent();
		if (movementComponent)
		{
			return movementComponent->consumeInputVector();
		}
		else
		{
			return internal_consumeMovementInputVector();
		}
	}

	float3 APawn::internal_consumeMovementInputVector()
	{
		mLastControlInputVector = mControlInputVector;
		mControlInputVector = float3::Zero;
		return mLastControlInputVector;
	}

	void APawn::addControllerPitchInput(float val)
	{
		if (val != 0.f && mController && mController->isLocalPlayerController())
		{
			std::shared_ptr<APlayerController> pc = std::dynamic_pointer_cast<APlayerController>(mController);
			pc->addPitchInput(val);
		}
	}

	void APawn::addControllerYawInput(float val)
	{
		if (val != 0.f && mController && mController->isLocalPlayerController())
		{
			std::shared_ptr<APlayerController> const pc = std::dynamic_pointer_cast<APlayerController>(mController);
			pc->addYawInput(val);
		}
	}

	void APawn::addControllerRollInput(float val)
	{
		if (val != 0.f && mController && mController->isLocalPlayerController())
		{
			std::shared_ptr<APlayerController> const pc = std::dynamic_pointer_cast<APlayerController>(mController);
			pc->addRollInput(val);
		}
	}

	void APawn::addMovementInput(float3 worldDirection, float scaleValue /* = 1.0f */, bool bForce /* = false */)
	{
		PawnMovementComponent* movementComponent = getMovementComponent();
		if (movementComponent)
		{
			movementComponent->addInputVector(worldDirection * scaleValue, bForce);
		}
		else
		{
			internal_addMovementInput(worldDirection * scaleValue, bForce);
		}
	}

	void APawn::internal_addMovementInput(float3 worldAccel, bool bForce /* = false */)
	{
		mControlInputVector += worldAccel;
	}

	float3 APawn::getPawnViewLocation() const
	{
		return getActorLocation() + float3(0.f, mBaseEyeHeight, 0.f);
	}

	Rotator APawn::getViewRotation() const
	{
		if (mController != nullptr)
		{
			return mController->getControllerRotation();
		}
		else if (mRole < ROLE_Authority)
		{
			for (ConstPlayerControllerIterator iterator = getWorld()->getPlayerControllerIterator(); iterator; ++iterator)
			{
				std::shared_ptr<APlayerController> playerController = *iterator;
				if (playerController && playerController->mPlayerCameraManager->getViewTargetPawn() == this)
				{
					return playerController->mTargetViewRotation;
				}
			}
		}
		return getActorRotation();
	}

	void APawn::getActorEyesViewPoint(float3& outLocation, Rotator& outRotation) const
	{
		outLocation = getPawnViewLocation();
		outRotation = getViewRotation();
	}

	void APawn::faceRotation(Rotator& newControlRotation, float deltaTime /* = 0.f */)
	{
		if (bUseControllerRotationPitch || bUseControllerRotationYaw || bUseControllerRotationRoll)
		{
			const Rotator currentRotation = getActorRotation();
			if (!bUseControllerRotationPitch)
			{
				newControlRotation.mPitch = currentRotation.mPitch;
			}
			if (!bUseControllerRotationYaw)
			{
				newControlRotation.mYaw = currentRotation.mYaw;
			}
			if (!bUseControllerRotationRoll)
			{
				newControlRotation.mRoll = currentRotation.mRoll;
			}
			setActorRotation(newControlRotation);
		}
	}

	DECLARE_SIMPLER_REFLECTION(APawn);
}