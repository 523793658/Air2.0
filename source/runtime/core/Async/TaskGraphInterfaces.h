#pragma once
#include "CoreType.h"
#include "Containers/Array.h"
#include "Containers/LockFreeListImpl.h"
#include "Containers/LockFreeFixedSizeAllocator.h"
#include "HAL/Event.h"
#include "Template/TypeCompatibleBytes.h"
#include "Template/RefCounting.h"
#include "HAL/IConsoleManager.h"
#include <functional>
#include <utility>
#include <mutex>
#include <atomic>

#include <iostream>
namespace Air
{
	namespace ENamedThreads
	{
		enum Type
		{
			UnusedAnchor = -1,
#if 0
			StatsThread,
#endif // 0
			RHIThread,
			AudioThread,
			GameThread,
			ActualRenderingThread = GameThread + 1,
			AnyThread = 0xff,

			MainQueue =			0x000,
			LocalQueue =		0x100,
			NumQueue =			2,
			ThreadIndexMask =	0xff,
			QueueIndexMask=		0x100,
			QueueIndexShift =	8,

			NormalTaskPriority =	0x000,
			HighTaskPriority =		0x200,

			NumTaskPriorities =		2,
			TaskPriorityMask =		0x200,
			TaskPriorityShift =		9,

			NormalThreadPriority =	0x000,
			HighThreadPriority =	0x400,
			BackgroundThreadPriority = 0x800,

			NumThreadPriorities	=		3,
			ThreadPriorityMask =	0xC00,
			ThreadPriorityShift =	10,



			GameThread_Local = GameThread | LocalQueue,
			ActualRenderingThread_Local = ActualRenderingThread | LocalQueue,

			AnyHiPriThreadNormalTask = AnyThread | HighThreadPriority | NormalTaskPriority,

			AnyHiPriThreadHiPriTask = AnyThread | HighThreadPriority | HighTaskPriority,

			AnyNormalThreadNormalTask = AnyThread | NormalThreadPriority | NormalTaskPriority,

			AnyNormalThreadHiPriTask = AnyThread | NormalThreadPriority | HighTaskPriority,

			AnyBackgroundThreadNormalTask = AnyThread | BackgroundThreadPriority | NormalTaskPriority,
			AnyBackgroundHiPriTask = AnyThread | BackgroundThreadPriority | HighTaskPriority,

		};

		
		extern CORE_API int32 bHasBackgroundThreads;

		FORCEINLINE Type getThreadIndex(Type ThreadAndINdex)
		{
			return ((ThreadAndINdex & ThreadIndexMask) == AnyThread) ? AnyThread : Type(ThreadAndINdex & ThreadIndexMask);
		}

		FORCEINLINE int32 getQueueIndex(Type threadAndIndex)
		{
			return (threadAndIndex & QueueIndexMask) >> QueueIndexShift;
		}

		FORCEINLINE Type setPriorities(Type ThreadAndIndex, int32 priorityIndex, bool bHiPri)
		{
			return Type(ThreadAndIndex | (priorityIndex << ThreadPriorityShift) | (bHiPri ? HighTaskPriority : NormalTaskPriority));
		}

		FORCEINLINE Type setTaskPriority(Type threadAndIndex, Type taskPriority)
		{
			BOOST_ASSERT(!(threadAndIndex & ~ThreadIndexMask) &&
				!(taskPriority & ~TaskPriorityMask));
			return Type(threadAndIndex | taskPriority);
		}

		FORCEINLINE int32 getTaskPriority(Type treadAndIndex)
		{
			return (treadAndIndex & TaskPriorityMask) >> TaskPriorityShift;
		}

		FORCEINLINE int32 getThreadPriorityIndex(Type threadAndIndex)
		{
			int32 result = (threadAndIndex & ThreadPriorityMask) >> ThreadPriorityShift;
			ASSUME(result >= 0 && result < NumThreadPriorities);
			return result;
		}

