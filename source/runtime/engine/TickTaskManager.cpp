#include "Classes/Engine/EngineBaseTypes.h"
#include "TickTaskManagerInterface.h"
#include "Async/TaskGraphInterfaces.h"
#include "Classes/Engine/World.h"
#include "Containers/Set.h"
#include "Object.h"
#include "Misc/App.h"
namespace Air
{
	template<typename InElementType, typename inAllocator = DefaultAllocator>
	class TArrayWithThreadsafeAdd : public TArray<InElementType, inAllocator>
	{
	public:
		typedef InElementType	ElementType;
		typedef	inAllocator		Allocator;


		template<typename... ArgsType>
		int32 emplaceThreadsafe(ArgsType&&... args)
		{
			const int32 index = addUninitializedThreadsafe(1);
			new(this->getData() + index)ElementType(std::forward<ArgsType>(args)...);
			return index;
		}

		int32 addUninitializedThreadsafe(int32 count = 1)
		{
			BOOST_ASSERT(count >= 0);
			const int32 oldNum = PlatformAtomics::interLockedAdd(&this->mArrayNum, count);
			return oldNum;
		}

		FORCEINLINE int32 addThreadsafe(const ElementType& item)
		{
			return emplaceThreadsafe(item);
		}
	};


	

	struct TickContext
	{
		float mDeltaSeconds;
		ELevelTick mTickType;
		ETickingGroup mTickGroup;
		ENamedThreads::Type mThread;
		World*		mWorld;

		TickContext(float inDeltaSeconds = 0.0f, ELevelTick inTickType = LEVELTICK_ALL, ETickingGroup inTickGroup = TG_PrePhysics, ENamedThreads::Type inThread = ENamedThreads::GameThread)
			:mDeltaSeconds(inDeltaSeconds)
			, mTickType(inTickType)
			, mTickGroup(inTickGroup)
			, mThread(inThread)
			, mWorld(nullptr)
		{

		}

		TickContext(const TickContext& other)
			:mDeltaSeconds(other.mDeltaSeconds)
			, mTickType(other.mTickType)
			, mTickGroup(other.mTickGroup)
			, mThread(other.mThread)
			, mWorld(other.mWorld)
		{}

		void operator = (const TickContext& other)
		{
			mDeltaSeconds = other.mDeltaSeconds;
			mTickType = other.mTickType;
			mTickGroup = other.mTickGroup;
			mThread = other.mThread;
			mWorld = other.mWorld;
		}
	};

	class TickFunctionTask
	{
		TickFunction* mTarget;
		TickContext	mContext;
		bool bLogTick;
		bool bLogTicksShowPrerequistes;
	public:
		FORCEINLINE TickFunctionTask(TickFunction* inTarget, const TickContext* inContext, bool inbLogTick, bool bInlogItcksShowPrerequistes)
			:mTarget(inTarget)
			,mContext(*inContext)
			,bLogTick(inbLogTick)
			,bLogTicksShowPrerequistes(bInlogItcksShowPrerequistes)
		{

		}

		static FORCEINLINE ESubsequentsMode::Type getSubsequentsMode()
		{
			return ESubsequentsMode::TrackSubsequents;
		}

		FORCEINLINE ENamedThreads::Type getDesiredThread()
		{
			return mContext.mThread;
		}

		void doTask(ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent)
		{
			if (mTarget->isTickFunctionEnabled())
			{
				mTarget->executeTick(mTarget->calculateDeltaTime(mContext), mContext.mTickType, currentThread, myCompletionGraphEvent);
			}
			mTarget->mTaskPointer = nullptr;
		}
	};

	class TickTaskSequencer
	{
		class DispatchTickGroupTask
		{
			TickTaskSequencer& tts;
			ETickingGroup worldTickGroup;
		public:
			FORCEINLINE DispatchTickGroupTask(TickTaskSequencer& inTTS, ETickingGroup inWorldTickgroup)
				:tts(inTTS)
				, worldTickGroup(inWorldTickgroup)
			{}
			static FORCEINLINE ENamedThreads::Type getDesiredThread()
			{
				return ENamedThreads::AnyThread;
			}

			static FORCEINLINE ESubsequentsMode::Type getSubsequentsMode()
			{
				return ESubsequentsMode::TrackSubsequents;
			}

			void doTask(ENamedThreads::Type currentThread, const GraphEventRef& myCopletionGraphEvent)
			{
				tts.dispatchTickGroup(currentThread, worldTickGroup);
			}
		};

		class ResetTickGroupTask
		{
			TickTaskSequencer &TTS;
			ETickingGroup mworldTickGroup;
		public:
			FORCEINLINE ResetTickGroupTask(TickTaskSequencer& inTTS, ETickingGroup inWorldTickGroup)
				:TTS(inTTS)
				,mworldTickGroup(inWorldTickGroup)
			{}
			static FORCEINLINE ENamedThreads::Type getDesiredThread()
			{
				return ENamedThreads::AnyThread;
			}

