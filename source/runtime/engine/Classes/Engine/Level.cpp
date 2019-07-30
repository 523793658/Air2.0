#include "Template/AirTemplate.h"

#include "Classes/Engine/Level.h"
#include "Classes/GameFramework/WorldSettings.h"
#include "Classes/GameFramework/Actor.h"
#include "SimpleReflection.h"
#include "TickTaskManagerInterface.h"
#include "Classes/Engine/World.h"
#include "Classes/GameFramework/PlayerController.h"
#include "Classes/GameFramework/Pawn.h"
#include "Classes/GameFramework/GameModeBase.h"
namespace Air
{

	static void sortActorsHierarchy(TArray<std::shared_ptr<AActor>>& actors, Object* level)
	{

	}

	Level::Level(const ObjectInitializer& objectInitializer /* = ObjectInitializer::get() */)
		:Object(objectInitializer),
		mTickTaskLevel(TickTaskManagerInterface::get().allocateTickTaskLevel())
	{

	}

	void Level::sortActorList()
	{

	}

	void Level::updateLevelComponents(bool bRerunConstructionScripts)
	{
		incrementalUpdateComponents(0, bRerunConstructionScripts);
	}

	void Level::updateModelComponents()
	{

	}

	void Level::incrementalUpdateComponents(int32 numComponentsToUpdate, bool bRerunConstructionScript)
	{
		if (numComponentsToUpdate != 0)
		{
			BOOST_ASSERT(mOwningWorld->isGameWorld());
		}

		if (mCurrentActorIndexForUpdateComponents == 0)
		{
			updateModelComponents();
			sortActorsHierarchy(mActors, this);
		}

		while (mCurrentActorIndexForUpdateComponents < mActors.size())
		{
			std::shared_ptr<AActor>& actor = mActors[mCurrentActorIndexForUpdateComponents];
			bool bAllComponentsRegistered = true;
			if (actor && !actor->isPendingKill())
			{
				bAllComponentsRegistered = actor->incrementalRegisterComponents(numComponentsToUpdate);
			}
			if (bAllComponentsRegistered)
			{
				mCurrentActorIndexForUpdateComponents++;
			}
			if (numComponentsToUpdate != 0)
			{
				break;
			}
		}
		if (mCurrentActorIndexForUpdateComponents == mActors.size())
		{
			mCurrentActorIndexForUpdateComponents = 0;
			mAreComponentsCurrentlyRegistered = true;
		}
		else
		{
			BOOST_ASSERT(mOwningWorld->isGameWorld());
		}
	}

	void Level::initialize(const URL& url)
	{
		mUrl = url;
	}

	World* Level::getWorld() const
	{
		return mOwningWorld.get();
	}

	void Level::setWorldSettings(std::shared_ptr<WorldSettings> worldSetting)
	{
		if (mWorldSettings != worldSetting)
		{
			const int32 newWorldSettingsIndex = mActors.findLast(worldSetting);
			if (newWorldSettingsIndex != 0)
			{
				if (!mActors[0] || std::dynamic_pointer_cast<WorldSettings>(mActors[0]))
				{
					exchange(mActors[0], mActors[newWorldSettingsIndex]);
				}
				else
				{
					mActors[newWorldSettingsIndex] = nullptr;
					mActors.insert(std::static_pointer_cast<AActor>(worldSetting), 0);

				}
			}
			if (mWorldSettings)
			{
				mWorldSettings->destroy();
			}
			mWorldSettings = worldSetting;
		}
	}

	void Level::pushPenddingAutoReceiveInput(APlayerController* pc)
	{
		BOOST_ASSERT(pc);
		int32 playerIndex = -1;
		int32 index = 0;
		for (ConstPlayerControllerIterator iterator = pc->getWorld()->getPlayerControllerIterator(); iterator; ++iterator)
		{
			std::shared_ptr<APlayerController> playerController = *iterator;
			if (pc == playerController.get())
			{
				playerIndex = index;
				break;
			}
			index++;
		}

		if (playerIndex >= 0)
		{
			TArray<AActor*> actorsToAdd;
			for (int32 pendingIndex = mPendingAutoReceiveInputActors.size() - 1; pendingIndex >= 0; --pendingIndex)
			{
				PendingAutoReceiveInputActor& pendingActor = mPendingAutoReceiveInputActors[pendingIndex];
				if (pendingActor.mPlayerIndex == playerIndex)
				{
					if (pendingActor.mActor != nullptr)
					{
						actorsToAdd.add(pendingActor.mActor);
					}
					mPendingAutoReceiveInputActors.removeAtSwap(pendingIndex);
				}
			}
			for (int32 toAddIndex = actorsToAdd.size() - 1; toAddIndex >= 0; --toAddIndex)
			{
				APawn* pawnToPossess = dynamic_cast<APawn*>(actorsToAdd[toAddIndex]);
				if (pawnToPossess)
				{

				}
				else
				{
					actorsToAdd[toAddIndex]->enableInput(pc);
				}
			}
		}
	}


	void Level::routeActorInitialize()
	{
		for (int32 index = 0; index < mActors.size(); ++index)
		{
			std::shared_ptr<AActor> const actor = mActors[index];
			if (actor && !actor->isActorInitialized())
			{
				actor->preInitializeComponents();
			}
		}

		const bool bCallBeginPlay = mOwningWorld->hasBegunPlay();
		TArray<std::shared_ptr<AActor>> actorsToBeginPlay;
		for (int32 index = 0; index < mActors.size(); ++index)
		{
			std::shared_ptr<AActor> const actor = mActors[index];
			if (actor)
			{
				if (!actor->isActorInitialized())
				{
					actor->initializeComponents();
					actor->postInitializeComponents();
					if (!actor->isActorInitialized() && !actor->isPendingKill())
					{

					}
					if (bCallBeginPlay && !actor->isChildActor())
					{
						actorsToBeginPlay.add(actor);
					}
				}
			}
		}

		for (int32 actorIndex = 0; actorIndex < actorsToBeginPlay.size(); actorIndex++)
		{
			std::shared_ptr<AActor> actor = actorsToBeginPlay[actorIndex];
			actor->dispatchBeginPlay();
		}
	}

	const WorldSettings* Level::getWorldSettings(bool bChecked) const
	{
		if (bChecked)
		{
			BOOST_ASSERT(mWorldSettings);
		}
		return mWorldSettings.get();
	}

	WorldSettings* Level::getWorldSettings(bool bChecked)
	{
		if (bChecked)
		{
			BOOST_ASSERT(mWorldSettings);
		}
		return mWorldSettings.get();
	}

	DECLARE_SIMPLER_REFLECTION(Level)
}