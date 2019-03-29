#include "RenderingThread.h"
#include "HAL/Runnable.h"
#include "HAL/Event.h"
#include "RHICommandList.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTLS.h"
#include "HAL/AirMemory.h"
#include "HAL/PlatformMisc.h"
#include "DynamicRHI.h"
#include "HAL/RunnableThread.h"
#include "RenderCommandFence.h"
#include "boost/lexical_cast.hpp"
#include "HAL/ThreadHeartBeat.h"
#include "Math/Math.h"
#include "HAL/PlatformTime.h"
#include "RenderingThread.h"
namespace Air
{
	RENDER_CORE_API bool GUseThreadedRendering = false;

	RENDER_CORE_API bool GIsThreadedRendering = false;

	RENDER_CORE_API bool GUseRHIThread = false;

	static Runnable* GRenderingThreadRunnable = nullptr;

	volatile bool GRunRenderingThreadHeartbeat = false;

	float GRenderingThreadMaxIdleTickFrequency = 40.0f;

	static int32 GTimeToBlockOnRenderFence = 1;

	RunnableThread* GRenderingThreadHearbeat = nullptr;
	Runnable* GRenderingThreadRunnableHeartbeat = nullptr;

	TickableObjectRenderThread::RenderThreadTickableObjectsArray TickableObjectRenderThread::mRenderingThreadHighFrequencyTickableObjects;

	TickableObjectRenderThread::RenderThreadTickableObjectsArray TickableObjectRenderThread::mRenderingThreadTickableObjects;

	static CompletionList FrameRenderPrerequisites;

	PendingCleanupObjects* getPendingCleanupObjects()
	{
		return new PendingCleanupObjects;
	}

	void flushRenderingCommands()
	{
		if (!GIsRHIInitialized)
		{
			return;
		}
		ENQUEUE_UNIQUE_RENDER_COMMAND(
			FlushPendingDeleteRHIResources,
			{
				GRHICommandList.getImmediateCommandList().immediateFlush(EImmediateFlushType::FlushRHIThreadFlushResources);
			}
		);
		advanceFrameRenderPrerequisite();
		PendingCleanupObjects* pendingCleanupObjects = getPendingCleanupObjects();
		RenderCommandFence fence;
		fence.beginFence();
		fence.wait();
		delete pendingCleanupObjects;
		
	}
	static void suspendRendering()
	{
		PlatformAtomics::interlockedIncrement(&GIsRenderingThreadSuspended);
	}
	void checkNotBlockedOnRenderThread()
	{

	}

	void gameThreadWaitForTask(const GraphEventRef& task, bool bEmpltyGameThreadTasks)
	{
		if (!task->isComplete())
		{
			{
				static int32 NumRecursiveCalls = 0;
				NumRecursiveCalls++;
				if (NumRecursiveCalls > 1)
				{

				}
				if (NumRecursiveCalls > 1 || TaskGraphInterface::get().isThreadProcessingTasks(ENamedThreads::GameThread))
				{
					bEmpltyGameThreadTasks = false;
				}
				Event* e = PlatformProcess::getSynchEventFromPool();
				TaskGraphInterface::get().triggerEventWhenTaskCompletes(e, task, ENamedThreads::GameThread);
				bool bDone;
				uint32 waitTime = Math::clamp<uint32>(GTimeToBlockOnRenderFence, 0, 33);
				const double startTime = PlatformTime::seconds();
				const double endTime = startTime + (GTimeToBlockOnRenderFence / 1000.0);
				do 
				{
					checkRenderingThreadHealth();
					if (bEmpltyGameThreadTasks)
					{
						TaskGraphInterface::get().processThreadUntilIdle(ENamedThreads::GameThread);
					}
					bDone = e->wait(waitTime);
					if (!bDone && PlatformMisc::isDebuggerPresent())
					{
						if (PlatformTime::seconds() >= endTime && ThreadHeartBeat::get().isBeating() && !PlatformMisc::isDebuggerPresent())
						{

						}
					}
				} while (!bDone);
				PlatformProcess::returnSynchEventToPool(e);
				e = nullptr;
				NumRecursiveCalls--;
			}
		}
	}

	void advanceFrameRenderPrerequisite()
	{
		GraphEventRef PendingComplete = FrameRenderPrerequisites.createPrerequisiteCompletionHandle(ENamedThreads::GameThread);
		if (PendingComplete.getReference())
		{
			gameThreadWaitForTask(PendingComplete);
		}
	}




	static TLockFreePointerListUnordered<DeferredCleanupInterface, PLATFORM_CACHE_LINE_SIZE> mPendingCleanupObjectsList;

	void beginCleanup(DeferredCleanupInterface* cleanupObject)
	{
		mPendingCleanupObjectsList.push(cleanupObject);
	}

	PendingCleanupObjects::PendingCleanupObjects()
	{
		mPendingCleanupObjectsList.popAll(mCleanupArray);
	}
	PendingCleanupObjects::~PendingCleanupObjects()
	{
		for (int32 index = 0; index < mCleanupArray.size(); index++)
		{
			mCleanupArray[index]->finishCleanup();
		}
	}

