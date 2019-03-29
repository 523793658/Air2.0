#pragma once
#include "CoreType.h"
#include "Misc/IQueuedWork.h"
#include "HAL/ThreadSafeCounter.h"
#include "Misc/QueuedThreadPool.h"
#include "HAL/Event.h"
namespace Air
{
	template<typename TTask>
	class AutoDeleteAsyncTask
		:private IQueuedWork
	{
		TTask mTask;

		void start(bool bForceSynchronous)
		{
			QueuedThreadPool* quequedPool = GThreadPool;
			if (bForceSynchronous)
			{
				quequedPool = 0;
			}
			if (quequedPool)
			{
				quequedPool->addQueuedWork(this);
			}
			else
			{
				doWork();
			}
		}

		void doWork()
		{
			mTask.doWork();
			delete this;
		}

		virtual void doThreadedWork()
		{
			doWork();
		}

		virtual void abandon(void)
		{
			if (mTask.canAbandon())
			{
				mTask.abandon();
				delete this;
			}
			else
			{
				doWork();
			}
		}

	public:
		template<typename...T>
		explicit AutoDeleteAsyncTask(T&&... args) : mTask(std::forward<T>(args)...)
		{}

		void startSynchronousTask()
		{
			start(true);
		}

		void startBackgroundTask()
		{
			start(false);
		}
	};

	template<typename TTask>
	class AsyncTask
		: private IQueuedWork
	{
		TTask mTask;
		ThreadSafeCounter mWorkNotFinishedCounter;
		Event* mDoneEvent;
		QueuedThreadPool* mQueuedPool;

		void destroyEvent()
		{
			PlatformProcess::returnSynchEventToPool(mDoneEvent);
			mDoneEvent = nullptr;
		}

		void checkIdle() const
		{
			BOOST_ASSERT(mWorkNotFinishedCounter.getValue() == 0);
			BOOST_ASSERT(!mQueuedPool);
		}

		void start(bool bForceSynchronous, QueuedThreadPool* inQueuedPool)
		{
			PlatformMisc::memoryBarrier();
			checkIdle();
			mWorkNotFinishedCounter.increment();
			mQueuedPool = inQueuedPool;
			if (bForceSynchronous)
			{
				mQueuedPool = 0;
			}
			if (mQueuedPool)
			{
				if (!mDoneEvent)
				{
					mDoneEvent = PlatformProcess::getSynchEventFromPool(true);
				}
				mDoneEvent->reset();
				mQueuedPool->addQueuedWork(this);
			}
			else
			{
				destroyEvent();
				doWork();
			}
		}

		void doWork()
		{
			mTask.doWork();
			BOOST_ASSERT(mWorkNotFinishedCounter.getValue() == 1);
			mWorkNotFinishedCounter.decrement();
		}

		void finishThreadedWork()
		{
			BOOST_ASSERT(mQueuedPool);
			if (mDoneEvent)
			{
				mDoneEvent->trigger();
			}
		}

		virtual void doThreadedWork()
		{
			doWork();
			finishThreadedWork();
		}

		virtual void abandon()
		{
			if (mTask.canAbandon())
			{
				mTask.abandon();
				BOOST_ASSERT(mWorkNotFinishedCounter.getValue() == 1);
				mWorkNotFinishedCounter.decrement();
			}
			else
			{
				doWork();
			}
			finishThreadedWork();
		}

		void syncCompletion()
		{
			PlatformMisc::memoryBarrier();
			if (mQueuedPool)
			{
				BOOST_ASSERT(mDoneEvent);
				mDoneEvent->wait();
				mQueuedPool = 0;
			}
			checkIdle();
		}

		void init()
		{
			mDoneEvent = 0;
			mQueuedPool = 0;

		}

	public:
		AsyncTask()
			:mTask()
		{
			init();
		}

		template<typename Arg0Type, typename... ArgTypes>
		AsyncTask(Arg0Type&& arg0, ArgTypes&&... Args)
			: mTask(forward<Arg0Type>(arg0), forward<ArgTypes>(Args)...)
		{
			init();
		}

		~AsyncTask()
		{
			checkIdle();
			destroyEvent();
		}

		TTask & getTask()
		{
			checkIdle();
			return  mTask;
		}

		const TTask & getTask() const
		{
			checkIdle();
			return  mTask;
		}

		void startSynchronousTask()
		{
			start(true, GThreadPool);
		}

		void startBackgroundTask(QueuedThreadPool* inQueuedPool = GThreadPool)
		{
			start(false, inQueuedPool);
		}

		void ensureCompletion(bool bDoWorkOnThisThreadIfNotStarted = true)
		{
			bool doSyncCompletion = true;
			if (bDoWorkOnThisThreadIfNotStarted)
			{
				if (mQueuedPool)
				{
					if (mQueuedPool->retractQueuedWork(this))
					{
						doSyncCompletion = false;
						doWork();
						finishThreadedWork();
						mQueuedPool = nullptr;
					}
				}
				else if (mWorkNotFinishedCounter.getValue())
				{
					doWork();
				}
			}
			if (doSyncCompletion)
			{
				syncCompletion();
			}
			checkIdle();
		}

		bool cancel()
		{
			if (mQueuedPool)
			{
				if (mQueuedPool->retractQueuedWork(this))
				{
					BOOST_ASSERT(mWorkNotFinishedCounter.getValue() == 1);
					mWorkNotFinishedCounter.decrement();
					finishThreadedWork();
					mQueuedPool = 0;
					return true;
				}
			}
			return false;
		}

		bool waitCompletionWithTimeout(float timeLimitSeconds)
		{
			BOOST_ASSERT(timeLimitSeconds > 0.0f);
			PlatformMisc::memoryBarrier();
			if (mQueuedPool)
			{
				uint32 ms = uint32(timeLimitSeconds * 1000.0f) + 1;
				BOOST_ASSERT(ms);
				BOOST_ASSERT(mDoneEvent);
				if (mDoneEvent->wait(ms))
				{
					mQueuedPool = 0;
					checkIdle();
					return true;
				}
				return false;
			}
			checkIdle();
			return true;
		}

		bool isDone()
		{
			if (!isWorkDown())
			{
				return false;
			}
			syncCompletion();
			return true;
		}

		bool isWorkDone() const
		{
			if (mWorkNotFinishedCounter.getValue())
			{
				return false;
			}
			return true;
		}

		bool isIdle() const
		{
			return mWorkNotFinishedCounter.getValue() == 0 && mQueuedPool == 0;
		}
	};

	class NonAbandonableTask
	{
	public:
		bool canAbandon()
		{
			return false;
		}

		void abandon()
		{

		}
	};
}