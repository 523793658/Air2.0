#include "Async/TaskGraph.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "HAL/ThreadSafeCounter.h"
#include "HAL/PlatformTLS.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "HAL/PlatformAffinity.h"
#include <algorithm>
#include "boost/boost/lexical_cast.hpp"
#include "Containers/Array.h"
#include "HAL/AirMemory.h"
#include "Containers/LockFreeListImpl.h"
#include "CoreGlobals.h"
#include "Misc/ScopedEvent.h"
#include "Template/AirTemplate.h"
namespace Air
{

#if 0
#else
#define CREATE_HIPRI_TASK_THREADS (0)
#define CREATE_BACKGROUND_TASK_THREADS (0)
#endif

#define PLATFORM_OK_TO_BURN_CPU (0)

	class TaskGraphImplementation;
	struct WorkerThread;

	static TaskGraphImplementation* TaskGraphImplementationSingleton = nullptr;

	static int32 GFastScheduler = 0;
	static int32 GFastSchedulerLatched = 0;

	static int32 GNumWorkerThreadsToIgnore = 0;

	static int32 GConsoleSpinModeLatched = 0;

	static int32 GConsoleSpinMode = PLATFORM_OK_TO_BURN_CPU * 2;

	std::mutex Mutex_FTriggerEventGraphTask;

#define AtomicStateBitfield_MAX_THREADS (13)

	namespace ENamedThreads
	{
		CORE_API Type RenderThread = ENamedThreads::GameThread;
		CORE_API Type RenderThread_Local = ENamedThreads::GameThread_Local;
		CORE_API int32 bHasBackgroundThreads = CREATE_BACKGROUND_TASK_THREADS;
		CORE_API int32 bHasHighPriorityThreads = CREATE_HIPRI_TASK_THREADS;
	}

	class TaskQueue
	{
	public:
		TaskQueue()
		{
		}

		FORCEINLINE int32 num() const
		{
			checkInvariants();
			return mEndIndex - mStartIndex;
		}

		FORCEINLINE void enqueue(BaseGraphTask* task)
		{
			checkInvariants();
			if (mEndIndex >= mTasks.size())
			{
				if (mStartIndex >= ARRAY_EXPAND)
				{
					ASSUME(mTasks[mStartIndex - 1] == nullptr);
					ASSUME(mTasks[0] == nullptr);
					Memory::memmove(&mTasks[0], &mTasks[mStartIndex], (mEndIndex - mStartIndex) * sizeof(BaseGraphTask*));
					mEndIndex -= mStartIndex;
					Memory::memzero(&mTasks[mEndIndex], mStartIndex * sizeof(BaseGraphTask*));
					mStartIndex = 0;
				}
				else
				{
					mTasks.addZeroed(ARRAY_EXPAND);
				}
			}
			ASSUME(mEndIndex < mTasks.size() && mTasks[mEndIndex] == nullptr);
			mTasks[mEndIndex++] = task;
		}

		FORCEINLINE BaseGraphTask* dequeue()
		{
			if (!num())
			{
				return nullptr;
			}
			BaseGraphTask* returnValue = mTasks[mStartIndex];
			mTasks[mStartIndex++] = nullptr;
			if (mStartIndex == mEndIndex)
			{
				mStartIndex = 0;
				mEndIndex = 0;
			}
			return returnValue;
		}

	private:
		enum
		{
			ARRAY_EXPAND = 1024
		};

		void checkInvariants() const
		{
			ASSUME(mStartIndex <= mEndIndex);
			ASSUME(mEndIndex <= mTasks.size());
			ASSUME(mStartIndex >= 0 && mEndIndex >= 0);
		}

		TArray<BaseGraphTask*> mTasks;

		int32 mStartIndex{ 0 };
		int32 mEndIndex{ 0 };

	};

	static BaseGraphTask::TSmallTaskAllocator TheSmallTaskAllocator;

	BaseGraphTask::TSmallTaskAllocator& BaseGraphTask::getSmallTaskAllocator()
	{
		return TheSmallTaskAllocator;
	}

	static struct ChaosMode
	{
		enum
		{
			NumSamples = 45771
		};
		ThreadSafeCounter mCurrent;
		float mDelayTimes[NumSamples + 1];
		int32 mEnable;
		ChaosMode()
			:mEnable(0)
		{
			
		}
	};

	class TaskThreadBase : public Runnable
	{
	public:
		virtual uint32 run() override
		{
			processTasksUntilQuit(0);
			return 0;
		}

		void setup(ENamedThreads::Type inThreadId, uint32 InPerThreadIDTLSSlot, WorkerThread* inOwnerWorker)
		{
			mThreadId = inThreadId;
			mPerThreadIDTLSSlot = InPerThreadIDTLSSlot;
			mOwnerWorker = inOwnerWorker;
		}

		void initializeForCurrentThread()
		{
			PlatformTLS::setTlsValue(mPerThreadIDTLSSlot, mOwnerWorker);
		}

		virtual void processTasksUntilQuit(int32 QueueIndex) = 0;

		virtual bool isProcessingTasks(int32 queueIndex) = 0;

		virtual void processTasksUntilIdle(int32 queueIndex)
		{
			BOOST_ASSERT(false);
		}

		virtual void wakeup()
		{
			BOOST_ASSERT(false);
		}

		ENamedThreads::Type getThreadId() const
		{
			ASSUME(mOwnerWorker);
			return mThreadId;
		}

		virtual void enqueueFromThisThread(int32 queueIndex, BaseGraphTask* task)
		{
			BOOST_ASSERT(false);
		}