		struct RenderThreadStatics
		{
		private:
			static CORE_API std::atomic_int32_t mRenderThread;
			static CORE_API std::atomic_int32_t mRenderThread_Local;

			friend Type getRenderThread();
			friend Type getRenderThread_Local();

			friend void setRenderThread(Type thread);
			friend void setRenderThread_Local(Type thread);
		};
		
		FORCEINLINE Type getRenderThread()
		{
			return (Type)RenderThreadStatics::mRenderThread.load(std::memory_order_relaxed);
		}

		FORCEINLINE Type getRenderThread_Local()
		{
			return (Type)(RenderThreadStatics::mRenderThread_Local.load(std::memory_order_relaxed));
		}

		FORCEINLINE void setRenderThread(Type thread)
		{
			RenderThreadStatics::mRenderThread.store((int32)thread, std::memory_order_relaxed);
		}

		FORCEINLINE void setRenderThread_Local(Type thread)
		{
			RenderThreadStatics::mRenderThread_Local.store((int32)thread, std::memory_order_relaxed);
		}
	}

	namespace ESubsequentsMode
	{
		enum Type
		{
			TrackSubsequents,
			FireAndForget
		};
	}

	typedef TRefCountPtr<class GraphEvent> GraphEventRef;

	typedef TArray<GraphEventRef, TInlineAllocator<4>> GraphEventArray;

	extern std::mutex Mutex_FTriggerEventGraphTask;

	class TaskGraphInterface
	{
	public:

		static CORE_API void startup(int32 numThreads);

		static CORE_API void shutdown();

		static CORE_API TaskGraphInterface& get();

		virtual void queueTask(class BaseGraphTask* task, ENamedThreads::Type threadToExecuteOn, ENamedThreads::Type currentThreadIfKnown = ENamedThreads::AnyThread) = 0;

		void triggerEventWhenTaskCompletes(Event* inEvent, const GraphEventRef& task, ENamedThreads::Type currentThreadIfKnown = ENamedThreads::AnyThread)
		{
			GraphEventArray prerequistes;
			prerequistes.push_back(task);
			triggerEventWhenTaskCompletes(inEvent, prerequistes, currentThreadIfKnown);
		}


		virtual void triggerEventWhenTaskCompletes(Event* inEvent, const GraphEventArray& tasks, ENamedThreads::Type currentThreadIdKnown = ENamedThreads::AnyThread) = 0;

		void waitUntilTaskCompletes(const GraphEventRef& task, ENamedThreads::Type currentThreadIfKnown = ENamedThreads::AnyThread)
		{
			GraphEventArray prerequisites;
			prerequisites.push_back(task);
			waitUntilTaskComplete(prerequisites, currentThreadIfKnown);
		}

		virtual void waitUntilTaskComplete(const GraphEventArray& tasks, ENamedThreads::Type currentThreadIfKnown = ENamedThreads::AnyThread) = 0;

		virtual void requestReturn(ENamedThreads::Type currentThread) = 0;

	public:
		virtual void attachToThread(ENamedThreads::Type currentThread) = 0;

		virtual bool isThreadProcessingTasks(ENamedThreads::Type threadToCheck) = 0;

		virtual void processThreadUntilIdle(ENamedThreads::Type currentThread) = 0;

		virtual void processThreadUntilRequestReturn(ENamedThreads::Type currentThread) = 0;

		virtual int32 getNumWorkerThreads() = 0;

	};



	class GraphEvent
	{
	public:
		static CORE_API GraphEventRef createGraphEvent();

		static CORE_API GraphEvent* createGraphEventWithInlineStorage();

		bool addSubSequent(class BaseGraphTask* task)
		{
			return mSubSequentList.pushIfNotClosed(task);
		}
		void checkDontcompleteUntilIsEmpty()
		{
			ASSUME(!mEventsToWaitFor.size());
		}

		bool isComplete() const
		{
			return mSubSequentList.isClosed();
		}

