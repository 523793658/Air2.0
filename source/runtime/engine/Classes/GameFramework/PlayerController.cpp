#include "Classes/GameFramework/PlayerController.h"
#include "UserInterface/PlayerInput.h"
#include "Classes/Engine/Player.h"
#include "Classes/Engine/LocalPlayer.h"
#include "Classes/Components/InputComponent.h"
#include "HAL/PlatformProperties.h"
#include "Classes/GameFramework/GameStateBase.h"
#include "Classes/GameFramework/PlayerState.h"
#include "Classes/Camera/PlayerCameraManager.h"
#include "Classes/GameFramework/SpectatorPawn.h"
#include "SimpleReflection.h"
namespace Air
{
	APlayerController::APlayerController(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		bIsPlayerController = true;
		bIsLocalPlayerController = false;
		mPrimaryActorTick.mTickGroup = TG_PrePhysics;
		mPrimaryActorTick.bTickEvenWhenPaused = true;
		bAllowTickBeforeBeginPlay = true;
		bInputEnabled = true;
		bAutoManagerActiveCameraTarget = true;
		bShouldShowCursor = true;
		if (mRootComponent)
		{
			mRootComponent->bAbsoluteRotation = true;
		}
	}

	AActor* APlayerController::getViewTarget() const
	{
		return mPlayerCameraManager ? mPlayerCameraManager->getViewTarget() : nullptr;
	}

	bool APlayerController::inputKey(Key key, EInputEvent eventType, float amountDepressed, bool bGamepad)
	{
		bool bResult = false;
		if (mPlayerInput)
		{
			bResult = mPlayerInput->inputKey(key, eventType, amountDepressed, bGamepad);
		}
		return bResult;
	}

	bool APlayerController::inputAxis(Key key, float delta, float deltaTime, int32 numSamples, bool bGamepad)
	{
		bool bResult = false;
		if (mPlayerInput)
		{
			bResult = mPlayerInput->inputAxis(key, delta, deltaTime, numSamples, bGamepad);
		}
		return bResult;
	}

	void APlayerController::setPlayer(Player* inPlayer)
	{
		BOOST_ASSERT(inPlayer != nullptr);
		const bool bIsSameLevel = inPlayer->mPlayerComtroller && (inPlayer->mPlayerComtroller->getLevel() == getLevel());

		if (bIsSameLevel)
		{
			inPlayer->mPlayerComtroller->mPlayer = nullptr;
		}

		mPlayer = std::dynamic_pointer_cast<Player>( inPlayer->shared_from_this());

		inPlayer->mPlayerComtroller = std::dynamic_pointer_cast<APlayerController>(this->shared_from_this());

		if (inPlayer != nullptr)
		{
			setAsLocalPlayerController();
			initInputSystem();
		}
		receivedPlayer();
	}

	void APlayerController::initInputSystem()
	{
		if (mPlayerInput == nullptr)
		{
			mPlayerInput = newObject<PlayerInput>(this);
		}
		setupInputComponent();

		World* world = getWorld();
		BOOST_ASSERT(world);
		world->mPersistentLevel->pushPenddingAutoReceiveInput(this);


	}
	bool APlayerController::popInputComponent(std::shared_ptr<InputComponent> inInputComponent)
	{
		if (inInputComponent)
		{
			if (mCurrentInputStack.removeSingle(inInputComponent) > 0)
			{
				inInputComponent->clearBindingValues();
				return true;
			}
		}
		return false;
	}

	void APlayerController::pushInputComponent(std::shared_ptr<InputComponent> inInputComponent)
	{
		if (inInputComponent)
		{
			bool bPushed = false;
			mCurrentInputStack.removeSingle(inInputComponent);
			for (int32 index = mCurrentInputStack.size() - 1; index >= 0; --index)
			{
				std::shared_ptr<InputComponent>& ic = mCurrentInputStack[index];
				if (ic == nullptr)
				{
					mCurrentInputStack.removeAt(index);
				}
				else if (ic->mPriority <= inInputComponent->mPriority)
				{
					mCurrentInputStack.insert(inInputComponent, index + 1);
					bPushed = true;
					break;
				}
			}
			if (!bPushed)
			{
				mCurrentInputStack.insert(inInputComponent, 0);
			}
		}
	}

