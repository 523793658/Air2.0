#pragma once
#include "RenderCore.h"
#include "Async/TaskGraphInterfaces.h"
#include "Async/TaskGraph.h"
#include <boost/call_traits.hpp>
#include "TickableObjectRenderThread.h"
#include <mutex>

namespace Air
{
	class DeferredCleanupInterface;

	extern RENDER_CORE_API bool GIsThreadedRendering;

	extern RENDER_CORE_API bool GUseThreadedRendering;

	extern RENDER_CORE_API void startRenderingThread();

	extern RENDER_CORE_API void flushRenderingCommands();

	extern RENDER_CORE_API void advanceFrameRenderPrerequisite();

	extern RENDER_CORE_API void checkRenderingThreadHealth();

	extern RENDER_CORE_API void beginCleanup(DeferredCleanupInterface* cleanupObject);

	class RENDER_CORE_API RenderCommand
	{
	public:
		static ENamedThreads::Type getDesiredThread()
		{
			return ENamedThreads::getRenderThread();
		}

		static ESubsequentsMode::Type getSubsequentsMode()
		{
			return ESubsequentsMode::FireAndForget;
		}
	};

	class RENDER_CORE_API DeferredCleanupInterface
	{
	public:
		virtual void finishCleanup() final {};
		virtual ~DeferredCleanupInterface() {}
	};

	class PendingCleanupObjects
	{
		TArray<DeferredCleanupInterface*> mCleanupArray;
	public:
		PendingCleanupObjects();
		RENDER_CORE_API ~PendingCleanupObjects();
	};


	class RENDER_CORE_API SuspendRenderingThread
	{
	public:
		SuspendRenderingThread(bool bRecreateThread);

		~SuspendRenderingThread();
	private:
		bool bUseRenderingThread;
		bool bWasRenderingThreadRunning;
		bool bRecreateThread;
	};

#define SCOPED_SUSPEND_RENDERING_THREAD(bRecreateThread) SuspendRenderingThread mSuspendRenderingThread(bRecreateThread)


	extern RENDER_CORE_API void checkNotBlockedOnRenderThread();

	extern void gameThreadWaitForTask(const GraphEventRef& task, bool bEmpltyGameThreadTasks = false);

	extern RENDER_CORE_API class RHICommandListImmediate& getImmediateCommandList_ForRenderCommand();


#define shouldExecuteOnRenderThread()	(LIKELY(GIsThreadedRendering || !isInGameThread()))


#define TASK_FUNCTION( Code )	\
	void doTask(ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent)	\
	{	\
		RHICommandListImmediate& RHICmdList = getImmediateCommandList_ForRenderCommand();\
		Code;	\
	}

#define TASKNAME_FUNCTION()	\
	FORCEINLINE std::mutex& getMutex()	\
	{		\
		static std::mutex mMutext;	\
		return mMutext;	\
	}

	template<typename TSTR, typename LAMBDA>
	class TEnqueueUniqueRenderCommandType : public RenderCommand
	{
	public:
		TEnqueueUniqueRenderCommandType(LAMBDA&& inLambda) : mLambda(std::forward<LAMBDA>(inLambda)) {}

		void doTask(ENamedThreads::Type currentThread, const GraphEventRef& myCompletionEvent)
		{
			RHICommandListImmediate& RHICmdList = getImmediateCommandList_ForRenderCommand();
			mLambda(RHICmdList);
		}
	private:
		LAMBDA mLambda;
	};

	template<typename TSTR, typename LAMBDA>
	FORCEINLINE_DEBUGGABLE void enqueueUniqueRenderCommand(LAMBDA&& lambda)
	{
		typedef TEnqueueUniqueRenderCommandType<TSTR, LAMBDA> EURCType;
#if 0

#endif // 0
		if (isInRenderingThread())
		{
			RHICommandListImmediate& RHICmdList = getImmediateCommandList_ForRenderCommand();
			lambda(RHICmdList);
		}
		else
		{
			if (shouldExecuteOnRenderThread())
			{
				checkNotBlockedOnRenderThread();
				TGraphTask<EURCType>::createTask().constructAndDispatchWhenReady(std::forward<LAMBDA>(lambda));
			}
			else
			{
				EURCType tempCommand(std::forward<LAMBDA>(lambda));
				tempCommand.doTask(ENamedThreads::GameThread, GraphEventRef());
			}
		}
	}


#define ENQUEUE_RENDER_COMMAND(Type) \
	struct Type##Name \
	{ \
		static const char* CStr() {return #Type;}\
		static const TCHAR* TStr() {return TEXT(#Type);}\
	};\
	enqueueUniqueRenderCommand<Type##Name>

	template<typename LAMBDA>
	FORCEINLINE_DEBUGGABLE void enqueueUniqueRenderCommand(LAMBDA lambda)
	{
		static_assert(sizeof(LAMBDA) == 0, "enqueueUniqueRenderCommand enforces use of rvalue and threfore move to avoid an extra copy of the lambda");
	}
}