		virtual bool enqueueFromOtherThread(int32 queueIndex, BaseGraphTask* task)
		{
			ASSUME(false);
			return false;
		}

		virtual void requestQuit(int32 queueIndex) = 0;
	protected:
		ENamedThreads::Type mThreadId;

		uint32 mPerThreadIDTLSSlot;
		
		WorkerThread* mOwnerWorker;

		TArray<BaseGraphTask*> mNewTasks;

	};

	class NamedTaskThread : public TaskThreadBase
	{
	public:
		void processTasksUntilQuit(int32 QueueIndex) override
		{
			queue(QueueIndex).mQuitWhenIdle.reset();
			++queue(QueueIndex).mRecursionGuard;
			while (queue(QueueIndex).mQuitWhenIdle.getValue() == 0)
			{
				processTasksNamedThread(QueueIndex, true);
				if (!PlatformProcess::supportsMultithreading())
				{
					break;
				}
			}
			--queue(QueueIndex).mRecursionGuard;
		}

		virtual bool isProcessingTasks(int32 queueIndex)
		{
			return !!queue(queueIndex).mRecursionGuard;
		}

		virtual void processTasksUntilIdle(int32 queueIndex) override
		{
			BOOST_VERIFY(++queue(queueIndex).mRecursionGuard == 1);
			processTasksNamedThread(queueIndex, false);
			BOOST_VERIFY(!--queue(queueIndex).mRecursionGuard);
		}

		virtual bool enqueueFromOtherThread(int32 queueIndex, BaseGraphTask* task) override
		{
			bool bWasReopenedByMe;
			bool bHiPri = !!ENamedThreads::getTaskPriority(task->mThreadToExecuteOn);
			{
				bWasReopenedByMe = queue(queueIndex).mIncomingQueue.reopenIfClosedAndPush(task);
			}
			if (bHiPri)
			{
				queue(queueIndex).mOutstandingHiPriTasks.increment();
			}
			if (bWasReopenedByMe)
			{
				queue(queueIndex).mStallRestartEvent->trigger();
			}
			return bWasReopenedByMe;
		}

		virtual void requestQuit(int32 queueIndex) override
		{
			if (!queue(0).mStallRestartEvent)
			{
				return;
			}
			if (queueIndex == -1)
			{
				queueIndex = 0;
			}
			queue(queueIndex).mQuitWhenIdle.increment();
			queue(queueIndex).mStallRestartEvent->trigger();
		}

		virtual void enqueueFromThisThread(int32 queueIndex, BaseGraphTask* task) override
		{
			BOOST_ASSERT(task && queue(queueIndex).mStallRestartEvent);
			if (ENamedThreads::getTaskPriority(task->mThreadToExecuteOn))
			{
				queue(queueIndex).mPrivateQueueHiPri.enqueue(task);
			}
			else
			{
				queue(queueIndex).mPrivateQueue.enqueue(task);
			}
		}

	private:
		struct ThreadTaskQueue
		{
			TaskQueue mPrivateQueue;
			TaskQueue mPrivateQueueHiPri;

			uint8 mPadToAvoidContention1[PLATFORM_CACHE_LINE_SIZE];
			ThreadSafeCounter mOutstandingHiPriTasks;
			uint8 mPadToAvoidContention2[PLATFORM_CACHE_LINE_SIZE];

			uint32 mRecursionGuard;

			TReopenableLockFreePointerListLIFO<BaseGraphTask> mIncomingQueue;

			Event* mStallRestartEvent;

			ThreadSafeCounter			mQuitWhenIdle;

			ThreadTaskQueue()
				: mRecursionGuard(0)
				, mStallRestartEvent(PlatformProcess::getSynchEventFromPool(true))
			{

			}
			~ThreadTaskQueue()
			{
				PlatformProcess::returnSynchEventToPool(mStallRestartEvent);
				mStallRestartEvent = nullptr;
			}
		};

		void processTasksNamedThread(int32 queueIndex, bool bAllowStall)
		{
			bool bCountAsStall = false;

			while (1)
			{
				BaseGraphTask* task = nullptr;
				task = queue(queueIndex).mPrivateQueueHiPri.dequeue();
				if (!task)
				{
					if (queue(queueIndex).mOutstandingHiPriTasks.getValue())
					{
						queue(queueIndex).mIncomingQueue.popAll(mNewTasks);
						if (mNewTasks.size())
						{
							for (int32 index = mNewTasks.size() - 1; index >= 0; index--)
							{
								BaseGraphTask* newTask = mNewTasks[index];
								if (ENamedThreads::getTaskPriority(newTask->mThreadToExecuteOn))
								{
									queue(queueIndex).mPrivateQueueHiPri.enqueue(newTask);
									queue(queueIndex).mOutstandingHiPriTasks.decrement();
								}
								else
								{
									queue(queueIndex).mPrivateQueue.enqueue(newTask);
								}
							}
							mNewTasks.clear();
							task = queue(queueIndex).mPrivateQueueHiPri.dequeue();
						}
					}
					if (!task)
					{
						task = queue(queueIndex).mPrivateQueue.dequeue();
					}
				}
				if (!task)
				{
					queue(queueIndex).mIncomingQueue.popAll(mNewTasks);
					if (!mNewTasks.size())
					{
						if (bAllowStall)
						{
							if (stall(queueIndex, bCountAsStall))
							{
								queue(queueIndex).mIncomingQueue.popAll(mNewTasks);
							}
						}
					}
					if (mNewTasks.size())
					{
						for (int32 index = mNewTasks.size() - 1; index >= 0; index--)
						{
							BaseGraphTask* newTask = mNewTasks[index];
							if (ENamedThreads::getTaskPriority(newTask->mThreadToExecuteOn))
							{
								queue(queueIndex).mPrivateQueueHiPri.enqueue(newTask);
								queue(queueIndex).mOutstandingHiPriTasks.decrement();
							}
							else
							{
								queue(queueIndex).mPrivateQueue.enqueue(newTask);
							}
						}
						mNewTasks.clear();
						task = queue(queueIndex).mPrivateQueueHiPri.dequeue();
						if (!task)
						{
							task = queue(queueIndex).mPrivateQueue.dequeue();
						}
					}
				}
				if (task)
				{
					task->execute(mNewTasks, ENamedThreads::Type(mThreadId | (queueIndex << ENamedThreads::QueueIndexShift)));

				}
				else
				{
					break;
				}
			}
		}