			FORCEINLINE static ESubsequentsMode::Type getSubsequentsMode()
			{
				return ESubsequentsMode::TrackSubsequents;
			}

			void doTask(ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent)
			{
				TTS.resetTickGroup(mworldTickGroup);
			}
		};
		ETickingGroup mWaitForTickGroup;
		TArrayWithThreadsafeAdd<GraphEventRef, TInlineAllocator<4>> mTickCompletionEvents[TG_MAX];
		TArrayWithThreadsafeAdd<TGraphTask<TickFunctionTask>*> mHiPriTickTasks[TG_MAX][TG_MAX];
		TArrayWithThreadsafeAdd<TGraphTask<TickFunctionTask>*> mTickTasks[TG_MAX][TG_MAX];

		GraphEventArray mCleanupTasks;
		bool bAllowConcurrentTicks;
		bool bLogTicks;
		bool bLogTicksShowPrerequistes;

	public:
		static TickTaskSequencer& get()
		{
			static TickTaskSequencer singletonInstance;
			return singletonInstance;
		}

		FORCEINLINE static bool singleThreadedMode()
		{
			if (!App::shouldUseThreadingForPerformance() || isRunningDedicatedServer())
			{
				return true;
			} 
			return false;
		}

		void dispatchTickGroup(ENamedThreads::Type currentThread, ETickingGroup worldTickGroup)
		{
			for (int32 indexInner = 0; indexInner < TG_MAX; indexInner++)
			{
				TArray<TGraphTask<TickFunctionTask>*>& tickArray = mHiPriTickTasks[worldTickGroup][indexInner];
				if (indexInner < worldTickGroup)
				{
					BOOST_ASSERT(tickArray.size() == 0);
				}
				else
				{
					for (int32 index = 0; index < tickArray.size(); index++)
					{
						tickArray[index]->unlock(currentThread);
					}
				}
				tickArray.clear();
			}
			for (int32 indexInner = 0; indexInner < TG_MAX; indexInner++)
			{
				TArray<TGraphTask<TickFunctionTask>*>& tickArray = mTickTasks[worldTickGroup][indexInner];
				if (indexInner < worldTickGroup)
				{
					BOOST_ASSERT(tickArray.size() == 0);
				}
				else
				{
					for (int32 index = 0; index < tickArray.size(); index++)
					{
						tickArray[index]->unlock(currentThread);
					}
				}
				tickArray.clear();
			}
		}


		void resetTickGroup(ETickingGroup worldTickGroup)
		{
			mTickCompletionEvents[worldTickGroup].clear();
		}

		void releaseTickGroup(ETickingGroup worldTickGroup, bool bBlackTillComplete)
		{
			BOOST_ASSERT(worldTickGroup >= 0 && worldTickGroup < TG_MAX);
			{
				if (singleThreadedMode())
				{
					dispatchTickGroup(ENamedThreads::GameThread, worldTickGroup);
				}
				else
				{
					TaskGraphInterface::get().waitUntilTaskCompletes(TGraphTask<DispatchTickGroupTask>::createTask(nullptr, ENamedThreads::GameThread).constructAndDispatchWhenReady(*this, worldTickGroup));
				}
			}
			if (bBlackTillComplete || singleThreadedMode())
			{
				for (ETickingGroup block = mWaitForTickGroup; block <= mWaitForTickGroup; block = ETickingGroup(block + 1))
				{
					if (mTickCompletionEvents[block].size())
					{
						TaskGraphInterface::get().waitUntilTaskComplete(mTickCompletionEvents[block], ENamedThreads::GameThread);
						if (singleThreadedMode() || block == TG_NewlySpawned)
						{
							resetTickGroup(block);
						}
						else
						{
							mCleanupTasks.add(TGraphTask<ResetTickGroupTask>::createTask(nullptr, ENamedThreads::GameThread).constructAndDispatchWhenReady(*this, block));
						}
					}
				}
				mWaitForTickGroup = ETickingGroup(worldTickGroup + (worldTickGroup == TG_NewlySpawned ? 0 : 1));
			}
			else
			{
				TaskGraphInterface::get().processThreadUntilIdle(ENamedThreads::GameThread);
				BOOST_ASSERT(worldTickGroup + 1 < TG_MAX && worldTickGroup != TG_NewlySpawned);
			}
		}
		
		void waitForCleanup()
		{
			if (mCleanupTasks.size() > 0)
			{
				TaskGraphInterface::get().waitUntilTaskComplete(mCleanupTasks, ENamedThreads::GameThread);
				mCleanupTasks.clear();
			}
		}