	class OwnershipOfRHIThreadTask : public CustomStatIDGraphTaskBase
	{
	public:
		OwnershipOfRHIThreadTask(bool bInAcquireOwnership)
			:CustomStatIDGraphTaskBase()
			, mAcquireOwnership(bInAcquireOwnership)
		{

		}
		ENamedThreads::Type getDesiredThread()
		{
			return ENamedThreads::RHIThread;
		}

		static ESubsequentsMode::Type getSubsequentsMode()
		{
			return ESubsequentsMode::TrackSubsequents;
		}

		void doTask(ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent)
		{
			if (mAcquireOwnership)
			{
				GDynamicRHI->RHIAcquireThreadOwnership();
			}
			else
			{
				GDynamicRHI->RHIReleaseThreadOwnership();
			}
		}
		
	private:
		bool mAcquireOwnership;
	};


	void stopRenderingThread()
	{
		if (GRunRenderingThreadHeartbeat)
		{
			GRunRenderingThreadHeartbeat = false;
			GRenderingThreadHearbeat->waitForCompletion();
			delete GRenderingThreadHearbeat;
			GRenderingThreadHearbeat = nullptr;
			delete GRenderingThreadRunnableHeartbeat;
			GRenderingThreadRunnableHeartbeat = nullptr;
		}
		if (GIsThreadedRendering)
		{
			PendingCleanupObjects* pendingCleanupObjects = getPendingCleanupObjects();
			(*GFlushStreamingFunc)();
			flushRenderingCommands();

			if (GIsThreadedRendering)
			{
				if (GRHIThread)
				{
					static std::mutex Mutex_WaitForRHIThreadFinish;
					GraphEventRef releaseTask = GraphTask<OwnershipOfRHIThreadTask>::createTask(NULL, ENamedThreads::GameThread).constructAndDispatchWhenReady(false);

					TaskGraphInterface::get().waitUntilTaskCompletes(releaseTask, ENamedThreads::GameThread_Local);
					GRHIThread = nullptr;

				}
				GIsThreadedRendering = false;
				{
					GraphEventRef  quitTask = GraphTask<ReturnGraphTask>::createTask(NULL, ENamedThreads::GameThread).constructAndDispatchWhenReady(ENamedThreads::RenderThread);

					if (TaskGraphInterface::get().isThreadProcessingTasks(ENamedThreads::GameThread))
					{
						while ((quitTask.getReference() != nullptr) && !quitTask->isComplete())
						{
							PlatformProcess::sleep(0.0f);
						}
					}
					else
					{
						TaskGraphInterface::get().waitUntilTaskCompletes(quitTask, ENamedThreads::GameThread_Local);
					}
				}
				GRenderingThread->waitForCompletion();
				delete GRenderingThread;
				GRenderingThread = nullptr;

				GRHICommandList.latchBypass();
				delete GRenderingThreadRunnable;
				GRenderingThreadRunnable = nullptr;
			}
			delete pendingCleanupObjects;
		}

	}

	static void waitAndResumeRendering()
	{
		while (GIsRenderingThreadSuspended)
		{
			PlatformProcess::sleep(0.001f);
		}
		PlatformProcess::SetRealTimeMode();
	}

	SuspendRenderingThread::SuspendRenderingThread(bool bInRecreateThread)
	{
		if (isAsyncLoadingMultithreaded())
		{
			suspendAsyncLoading();
		}
		bRecreateThread = bInRecreateThread;
		bUseRenderingThread = GUseThreadedRendering;
		bWasRenderingThreadRunning = GIsThreadedRendering;
		if (bRecreateThread)
		{
			stopRenderingThread();
			GUseThreadedRendering = false;
			PlatformAtomics::interlockedIncrement(&GIsRenderingThreadSuspended);
		}
		else
		{
			if (GIsRenderingThreadSuspended == 0)
			{
				flushRenderingCommands();
				if (GIsThreadedRendering)
				{
					GraphEventRef completeHandle = SimpleDelegateGraphTask::createAndDispatchWhenReady(std::bind(suspendRendering), NULL, ENamedThreads::RenderThread);

					if (TaskGraphInterface::get().isThreadProcessingTasks(ENamedThreads::GameThread))
					{
						while (!GIsRenderingThreadSuspended)
						{
							PlatformProcess::sleep(0.0f);
						}
					}
					else
					{
						TaskGraphInterface::get().waitUntilTaskCompletes(completeHandle, ENamedThreads::GameThread);
					}

					SimpleDelegateGraphTask::createAndDispatchWhenReady(std::bind(waitAndResumeRendering), nullptr, ENamedThreads::RenderThread);
				}
				else
				{
					suspendRendering();
				}
			}
			else
			{
				PlatformAtomics::interlockedIncrement(&GIsRenderingThreadSuspended);
			}
		}
	}