		bool stall(int32 queueIndex, bool bCountAsStall)
		{
			return true;
		}

		FORCEINLINE ThreadTaskQueue& queue(int32 QueueIndex)
		{
			ASSUME(QueueIndex >= 0 && QueueIndex < ENamedThreads::NumQueue);
			return mQueues[QueueIndex];
		}



		FORCEINLINE const ThreadTaskQueue& queue(int32 queueIndex) const
		{
			ASSUME(queueIndex >= 0 && queueIndex < ENamedThreads::NumQueue);
			return mQueues[queueIndex];
		}

	public:
		ThreadTaskQueue mQueues[ENamedThreads::NumQueue];
	};

	class TaskThreadAnyThread : public TaskThreadBase
	{
	public:
		TaskThreadAnyThread(int32 inPriorityIndex)
			:mPriorityIndex(inPriorityIndex)
		{

		}

		void processTasksUntilQuit(int32 QueueIndex) override
		{
			if (mPriorityIndex != (ENamedThreads::BackgroundThreadPriority >> ENamedThreads::ThreadPriorityShift))
			{
				Memory::setupTLSCachesOnCurrentThread();
			}
			BOOST_ASSERT(!QueueIndex);
			mQueue.mQuitWhenIdle.reset();
			while (mQueue.mQuitWhenIdle.getValue() == 0)
			{
				processTasks();
				if (!PlatformProcess::supportsMultithreading())
				{
					break;
				}
			}
		}

		virtual void processTasksUntilIdle(int32 queueIndex) override
		{
			if (!PlatformProcess::supportsMultithreading())
			{
				mQueue.mQuitWhenIdle.set(1);
				processTasks();
				mQueue.mQuitWhenIdle.reset();
			}
			else
			{
				TaskThreadBase::processTasksUntilIdle(queueIndex);
			}
		}




		virtual bool isProcessingTasks(int32 queueIndex) override
		{
			return !!mQueue.mRecursionGuard;
		}

		virtual void requestQuit(int32 queueIndex)
		{
			BOOST_ASSERT(queueIndex < 1);
			BOOST_ASSERT(mQueue.mStallRestartEvent);

			mQueue.mQuitWhenIdle.increment();
			mQueue.mStallRestartEvent->trigger();
		}

		virtual void wakeup() final override
		{
			mQueue.mStallRestartEvent->trigger();
		}
	private:
		void processTasks()
		{
			bool bCountAsStall = true;
			BOOST_ASSERT(++mQueue.mRecursionGuard == 1);
			while (1)
			{
				BaseGraphTask* task = findWork();
				if (!task)
				{
					stall(bCountAsStall);
					if (mQueue.mQuitWhenIdle.getValue())
					{
						break;
					}
					continue;
				}
				task->execute(mNewTasks, ENamedThreads::Type(mThreadId));
			}
			BOOST_ASSERT(!--mQueue.mRecursionGuard);
		}

		BaseGraphTask* findWork();

		void stall(bool bCountAsStall)
		{
			BOOST_ASSERT(mQueue.mStallRestartEvent);
			if (mQueue.mQuitWhenIdle.getValue() == 0)
			{
				if (PlatformProcess::supportsMultithreading())
				{
					PlatformMisc::memoryBarrier();
					notifyStalling();
					mQueue.mStallRestartEvent->wait(std::numeric_limits<uint32>::max(), bCountAsStall);
					mQueue.mStallRestartEvent->reset();
				}
			}
		}

		void notifyStalling();


	private:
		struct ThreadTaskQueue
		{
			uint32 mRecursionGuard;
			ThreadSafeCounter mQuitWhenIdle;

			Event* mStallRestartEvent;

			ThreadTaskQueue()
				:mRecursionGuard(0)
				,mStallRestartEvent(PlatformProcess::getSynchEventFromPool(true))
			{}
			~ThreadTaskQueue()
			{
				PlatformProcess::returnSynchEventToPool(mStallRestartEvent);
				mStallRestartEvent = nullptr;
			}
		};

		ThreadTaskQueue mQueue;
		int32 mPriorityIndex;
	};



	struct WorkerThread
	{
		TaskThreadBase* mTaskGraphWorker;
		RunnableThread*	mRunnableThread;
		bool			bAttached;

		WorkerThread()
			:mTaskGraphWorker(nullptr)
			,mRunnableThread(nullptr)
			,bAttached(false)
		{}
	};


	class TaskGraphImplementation : public TaskGraphInterface
	{
	public:
		static TaskGraphImplementation& get()
		{
			BOOST_ASSERT(TaskGraphImplementationSingleton);
			return *TaskGraphImplementationSingleton;
		}