		void startFrame()
		{
			bLogTicks = false;
			bLogTicksShowPrerequistes = false;
			if (bLogTicks)
			{

			}
			if (singleThreadedMode())
			{
				bAllowConcurrentTicks = false;
			}
			else
			{
				bAllowConcurrentTicks = true;
			}
			waitForCleanup();
			for (int32 index = 0; index < TG_MAX; index++)
			{
				BOOST_ASSERT(!mTickCompletionEvents[index].size());
				mTickCompletionEvents[index].clear();
				for (int32 indexInner = 0; indexInner < TG_MAX; indexInner++)
				{
					BOOST_ASSERT(!mTickTasks[index][indexInner].size() && !mHiPriTickTasks[index][indexInner].size());
					mTickTasks[index][indexInner].clear();
					mTickTasks[index][indexInner].clear();
				}
			}
			mWaitForTickGroup = (ETickingGroup)0;
		}

		void endFrame()
		{

		}

		FORCEINLINE void startTickTask(const GraphEventArray* prerequisites, TickFunction* tickFunction, const TickContext& tickContext)
		{
			BOOST_ASSERT(tickFunction->mActualStartTickGroup >= 0 && tickFunction->mActualStartTickGroup < TG_MAX);
			TickContext useContext = tickContext;
			bool bIsOriginalTickgroup = (tickFunction->mActualStartTickGroup == tickFunction->mTickGroup);
			if (tickFunction->bRunOnAnyThread && bAllowConcurrentTicks && bIsOriginalTickgroup)
			{
				if (tickFunction->bHighPriority)
				{
					useContext.mThread = ENamedThreads::AnyHiPriThreadHiPriTask;
				}
				else
				{
					ENamedThreads::AnyNormalThreadNormalTask;
				}
			}
			else
			{
				useContext.mThread = ENamedThreads::GameThread;
			}
			tickFunction->mTaskPointer = TGraphTask<TickFunctionTask>::createTask(prerequisites, tickContext.mThread).constructAndHold(tickFunction, &useContext, bLogTicks, bLogTicksShowPrerequistes);
		}

		FORCEINLINE void addTickCompletion(ETickingGroup startTickGroup, ETickingGroup endTickGroup, TGraphTask<TickFunctionTask>* task, bool bHiPri)
		{
			BOOST_ASSERT(startTickGroup >= 0 && startTickGroup < TG_MAX && endTickGroup >= 0 && endTickGroup < TG_MAX && startTickGroup <= endTickGroup);
			if (bHiPri)
			{
				mHiPriTickTasks[startTickGroup][endTickGroup].add(task);
			}
			else
			{
				mTickTasks[startTickGroup][endTickGroup].add(task);
			}
			new (mTickCompletionEvents[endTickGroup])GraphEventRef(task->getCompletionEvent());
		}

		FORCEINLINE void queueTickTask(const GraphEventArray* prerequisites, TickFunction* tickFunction, const TickContext& tickContext)
		{
			BOOST_ASSERT(tickContext.mThread == ENamedThreads::GameThread);
			startTickTask(prerequisites, tickFunction, tickContext);
			TGraphTask<TickFunctionTask>* task = (TGraphTask<TickFunctionTask>*)tickFunction->mTaskPointer;
			addTickCompletion(tickFunction->mActualStartTickGroup, tickFunction->mActualEndTickGroup, task, tickFunction->bHighPriority);
		}
	};


	class TickTaskLevel
	{
	private:
		struct CoolingDownTickFunctionList
		{
			CoolingDownTickFunctionList()
				:mHead(nullptr)
			{}

			bool contains(TickFunction* tickFunction) const
			{
				TickFunction* node = mHead;
				while (node)
				{
					if (node == tickFunction)
					{
						return true;
					}
					node = node->mNext;
				}
				return false;
			}
			TickFunction* mHead;
		};

		struct TickScheduleDetails 
		{
			TickScheduleDetails(TickFunction* inTickfunction, const float inCooldown, bool bInDeferredRemove = false)
				:mTickFunction(inTickfunction)
				,mCooldown(inCooldown)
				,bDeferredRemove(bInDeferredRemove)
			{}
			TickFunction* mTickFunction;
			float mCooldown;
			bool bDeferredRemove;
		};

	private:
		TickTaskSequencer&		mTickTaskSequencer;

		bool					bTickNewlySpawned;

		TSet<TickFunction*>		mAllEnabledTickFunctions;

		CoolingDownTickFunctionList	mAllCoolingDownTickFunctions;
		TSet<TickFunction*>		mAllDisabledTickFunctions;
		TArrayWithThreadsafeAdd<TickScheduleDetails>		mTickFunctionsToReschedule;
		TSet<TickFunction*>		mNewlySpawnedTickFunctions;
		TickContext				mContext;

	public:
		TickTaskLevel()
			:mTickTaskSequencer(TickTaskSequencer::get())
			, bTickNewlySpawned(false)
		{}
		~TickTaskLevel()
		{
			for (TSet<TickFunction*>::TIterator it(mAllEnabledTickFunctions); it; ++it)
			{
				(*it)->bRegistered = false;
			}

			for (TSet<TickFunction*>::TIterator it(mAllDisabledTickFunctions); it; ++it)
			{
				(*it)->bRegistered = false;
			}

			TickFunction* coolingDownNode = mAllCoolingDownTickFunctions.mHead;
			while (coolingDownNode)
			{
				coolingDownNode->bRegistered = false;
				coolingDownNode = coolingDownNode->mNext;
			}
		}

