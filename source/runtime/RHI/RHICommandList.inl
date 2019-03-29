#pragma once

namespace Air
{

	class RHICommandListIterator
	{
	public:
		RHICommandListIterator(RHICommandListBase& cmdlist)
			:mCmdPtr(cmdlist.mRoot),
			mNumCommands(0),
			mCmdListNumCommands(cmdlist.mNumCommands)
		{
		}
		~RHICommandListIterator()
		{
			BOOST_ASSERT(mCmdListNumCommands == mNumCommands);
		}

		FORCEINLINE_DEBUGGABLE bool hasCommandsLeft() const
		{
			return !!mCmdPtr;
		}

		FORCEINLINE_DEBUGGABLE RHICommandBase* nextCommand()
		{
			RHICommandBase* RHICmd = mCmdPtr;
			mCmdPtr = RHICmd->mNext;
			mNumCommands++;
			return RHICmd;
		}
	private:
		RHICommandBase* mCmdPtr;
		uint32 mNumCommands;
		uint32 mCmdListNumCommands;
	};


	FORCEINLINE void RHICommandListImmediate::immediateFlush(EImmediateFlushType::Type flushType)
	{
		switch (flushType)
		{
		case Air::EImmediateFlushType::WaitForOutstandingTasksOnly:
		{
			waitForTasks();
		}
			break;
		case Air::EImmediateFlushType::DispatchToRHIThread:
		{
			if (hasCommands())
			{
				GRHICommandList.executeList(*this);
			}
		}
			break;
		case Air::EImmediateFlushType::WaitForDispatchToRHIThread:
		{
			if (hasCommands())
			{
				GRHICommandList.executeList(*this);
			}
			waitForDispatch();
		}
			break;
		case Air::EImmediateFlushType::FlushRHIThread:
		{
			if (hasCommands())
			{
				GRHICommandList.executeList(*this);
			}
			waitForDispatch();
			if (GRHIThread)
			{
				waitForRHIThreadTasks();
			}
			waitForTasks(true);
		}
			break;
		case Air::EImmediateFlushType::FlushRHIThreadFlushResources:
		{
			if (hasCommands())
			{
				GRHICommandList.executeList(*this);
			}
			waitForDispatch();
			waitForRHIThreadTasks();
			waitForTasks(true);
			RHIResource::flushPendingDeletes();
		}
			break;
		default:
			BOOST_ASSERT(false);
			break;
		}
	}


	FORCEINLINE_DEBUGGABLE bool RHICommandListBase::isImmediate()
	{
		return this == &RHICommandListExecutor::getImmediateCommandList();
	}

	FORCEINLINE_DEBUGGABLE bool RHICommandListBase::isImmediateAsyncCompute()
	{
		return this == &RHICommandListExecutor::getImmediateAsyncComputeCommandList();
	}

	FORCEINLINE_DEBUGGABLE bool RHICommandListBase::bypass()
	{
		return GRHICommandList.bypass();
	}

	FORCEINLINE_DEBUGGABLE 	ScopedRHIThreadStaller::ScopedRHIThreadStaller(class RHICommandListImmediate& inImmed)
		:mImmed(nullptr)
	{
		if (GRHIThread)
		{
			BOOST_ASSERT(isInRenderingThread());
			if (inImmed.stallRHIThread())
			{
				mImmed = &inImmed;
			}
		}
	}
	FORCEINLINE_DEBUGGABLE ScopedRHIThreadStaller::~ScopedRHIThreadStaller()
	{
		if (mImmed)
		{
			mImmed->unStallRHIThread();
		}
	}

}