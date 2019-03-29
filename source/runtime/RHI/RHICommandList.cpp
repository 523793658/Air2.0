#include "HAL/PlatformProcess.h"

#include "RHICommandList.h"
#include "DynamicRHI.h"
#include "Async/TaskGraphInterfaces.h"



namespace Air
{
#include "RHICommandListCommandExecutes.inl"


	int32 RHICommandListBase::mStateCacheEnabled = 1;

	RHI_API RHICommandListExecutor GRHICommandList;

	RHI_API bool GEnableAsyncCompute = true;

	static GraphEventArray	ALLOutstandingTasks;
	static GraphEventArray	WaitOutstandingTasks;
	static GraphEventRef	RHIThreadTask;
	static GraphEventRef	RenderThreadSublistDispatchTask;
	static GraphEventRef	RHIThreadBufferLockFence;

	static GraphEventRef	GRHIThreadEndDrawingViewportFences[2];
	static uint32 GRHIThreadEndDrawingViewportFenceIndex = 0;


	RHICommandBase* GCurrentCommand = nullptr;


	class ExecuteRHIThreadTask
	{
		RHICommandListBase* RHICmdList;
	public:
		ExecuteRHIThreadTask(RHICommandListBase* inRHICmdList)
			:RHICmdList(inRHICmdList)
		{}

		ENamedThreads::Type getDesiredThread()
		{
			BOOST_ASSERT(GRHIThread);
			return ENamedThreads::RHIThread;
		}

		static ESubsequentsMode::Type getSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

		void doTask(ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent)
		{
			RHICommandListExecutor::executeInner_DoExecute(*RHICmdList);
			delete RHICmdList;
		}
	};