		void dontCompleteUntil(GraphEventRef eventToWaitFor)
		{
			ASSUME(!isComplete());
			mEventsToWaitFor.push_back(eventToWaitFor);
		}

		CORE_API void dispatchSubsequents(TArray<BaseGraphTask*>& newTask, ENamedThreads::Type currentThreadIfKnown = ENamedThreads::AnyThread);

	private:
		friend class TRefCountPtr<GraphEvent>;
		friend class TLockFreeClassAllocator_TLSCache<GraphEvent, PLATFORM_CACHE_LINE_SIZE>;

		static CORE_API void recycle(GraphEvent* toRecycle);

		


	private:
		TClosableLockFreePointerListUnorderedSingleConsumer<BaseGraphTask, 0>		mSubSequentList;

		GraphEventArray	mEventsToWaitFor;

		ThreadSafeCounter mReferenceCount;
	public:
		uint32 AddRef()
		{
			int32 refCount = mReferenceCount.increment();
			return refCount;
		}

		uint32 Release()
		{
			int32 refCount = mReferenceCount.decrement();
			if (refCount == 0)
			{
				recycle(this);
			}
			return refCount;
		}
	};

	enum class EPowerSavingEligibility
	{
		Unknown,
		Eligible,
		NotEligible
	};

	class CORE_API AutoConsoleTaskPriority
	{
		AutoConsoleCommand mCommand;
		wstring mCommandName;
		ENamedThreads::Type mThreadPriority;
		ENamedThreads::Type mTaskPriority;
		ENamedThreads::Type mTaskPriorityIfForcedToNormalThreadPriority;
		EPowerSavingEligibility mPowerSavingEligibility;
		void commandExecute(const TArray<wstring>& args);
	public:
		AutoConsoleTaskPriority(const TCHAR* name, const TCHAR* help, ENamedThreads::Type defaultThreadPriority, ENamedThreads::Type defaultTaskPriority, ENamedThreads::Type defaultTaskPriorityIfForceToNormalThreadPriority = ENamedThreads::UnusedAnchor, EPowerSavingEligibility defaultPowerSavingEligibility = EPowerSavingEligibility::Eligible)
			:mCommand(name, help, std::function<void(const TArray<wstring>&)>(std::bind(&AutoConsoleTaskPriority::commandExecute, this, std::placeholders::_1)))
			,mCommandName(name)
			,mThreadPriority(defaultThreadPriority)
			,mTaskPriority(defaultTaskPriority)
			,mTaskPriorityIfForcedToNormalThreadPriority(defaultTaskPriorityIfForceToNormalThreadPriority)
			,mPowerSavingEligibility(defaultPowerSavingEligibility)
		{
			BOOST_ASSERT(mTaskPriorityIfForcedToNormalThreadPriority != ENamedThreads::UnusedAnchor || mThreadPriority == ENamedThreads::NormalThreadPriority);
		}

		FORCEINLINE ENamedThreads::Type get(ENamedThreads::Type thread = ENamedThreads::AnyThread)
		{
			if (mThreadPriority == ENamedThreads::BackgroundThreadPriority && !ENamedThreads::bHasBackgroundThreads)
			{
				return ENamedThreads::setTaskPriority(thread, mTaskPriorityIfForcedToNormalThreadPriority);
			}
			return ENamedThreads::setPriorities(thread, mThreadPriority, mTaskPriority);
		}
	};


	class BaseGraphTask
	{
	public:
		enum
		{
			SMALL_TASK_SIZE = 256
		};
		typedef TLockFreeFixedSizeAllocator_TLSCache<SMALL_TASK_SIZE, PLATFORM_CACHE_LINE_SIZE> TSmallTaskAllocator;

	protected:
		BaseGraphTask(int32 inNumberOfPrerequistitesOutstanding)
			: mThreadToExecuteOn(ENamedThreads::AnyThread)
			, mNumberOfPrerequistitesOutstanding(inNumberOfPrerequistitesOutstanding + 1)
		{
			ASSUME(mLifeStage.increment() == int32(LS_Contructed));
		}