		int32 startFrame(const TickContext& inContext)
		{
			BOOST_ASSERT(!mNewlySpawnedTickFunctions.size());
			mContext.mTickGroup = ETickingGroup(0);
			mContext.mTickType = inContext.mTickType;
			mContext.mDeltaSeconds = inContext.mDeltaSeconds;
			mContext.mThread = ENamedThreads::GameThread;
			mContext.mWorld = inContext.mWorld;
			bTickNewlySpawned = true;
			int32 cooldownTicksEnabled = 0;
			{
				float cumulativeCooldown = 0.0f;
				TickFunction* tickFunction = mAllCoolingDownTickFunctions.mHead;
				while (tickFunction)
				{
					if (cumulativeCooldown + tickFunction->mRelativeTickCooldown >= mContext.mDeltaSeconds)
					{
						tickFunction->mRelativeTickCooldown -= (mContext.mDeltaSeconds);
						break;
					}
					cumulativeCooldown += tickFunction->mRelativeTickCooldown;

					tickFunction->mTickState = TickFunction::ETickState::Enabled;
					tickFunction = tickFunction->mNext;
					++cooldownTicksEnabled;
				}
			}
			return mAllEnabledTickFunctions.size() + cooldownTicksEnabled;
		}

		void queueAllTicks()
		{
			TickTaskSequencer& tts = TickTaskSequencer::get();
			for (TSet<TickFunction*>::TIterator it(mAllEnabledTickFunctions); it; ++it)
			{
				TickFunction* tickFunction = *it;
				tickFunction->queuTickFunction(tts, mContext);
				if (tickFunction->mTickInterval > 0.0f)
				{
					it.removeCurrent();
					mTickFunctionsToReschedule.add(TickScheduleDetails(tickFunction, tickFunction->mTickInterval));

				}
			}
			int32 enabledCooldownTicks = 0;
			float cumulativeCooldown = 0.0f;
			while (TickFunction* tickFunction = mAllCoolingDownTickFunctions.mHead)
			{
				if (tickFunction->mTickState == TickFunction::ETickState::Enabled)
				{
					cumulativeCooldown += tickFunction->mRelativeTickCooldown;
					tickFunction->queuTickFunction(tts, mContext);
					mTickFunctionsToReschedule.add(TickScheduleDetails(tickFunction, tickFunction->mTickInterval - (mContext.mDeltaSeconds - cumulativeCooldown)));
					mAllCoolingDownTickFunctions.mHead = tickFunction->mNext;
				}
				else
				{
					break;
				}
			}
			scheduleTickFunctionCooldowns();
		}
		void scheduleTickFunctionCooldowns()
		{
			if (mTickFunctionsToReschedule.size() > 0)
			{
				mTickFunctionsToReschedule.sort([](const TickScheduleDetails& A, const TickScheduleDetails& B) {
					return A.mCooldown < B.mCooldown;
				});
				int32 rescheduleIndex = 0;
				float cumulativeCooldown = 0.0f;
				TickFunction* prevComparisionTickFunction = nullptr;
				TickFunction* comparisonTickFunction = mAllCoolingDownTickFunctions.mHead;
				while (comparisonTickFunction && rescheduleIndex < mTickFunctionsToReschedule.size())
				{
					const float cooldownTime = mTickFunctionsToReschedule[rescheduleIndex].mCooldown;
					if ((cumulativeCooldown + comparisonTickFunction->mRelativeTickCooldown) > cooldownTime)
					{
						TickFunction* tickFunction = mTickFunctionsToReschedule[rescheduleIndex].mTickFunction;
						if (tickFunction->mTickState != TickFunction::ETickState::Disabled)
						{
							if (mTickFunctionsToReschedule[rescheduleIndex].bDeferredRemove)
							{
								BOOST_ASSERT(mAllEnabledTickFunctions.remove(tickFunction) == 1);
							}
							tickFunction->mTickState = TickFunction::ETickState::CoolingDown;
							tickFunction->mRelativeTickCooldown = cooldownTime - cumulativeCooldown;
							if (prevComparisionTickFunction)
							{
								prevComparisionTickFunction->mNext = tickFunction;
							}
							else
							{
								BOOST_ASSERT(comparisonTickFunction == mAllCoolingDownTickFunctions.mHead);
								mAllCoolingDownTickFunctions.mHead = tickFunction;
							}
							tickFunction->mNext = comparisonTickFunction;
							prevComparisionTickFunction = tickFunction;
							comparisonTickFunction->mRelativeTickCooldown -= tickFunction->mRelativeTickCooldown;
							cumulativeCooldown += tickFunction->mRelativeTickCooldown;
						}
						++rescheduleIndex;
					}
					else
					{
						cumulativeCooldown += comparisonTickFunction->mRelativeTickCooldown;
						prevComparisionTickFunction = comparisonTickFunction;
						comparisonTickFunction = comparisonTickFunction->mNext;
					}

				}
				for (; rescheduleIndex < mTickFunctionsToReschedule.size(); ++rescheduleIndex)
				{
					TickFunction* tickFunction = mTickFunctionsToReschedule[rescheduleIndex].mTickFunction;
					BOOST_ASSERT(tickFunction);
					if (tickFunction->mTickState != TickFunction::ETickState::Disabled)
					{
						if (mTickFunctionsToReschedule[rescheduleIndex].bDeferredRemove)
						{
							BOOST_ASSERT(mAllEnabledTickFunctions.remove(tickFunction) == 1);
						}
						const float cooldownTime = mTickFunctionsToReschedule[rescheduleIndex].mCooldown;
						tickFunction->mTickState = TickFunction::ETickState::CoolingDown;
						tickFunction->mRelativeTickCooldown = cooldownTime - cumulativeCooldown;
						tickFunction->mNext = nullptr;
						if (prevComparisionTickFunction)
						{
							prevComparisionTickFunction->mNext = tickFunction;
						}
						else
						{
							BOOST_ASSERT(comparisonTickFunction == mAllCoolingDownTickFunctions.mHead);
							mAllCoolingDownTickFunctions.mHead = tickFunction;
						}
						prevComparisionTickFunction = tickFunction;
						cumulativeCooldown += tickFunction->mRelativeTickCooldown; 
					}
				}
				mTickFunctionsToReschedule.clear();
			}
		}