		TaskGraphImplementation(int32)
		{
			mCreateHiPriorityThreads = !!ENamedThreads::bHasHighPriorityThreads;
			mCreateBackgroundThreads = !!ENamedThreads::bHasBackgroundThreads;
			int32 maxTaskThreads = MAX_THREADS;
			int32 numTaskThreads = PlatformMisc::numberOfWorkerThreadsToSpawn();

			if (!PlatformProcess::supportsMultithreading())
			{
				maxTaskThreads = 1;
				numTaskThreads = 1;
				mLastExternalThread = (ENamedThreads::Type)(ENamedThreads::ActualRenderingThread - 1);
				mCreateHiPriorityThreads = false;
				mCreateBackgroundThreads = false;
				ENamedThreads::bHasHighPriorityThreads = 0;
				ENamedThreads::bHasBackgroundThreads = 0;
			}
			else
			{
				mLastExternalThread = ENamedThreads::ActualRenderingThread;
			}
			mNumNamedThreads = mLastExternalThread + 1;
			mNumTaskThreadSets = 1 + mCreateHiPriorityThreads + mCreateBackgroundThreads;
			mNumThreads = std::max<int32>(std::min<int32>(numTaskThreads * mNumTaskThreadSets + mNumNamedThreads, MAX_THREADS), mNumNamedThreads + 1);

			mNumThreads = std::min(mNumThreads, mNumNamedThreads + numTaskThreads * mNumTaskThreadSets);

			mNumTaskThreadPerSet = (mNumThreads - mNumNamedThreads) / mNumTaskThreadSets;
			mReentrancyCheck.increment();
			mPerThreadIDTLSSlot = PlatformTLS::allocTLSSlot();

			for (int32 threadIndex = 0; threadIndex < mNumThreads; threadIndex++)
			{
				bool bAnyTaskThread = threadIndex >= mNumNamedThreads;
				if (bAnyTaskThread)
				{
					mWorkerThreads[threadIndex].mTaskGraphWorker = new TaskThreadAnyThread(threadIndexToPriorityIndex(threadIndex));
				}
				else
				{
					mWorkerThreads[threadIndex].mTaskGraphWorker = new NamedTaskThread();
				}
				mWorkerThreads[threadIndex].mTaskGraphWorker->setup(ENamedThreads::Type(threadIndex), mPerThreadIDTLSSlot, &mWorkerThreads[threadIndex]);
			}
			TaskGraphImplementationSingleton = this;

			for (int32 threadIndex = mLastExternalThread + 1; threadIndex < mNumThreads; ++threadIndex)
			{
				wstring name;

				int32 Priority = threadIndexToPriorityIndex(threadIndex);
				wstring pristr = boost::lexical_cast<wstring>(threadIndex - (mLastExternalThread + 1));
				EThreadPriority threadPri;
				if (Priority == 1)
				{
					name = L"TaskGraphThreadHP " + pristr;
					threadPri = TPri_SlightlyBelowNormal;
				}
				else if (Priority == 2)
				{
					name = L"TaskGraphThreadBP" + pristr;
					threadPri = TPri_Lowest;
				}
				else
				{
					name = L"TaskGraphThreadNP" + pristr;
					threadPri = TPri_BelowNormal;
				}

				uint32 StackSize = 384 * 1024;
				mWorkerThreads[threadIndex].mRunnableThread = RunnableThread::create(&thread(threadIndex), name.c_str(), StackSize, threadPri, PlatformAffinity::getTaskGraphThreadMask());
				mWorkerThreads[threadIndex].bAttached = true;
			}
		}

		TaskThreadBase& thread(int32 index)
		{
			return *mWorkerThreads[index].mTaskGraphWorker;
		}

		int32 threadIndexToPriorityIndex(int32 threadIndex)
		{
			int32 result = (threadIndex - mNumNamedThreads) / mNumTaskThreadPerSet;
			return result;
		}

		virtual void attachToThread(ENamedThreads::Type currentThread) override
		{
			currentThread = ENamedThreads::getThreadIndex(currentThread);
			thread(currentThread).initializeForCurrentThread();
		}

		virtual bool isThreadProcessingTasks(ENamedThreads::Type threadToCheck) final override
		{
			int32 queueIndex = ENamedThreads::getQueueIndex(threadToCheck);
			threadToCheck = ENamedThreads::getThreadIndex(threadToCheck);

			return thread(threadToCheck).isProcessingTasks(queueIndex);
		}

		ENamedThreads::Type getCurrentThread()
		{
			ENamedThreads::Type currentThreadIfKnown = ENamedThreads::AnyThread;
			WorkerThread* tlsPointer = (WorkerThread*)PlatformTLS::getTLSValue(mPerThreadIDTLSSlot);
			if (tlsPointer)
			{
				ASSUME(tlsPointer - mWorkerThreads >= 0 && tlsPointer - mWorkerThreads < mNumThreads);
				int32 threadIndex = tlsPointer - mWorkerThreads;
				ASSUME(thread(threadIndex).getThreadId() == threadIndex);

				if (threadIndex < mNumNamedThreads)
				{
					currentThreadIfKnown = ENamedThreads::Type(threadIndex);
				}
				else
				{
					int32 priority = (threadIndex - mNumNamedThreads) / mNumTaskThreadSets;
					currentThreadIfKnown = ENamedThreads::setPriorities(ENamedThreads::Type(threadIndex), priority, false);
				}
			}
			return currentThreadIfKnown;
		}

		virtual void processThreadUntilRequestReturn(ENamedThreads::Type currentThread) final override
		{
			int32 queueIndex = ENamedThreads::getQueueIndex(currentThread);
			currentThread = ENamedThreads::getThreadIndex(currentThread);
			thread(currentThread).processTasksUntilQuit(queueIndex);
		}