	void APlayerController::setupInputComponent()
	{
		if (mInputComponent == nullptr)
		{
			mInputComponent = newObject<InputComponent>(this, TEXT("PC_InputComponent0"));
			mInputComponent->registerComponent();
		}
	}

	void APlayerController::setViewTarget(class AActor* newViewTarget)
	{
		if (mPlayerCameraManager)
		{
			mPlayerCameraManager->setViewTarget(newViewTarget);
		}
	}

	void APlayerController::spawnPlayerCameraManager()
	{
		ActorSpawnParameters spawnInfo;
		spawnInfo.mOwner = std::dynamic_pointer_cast<AActor>(this->shared_from_this());
		spawnInfo.objectFlags |= RF_Transient;
		mPlayerCameraManager = getWorld()->spawnActor<PlayerCameraManager>(spawnInfo);
		if (mPlayerCameraManager != nullptr)
		{
			mPlayerCameraManager->initializeFor(this);
		}
	}

	bool APlayerController::isLocalController() const
	{
		if (PlatformProperties::isServerOnly())
		{
			return false;
		}
		if (bIsLocalPlayerController)
		{
			return true;
		}
		ENetMode netMode = getNetMode();
		if (netMode == NM_DedicatedServer)
		{
			return false;
		}

		if (netMode == NM_Client || netMode == NM_Standalone)
		{
			bIsLocalPlayerController = true;
			return true;
		}

		return bIsLocalPlayerController;
	}

	void APlayerController::receivedPlayer()
	{
		if (isInState(Spectating))
		{
			if (getSpectatorPawn() == nullptr)
			{
				beginSpectatingState();
			}
		}
	}

	void APlayerController::beginSpectatingState()
	{
		if (getPawn() != nullptr && mRole == ROLE_Authority)
		{
			unPossess();
		}
		destroySpectatorPawn();
		setSpectatorPawn(spawnSpectatorPawn());
	}

	std::shared_ptr<ASpectatorPawn> APlayerController::spawnSpectatorPawn()
	{
		std::shared_ptr< ASpectatorPawn> spawnedSpectator;

		if ((getSpectatorPawn() == nullptr) && isLocalController())
		{
			World* world = getWorld();
			if (AGameStateBase const* const gameState = world->getGameState())
			{
				if (RClass* spectatorClass = gameState->mSpectatorClass)
				{
					ActorSpawnParameters spawnParams;
					spawnParams.mOwner = std::dynamic_pointer_cast<AActor>(this->shared_from_this());
					spawnParams.objectFlags |= RF_Transient;
					spawnedSpectator = world->spawnActor<ASpectatorPawn>(spectatorClass, getSpawnLocation(), getControllerRotation(), spawnParams);
					if (spawnedSpectator)
					{
						spawnedSpectator->possessedBy(this);
						spawnedSpectator->pawnClientRestart();
						if (spawnedSpectator->mPrimaryActorTick.bStartWithTickEnabled)
						{
							spawnedSpectator->setActorTickEnabled(true);
						}
					}
				}
			}
		}
		return spawnedSpectator;
	}

	void APlayerController::postInitializeComponents()
	{
		ParentType::postInitializeComponents();
		if (!isPendingKill() && (getNetMode() != NM_Client))
		{
			initPlayerState();
		}
		spawnPlayerCameraManager();
		resetCameraMode();
		bPlayerIsWaiting = true;
	}

	void APlayerController::addPitchInput(float val)
	{
		mRotationInput.mPitch += !isLookInputIgnored() ? val * mInputPitchScale : 0.f;
	}
	void APlayerController::addYawInput(float val)
	{
		mRotationInput.mYaw += !isLookInputIgnored() ? val * mInputYawScale : 0.f;
	}

	void APlayerController::addRollInput(float val)
	{
		mRotationInput.mRoll += !isLookInputIgnored() ? val * mInputRollScale : 0.f;
	}

	void APlayerController::destroySpectatorPawn()
	{
		if (getSpectatorPawn())
		{
			if (getViewTarget() == getSpectatorPawn())
			{
				setViewTarget(this);
			}
			getSpectatorPawn()->unPossessed();
			getWorld()->destroyActor(getSpectatorPawn());
			setSpectatorPawn(nullptr);
		}
	}

	void APlayerController::resetCameraMode()
	{

	}