		void runPauseFrame(const TickContext& context)
		{
		}

		int32 queueNewlySpawned(ETickingGroup currentTickgroup)
		{
			mContext.mTickGroup = currentTickgroup;
			int32 num = 0;
			TickTaskSequencer& tts = TickTaskSequencer::get();
			for (TSet<TickFunction*>::TIterator it(mNewlySpawnedTickFunctions); it; ++it)
			{
				TickFunction* tickFunction = *it;
				tickFunction->queuTickFunction(tts, mContext);
				num++;
				if (tickFunction->mTickInterval > 0.0f)
				{
					mAllEnabledTickFunctions.remove(tickFunction);
					mTickFunctionsToReschedule.add(TickScheduleDetails(tickFunction, tickFunction->mTickInterval));

				}
			}
			scheduleTickFunctionCooldowns();
			mNewlySpawnedTickFunctions.empty();
			return num;
		}


		void logAndDisardRunawayNewlySpawned(ETickingGroup currentTickGroup)
		{
			mContext.mTickGroup = currentTickGroup;
			TickTaskSequencer& tts = TickTaskSequencer::get();
			for (TSet<TickFunction*>::TIterator it(mNewlySpawnedTickFunctions); it; ++it)
			{
				TickFunction* tickFunction = *it;
				if (tickFunction->mTickInterval > 0.0f)
				{
					mAllEnabledTickFunctions.remove(tickFunction);
					mTickFunctionsToReschedule.add(TickScheduleDetails(tickFunction, tickFunction->mTickInterval));
				}
			}
			scheduleTickFunctionCooldowns();
			mNewlySpawnedTickFunctions.empty();
		}

		void endFrame()
		{
			bTickNewlySpawned = false;
			BOOST_ASSERT(!mNewlySpawnedTickFunctions.size());
		}

		bool hasTickFunction(TickFunction* tickFunction)
		{
			return mAllEnabledTickFunctions.contains(tickFunction) || mAllDisabledTickFunctions.contains(tickFunction) || mAllCoolingDownTickFunctions.contains(tickFunction);
		}




		void addTickFunction(TickFunction* tickFunction)
		{
			BOOST_ASSERT(!hasTickFunction(tickFunction));
			if (tickFunction->mTickState == TickFunction::ETickState::Enabled)
			{
				mAllEnabledTickFunctions.add(tickFunction);
				if (bTickNewlySpawned)
				{
					mNewlySpawnedTickFunctions.add(tickFunction);
				}
			}
			else
			{
				BOOST_ASSERT(tickFunction->mTickState == TickFunction::ETickState::Disabled);
				mAllDisabledTickFunctions.add(tickFunction);
			}
		}