		virtual void processThreadUntilIdle(ENamedThreads::Type currentThread) final override
		{
			int32 queueIndex = ENamedThreads::getQueueIndex(currentThread);
			currentThread = ENamedThreads::getThreadIndex(currentThread);
			ASSUME(currentThread >= 0 && currentThread < mNumNamedThreads);
			ASSUME(currentThread == getCurrentThread());
			thread(currentThread).processTasksUntilIdle(queueIndex);
		}
		virtual int32 getNumWorkerThreads() final override
		{
			int32 result = (mNumThreads - mNumNamedThreads) / mNumTaskThreadSets - GNumWorkerThreadsToIgnore;
			ASSUME(result > 0);
			return result;
		}

		virtual void queueTask(class BaseGraphTask* task, ENamedThreads::Type threadToExecuteOn, ENamedThreads::Type inCurrentThreadIfKnown = ENamedThreads::AnyThread) final override
		{
			ASSUME(mNumTaskThreadPerSet);
			if (GFastSchedulerLatched != GFastScheduler && isInGameThread())
			{
				if (GFastSchedulerLatched != GFastScheduler)
				{
					if (GFastScheduler)
					{
						for (int32 priorityIndex = 0; priorityIndex < mNumTaskThreadSets; priorityIndex++)
						{
							mAtomicForConsoleApproach[priorityIndex] = AtomicStateBitfield();
							mAtomicForConsoleApproach[priorityIndex].mStalled = (1 << getNumWorkerThreads()) - 1;
						}
					}
					PlatformProcess::sleep(.1f);
					GFastSchedulerLatched = GFastScheduler;
					if (GFastSchedulerLatched)
					{
						GConsoleSpinModeLatched = 0;
					}
					else
					{
						startAllTaskThreads(true);
					}
				}
			}
			if (GFastSchedulerLatched && GConsoleSpinModeLatched != GConsoleSpinMode && isInGameThread())
			{
				bool bStartTask = GConsoleSpinMode && ((!GConsoleSpinModeLatched) != !!GConsoleSpinMode);
				GConsoleSpinModeLatched = GConsoleSpinMode;
				if (bStartTask)
				{
					for (int32 priority = 0; priority < ENamedThreads::NumThreadPriorities; priority++)
					{
						if (priority == (ENamedThreads::NormalThreadPriority >> ENamedThreads::ThreadPriorityShift) || (priority == (ENamedThreads::HighThreadPriority >> ENamedThreads::ThreadPriorityShift) && mCreateHiPriorityThreads))
						{
							startTaskThreadFastMode(priority, -1);
						}
					}
				}
			}
			if (ENamedThreads::getThreadIndex(threadToExecuteOn) == ENamedThreads::AnyThread)
			{
				if (PlatformProcess::supportsMultithreading())
				{
					int32 taskPriority = ENamedThreads::getTaskPriority(task->mThreadToExecuteOn);
					int32 priority = ENamedThreads::getThreadPriorityIndex(task->mThreadToExecuteOn);
					if (priority == (ENamedThreads::BackgroundThreadPriority >> ENamedThreads::ThreadPriorityShift) && (!mCreateBackgroundThreads || !ENamedThreads::bHasBackgroundThreads))
					{
						priority = ENamedThreads::NormalThreadPriority >> ENamedThreads::ThreadPriorityShift;
						taskPriority = ENamedThreads::NormalTaskPriority >> ENamedThreads::TaskPriorityShift;
					}
					else if (priority == (ENamedThreads::HighThreadPriority >> ENamedThreads::ThreadPriorityShift) && (!mCreateHiPriorityThreads || !ENamedThreads::bHasHighPriorityThreads))
					{
						priority = ENamedThreads::NormalThreadPriority >> ENamedThreads::ThreadPriorityShift;
						taskPriority = ENamedThreads::HighTaskPriority >> ENamedThreads::TaskPriorityShift;
					}
					ASSUME(priority >= 0 && priority < MAX_THREAD_PRIORITIES);
					{
						if (taskPriority)
						{
							mIncomingAnyThreadTasksHiPri[priority].push(task);
						}
						else
						{
							mIncomingAnyThreadTasks[priority].push(task);
						}
					}
					if (GFastSchedulerLatched)
					{
						if (!GConsoleSpinModeLatched || priority == (ENamedThreads::BackgroundThreadPriority >> ENamedThreads::ThreadPriorityShift))
						{
							startTaskThreadFastMode(priority, -1);
						}
					}
					else
					{
						ENamedThreads::Type currentThreadIfKnown = ENamedThreads::AnyThread;
						if (ENamedThreads::getThreadIndex(inCurrentThreadIfKnown) == ENamedThreads::AnyThread)
						{
							currentThreadIfKnown = getCurrentThread();
						}
						else
						{
							currentThreadIfKnown = ENamedThreads::getThreadIndex(inCurrentThreadIfKnown);
							ASSUME(currentThreadIfKnown == getCurrentThread());
						}
						TaskThreadBase* tempTarget;
						{
							tempTarget = mStalledUnnamedThreads[priority].pop();
							if (tempTarget && GNumWorkerThreadsToIgnore && (tempTarget->getThreadId() - mNumNamedThreads) % mNumTaskThreadPerSet >= getNumWorkerThreads())
							{
								tempTarget = nullptr;
							}
						}
						if (tempTarget)
						{
							threadToExecuteOn = tempTarget->getThreadId();
						}
						else
						{
							ASSUME(mNumTaskThreadPerSet - GNumWorkerThreadsToIgnore > 0);
							threadToExecuteOn = ENamedThreads::Type((uint32(mNextUnnamedThreadForTaskFromUnknownThread[priority].increment()) % uint32(mNumTaskThreadPerSet - GNumWorkerThreadsToIgnore)) + priority * mNumTaskThreadSets + mNumNamedThreads);
						}
						TaskThreadBase* target = &thread(threadToExecuteOn);
						if (ENamedThreads::getThreadIndex(threadToExecuteOn) != ENamedThreads::getThreadIndex(currentThreadIfKnown))
						{
							target->wakeup();
						}
					}
					return;
				}
				else
				{
					threadToExecuteOn = ENamedThreads::GameThread;
				}
			}
			ENamedThreads::Type currentThreadIfKnown;
			if (ENamedThreads::getThreadIndex(inCurrentThreadIfKnown) == ENamedThreads::AnyThread)
			{
				currentThreadIfKnown = getCurrentThread();
			}
			else
			{
				currentThreadIfKnown = ENamedThreads::getThreadIndex(inCurrentThreadIfKnown);
				ASSUME(currentThreadIfKnown == ENamedThreads::getThreadIndex(getCurrentThread()));
			}
			{
				int32 queueToExecuteOn = ENamedThreads::getQueueIndex(threadToExecuteOn);
				threadToExecuteOn = ENamedThreads::getThreadIndex(threadToExecuteOn);
				TaskThreadBase* target = &thread(threadToExecuteOn);
				if (threadToExecuteOn == ENamedThreads::getThreadIndex(currentThreadIfKnown))
				{
					target->enqueueFromThisThread(queueToExecuteOn, task);
				}
				else
				{
					target->enqueueFromOtherThread(queueToExecuteOn, task);
				}
			}
		}