	class DispatchRHIThreadTask
	{
		RHICommandListBase* RHICmdList;
		bool bRHIThread;
	public:
		DispatchRHIThreadTask(RHICommandListBase* inRHICmdList, bool bInRHIThread)
			:RHICmdList(inRHICmdList)
			,bRHIThread(bInRHIThread)
		{}
		ENamedThreads::Type getDesiredThread()
		{
			BOOST_ASSERT(GRHIThread);
			return bRHIThread ? ENamedThreads::RHIThread : ENamedThreads::RenderThread_Local;
		}
		static ESubsequentsMode::Type getSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }
		void doTask(ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent)
		{
			BOOST_ASSERT(bRHIThread || isInRenderingThread());
			GraphEventArray prereq;
			if (RHIThreadTask.getReference())
			{
				prereq.add(RHIThreadTask);
			}
			RHIThreadTask = GraphTask<ExecuteRHIThreadTask>::createTask(&prereq, currentThread).constructAndDispatchWhenReady(RHICmdList);
		}
	};


	RHICommandListBase::RHICommandListBase()
		:StrictGraphicsPipelineStateUse(0)
	{
		GRHICommandList.mOutstandingCmdListCount.increment();
		reset();
	}

	RHICommandListBase::~RHICommandListBase()
	{
		flush();
		GRHICommandList.mOutstandingCmdListCount.decrement();
	}

	struct RHICommandWaitForAndSubmitRTSubList : public RHICommand<RHICommandWaitForAndSubmitRTSubList>
	{
		GraphEventRef mEventToWaitFor;
		RHICommandList* mRHICmdList;
		FORCEINLINE_DEBUGGABLE RHICommandWaitForAndSubmitRTSubList(GraphEventRef& inEventToWaitFor, RHICommandList* inRHICmdList)
			:mEventToWaitFor(inEventToWaitFor)
			,mRHICmdList(inRHICmdList)
		{}

		void execute(RHICommandListBase& cmdList)
		{
			if (mEventToWaitFor.getReference() && !mEventToWaitFor->isComplete())
			{
				BOOST_ASSERT(!GRHIThread || !isInRHIThread());
				if (isInRenderingThread())
				{
					if (TaskGraphInterface::get().isThreadProcessingTasks(ENamedThreads::RenderThread_Local))
					{

					}
					TaskGraphInterface::get().waitUntilTaskCompletes(mEventToWaitFor, ENamedThreads::RenderThread_Local);
				}
				else
				{
					TaskGraphInterface::get().waitUntilTaskCompletes(mEventToWaitFor);
				}
			}
			{
				mRHICmdList->copyContext(cmdList);
				delete mRHICmdList;
			}
		}
	};

	const int32 RHICommandListBase::getUsedMemory() const
	{
		return mMemManager.getByteCount();
	}

	void RHICommandListBase::queueRenderThreadCommandListSubmit(GraphEventRef& renderThreadCompletionEvent, class RHICommandList* cmdList)
	{
		BOOST_ASSERT(!isInRHIThread());
		flushStateCache();
		if (renderThreadCompletionEvent.getReference())
		{
			BOOST_ASSERT(!isInActualRenderinThread() && !isInGameThread() && !isImmediate());
			mRTTasks.add(renderThreadCompletionEvent);
		}
		new (allocCommand<RHICommandWaitForAndSubmitRTSubList>())RHICommandWaitForAndSubmitRTSubList(renderThreadCompletionEvent, cmdList);
	}

	void RHICommandListBase::flush()
	{
		if (hasCommands())
		{
			GRHICommandList.executeList(*this);
		}
	}

	void RHICommandListBase::reset()
	{
		mExecuting = false;
		mMemManager.flush();
		mNumCommands = 0;
		mRoot = nullptr;
		mCommandLink = &mRoot;
		mContext = GDynamicRHI ? RHIGetDefualtContext() : nullptr;

		if (GEnableAsyncCompute)
		{
			mComputeContext = GDynamicRHI ? RHIGetDefualtAsyncComputeContext() : nullptr;
		}
		else
		{
			mComputeContext = mContext;
		}
		mUID = GRHICommandList.mUIDCounter.increment();
		for (int32 index = 0; ERenderThreadContext(index) < ERenderThreadContext::Num; index++)
		{
			mRenderThreadContext[index] = nullptr;
		}
	}

	void RHICommandListBase::waitForDispatch()
	{
		BOOST_ASSERT(isImmediate() && isInRenderingThread());
		BOOST_ASSERT(!ALLOutstandingTasks.size());
		if (RenderThreadSublistDispatchTask.getReference() && RenderThreadSublistDispatchTask->isComplete())
		{
			RenderThreadSublistDispatchTask = nullptr;
		}
		while (RenderThreadSublistDispatchTask.getReference())
		{
			if (TaskGraphInterface::get().isThreadProcessingTasks(ENamedThreads::RenderThread_Local))
			{

			}
			TaskGraphInterface::get().waitUntilTaskCompletes(RenderThreadSublistDispatchTask, ENamedThreads::RenderThread_Local);
			if (RenderThreadSublistDispatchTask.getReference() && RenderThreadSublistDispatchTask->isComplete())
			{
				RenderThreadSublistDispatchTask = nullptr;
			}
		}
	}

	void RHICommandListBase::waitForRHIThreadTasks()
	{
		BOOST_ASSERT(isImmediate() && isInRenderingThread());
		if (RHIThreadTask.getReference() && RHIThreadTask->isComplete())
		{
			RHIThreadTask = nullptr;
		}
		while (RHIThreadTask.getReference())
		{
			if (TaskGraphInterface::get().isThreadProcessingTasks(ENamedThreads::RenderThread_Local))
			{
				while (!RHIThreadTask->isComplete())
				{
					PlatformProcess::sleepNoStats(0);
				}
			}
			else
			{
				TaskGraphInterface::get().waitUntilTaskCompletes(RHIThreadTask, ENamedThreads::RenderThread_Local);
			}
			if (RHIThreadTask.getReference() && RHIThreadTask->isComplete())
			{
				RHIThreadTask = nullptr;
			}
		}
	}

	void RHICommandListBase::waitForTasks(bool bKnownToBeComplete)
	{
		BOOST_ASSERT(isImmediate() && isInRenderingThread());
		if (WaitOutstandingTasks.size())
		{
			bool bAny = false;
			for (int32 index = 0; index < WaitOutstandingTasks.size(); index++)
			{
				if (!WaitOutstandingTasks[index]->isComplete())
				{
					bAny = true;
				}
			}
			if (bAny)
			{
				TaskGraphInterface::get().waitUntilTaskComplete(WaitOutstandingTasks, ENamedThreads::RenderThread_Local);
			}
			WaitOutstandingTasks.clear();
		}
	}

	RHICommandListImmediate& RHICommandListExecutor::getImmediateCommandList()
	{
		return GRHICommandList.mCommandListImmediate;
	}

	RHIAsyncComputeCommandListImmediate& RHICommandListExecutor::getImmediateAsyncComputeCommandList()
	{
		return GRHICommandList.mAsyncComputeCmdListImmediate;
	}

	void RHICommandListExecutor::executeList(RHICommandListBase& cmdList)
	{
		executeInner(cmdList);
	}

	void RHICommandListExecutor::executeInner_DoExecute(RHICommandListBase& cmdList)
	{
		cmdList.mExecuting = true;
		RHICommandListIterator iter(cmdList);
		{
			while (iter.hasCommandsLeft())
			{
				RHICommandBase* cmd = iter.nextCommand();
				GCurrentCommand = cmd;
				cmd->CallExecuteAndDestruct(cmdList);
			}
		}
		cmdList.reset();
	}
	void* RHICommandList::operator new(size_t size)
	{
		return Memory::malloc(size);
	}

	void RHICommandList::operator delete(void* rawMemory)
	{
		Memory::free(rawMemory);
	}

	void* RHICommandListBase::operator new(size_t size)
	{
		return Memory::malloc(size);
	}

	void RHICommandListBase::operator delete(void* rawMemory)
	{
		Memory::free(rawMemory);
	}



	void RHICommandListExecutor::executeInner(RHICommandListBase& cmdList)
	{
		bool bIsInRenderingThread = isInRenderingThread();
		bool bIsInGameThread = isInGameThread();
		if (GRHIThread)
		{
			bool bAsyncSubmit = false;
			if (bIsInRenderingThread)
			{
				if (!bIsInGameThread && !TaskGraphInterface::get().isThreadProcessingTasks(ENamedThreads::RenderThread_Local))
				{
					TaskGraphInterface::get().processThreadUntilIdle(ENamedThreads::RenderThread_Local);
				}
				bAsyncSubmit = false;
				if (!bAsyncSubmit&& RHIThreadTask.getReference() && RHIThreadTask->isComplete())
				{
					RHIThreadTask = nullptr;
				}
			}
			if (bIsInRenderingThread && !bIsInGameThread)
			{
				RHICommandList* swapCmdList;
				{
					swapCmdList = new RHICommandList;
					static_assert(sizeof(RHICommandList) == sizeof(RHICommandListImmediate), "we are memswapping RHICommandList and RHICommandListImmediate, the need to be swapable");
					swapCmdList->exchangeCmdList(cmdList);
				}
				if (ALLOutstandingTasks.size() || RenderThreadSublistDispatchTask.getReference())
				{
					GraphEventArray prereq(ALLOutstandingTasks);
					ALLOutstandingTasks.clear();
					if (RenderThreadSublistDispatchTask.getReference())
					{
						prereq.add(RenderThreadSublistDispatchTask);
					}
					RenderThreadSublistDispatchTask = GraphTask<DispatchRHIThreadTask>::createTask(&prereq, ENamedThreads::RenderThread).constructAndDispatchWhenReady(swapCmdList, bAsyncSubmit);

				}
				else
				{
					BOOST_ASSERT(!RenderThreadSublistDispatchTask.getReference());
					GraphEventArray prereq;
					if (RHIThreadTask.getReference())
					{
						prereq.add(RHIThreadTask);
					}
					RHIThreadTask = GraphTask<ExecuteRHIThreadTask>::createTask(&prereq, ENamedThreads::RenderThread).constructAndDispatchWhenReady(swapCmdList);
				}
				return;
			}
			if (bIsInRenderingThread)
			{
				if (RenderThreadSublistDispatchTask.getReference())
				{
					if (TaskGraphInterface::get().isThreadProcessingTasks(ENamedThreads::RenderThread_Local))
					{

					}
					TaskGraphInterface::get().waitUntilTaskCompletes(RenderThreadSublistDispatchTask, ENamedThreads::RenderThread_Local);
					RenderThreadSublistDispatchTask = nullptr;
				}
				while (RHIThreadTask.getReference())
				{
					if (TaskGraphInterface::get().isThreadProcessingTasks(ENamedThreads::RenderThread_Local))
					{

					}
					TaskGraphInterface::get().waitUntilTaskCompletes(RHIThreadTask, ENamedThreads::RenderThread_Local);
					if (RHIThreadTask.getReference() && RHIThreadTask->isComplete())
					{
						RHIThreadTask = nullptr;
					}
				}
			}
		}
		executeInner_DoExecute(cmdList);
	}

	void RHICommandListExecutor::checkNoOutstandingCmdLists()
	{
		BOOST_ASSERT(GRHICommandList.mOutstandingCmdListCount.getValue() == 2, "outstanding cmdlists");
	}

	void RHICommandListExecutor::latchBypass()
	{

	}

	void RHICommandListExecutor::waitOnRHIThreadFence(GraphEventRef& fence)
	{
		BOOST_ASSERT(isInRenderingThread());
		if (fence.getReference() && !fence->isComplete())
		{
			{
				getImmediateCommandList().immediateFlush(EImmediateFlushType::DispatchToRHIThread);
			}
			BOOST_ASSERT(GRHIThread);
			if (TaskGraphInterface::get().isThreadProcessingTasks(ENamedThreads::RenderThread_Local))
			{

			}
			TaskGraphInterface::get().waitUntilTaskCompletes(fence, ENamedThreads::RenderThread_Local);
		}
	}

	void RHICommandList::beginDrawingViewport(ViewportRHIParamRef viewport, TextureRHIParamRef renderTargetRHI)
	{
		BOOST_ASSERT(isImmediate() && isInRenderingThread());
		if (bypass())
		{
			CMD_CONTEXT(RHIBeginDrawingViewport)(viewport, renderTargetRHI);
			return;
		}
		new (allocCommand<RHICommandBeginDrawingViewport>())RHICommandBeginDrawingViewport(viewport, renderTargetRHI);
		if (!GRHIThread)
		{
			RHICommandListExecutor::getImmediateCommandList().immediateFlush(EImmediateFlushType::FlushRHIThread);
		}
	}

	void endDrawingViewport(ViewportRHIParamRef viewport, bool bPreset, bool bLockToVsync);

	void RHICommandList::endDrawingViewport(ViewportRHIParamRef viewport, bool bPreset, bool bLockToVsync)
	{
		BOOST_ASSERT(isImmediate() && isInRenderingThread());
		if(bypass())
		{
			CMD_CONTEXT(RHIEndDrawingViewport)(viewport, bPreset, bLockToVsync);
		}
		else
		{
			new (allocCommand<RHICommandEndDrawingViewport>())RHICommandEndDrawingViewport(viewport, bPreset, bLockToVsync);
			if (GRHIThread)
			{
				GRHIThreadEndDrawingViewportFences[GRHIThreadEndDrawingViewportFenceIndex] = static_cast<RHICommandListImmediate*>(this)->RHIThreadFence();
			}
			{
				RHICommandListExecutor::getImmediateCommandList().immediateFlush(EImmediateFlushType::DispatchToRHIThread);
			}
		}
		if (GRHIThread)
		{

		}
		RHIAdvanceFrameForGetViewportBackBuffer();
	}

	void RHICommandList::beginFrame()
	{
		BOOST_ASSERT(isImmediate() && isInRenderingThread());
		if (bypass())
		{
			CMD_CONTEXT(RHIBeginFrame)();
			return;
		}
		new (allocCommand<RHICommandBeginFrame>())RHICommandBeginFrame();
		if (!GRHIThread)
		{
			RHICommandListExecutor::getImmediateCommandList().immediateFlush(EImmediateFlushType::FlushRHIThread);
		}
	}

	void RHICommandList::endFrame()
	{
		BOOST_ASSERT(isImmediate() && isInRenderingThread());
		if (bypass())
		{
			CMD_CONTEXT(RHIEndFrame)();
			return;
		}
		new(allocCommand<RHICommandEndFrame>())RHICommandEndFrame();
		if (!GRHIThread)
		{
			RHICommandListExecutor::getImmediateCommandList().immediateFlush(EImmediateFlushType::FlushRHIThread);
		}
	}

	void RHICommandList::beginScene()
	{
		BOOST_ASSERT(isImmediate() && isInRenderingThread());
		if (bypass())
		{
			CMD_CONTEXT(RHIBeginScene)();
			return;
		}
		new (allocCommand<RHICommandBeginScene>())RHICommandBeginScene();
		if (!GRHIThread)
		{
			RHICommandListExecutor::getImmediateCommandList().immediateFlush(EImmediateFlushType::FlushRHIThread);
		}
	}


	void RHICommandList::endScene()
	{
		BOOST_ASSERT(isImmediate() && isInRenderingThread());
		if (bypass())
		{
			CMD_CONTEXT(RHIEndScene)();
			return;
		}
		new (allocCommand<RHICommandEndScene>())RHICommandEndScene();
		if (!GRHIThread)
		{
			RHICommandListExecutor::getImmediateCommandList().immediateFlush(EImmediateFlushType::FlushRHIThread);
		}
	}

	

	struct RHICommandRHIThreadFence : public RHICommand<RHICommandRHIThreadFence>
	{
		GraphEventRef mFence;
		FORCEINLINE_DEBUGGABLE RHICommandRHIThreadFence()
			:mFence(GraphEvent::createGraphEvent())
		{

		}
		void execute(RHICommandListBase& cmdlist)
		{
			BOOST_ASSERT(isInRHIThread());
			static TArray<BaseGraphTask*> newTask;
			mFence->dispatchSubsequents(newTask, ENamedThreads::RHIThread);
			mFence = nullptr;
		}
	};

	GraphEventRef RHICommandListImmediate::RHIThreadFence(bool bSetLockFence /* = false */)
	{
		BOOST_ASSERT(isInRenderingThread() && GRHIThread);

		RHICommandRHIThreadFence* cmd = new (allocCommand<RHICommandRHIThreadFence>())RHICommandRHIThreadFence();
		if (bSetLockFence)
		{
			RHIThreadBufferLockFence = cmd->mFence;
		}
		return cmd->mFence;
	}
	static GraphEventRef GRHIThreadStallTask;
	static Event* GRHIThreadStallEvent = nullptr;

	int32 StallCount = 0;
	bool RHICommandListImmediate::stallRHIThread()
	{
		PlatformAtomics::interlockedIncrement(&StallCount);
		BOOST_ASSERT(isInRenderingThread() && GRHIThread && !GRHIThreadStallTask.getReference());
		bool bAsyncSubmit = false;
		if (bAsyncSubmit)
		{

		}
		else
		{
			waitForRHIThreadTasks();
			return false;
		}
	}

	void RHICommandListImmediate::unStallRHIThread()
	{
		BOOST_ASSERT(isInRenderingThread() && GRHIThread && GRHIThreadStallTask.getReference() && !GRHIThreadStallTask->isComplete() && GRHIThreadStallEvent);
		GRHIThreadStallEvent->trigger();
		while (!GRHIThreadStallTask->isComplete())
		{
			PlatformProcess::sleepNoStats(0);
		}
		GRHIThreadStallTask = nullptr;
		PlatformAtomics::interLockedDecrement(&StallCount);
	}
	
	void RHICommandListImmediate::updateTextureReference(TextureReferenceRHIParamRef textureRef, TextureRHIParamRef newTexture)
	{
		if (bypass() || !GRHIThread)
		{
			{
				immediateFlush(EImmediateFlushType::FlushRHIThread);
			}
			CMD_CONTEXT(RHIUpdateTextureReference)(textureRef, newTexture);
			return;
		}

		new (allocCommand<RHICommandUpdateTextureReference>())RHICommandUpdateTextureReference(textureRef, newTexture);
		RHIThreadFence(true);
		if (getUsedMemory() > 256 > 1024)
		{
			immediateFlush(EImmediateFlushType::DispatchToRHIThread);
		}
	}
}