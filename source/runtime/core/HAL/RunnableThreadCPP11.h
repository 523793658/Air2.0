#pragma once
/*
	目前c++11的线程一些功能还不全，很多操作（像设置优先级等）还是需要依赖操作系统函数。因此先不单独做一个c++11的版本。

*/




#include "HAL/RunnableThread.h"
#include "HAL/PlatformProcess.h"

#ifndef _PLATFORM_DISABLE_CPP11_THREAD_
#include <functional>
namespace Air
{
	class RunnableTrheadCPP11 : public RunnableThread
	{
	protected:
		std::thread* mThread;
		
		bool bThreadStartedAndNotCleanedUp;

		typedef std::function<void*(void*)>	ThreadEntryPoint;

		static void* STDCALL _ThreadProc(RunnableTrheadCPP11* pThis)
		{

		}

		bool spinThread(std::thread** threadPtr, bool* outThreadCreated, ThreadEntryPoint proc, uint32 inStackSize)
		{
			*outThreadCreated = false;
			std::thread* th = new std::thread(proc);
		}

	protected:

		virtual void setThreadPriority(EThreadPriority newPriority) override
		{

		}

		virtual bool createInternal(Runnable* inRunnable, const CHAR * inName, uint32 inStackSize = 0, EThreadPriority inThreadPri = TPri_Normal, uint64 inThreadAffinityMask = 0) override
		{
			mRunnable = inRunnable;
			mThreadInitSyncEvent = PlatformProcess::getSynchEventFromPool(true);
			mThreadName = inName ? inName : L"Unnamed Air";
			mThreadAffinityMask = inThreadAffinityMask;
			const bool threadCreated = spinThread(&mThread, &bThreadStartedAndNotCleanedUp, std::bind(RunnableTrheadCPP11::_ThreadProc, this), inStackSize);

			if (threadCreated)
			{
				mThreadInitSyncEvent->wait((uint32)-1);
				setThreadPriority(inThreadPri);
			}
			else
			{
				mRunnable = nullptr;
			}
			PlatformProcess::returnSynchEventToPool(mThreadInitSyncEvent);
			mThreadInitSyncEvent = nullptr;
			return bThreadStartedAndNotCleanedUp;
		}
	};
}



#endif
