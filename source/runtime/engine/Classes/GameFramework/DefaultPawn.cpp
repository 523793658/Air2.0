#include "Classes/GameFramework/DefaultPawn.h"
#include "Classes/GameFramework/PlayerController.h"
#include "Classes/Components/FloatingPawnMovement.h"
#include "Classes/Components/SphereComponent.h"
#include "Classes/Components/InputComponent.h"
#include "UserInterface/PlayerInput.h"
#include "Classes/Engine/World.h"
#include "SimpleReflection.h"
namespace Air
{
	wstring ADefaultPawn::mMovementComponentName(TEXT("MovementComponent0"));
	wstring ADefaultPawn::mCollisionComponentName(TEXT("CollisionComponent0"));

	ADefaultPawn::ADefaultPawn(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		mBaseEyeHeight = 0.0f;
		
		mCollisionComponent = createDefaultSubObject<SphereComponent>(ADefaultPawn::mCollisionComponentName);
		mRootComponent = mCollisionComponent;


		
		mMovementComponent = createDefaultSubObject<PawnMovementComponent, FloatingPawnMovement>(ADefaultPawn::mMovementComponentName);



		mMovementComponent->mUpdatedComponent = mRootComponent;

		bAddDefaultMovementBindings = true;
		mBaseTurnRate = 45.0f;
		mBaseLookUpRate = 45.f;
	}

	PawnMovementComponent* ADefaultPawn::getMovementComponent() const
	{
		return mMovementComponent.get();
	}

	void ADefaultPawn::lookUp(float val)
	{
		APlayerController* pc = dynamic_cast<APlayerController*>(getController());
		if (pc)
		{
			pc->addPitchInput(val);
		}
	}

	void ADefaultPawn::turn(float val)
	{
		APlayerController* pc = dynamic_cast<APlayerController*>(getController());
		if (pc)
		{
			pc->addYawInput(val);
		}
	}

	void initDefaultPawnInputBindings()
	{
		static bool bBindingsAdded = false;
		if (!bBindingsAdded)
		{
			bBindingsAdded = true;
			PlayerInput::addEngineDefinedAxisMapping(InputAxisKeyMapping(TEXT("DefaultPawn_MoveForward"), EKeys::W, 1.f));
			PlayerInput::addEngineDefinedAxisMapping(InputAxisKeyMapping(TEXT("DefaultPawn_MoveForward"), EKeys::S, -1.f));
			PlayerInput::addEngineDefinedAxisMapping(InputAxisKeyMapping(TEXT("DefaultPawn_MoveForward"), EKeys::Up, 1.f));
			PlayerInput::addEngineDefinedAxisMapping(InputAxisKeyMapping(TEXT("DefaultPawn_MoveForward"), EKeys::Down, -1.f));

			PlayerInput::addEngineDefinedAxisMapping(InputAxisKeyMapping(TEXT("DefaultPawn_MoveRight"), EKeys::D, 1.f));
			PlayerInput::addEngineDefinedAxisMapping(InputAxisKeyMapping(TEXT("DefaultPawn_MoveRight"), EKeys::A, -1.f));

			PlayerInput::addEngineDefinedAxisMapping(InputAxisKeyMapping(TEXT("DefaultPawn_MoveUp"), EKeys::E, 1.f));
			PlayerInput::addEngineDefinedAxisMapping(InputAxisKeyMapping(TEXT("DefaultPawn_MoveUp"), EKeys::Q, -1.f));

			PlayerInput::addEngineDefinedAxisMapping(InputAxisKeyMapping(TEXT("DefaultPawn_TurnRate"), EKeys::Left, -1.f));
			PlayerInput::addEngineDefinedAxisMapping(InputAxisKeyMapping(TEXT("DefaultPawn_TurnRate"), EKeys::Right, 1.f));
			PlayerInput::addEngineDefinedAxisMapping(InputAxisKeyMapping(TEXT("DefaultPawn_Turn"), EKeys::MouseX, 1.0f));

			PlayerInput::addEngineDefinedAxisMapping(InputAxisKeyMapping(TEXT("DefaultPawn_LookUp"), EKeys::MouseY, -1.f));
		}
	}

	void ADefaultPawn::setupPlayerInputComponent(InputComponent* playerInputComponent)
	{
		BOOST_ASSERT(playerInputComponent);
		if (bAddDefaultMovementBindings)
		{
			initDefaultPawnInputBindings();
			playerInputComponent->bindAxis(TEXT("DefaultPawn_MoveForward"), std::bind(&ADefaultPawn::moveForward, this, std::placeholders::_1));
			playerInputComponent->bindAxis(TEXT("DefaultPawn_MoveRight"), std::bind(&ADefaultPawn::moveRight, this, std::placeholders::_1));
			playerInputComponent->bindAxis(TEXT("DefaultPawn_MoveUp"), std::bind(&ADefaultPawn::moveUp_World, this, std::placeholders::_1));
			playerInputComponent->bindAxis(TEXT("DefaultPawn_Turn"), std::bind(&ADefaultPawn::addControllerYawInput, this, std::placeholders::_1));
			playerInputComponent->bindAxis(TEXT("DefaultPawn_TurnRate"), std::bind(&ADefaultPawn::turnAtRate, this, std::placeholders::_1));
			playerInputComponent->bindAxis(TEXT("DefaultPawn_LookUp"), std::bind(&ADefaultPawn::addControllerPitchInput, this, std::placeholders::_1));
		}
	}


	void ADefaultPawn::turnAtRate(float rate)
	{
		addControllerYawInput(rate * mBaseTurnRate * getWorld()->getDeltaSeconds());
	}

	void ADefaultPawn::moveForward(float val)
	{
		if (val != 0.f)
		{
			if (mController)
			{
				Rotator const controlSpaceRot = mController->getControllerRotation();
				addMovementInput(RotationMatrix(controlSpaceRot).getScaleAxis(EAxis::Z), val);
			}
		}
	}

	void ADefaultPawn::moveRight(float val)
	{
		if (val != 0.f)
		{
			if (mController)
			{
				Rotator const controlSpaceRot = mController->getControllerRotation();
				addMovementInput(RotationMatrix(controlSpaceRot).getScaleAxis(EAxis::X), val);
			}
		}
	}

	void ADefaultPawn::moveUp_World(float val)
	{
		if (val != 0.0f)
		{
			addMovementInput(float3::Up, val);
		}
	}

	DECLARE_SIMPLER_REFLECTION(ADefaultPawn);
}