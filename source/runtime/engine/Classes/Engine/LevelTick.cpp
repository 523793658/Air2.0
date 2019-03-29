#include "Classes/Engine/World.h"
#include "SceneUtils.h"
#include "RHICommandList.h"
#include "RenderingThread.h"
#include "Classes/GameFramework/PlayerController.h"
#include "TickTaskManagerInterface.h"
namespace Air
{
	TDrawEvent<RHICommandList>* beginTickDrawEvent()
	{
		TDrawEvent<RHICommandList>* tickDrawEvent = new TDrawEvent<RHICommandList>();
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			BeginDrawEventCommand,
			TDrawEvent<RHICommandList>*, tickDrawEvent, tickDrawEvent,
			{
				BEGIN_DRAW_EVENTF(RHICmdList, WorldTick, (*tickDrawEvent), TEXT("WorldTick"));
			}
		);
		return tickDrawEvent;
	}

	void endTickDrawEvent(TDrawEvent<RHICommandList>* tickDrawEvent)
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			EndDrawEventCommand,
			TDrawEvent<RHICommandList>*, tickDrawEvent, tickDrawEvent,
			{
				STOP_DRAW_EVENT((*tickDrawEvent));
			delete tickDrawEvent;
			});
	}

	void World::tick(ELevelTick tickType, float deltaTime)
	{
		TDrawEvent<RHICommandList>* tickDrawEvent = beginTickDrawEvent();

		MemMark mark(MemStack::get());
		bInTick = true;

		bool bIsPaused = isPaused();

		mRealTimeSeconds += deltaTime;
		float realDeltaSeconds = deltaTime;
		mUnpausedTimeSeconds += deltaTime;

		if (!bIsPaused)
		{
			mTimeSeconds += deltaTime;
		}



		bool bDoingActorTicks = (tickType != LEVELTICK_TimeOnly) && !bIsPaused;

		for (LevelCollection& lc : mLevelCollections)
		{
			TArray<Level*> levelsToTick;
			for (Level* collectionLevel : lc.getLevels())
			{
				if (mLevels.contains(collectionLevel))
				{
					levelsToTick.add(collectionLevel);
				}
			}
			ScopedLevelCollectionContextSwitch levelContext(&lc, this);

			if (bDoingActorTicks)
			{
				setupPhysicsTickFunctions(deltaTime);
				mTickGroup = TG_PrePhysics;
				TickTaskManagerInterface::get().startFrame(this, deltaTime, tickType, levelsToTick);
				{
					runTickGroup(TG_PrePhysics);
				}

				bInTick = true;

				{
					runTickGroup(TG_StartPhysics);
				}
				{
					runTickGroup(TG_DuringPhysics, false);
				}

				mTickGroup = TG_EndPhysics;
				{
					runTickGroup(TG_EndPhysics);
				}
				{
					runTickGroup(TG_PostPhysics);
				}
			}
			else if (bIsPaused)
			{

			}
			if (lc.getType() == ELevelCollectionType::DynamicSourceLevels)
			{
				if (!bIsPaused)
				{

				}
				{
					if (tickType != LEVELTICK_TimeOnly && !bIsPaused)
					{
						
					}

				}

				{
					for (ConstPlayerControllerIterator it = getPlayerControllerIterator(); it; ++it)
					{
						APlayerController* playerController = *it;
						if (!bIsPaused)
						{
							playerController->updateCameraManager(deltaTime);
						}
						else if (false)
						{

						}
					}
					if (!bIsPaused)
					{
						if (isGameWorld())
						{
							
						}
					}
				}
			}
			if (bDoingActorTicks)
			{
				{
					runTickGroup(TG_PostUpdateWork);
				}
				{
					runTickGroup(TG_LastDemotable);
				}

				TickTaskManagerInterface::get().endFrame();
			}
		}
		if (bDoingActorTicks)
		{
			//更新物理场景
		
			{
			}
		}

		bInTick = false;
		mark.pop();

		endTickDrawEvent(tickDrawEvent);
	}

	void World::runTickGroup(ETickingGroup group, bool bBlockTillComplete /* = true */)
	{
		BOOST_ASSERT(mTickGroup == group);
		TickTaskManagerInterface::get().runTickGroup(group, bBlockTillComplete);
		mTickGroup = ETickingGroup(mTickGroup + 1);
	}

	void World::setupPhysicsTickFunctions(float deltaSeconds)
	{
		//mStartPhysicsTickFunction.bCanEverTick = true;
		//mStartPhysicsTickFunction.mTarget = this;
		 
	}

	void StartPhysicsTickFunction::executeTick(float deltaTime, enum ELevelTick tickType, ENamedThreads::Type currentThread, const GraphEventRef & myCompletionGraphEvent)
	{

	}

	wstring StartPhysicsTickFunction::diagnosticMessage()
	{
		return TEXT("StartPhysicsTickFunction");
	}

	void EndPhysicsTickFunction::executeTick(float deltaTime, ELevelTick tickType, ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent)
	{

	}

	wstring EndPhysicsTickFunction::diagnosticMessage()
	{
		return TEXT("EndPhysicsTickFunction");
	}
}
