#include "Windows/WindowsRunnableThread.h"
#include "HAL/PlatformProcess.h"
namespace Air
{
	DWORD STDCALL RunnableThreadWin::_threadProc(LPVOID pThis)
	{
		return static_cast<RunnableThreadWin*>(pThis)->guardedRun();
	}

	uint32 RunnableThreadWin::guardedRun()
	{
		uint32 exitCode = 0;
		PlatformProcess::setThreadAffinityMask(mThreadAffinityMask);
		exitCode = run();
		return exitCode;
	}


	uint32 RunnableThreadWin::run()
	{
		uint32 exitCode = 1;
		if (mRunnable->init() == true)
		{
			mThreadInitSyncEvent->trigger();
			//setTLS();

			exitCode = mRunnable->run();

			mRunnable->exit();

			//freeTLS();
		}
		else
		{
			mThreadInitSyncEvent->trigger();
		}
		return exitCode;
	}

}