	SuspendRenderingThread::~SuspendRenderingThread()
	{
		if (bRecreateThread)
		{
			GUseThreadedRendering = bUseRenderingThread;
			PlatformAtomics::interLockedDecrement(&GIsRenderingThreadSuspended);
			if (bUseRenderingThread && bWasRenderingThreadRunning)
			{
				startRenderingThread();
				SimpleDelegateGraphTask::createAndDispatchWhenReady(std::bind(PlatformProcess::SetRealTimeMode), NULL, ENamedThreads::RenderThread);
			}
		}
		else
		{
			PlatformAtomics::interLockedDecrement(&GIsRenderingThreadSuspended);
		}
		if (isAsyncLoadingMultithreaded())
		{
			resumeAsyncLoading();
		}
	}

	void renderingThreadMain(Event* taskGrapthBoundSyncEvent)
	{
		ENamedThreads::RenderThread = ENamedThreads::Type(ENamedThreads::ActualRenderingThread);
		ENamedThreads::RenderThread_Local = ENamedThreads::Type(ENamedThreads::ActualRenderingThread_Local);
		TaskGraphInterface::get().attachToThread(ENamedThreads::RenderThread);
		if (taskGrapthBoundSyncEvent != nullptr)
		{
			taskGrapthBoundSyncEvent->trigger();
		}
		PlatformProcess::SetRealTimeMode();
		TaskGraphInterface::get().processThreadUntilRequestReturn(ENamedThreads::RenderThread);


	}

	ThreadSafeCounter mOutstandingHeartbeats;

	void tickRenderingTickables()
	{

	}


	class RenderingThreadTickHearbeat : public Runnable
	{
	public:
		virtual bool init(void)
		{
			mOutstandingHeartbeats.reset();
			return true;
		}

		virtual void exit()
		{

		}

		virtual void stop()
		{

		}

		virtual uint32 run()
		{
			while (GRunRenderingThreadHeartbeat)
			{
				PlatformProcess::sleep(1.f / (4.0f * GRenderingThreadMaxIdleTickFrequency));
				if (!GIsRenderingThreadSuspended && mOutstandingHeartbeats.getValue() < 4)
				{
					mOutstandingHeartbeats.increment();
					ENQUEUE_UNIQUE_RENDER_COMMAND(HeartbeatTickTickables, {
						mOutstandingHeartbeats.decrement();
						if (!GIsRenderingThreadSuspended)
						{
							tickRenderingTickables();
						}
					});
				}
			}
			return 0;
		}

	};



	class RenderingThread : public Runnable
	{
	private:
		bool bAcquiredThreadOwnership;
	public:
		Event* mTaskGraphBoundSyncEvent;

		RenderingThread()
		{
			bAcquiredThreadOwnership = false;
			mTaskGraphBoundSyncEvent = PlatformProcess::getSynchEventFromPool(true);
			RHIFlushResource();
		}

		virtual ~RenderingThread()
		{
			PlatformProcess::returnSynchEventToPool(mTaskGraphBoundSyncEvent);
			mTaskGraphBoundSyncEvent = nullptr;
		}

		virtual bool init() override
		{
			GRenderThreadId = PlatformTLS::getCurrentThreadId();
			if (!GUseRHIThread)
			{
				bAcquiredThreadOwnership = true;
				RHIAcquireThreadOwnership();
			}
			return true;
		}

		virtual void exit() override
		{
			if (bAcquiredThreadOwnership)
			{
				bAcquiredThreadOwnership = false;
				RHIReleaseThreadOwnership();
			}
			GRenderThreadId = 0;
		}

		virtual uint32 run() override
		{
			Memory::setupTLSCachesOnCurrentThread();
			PlatformProcess::setupRenderThread();
			renderingThreadMain(mTaskGraphBoundSyncEvent);
			Memory::clearAndDisableTLSCachesOnCurrentThread();
			return 0;
		}
	};

	static wstring buildRenderingThreadName(uint32 threadIndex)
	{
		return L"RenderingThread " + threadIndex;
	}

	void startRenderingThread()
	{
		static uint32 threadCount = 0;

		if (GUseRHIThread)
		{

		}
		GIsThreadedRendering = true;

		GRenderingThreadRunnable = new RenderingThread();

		GRenderingThread = RunnableThread::create(GRenderingThreadRunnable, buildRenderingThreadName(threadCount).c_str(), 0,
			PlatformAffinity::getRenderingThreadPriority(),
			PlatformAffinity::getRenderingThreadMask());

		((RenderingThread*)GRenderingThreadRunnable)->mTaskGraphBoundSyncEvent->wait();

		RenderCommandFence fence;
		fence.beginFence();
		fence.wait();

		GRunRenderingThreadHeartbeat = true;

		GRenderingThreadRunnableHeartbeat = new RenderingThreadTickHearbeat();

		GRenderingThreadHearbeat = RunnableThread::create(GRenderingThreadRunnableHeartbeat, L"RTHeartBeat" + threadCount, 16 * 1024, TPri_AboveNormal, PlatformAffinity::getRTHeartBeatMask());

		threadCount++;

	}

	class RHICommandListImmediate& getImmediateCommandList_ForRenderCommand()
	{
		return RHICommandListExecutor::getImmediateCommandList();
	}
}