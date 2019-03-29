#include "Classes/GameFramework/GameStateBase.h"
#include "Classes/Engine/World.h"
#include "Classes/GameFramework/PlayerController.h"
#include "SimpleReflection.h"
namespace Air
{
	AGameStateBase::AGameStateBase(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{

	}

	void AGameStateBase::receiveGameModeClass()
	{
		for (ConstPlayerControllerIterator iterator = getWorld()->getPlayerControllerIterator(); iterator; ++iterator)
		{
			APlayerController* const playerController = *iterator;
			if (playerController)
			{
				playerController->receivedGameModeClass(mGameModeClass);
			}
		}
	}

	void AGameStateBase::receiveSpectatorClass()
	{
		for (ConstPlayerControllerIterator iterator = getWorld()->getPlayerControllerIterator(); iterator; ++iterator)
		{
			APlayerController* const playerController = *iterator;
			if (playerController && playerController->isLocalController())
			{
				playerController->receivedSpectatorClass(mSpectatorClass);
			}
		}
	}

	void AGameStateBase::handleBeginPlay()
	{
		getWorldSettings()->notifyBeginPlay();
	}

	DECLARE_SIMPLER_REFLECTION(AGameStateBase);
}