		void startTaskThreadFastMode(int32 priority, int32 myIndex)
		{

		}

		void startAllTaskThreads(bool bDoBackgroundThreads)
		{

		}

		virtual void triggerEventWhenTaskCompletes(Event* inEvent, const GraphEventArray& tasks, ENamedThreads::Type currentThreadIdKnown  = ENamedThreads::AnyThread ) final override
		{
			bool bAnyPending = true;
			if (tasks.size() > 8)
			{
				bAnyPending = false;
				for (int32 index = 0; index < tasks.size(); ++index)
				{
					if (!tasks[index]->isComplete())
					{
						bAnyPending = true;
						break;
					}
				}
			}
			if (!bAnyPending)
			{
				inEvent->trigger();
				return;
			}
			GraphTask<TriggerEventGraphTask>::createTask(&tasks, currentThreadIdKnown).constructAndDispatchWhenReady(inEvent);
		}

		virtual void requestReturn(ENamedThreads::Type currentThread) override
		{
			int32 queueIndex = ENamedThreads::getQueueIndex(currentThread);
			currentThread = ENamedThreads::getThreadIndex(currentThread);
			thread(currentThread).requestQuit(queueIndex);
		}

		void notifyStalling(ENamedThreads::Type stallingThread)
		{
			if (stallingThread >= mNumNamedThreads && !GFastSchedulerLatched)
			{
				int32 localNumWorkingThread = getNumWorkerThreads();
				uint32 myIndex = (uint32(stallingThread) - mNumNamedThreads) % mNumTaskThreadPerSet;
				uint32 priority = (uint32(stallingThread) - mNumNamedThreads) / mNumTaskThreadPerSet;
				BOOST_ASSERT(myIndex >= 0 && (int32)myIndex < localNumWorkingThread);
				BOOST_ASSERT(priority >= 0 && priority < ENamedThreads::NumThreadPriorities && (ENamedThreads::bHasHighPriorityThreads || (priority << ENamedThreads::ThreadPriorityShift) != ENamedThreads::HighThreadPriority) && (ENamedThreads::bHasBackgroundThreads || (priority << ENamedThreads::ThreadPriorityShift) != ENamedThreads::BackgroundThreadPriority));
				mStalledUnnamedThreads[priority].push(&thread(stallingThread));
			}
		}

		virtual void waitUntilTaskComplete(const GraphEventArray& tasks, ENamedThreads::Type currentThreadIfKnown /* = ENamedThreads::AnyThread */) final override
		{
			ENamedThreads::Type currentThread = currentThreadIfKnown;
			if (ENamedThreads::getThreadIndex(currentThreadIfKnown) == ENamedThreads::AnyThread)
			{
				bool bisHiPri = !!ENamedThreads::getTaskPriority(currentThreadIfKnown);
				int32 priority = ENamedThreads::getThreadPriorityIndex(currentThreadIfKnown);

				currentThreadIfKnown = ENamedThreads::getThreadIndex(getCurrentThread());
				currentThread = ENamedThreads::setPriorities(currentThreadIfKnown, priority, bisHiPri);
			}
			else
			{
				currentThreadIfKnown = ENamedThreads::getThreadIndex(currentThreadIfKnown);

			}

			if (currentThreadIfKnown != ENamedThreads::AnyThread && currentThreadIfKnown < mNumNamedThreads && !isThreadProcessingTasks(currentThread))
			{
				if (tasks.size() > 8)
				{
					bool bAnyPending = false;
					for (int32 index = 0; index < tasks.size(); index++)
					{
						if (!tasks[index]->isComplete())
						{
							bAnyPending = true;
							break;
						}
					}
					if (!bAnyPending)
					{
						return;
					}
				}

				GraphTask<ReturnGraphTask>::createTask(&tasks, currentThread).constructAndDispatchWhenReady(currentThread);
				processThreadUntilRequestReturn(currentThread);
			}
			else
			{
				ScopedEvent vEvent;
				triggerEventWhenTaskCompletes(vEvent.get(), tasks, currentThreadIfKnown);
			}
		}