		void setThreadToExecuteOn(ENamedThreads::Type inThreadToExecuteOn)
		{
			mThreadToExecuteOn = inThreadToExecuteOn;
			ASSUME(mLifeStage.increment() == int32(LS_ThreadSet));
		}
		void prerequisitesComplete(ENamedThreads::Type currentThread, int32 numAlreadyFinishedPrequistes, bool bUnlock = true)
		{
			ASSUME(mLifeStage.increment() == int32(LS_PrequisitesSetup));
			int32 numToSub = numAlreadyFinishedPrequistes + (bUnlock ? 1 : 0);
			if (mNumberOfPrerequistitesOutstanding.subtract(numToSub) == numToSub)
			{
				queueTask(currentThread);
			}
		}
		virtual ~BaseGraphTask()
		{
			ASSUME(mLifeStage.increment() == int32(LS_Deconstucted));
		}

		static CORE_API TSmallTaskAllocator& getSmallTaskAllocator();

		void conditionalQueueTask(ENamedThreads::Type currentThread)
		{
			if (mNumberOfPrerequistitesOutstanding.decrement() == 0)
			{
				queueTask(currentThread);
			}
		}
	private:
		friend class NamedTaskThread;
		friend class TaskThreadBase;
		friend class TaskThreadAnyThread;
		friend class GraphEvent;
		friend class TaskGraphImplementation;

		virtual void executeTask(TArray<BaseGraphTask*>& newTasks, ENamedThreads::Type currentThread) = 0;

		FORCEINLINE void execute(TArray<BaseGraphTask*>& newTasks, ENamedThreads::Type currentThread)
		{
			ASSUME(mLifeStage.increment() == int32(LS_Executing));
			executeTask(newTasks, currentThread);
		}

		void queueTask(ENamedThreads::Type currentThreadIfKnown)
		{
			ASSUME(mLifeStage.increment() == int32(LS_Queued));
			TaskGraphInterface::get().queueTask(this, mThreadToExecuteOn, currentThreadIfKnown);
		}

		ENamedThreads::Type		mThreadToExecuteOn;
		ThreadSafeCounter		mNumberOfPrerequistitesOutstanding;

		enum ELifeStage
		{
			LS_BaseContructed = 0,
			LS_Contructed,
			LS_ThreadSet,
			LS_PrequisitesSetup,
			LS_Queued,
			LS_Executing,
			LS_Deconstucted,
		};
		ThreadSafeCounter		mLifeStage;
	};

	template<typename TTask>
	class TGraphTask : public BaseGraphTask
	{
	public:

		class Constructor
		{
		public:
			template<typename...T>
			GraphEventRef constructAndDispatchWhenReady(T&&... args)
			{
				new ((void*)&mOwner->mTaskStorage) TTask(std::forward<T>(args)...);
				return mOwner->setup(mPrerequisites, mCurrentThreadIfKnown);
			}
			
			template<typename... T>
			TGraphTask* constructAndHold(T&&... args)
			{
				new ((void*)&mOwner->mTaskStorage) TTask(std::forward<T>(args)...);
				return mOwner->hold(mPrerequisites, mCurrentThreadIfKnown);
			}
		private:
			friend class TGraphTask;
			TGraphTask*		mOwner;
			const GraphEventArray* mPrerequisites;

			ENamedThreads::Type mCurrentThreadIfKnown;

		private:
			Constructor(TGraphTask* inOwner, const GraphEventArray* inPrerequisites, ENamedThreads::Type inCurrentThreadIfKnown)
				: mOwner(inOwner)
				, mPrerequisites(inPrerequisites)
				, mCurrentThreadIfKnown(inCurrentThreadIfKnown)
			{}

		};