		void removeTickFunction(TickFunction* tickFunction)
		{
			switch (tickFunction->mTickState)
			{
			case TickFunction::ETickState::Enabled:
				if (tickFunction->mTickInterval > 0.0f)
				{
					if (mAllEnabledTickFunctions.remove(tickFunction) == 0)
					{
						TickFunction* prevComparsisionFunction = nullptr;
						TickFunction* comparisonFunction = mAllCoolingDownTickFunctions.mHead;
						bool bFound = false;
						while (comparisonFunction && !bFound)
						{
							if (comparisonFunction == tickFunction)
							{
								bFound = true;
								if (prevComparsisionFunction)
								{
									prevComparsisionFunction->mNext = tickFunction;
								}
								else
								{
									BOOST_ASSERT(tickFunction == mAllCoolingDownTickFunctions.mHead);
									mAllCoolingDownTickFunctions.mHead = tickFunction->mNext;
								}
								tickFunction->mNext = nullptr;
							}
							else
							{
								prevComparsisionFunction = comparisonFunction;
								comparisonFunction = comparisonFunction->mNext;
							}
						}
						BOOST_ASSERT(bFound);
					}
				}
				else
				{
					BOOST_ASSERT(mAllEnabledTickFunctions.remove(tickFunction) == 1);
				}
				break;
			case  TickFunction::ETickState::Disabled:
				BOOST_ASSERT(mAllDisabledTickFunctions.remove(tickFunction) == 1);
				break;
			case TickFunction::ETickState::CoolingDown:
				TickFunction* prevComparisionFunctions = nullptr;
				TickFunction* comparisonFunction = mAllCoolingDownTickFunctions.mHead;
				bool bFound = false;
				while (comparisonFunction && !bFound)
				{
					if (comparisonFunction == tickFunction)
					{
						bFound = true;
						if (prevComparisionFunctions)
						{
							prevComparisionFunctions->mNext = tickFunction->mNext;
						}
						else
						{
							BOOST_ASSERT(tickFunction == mAllCoolingDownTickFunctions.mHead);
							mAllCoolingDownTickFunctions.mHead = tickFunction->mNext;
						}
						if (tickFunction->mNext)
						{
							tickFunction->mNext->mRelativeTickCooldown += tickFunction->mRelativeTickCooldown;
							tickFunction->mNext = nullptr;
						}
					}
					else
					{
						prevComparisionFunctions = comparisonFunction;
						comparisonFunction = comparisonFunction->mNext;
					}
				}
				BOOST_ASSERT(bFound);
				break;
			}
			if (bTickNewlySpawned)
			{
				mNewlySpawnedTickFunctions.remove(tickFunction);
			}
		}
	};


	
	

	class TickTaskManager : public TickTaskManagerInterface
	{
	public:
		static TickTaskManager& get()
		{
			static TickTaskManager singletonInstance;
			return singletonInstance;
		}

		virtual TickTaskLevel* allocateTickTaskLevel() override
		{
			return new TickTaskLevel();
		}

		virtual void freeTickTaskLevel(TickTaskLevel* tickTaskLevel) override
		{
			delete tickTaskLevel;
		}

		virtual void startFrame(World* inWorld, float deltaSeconds, ELevelTick tickType, const TArray<Level*>& levelsToTick) override
		{
			mContext.mTickGroup = ETickingGroup(0);
			mContext.mDeltaSeconds = deltaSeconds;
			mContext.mTickType = tickType;
			mContext.mThread = ENamedThreads::GameThread;
			mContext.mWorld = inWorld;

			bTickNewLySpawned = true;
			mTickTaskSequencer.startFrame();
			fillLevelList(levelsToTick);
			int32 numWorkerThread = 0;
			bool bConcurrentQueue = false;
			if (!bConcurrentQueue)
			{
				int32 totalTickFunctions = 0;
				for (int32 levelIndex = 0; levelIndex < mLevelList.size(); levelIndex++)
				{
					totalTickFunctions += mLevelList[levelIndex]->startFrame(mContext);
				}
				for (int32 levelIndex = 0; levelIndex < mLevelList.size(); levelIndex++)
				{
					mLevelList[levelIndex]->queueAllTicks();
				}
			}
			else
			{

			}
		}

		virtual void runPauseFrame(World* inWorld, float deltaSeconds, ELevelTick tickType, const TArray<Level*>& levelsToTick) override
		{
			bTickNewLySpawned = true;
			mContext.mTickGroup = ETickingGroup(0);
			mContext.mDeltaSeconds = deltaSeconds;
			mContext.mTickType = tickType;
			mContext.mThread = ENamedThreads::GameThread;
			mContext.mWorld = inWorld;
			fillLevelList(levelsToTick);
			for (int32 levelIndex = 0; levelIndex < mLevelList.size(); levelIndex++)
			{
				mLevelList[levelIndex]->runPauseFrame(mContext);
			}

			mContext.mWorld = nullptr;
			bTickNewLySpawned = false;
			mLevelList.clear();
		}

		virtual void runTickGroup(ETickingGroup group, bool bBlockTillComplete) override
		{
			BOOST_ASSERT(mContext.mTickGroup == group);
			BOOST_ASSERT(bTickNewLySpawned);
			mTickTaskSequencer.releaseTickGroup(group, bBlockTillComplete);
			mContext.mTickGroup = ETickingGroup(mContext.mTickGroup + 1);
			if (bBlockTillComplete)
			{
				bool bFinished = false;
				for (int32 iterations = 0; iterations < 101; iterations++)
				{
					int32 num = 0;
					for (int32 levelIndex = 0; levelIndex < mLevelList.size(); levelIndex++)
					{
						num += mLevelList[levelIndex]->queueNewlySpawned(mContext.mTickGroup);

					}
					if (num && mContext.mTickGroup == TG_NewlySpawned)
					{
						mTickTaskSequencer.releaseTickGroup(TG_NewlySpawned, true);
					}
					else
					{
						bFinished = true;
						break;
					}
				}
				if (!bFinished)
				{
					for (int32 levelIndex = 0; levelIndex < mLevelList.size(); levelIndex++)
					{
						mLevelList[levelIndex]->logAndDisardRunawayNewlySpawned(mContext.mTickGroup);
					}
				}
			}
		}