	void APlayerController::setSpectatorPawn(std::shared_ptr<class ASpectatorPawn> newSpectatorPawn)
	{
		if (isInState(Spectating))
		{
			removePawnTickDependency(mSpectatorPawn);
			mSpectatorPawn = std::dynamic_pointer_cast<ASpectatorPawn>(newSpectatorPawn->shared_from_this());
			if (newSpectatorPawn)
			{
				attachToPawn(newSpectatorPawn.get());
				addPawnTickDependency(newSpectatorPawn.get());
				autoManageActiveCameraTarget(newSpectatorPawn.get());
			}
			else
			{
				APawn* myPawn = getPawn();
				attachToPawn(myPawn);
				addPawnTickDependency(myPawn);
				if (myPawn)
				{
					autoManageActiveCameraTarget(myPawn);
				}
				else
				{
					autoManageActiveCameraTarget(this);
				}
			}
		}
	}

	void APlayerController::autoManageActiveCameraTarget(AActor* suggestedTarget)
	{
		if (bAutoManagerActiveCameraTarget)
		{

		}
		setViewTarget(suggestedTarget);
	}

	void APlayerController::possess(APawn* inPawn)
	{
		if (inPawn != nullptr && (mPlayerState == nullptr || !mPlayerState->bOnlySpectator))
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

			setControlRotation(inPawn->getActorRotation());
			setPawn(inPawn);
			BOOST_ASSERT(getPawn() != nullptr);

			if (getPawn() && getPawn()->mPrimaryActorTick.bStartWithTickEnabled)
			{
				getPawn()->setActorTickEnabled(true);
			}

			if (!isLocalPlayerController())
			{
				getPawn()->restart();
			}

			clientRestart(getPawn());

			changeState(Playing);
			if (bAutoManagerActiveCameraTarget)
			{
				autoManageActiveCameraTarget(getPawn());
				resetCameraMode();
			}
		}
	}

	void APlayerController::unPossess()
	{
		if (getPawn() != nullptr)
		{
			getPawn()->unPossessed();
			if (getViewTarget() == getPawn())
			{
				setViewTarget(this);
			}
		}
		setPawn(nullptr);
	}

	void APlayerController::clientRestart(class APawn* newPawn)
	{
		setPawn(newPawn);
		getPawn()->mController = std::dynamic_pointer_cast<AController>(this->shared_from_this());
		getPawn()->pawnClientRestart();
	}

	void APlayerController::receivedGameModeClass(TSubclassOf<class GameModeBase> gameModeClass)
	{

	}

	void APlayerController::receivedSpectatorClass(TSubclassOf<class ASpectatorPawn> spectatorClass)
	{
		if (isInState(Spectating))
		{
			if (getSpectatorPawn() == nullptr)
			{
				beginSpectatingState();
			}
		}
	}

	void APlayerController::tickActor(float deltaTime, enum ELevelTick tickType, ActorTickFunction& thisTickFunction)
	{
		if (mRole > ROLE_SimulatedProxy)
		{
			if (!mPlayerInput && (mPlayer || std::dynamic_pointer_cast<LocalPlayer>(mPlayer) ))
			{
				initInputSystem();
			}

			if (mPlayerInput)
			{
				playerTick(deltaTime);
			}
			if (isPendingKill())
			{
				return;
			}

			if (mPlayerCameraManager != nullptr)
			{
				APawn* targetPawn = mPlayerCameraManager->getViewTargetPawn();
				if ((targetPawn != getPawn()) && (targetPawn != nullptr))
				{
				}
			}
		}
		if (!isPendingKill())
		{
			tick(deltaTime);
		}

		mRotationInput = Rotator::ZeroRotator;


	}

	void APlayerController::playerTick(float deltaTime)
	{
		tickPlayerInput(deltaTime, deltaTime == 0.f);
		if ((mPlayer != nullptr) && (mPlayer->mPlayerComtroller.get() == this))
		{
			bool bUpdateRotation = false;
			if (isInState(Playing))
			{
				if (getPawn() == nullptr)
				{
					changeState(Inactive);
				}
			}
			else if (isInState(Spectating))
			{
				if (mRole < ROLE_Authority)
				{

				}
				bUpdateRotation = true;
			}
			if (bUpdateRotation)
			{
				updateRotation(deltaTime);
			}
		}
	}

	void APlayerController::updateRotation(float deltaTime)
	{
		Rotator deltaRot(mRotationInput);
		Rotator viewRotation = getControllerRotation();
		if (mPlayerCameraManager)
		{
			mPlayerCameraManager->processViewRotation(deltaTime, viewRotation, deltaRot);
		}

		AActor* viewTarget = getViewTarget();
		if (!mPlayerCameraManager || !viewTarget || !viewTarget->hasActiveCameraComponent() || viewTarget->hasActivePawnControlCameraComponent())
		{
		}
		setControlRotation(viewRotation);
		APawn* p = getPawnOrSpectator();
		if (p)
		{
			p->faceRotation(viewRotation, deltaTime);
		}
	}

	void APlayerController::tickPlayerInput(const float deltaSeconds, const bool bGamePaused)
	{
		BOOST_ASSERT(mPlayerInput);
		mPlayerInput->tick(deltaSeconds);
		processPlayerInput(deltaSeconds, bGamePaused);
		processForceFeedbackAndHaptics(deltaSeconds, bGamePaused);
	}

	void APlayerController::processPlayerInput(const float deltaTime, const bool bGamePaused)
	{
		TArray<std::shared_ptr<InputComponent>> inputStack;
		{
			buildInputStack(inputStack);
		}

		{
			mPlayerInput->processInputStack(inputStack, deltaTime, bGamePaused);
		}
	}

	APawn* APlayerController::getPawnOrSpectator() const
	{
		return getPawn() ? getPawn() : getSpectatorPawn();
	}

	void APlayerController::preProcessInput(const float deltaTime, const bool bGamePaused)
	{

	}

	void APlayerController::postProcessInput(const float deltaTime, const bool bGamePaused)
	{

	}
	void APlayerController::buildInputStack(TArray<std::shared_ptr<InputComponent>>& inputStack)
	{
		APawn* controlledPawn = getPawnOrSpectator();
		if (controlledPawn)
		{
			if (controlledPawn->inputEnabled())
			{
				if (controlledPawn->mInputComponent)
				{
					inputStack.push(controlledPawn->mInputComponent);
				}
				for (const std::shared_ptr<ActorComponent>& actorComponent : controlledPawn->getComponents())
				{
					std::shared_ptr<InputComponent> pawnInputComponent = std::dynamic_pointer_cast<InputComponent>(actorComponent);
					if (pawnInputComponent && pawnInputComponent != controlledPawn->mInputComponent)
					{
						inputStack.push(pawnInputComponent);
					}
				}
			}
		}
		for (const std::shared_ptr<Level>& level : getWorld()->getLevels())
		{
		}

		if (inputEnabled())
		{
			inputStack.push(mInputComponent);
		}

		for (int32 idx = 0; idx < mCurrentInputStack.size(); ++idx)
		{
			std::shared_ptr<InputComponent> ic = mCurrentInputStack[idx];
			if (ic)
			{
				inputStack.push(ic);
			}
			else
			{
				mCurrentInputStack.removeAt(idx--);
			}
		}
	}

	void APlayerController::updateCameraManager(float deltaSeconds)
	{
		if (mPlayerCameraManager != nullptr)
		{
			mPlayerCameraManager->updateCamera(deltaSeconds);
		}
	}

	void APlayerController::getPlayerViewPoint(float3& outLocation, Rotator& outRotation) const
	{
		if (isInState(Spectating) && hasAuthority() && !isLocalController())
		{
		}
		else if (mPlayerCameraManager != nullptr && mPlayerCameraManager->mCameraCache.mTimeStamp > 0.f)
		{
			mPlayerCameraManager->getCameraViewPoint(outLocation, outRotation);
		}
		else
		{
			AActor* theViewTarget = getViewTarget();
			if (theViewTarget != nullptr)
			{
				outLocation = theViewTarget->getActorLocation();
				outRotation = theViewTarget->getActorRotation();
			}
			else
			{
				ParentType::getPlayerViewPoint(outLocation, outRotation);
			}
		}
	}

	bool APlayerController::shouldShowMouseCursor() const
	{
		return bShouldShowCursor;
	}

	EMouseCursor::Type APlayerController::getMouseCursor() const
	{
		if (shouldShowMouseCursor())
		{
			return EMouseCursor::None;
		}
		return EMouseCursor::None;
	}

	DECLARE_SIMPLER_REFLECTION(APlayerController);
}