		static Constructor createTask(const GraphEventArray* prerequisites = nullptr, ENamedThreads::Type currentThreadIfKnown = ENamedThreads::AnyThread)
		{
			int32 numPrereq = prerequisites ? prerequisites->size() : 0;
			if (sizeof(TGraphTask) <= BaseGraphTask::SMALL_TASK_SIZE)
			{
				void *mem = BaseGraphTask::getSmallTaskAllocator().allocate();
				return Constructor(new(mem) TGraphTask(TTask::getSubsequentsMode() == ESubsequentsMode::FireAndForget ? GraphEventRef() : GraphEvent::createGraphEvent(), numPrereq), prerequisites, currentThreadIfKnown);
			}
			return Constructor(new TGraphTask(TTask::getSubsequentsMode() == ESubsequentsMode::FireAndForget ? nullptr : GraphEvent::createGraphEvent(), numPrereq), prerequisites, currentThreadIfKnown);
		}

		static Constructor createTask(GraphEventRef subsequentsToAssume, const GraphEventArray* prerequeistes = nullptr, ENamedThreads::Type currentThreadIfknown = ENamedThreads::AnyThread)
		{
			if (sizeof(TGraphTask) <= BaseGraphTask::SMALL_TASK_SIZE)
			{
				void *mem = BaseGraphTask::getSmallTaskAllocator().allocate();
				return Constructor(new (mem) TGraphTask(subsequentsToAssume, prerequeistes ? prerequeistes->size() : 0), prerequeistes, currentThreadIfknown);
			}
			return Constructor(new TGraphTask(subsequentsToAssume, prerequeistes ? prerequeistes->size() : 0), prerequeistes, currentThreadIfknown);
		}

		virtual void executeTask(TArray<BaseGraphTask *>& newTasks, ENamedThreads::Type currentThread) final override
		{
			if (TTask::getSubsequentsMode() == ESubsequentsMode::TrackSubsequents)
			{
				mSubSequents->checkDontcompleteUntilIsEmpty();
			}
			TTask& task = *(TTask*)&mTaskStorage;
			task.doTask(currentThread, mSubSequents);
			task.~TTask();
			mTaskConstructed = false;
			if (TTask::getSubsequentsMode() == ESubsequentsMode::TrackSubsequents)
			{
				mSubSequents->dispatchSubsequents(newTasks, currentThread);
			}

			if (sizeof(TGraphTask) <= BaseGraphTask::SMALL_TASK_SIZE)
			{
				this->TGraphTask::~TGraphTask();
				BaseGraphTask::getSmallTaskAllocator().free(this);
			}
			else
			{
				delete this;
			}
		}
	public:
		GraphEventRef setup(const GraphEventArray* prerequisites = nullptr, ENamedThreads::Type currentthreadIfKnown = ENamedThreads::AnyThread)
		{
			GraphEventRef returnedEvent = mSubSequents;
			setupPrereqs(prerequisites, currentthreadIfKnown, true);
			return returnedEvent;
		}

		TGraphTask* hold(const GraphEventArray* prerequisites = nullptr, ENamedThreads::Type currentthreadIfKnown = ENamedThreads::AnyThread)
		{
			setupPrereqs(prerequisites, currentthreadIfKnown, false);
			return this;
		}

		void setupPrereqs(const GraphEventArray* prequisities, ENamedThreads::Type currentThreadIfKnown, bool bUnlock)
		{
			mTaskConstructed = true;
			TTask& task = *(TTask*)&mTaskStorage;
			setThreadToExecuteOn(task.getDesiredThread());
			int32 alreadyCompletePrerequisites = 0;
			if (prequisities)
			{
				for (int32 index = 0; index < prequisities->size(); ++index)
				{
					if (!(*prequisities)[index]->addSubSequent(this))
					{
						alreadyCompletePrerequisites++;
					}
				}
			}
			prerequisitesComplete(currentThreadIfKnown, alreadyCompletePrerequisites, bUnlock);
		}