		virtual void endFrame() override
		{
			mTickTaskSequencer.endFrame();
			bTickNewLySpawned = false;
			for (int32 levelIndex = 0; levelIndex < mLevelList.size(); levelIndex++)
			{
				mLevelList[levelIndex]->endFrame();
			}
			mContext.mWorld = nullptr;
			mLevelList.clear();
		}
		TickTaskLevel* tickTaskLevelForLevel(Level* level)
		{
			BOOST_ASSERT(level);
			BOOST_ASSERT(level->mTickTaskLevel);
			return level->mTickTaskLevel;
		}

		bool hasTickFunction(Level* inLevel, TickFunction* tickFunction)
		{
			TickTaskLevel* level = tickTaskLevelForLevel(inLevel);
			return level->hasTickFunction(tickFunction);
		}

		void addTickFunction(Level* inLevel, TickFunction* tickFunction)
		{
			BOOST_ASSERT(tickFunction->mTickGroup >= 0 && tickFunction->mTickGroup < TG_MAX);
			TickTaskLevel* level = tickTaskLevelForLevel(inLevel);
			level->addTickFunction(tickFunction);
			tickFunction->mTickTaskLevel = level;
		}

		void removeTickFunction(TickFunction* tickFunction)
		{
			TickTaskLevel* level = tickFunction->mTickTaskLevel;
			BOOST_ASSERT(level);
			level->removeTickFunction(tickFunction);
		}
	private:
		TickTaskManager()
			:mTickTaskSequencer(TickTaskSequencer::get())
			, bTickNewLySpawned(false)
		{

		}
		void fillLevelList(const TArray<Level*>& levels)
		{
			BOOST_ASSERT(!mLevelList.size());
			if (!mContext.mWorld->getActiveLevelCollection() || mContext.mWorld->getActiveLevelCollection()->getType() == ELevelCollectionType::DynamicSourceLevels)
			{
				BOOST_ASSERT(mContext.mWorld->mTickTaskLevel);
				mLevelList.add(mContext.mWorld->mTickTaskLevel);
			}
			for (int32 levelIndex = 0; levelIndex < levels.size(); levelIndex++)
			{
				Level* level = levels[levelIndex];
				if (level->bIsVisible)
				{
					BOOST_ASSERT(level->mTickTaskLevel);
					mLevelList.add(level->mTickTaskLevel);
				}
			}
		}




		TickTaskSequencer&	mTickTaskSequencer;
		TArray<TickTaskLevel*>	mLevelList;
		TickContext			mContext;
		bool				bTickNewLySpawned;
		TArray<TickFunction*> mAllTickFunctions;
	};






	TickFunction::TickFunction()
		:mTickGroup(TG_PrePhysics)
		,mEndTickGroup(TG_PrePhysics)
		,mActualStartTickGroup(TG_PrePhysics)
		,mActualEndTickGroup(TG_PrePhysics)
		,bTickEvenWhenPaused(false)
		,bCanEverTick(false)
		,bHighPriority(false)
		,bRunOnAnyThread(false)
		,bRegistered(false)
		,bWasInterval(false)
		,mTickState(ETickState::Enabled)
		,mTickVisitedGFrameCounter(0)
		,mTickQueueGFrameCounter(0)
		,mRelativeTickCooldown(0.0f)
		,mLastTickGameTimeSeconds(-1.0f)
		,mTickInterval(0.0f)
		,mTickTaskLevel(nullptr)
		,mTaskPointer(nullptr)
	{

	}

	TickFunction::~TickFunction()
	{
		unRegisterTickFunction();
	}

	void TickFunction::registerTickFunction(class Level* level)
	{
		if (!bRegistered)
		{
			if (true)
			{
				TickTaskManager::get().addTickFunction(level, this);
				bRegistered = true;
			}
		}
		else
		{
			BOOST_ASSERT(TickTaskManager::get().hasTickFunction(level, this));
		}
	}

	void TickFunction::unRegisterTickFunction()
	{
		if (bRegistered)
		{
			TickTaskManager::get().removeTickFunction(this);
			bRegistered = false;
		}
	}

	FORCEINLINE bool canDemoteIntoTickGroup(ETickingGroup TickGroup)
	{
		switch (TickGroup)
		{
		case TG_StartPhysics:
		case  TG_EndPhysics:
			return false;
		}
		return true;
	}