		BaseGraphTask* findWork(ENamedThreads::Type threadInNeed)
		{
			int32 localNumWorkerThread = getNumWorkerThreads();
			uint32 myIndex = (uint32(threadInNeed) - mNumNamedThreads) % mNumTaskThreadPerSet;
			uint32 priority = (uint32(threadInNeed) - mNumNamedThreads) / mNumTaskThreadPerSet;
			BOOST_ASSERT(myIndex >= 0 && (int32)myIndex < localNumWorkerThread);
			BOOST_ASSERT(priority >= 0 && priority < ENamedThreads::NumThreadPriorities && (ENamedThreads::bHasHighPriorityThreads || (priority << ENamedThreads::ThreadPriorityShift) != ENamedThreads::HighThreadPriority) && (ENamedThreads::bHasBackgroundThreads || (priority << ENamedThreads::ThreadPriorityShift) != ENamedThreads::BackgroundThreadPriority));

			{
				BaseGraphTask* task = mSortedAnyThreadTasksHiPri[priority].pop();
				{
					if (task)
					{
						return task;
					}
				}
				if (!mIncomingAnyThreadTasksHiPri[priority].isEmpty())
				{
					do 
					{
						std::lock_guard<std::mutex> l(mMutexForSortingIncomingAnyThreadTasksHiPri[priority]);
						if (GFastSchedulerLatched)
						{
							return nullptr;
						}
						if (!mIncomingAnyThreadTasksHiPri[priority].isEmpty() && mSortedAnyThreadTasksHiPri[priority].isEmpty())
						{
							static TArray<BaseGraphTask*> newTasks;
							newTasks.clear();
							mIncomingAnyThreadTasksHiPri[priority].popAll(newTasks);
							BOOST_ASSERT(newTasks.size());
							if (newTasks.size() > 1)
							{
								TLockFreePointerListLIFO<BaseGraphTask> tempSortedAnyThreadTasks;
								for (int32 index = 0; index < newTasks.size() - 1; index++)
								{
									tempSortedAnyThreadTasks.push(newTasks[index]);
								}
								BOOST_ASSERT(mSortedAnyThreadTasksHiPri[priority].replaceListIfEmpty(tempSortedAnyThreadTasks));
							}
							return newTasks[newTasks.size() - 1];
						}
						{
							BaseGraphTask* task = mSortedAnyThreadTasksHiPri[priority].pop();
							if (task)
							{
								return task;
							}
						}
					} while (!mIncomingAnyThreadTasksHiPri[priority].isEmpty() || !mSortedAnyThreadTasksHiPri[priority].isEmpty());
				}
				{
					BaseGraphTask* task = mSortedAnyThreadTasks[priority].pop();
					if (task)
					{
						return task;
					}
				}
				do 
				{
					std::lock_guard<std::mutex> l(mMutexForSortingIncomingAnyThreadTasks[priority]);
					if (GFastSchedulerLatched)
					{
						return nullptr;
					}
					if (!mIncomingAnyThreadTasks[priority].isEmpty() && mSortedAnyThreadTasks[priority].isEmpty())
					{
						struct PadArray
						{
							uint8 mPad1[PLATFORM_CACHE_LINE_SIZE / 2];
							TArray<BaseGraphTask*> newTaskArray;
							uint8 mPad2[PLATFORM_CACHE_LINE_SIZE / 2];
						};
						static PadArray prioTask[MAX_THREAD_PRIORITIES];
						TArray<BaseGraphTask*> & newTask = prioTask[priority].newTaskArray;
						newTask.clear();
						mIncomingAnyThreadTasks[priority].popAll(newTask);
						BOOST_ASSERT(newTask.size());
						if (newTask.size() > 1)
						{
							TLockFreePointerListLIFO<BaseGraphTask> tempSortedAnyThreadTasks;
							for (int32 index = 0; index < newTask.size() - 1; index++)
							{
								tempSortedAnyThreadTasks.push(newTask[index]);
							}
							BOOST_ASSERT(mSortedAnyThreadTasks[priority].replaceListIfEmpty(tempSortedAnyThreadTasks));
						}
						return newTask[newTask.size() - 1];
					}
					{
						BaseGraphTask* task = mSortedAnyThreadTasks[priority].pop();
						if (task)
						{
							return task;
						}
					}
				} while (!mIncomingAnyThreadTasks[priority].isEmpty() || !mSortedAnyThreadTasks[priority].isEmpty());
				return nullptr;
			}
		}

	public:

		enum
		{
			MAX_THREADS = 24,
			MAX_THREAD_PRIORITIES = 3
		};

		ENamedThreads::Type mLastExternalThread;

		bool mCreateHiPriorityThreads;
		bool mCreateBackgroundThreads;
		uint32 mNumNamedThreads;
		uint32 mNumTaskThreadSets;
		uint32 mNumThreads;
		uint32 mNumTaskThreadPerSet;
		uint32 mPerThreadIDTLSSlot;
		ThreadSafeCounter mReentrancyCheck;
		ThreadSafeCounter mNextUnnamedThreadForTaskFromUnknownThread[MAX_THREAD_PRIORITIES];
		WorkerThread	mWorkerThreads[MAX_THREADS];

		struct AtomicStateBitfield
		{
			uint32 mQueueOwned : 1;
			uint32 mQueueOwnerIndex : 4;
			uint32 mStalled : AtomicStateBitfield_MAX_THREADS ;
			uint32 mWorking : AtomicStateBitfield_MAX_THREADS ;