		void unlock(ENamedThreads::Type currentThreadIfKnown = ENamedThreads::AnyThread)
		{
			conditionalQueueTask(currentThreadIfKnown);
		}

		GraphEventRef getCompletionEvent()
		{
			return mSubSequents;
		}
	private:
		TGraphTask(GraphEventRef inSubsequents, int32 numberOfPrerequistiesOutstanding) : BaseGraphTask(numberOfPrerequistiesOutstanding),
			mTaskConstructed(false)
		{
			mSubSequents.swap(inSubsequents);
		}

		virtual ~TGraphTask()
		{
			
		}

	private:


		TAlignedBytes<sizeof(TTask), ALIGNOF(TTask)>	mTaskStorage;

		GraphEventRef		mSubSequents;

		bool mTaskConstructed;
	};

	class CustomStatIDGraphTaskBase
	{
	public:
		CustomStatIDGraphTaskBase()
		{

		}
		virtual ~CustomStatIDGraphTaskBase()
		{

		}
	private:

	};

	class NullGraphTask : public CustomStatIDGraphTaskBase
	{
	public:
		NullGraphTask(ENamedThreads::Type inDesiredThread)
			:mDesiredThread(inDesiredThread)
		{}

		ENamedThreads::Type getDesiredThread()
		{
			return mDesiredThread;
		}

		static ESubsequentsMode::Type getSubsequentsMode()
		{
			return ESubsequentsMode::TrackSubsequents;
		}

		CORE_API void doTask(ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent)
		{}


	private:
		ENamedThreads::Type mDesiredThread;
	};

	
	class TriggerEventGraphTask
	{
	public:
		TriggerEventGraphTask(Event* inEvent)
			:mEvent(inEvent)
		{

		}

		ENamedThreads::Type getDesiredThread()
		{
			return ENamedThreads::AnyHiPriThreadHiPriTask;
		}

		static ESubsequentsMode::Type getSubsequentsMode() 
		{
			return ESubsequentsMode::TrackSubsequents;
		}

		void doTask(ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent)
		{
			mEvent->trigger();
		}


	private:
		Event* mEvent;
	};

	class SimpleDelegateGraphTask : public CustomStatIDGraphTaskBase
	{
	public:
		std::function<void()>	mTaskDelegate;
		const ENamedThreads::Type	mDesiredThread;
	public:
		ENamedThreads::Type getDesiredThread()
		{
			return mDesiredThread;
		}

		static ESubsequentsMode::Type getSubsequentsMode()
		{
			return ESubsequentsMode::TrackSubsequents;
		}
		void doTask(ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent)
		{
			mTaskDelegate();
		}

		SimpleDelegateGraphTask(const std::function<void()>& inTaskDelegete, ENamedThreads::Type inDesiredThread)
			: mTaskDelegate(inTaskDelegete)
			, mDesiredThread(inDesiredThread)
		{

		}

		static GraphEventRef createAndDispatchWhenReady(const std::function<void()>& inTaskDeletegate, const GraphEventArray* inPrerequistes = nullptr, ENamedThreads::Type inDesiredThread = ENamedThreads::AnyThread)
		{
			return TGraphTask<SimpleDelegateGraphTask>::createTask(inPrerequistes).constructAndDispatchWhenReady < const std::function<void()> &>(inTaskDeletegate, inDesiredThread);
		}

		static GraphEventRef createAndDispatchWhenReady(const std::function<void()>& inTaskDelegete, const GraphEventRef& inPrerequisite, ENamedThreads::Type inDesiredThread = ENamedThreads::AnyThread)
		{
			GraphEventArray prerequisites;
			prerequisites.push_back(inPrerequisite);
			return createAndDispatchWhenReady(inTaskDelegete, &prerequisites, inDesiredThread);
		}
	};

	class DelegateGraphTask : public CustomStatIDGraphTaskBase
	{
	public:
		typedef std::function<void(ENamedThreads::Type, const GraphEventRef&)> DelegateType;