	void TickFunction::queuTickFunction(class TickTaskSequencer& tts, const TickContext& tickContext)
	{
		BOOST_ASSERT(tickContext.mThread == ENamedThreads::GameThread);
		BOOST_ASSERT(bRegistered);
		if (mTickVisitedGFrameCounter != GFrameCounter)
		{
			mTickVisitedGFrameCounter = GFrameCounter;
			if (mTickState != TickFunction::ETickState::Disabled)
			{
				ETickingGroup maxPrerequisiteTickGroup = ETickingGroup(0);
				GraphEventArray taskPrerequisites;
				for (int32 prereqIndex = 0; prereqIndex < mPrerequisites.size(); prereqIndex++)
				{
					TickFunction* prereq = mPrerequisites[prereqIndex].get();
					if (!prereq)
					{
						mPrerequisites.removeAtSwap(prereqIndex--);
					}
					else if (prereq->bRegistered)
					{
						prereq->queuTickFunction(tts, tickContext);
						if (prereq->mTickQueueGFrameCounter != GFrameCounter)
						{

						}
						else if (!prereq->mTaskPointer)
						{

						}
						else
						{
							maxPrerequisiteTickGroup = Math::max<ETickingGroup>(maxPrerequisiteTickGroup, prereq->mActualStartTickGroup);
							taskPrerequisites.add(prereq->getCompletionHandle());
						}
					}
				}
				ETickingGroup myActualTickGroup = Math::max<ETickingGroup>(maxPrerequisiteTickGroup, Math::max<ETickingGroup>(mTickGroup, tickContext.mTickGroup));
				if (myActualTickGroup != mTickGroup)
				{
					while (!canDemoteIntoTickGroup(myActualTickGroup))
					{
						myActualTickGroup = ETickingGroup(myActualTickGroup + 1);
					}
				}
				mActualStartTickGroup = myActualTickGroup;
				mActualEndTickGroup = myActualTickGroup;
				if (mEndTickGroup > mActualStartTickGroup)
				{
					BOOST_ASSERT(mEndTickGroup <= TG_NewlySpawned);
					ETickingGroup testTickGroup = ETickingGroup(mActualEndTickGroup + 1);
					while (testTickGroup <= mEndTickGroup)
					{
						if (canDemoteIntoTickGroup(testTickGroup))
						{
							mActualEndTickGroup = testTickGroup;
						}
						testTickGroup = ETickingGroup(testTickGroup + 1);
					}
				}
				if (mTickState == TickFunction::ETickState::Enabled)
				{
					tts.queueTickTask(&taskPrerequisites, this, tickContext);
				}
			}
			mTickQueueGFrameCounter = GFrameCounter;
		}
	}

	GraphEventRef TickFunction::getCompletionHandle() const
	{
		BOOST_ASSERT(mTaskPointer);
		TGraphTask<TickFunctionTask>* task = (TGraphTask<TickFunctionTask>*)mTaskPointer;
		return task->getCompletionEvent();
	}

	float TickFunction::calculateDeltaTime(const TickContext& tickContext)
	{
		float deltaTimeForFunction = tickContext.mDeltaSeconds;
		if (mTickInterval == 0.f)
		{
			mLastTickGameTimeSeconds = -1.0f;
		}
		else
		{
			const float currentWorldTime = (bTickEvenWhenPaused ? tickContext.mWorld->getUnpausedTimeSeconds() : tickContext.mWorld->getTimeSecondes());
			if (mLastTickGameTimeSeconds >= 0.f)
			{
				deltaTimeForFunction = currentWorldTime - mLastTickGameTimeSeconds;
			}
			mLastTickGameTimeSeconds = currentWorldTime;
		}
		return deltaTimeForFunction;
	}

	TickTaskManagerInterface& TickTaskManagerInterface::get()
	{
		return TickTaskManager::get();
	}

	void TickFunction::setTickFunctionEnable(bool bInEnable)
	{
		if (bRegistered && (bInEnable == (mTickState == ETickState::Disabled)))
		{
			BOOST_ASSERT(mTickTaskLevel);
			mTickTaskLevel->removeTickFunction(this);
			mTickState = (bInEnable ? ETickState::Enabled : ETickState::Disabled);
			mTickTaskLevel->addTickFunction(this);
		}
		else
		{
			mTickState = (bInEnable ? ETickState::Enabled : ETickState::Disabled);
		}
	}
	void TickFunction::removePrerequisite(Object* targetObject, struct TickFunction& targetTickFunction) 
	{
		mPrerequisites.removeSwap(TickPrerequisite(targetObject, targetTickFunction));
	}

	void TickFunction::addPrerequisite(Object* targetObject, struct TickFunction& targetTickFunction)
	{
		const bool bThisCanTick = (bCanEverTick || isTickFunctionRegistered());
		const bool bTargetCanTick = (targetTickFunction.bCanEverTick || targetTickFunction.isTickFunctionRegistered());
		if (bThisCanTick || bTargetCanTick)
		{
			mPrerequisites.addUnique(TickPrerequisite(targetObject, targetTickFunction));
		}
	}
}