			uint32 mUnused;
			uint8 mPadToAvoidContention[PLATFORM_CACHE_LINE_SIZE - 2 * sizeof(uint32)];
			AtomicStateBitfield()
				:mQueueOwned(false),
				mQueueOwnerIndex(0),
				mStalled(0),
				mWorking(0)
			{
				static_assert(offsetof(AtomicStateBitfield, mUnused) == sizeof(uint32), "The bitfields in FAtomicStateBitfield must fit into a single uint32 and be the first member");
			}

			static FORCEINLINE AtomicStateBitfield interlockedCompareExchange(AtomicStateBitfield* dest, AtomicStateBitfield exchange, AtomicStateBitfield comparand)
			{
				AtomicStateBitfield result;
				*(int32*)&result = PlatformAtomics::interlockedCompareExchange((volatile int32*)dest, *(int32*)&exchange, *(int32*)&comparand);
				return result;
			}

			static int32 countBits(uint32 bits)
			{
				MS_ALIGN(64) static uint8 table[64] GCC_ALIGN(64) = { 0 };
				if (!table[63])
				{
					for (uint32 index = 0; index < 63; ++index)
					{
						uint32 localIndex = index;
						uint8 result = 0;
						while (localIndex)
						{
							if (localIndex & 1)
							{
								result++;
							}
							localIndex >>= 1;
						}
						table[index] = result;
					}
					table[63] = 6;
				}
				int32 count = 0;
				while (true)
				{
					count += table[bits & 63];
					if (!(bits & !63))
					{
						break;
					}
					bits >>= 6;
				}
				return count;
			}

			FORCEINLINE int32 numberOfStalledThreads()
			{
				return countBits(mStalled);
			}

			FORCEINLINE int32 numberOfWorkingThreads()
			{
				return countBits(mWorking);
			}

			bool operator == (AtomicStateBitfield other) const
			{
				return mQueueOwned == other.mQueueOwned && mQueueOwnerIndex == other.mQueueOwnerIndex && mStalled == other.mStalled && mWorking == other.mWorking;
			}

			bool operator != (AtomicStateBitfield other)
			{
				return mQueueOwned != other.mQueueOwned || mQueueOwnerIndex != other.mQueueOwnerIndex || mStalled != other.mStalled || mWorking != other.mWorking;
			}
		};

		TLockFreePointerListUnordered<TaskThreadBase, PLATFORM_CACHE_LINE_SIZE> mStalledUnnamedThreads[MAX_THREAD_PRIORITIES];

		uint8 mPadToAvoidContention6[PLATFORM_CACHE_LINE_SIZE];
		AtomicStateBitfield mAtomicForConsoleApproach[MAX_THREAD_PRIORITIES];

		TLockFreePointerListLIFO<BaseGraphTask> mIncomingAnyThreadTasks[MAX_THREAD_PRIORITIES];

		TLockFreePointerListLIFO<BaseGraphTask> mIncomingAnyThreadTasksHiPri[MAX_THREAD_PRIORITIES];
		TLockFreePointerListLIFO<BaseGraphTask> mSortedAnyThreadTasks[MAX_THREAD_PRIORITIES];

		TLockFreePointerListLIFO<BaseGraphTask> mSortedAnyThreadTasksHiPri[MAX_THREAD_PRIORITIES];

		std::mutex	mMutexForSortingIncomingAnyThreadTasksHiPri[MAX_THREAD_PRIORITIES];
		std::mutex	mMutexForSortingIncomingAnyThreadTasks[MAX_THREAD_PRIORITIES];
	};


	void TaskGraphInterface::startup(int32 numThreads)
	{
		new TaskGraphImplementation(numThreads);
	}

	TaskGraphInterface& TaskGraphInterface::get()
	{
		return *TaskGraphImplementationSingleton;
	}

	static TLockFreeClassAllocator_TLSCache<GraphEvent, PLATFORM_CACHE_LINE_SIZE> mTheGraphEventAllocator;

	GraphEventRef GraphEvent::createGraphEvent()
	{
		return mTheGraphEventAllocator.New();
	}

	void GraphEvent::recycle(GraphEvent* toRecycle)
	{
		mTheGraphEventAllocator.free(toRecycle);
	}

	void GraphEvent::dispatchSubsequents(TArray<BaseGraphTask*>& newTasks, ENamedThreads::Type currentThreadIfKnown /* = ENamedThreads::AnyThread */
	)
	{
		if (mEventsToWaitFor.size())
		{
			GraphEventArray temEventsToWaitFor;
			exchange(mEventsToWaitFor, temEventsToWaitFor);
			GraphTask<NullGraphTask>::createTask(GraphEventRef(this), &temEventsToWaitFor, currentThreadIfKnown).constructAndDispatchWhenReady(ENamedThreads::AnyHiPriThreadHiPriTask);
			return;
		}
		mSubSequentList.popAllAndClose(newTasks);
		bool closed = mSubSequentList.isClosed();
		for (int32 index = newTasks.size() - 1; index >= 0; index--)
		{
			BaseGraphTask* newTask = newTasks[index];
			newTask->conditionalQueueTask(currentThreadIfKnown);
		}
		newTasks.clear();
	}

	BaseGraphTask* TaskThreadAnyThread::findWork()
	{
		return TaskGraphImplementation::get().findWork(mThreadId);
	}

	void TaskThreadAnyThread::notifyStalling()
	{
		return TaskGraphImplementation::get().notifyStalling(mThreadId);
	}
}