		DelegateType mTaskDelegate;
		const ENamedThreads::Type	mDesiredThread;
	public:
		ENamedThreads::Type getDesiredThread()
		{
			return mDesiredThread;
		}
		static ESubsequentsMode::Type getSubsequentsMode()
		{
			return ESubsequentsMode::TrackSubsequents;
		}

		void doTask(ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent)
		{
			mTaskDelegate(currentThread, myCompletionGraphEvent);
		}

		DelegateGraphTask(const DelegateType& inTaskDelegate, ENamedThreads::Type inDesiredThread)
			:CustomStatIDGraphTaskBase(),
			mTaskDelegate(inTaskDelegate),
			mDesiredThread(inDesiredThread)
		{

		}

		static GraphEventRef createAndDispatchWhenReady(const DelegateType& InTaskDeletegate, const GraphEventArray* InPrerequisites = NULL, ENamedThreads::Type InCurrentThreadIfKnown = ENamedThreads::AnyThread, ENamedThreads::Type InDesiredThread = ENamedThreads::AnyThread)
		{
			return TGraphTask<DelegateGraphTask>::createTask(InPrerequisites, InCurrentThreadIfKnown).constructAndDispatchWhenReady(InTaskDeletegate, InDesiredThread);
		}

		static GraphEventRef createAndDispatchWhenReady(const DelegateType& InTaskDeletegate, const GraphEventRef& InPrerequisite, ENamedThreads::Type InCurrentThreadIfKnown = ENamedThreads::AnyThread, ENamedThreads::Type InDesiredThread = ENamedThreads::AnyThread)
		{
			GraphEventArray prerequisites;
			prerequisites.push_back(InPrerequisite);
			return createAndDispatchWhenReady(InTaskDeletegate, &prerequisites, InCurrentThreadIfKnown, InDesiredThread);
		}
	};


	class ReturnGraphTask
	{
	public:
		ReturnGraphTask(ENamedThreads::Type inThreadToReturnFrom)
			:mThreadToReturnFrom(inThreadToReturnFrom)
		{

		}


		ENamedThreads::Type getDesiredThread()
		{
			return mThreadToReturnFrom;
		}

		static ESubsequentsMode::Type getSubsequentsMode()
		{
			return ESubsequentsMode::TrackSubsequents;
		}

		void doTask(ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent)
		{
			TaskGraphInterface::get().requestReturn(mThreadToReturnFrom);
		}

	private:
		ENamedThreads::Type mThreadToReturnFrom;
	};

	class CompletionList
	{
		TLockFreePointerListUnordered<GraphEvent, 0> mPrerequisites;
	public:
		void add(const GraphEventRef& taskToAdd)
		{
			GraphEvent* task = taskToAdd.getReference();
			task->AddRef();
			mPrerequisites.push(task);
		}

		GraphEventRef createPrerequisiteCompletionHandle(ENamedThreads::Type currentThread)
		{
			GraphEventRef completeHandle;
			TArray<GraphEvent*> pending;
			mPrerequisites.popAll(pending);
			if (pending.size())
			{
				GraphEventArray	pendingHandles;
				
				for (size_t index = 0; index < pending.size(); index++)
				{
					pendingHandles.push_back(GraphEventRef(pending[index]));
					pending[index]->Release();
				}

				const DelegateGraphTask::DelegateType deleg = std::bind(&CompletionList::chainWaitForPrerequisites, this, std::placeholders::_1, std::placeholders::_2);
				completeHandle = DelegateGraphTask::createAndDispatchWhenReady(deleg, &pendingHandles, currentThread, ENamedThreads::AnyHiPriThreadHiPriTask);
			}
			return completeHandle;
		}

		void chainWaitForPrerequisites(ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent)
		{
			GraphEventRef pendingComplete = createPrerequisiteCompletionHandle(currentThread);
			if (pendingComplete.getInitReference())
			{
				myCompletionGraphEvent->dontCompleteUntil(pendingComplete);
			}
		}
	};
}