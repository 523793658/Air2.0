#include "Classes/GameFramework/GameModeBase.h"
#include "Classes/GameFramework/PlayerController.h"
#include "Classes/Engine/World.h"
#include "Classes/GameFramework/GameStateBase.h"
#include "Classes/GameFramework/DefaultPawn.h"
#include "Classes/GameFramework/PlayerState.h"
#include "Classes/GameFramework/SpectatorPawn.h"
#include "SimpleReflection.h"

namespace Air
{
	GameModeBase::GameModeBase(const ObjectInitializer& objectInitializer)
		:ParentType(objectInitializer)
	{
		bPauseable = true;
		bStartPlayersAsSpectator = false;

		mDefaultPawnClass = ADefaultPawn::StaticClass();
		mPlayerControllerClass = APlayerController::StaticClass();
		mPlayerStateClass = APlayerState::StaticClass();
		mGameStateClass = AGameStateBase::StaticClass();

		mSpectatorClass = ASpectatorPawn::StaticClass();
		mReplaySpectatorPlayerControllerClass = APlayerController::StaticClass();
	}

	std::shared_ptr<APlayerController> GameModeBase::spawnPlayerController(ENetRole inRemoteRole, float3 const & spawnLocation, Rotator const& spawnRotation)
	{
		ActorSpawnParameters spawnInfo;
		spawnInfo.objectFlags |= RF_Transient;
		std::shared_ptr<APlayerController> newPC = getWorld()->spawnActor<APlayerController>(spawnInfo);
		if (newPC)
		{
			if (inRemoteRole == ROLE_SimulatedProxy)
			{
				newPC->setAsLocalPlayerController();
			}
		}
		return newPC;
	}

	std::shared_ptr<APlayerController> GameModeBase::login(Player* newPlayer, ENetRole inRemoteRole, const wstring& portal, const wstring& options)
	{
		std::shared_ptr<APlayerController> newPlayerController = spawnPlayerController(inRemoteRole, float3::Zero, Rotator::ZeroRotator);

		if (newPlayerController == nullptr)
		{
			return newPlayerController;
		}

		initNewPlayer(newPlayerController.get());
		return newPlayerController;
	}

	wstring GameModeBase::initNewPlayer(APlayerController* newPlayerController)
	{
		return TEXT("");
	}

	void GameModeBase::preInitializeComponents()
	{
		ParentType::preInitializeComponents();

		ActorSpawnParameters spawnInfo;
		spawnInfo.objectFlags |= RF_Transient;

		if (mGameStateClass == nullptr)
		{
			mGameStateClass = AGameStateBase::StaticClass();
		}

		mGameState = getWorld()->spawnActor<AGameStateBase>(mGameStateClass, spawnInfo);

		getWorld()->setGameState(mGameState.get());
		if (mGameState)
		{
			mGameState->mAuthorityGameMode = this;
		}
		initGameState();

	}

	void GameModeBase::initGameState()
	{
		mGameState->mGameModeClass = getClass();
		mGameState->receiveGameModeClass();

		mGameState->mSpectatorClass = mSpectatorClass;
		mGameState->receiveSpectatorClass();
	}

	void GameModeBase::startPlay()
	{
		mGameState->handleBeginPlay();
	}

	DECLARE_SIMPLER_REFLECTION(GameModeBase);
}