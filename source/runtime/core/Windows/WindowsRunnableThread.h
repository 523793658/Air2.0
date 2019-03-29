#pragma once
#include "CoreType.h"
#include "HAL/Runnable.h"
#include "Windows/WindowsSystemIncludes.h"
#include "windows/WindowsHWrapper.h"
#include "HAL/RunnableThread.h"
#include "HAL/Event.h"
#include "HAL/ThreadManager.h"
#include "HAL/PlatformProcess.h"
#include "Containers/StringUtil.h"
namespace Air
{
	class Runnable;

	class RunnableThreadWin : public RunnableThread
	{
	private:
		static DWORD STDCALL _threadProc(LPVOID pThis);

		static void setThreadName(uint32 threadId, LPCSTR ThreadName)
		{
			const uint32 MS_VC_EXCEPTION = 0x406D1388;
			struct THREADNAME_INFO
			{
				uint32 dwType;
				LPCSTR szName;
				uint32 dwThreadID;
				uint32 dwFlags;
			};
			Sleep(10);
			THREADNAME_INFO threadNameInfo;
			threadNameInfo.dwType = 0x1000;
			threadNameInfo.szName = ThreadName;
			threadNameInfo.dwThreadID = threadId;
			threadNameInfo.dwFlags = 0;
			__try
			{
				RaiseException(MS_VC_EXCEPTION, 0, sizeof(threadNameInfo) / sizeof(ULONG_PTR), (ULONG_PTR*)&threadNameInfo);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
				CA_SUPPRESS(6322)
			{
			}
		}

	public:
		RunnableThreadWin()
			: mThread(NULL)
		{}

		~RunnableThreadWin()
		{
			if (mThread != NULL)
			{
				kill(true);
			}
		}

		uint32 guardedRun();


		virtual bool kill(bool shouldWait = false) override
		{
			bool bDidExitOk = true;
			if (mRunnable)
			{
				mRunnable->stop();
			}
			if (shouldWait == true)
			{
				WaitForSingleObject(mThread, INFINITE);
			}
			CloseHandle(mThread);
			mThread = NULL;
			return bDidExitOk;
		}

		virtual bool createInternal(Runnable* inRunnable, const TCHAR * inName, uint32 inStackSize = 0, EThreadPriority inThreadPri = TPri_Normal, uint64 inThreadAffinityMask = 0) override
		{
			mRunnable = inRunnable;
			mThreadAffinityMask = inThreadAffinityMask;

			mThreadInitSyncEvent = PlatformProcess::getSynchEventFromPool(true);

			mThread = CreateThread(NULL, inStackSize, _threadProc, this, STACK_SIZE_PARAM_IS_A_RESERVATION, (::DWORD*)&mThreadID);

			if (mThreadID == NULL)
			{
				mRunnable = nullptr;
			}
			else
			{
				ThreadManager::get().addThread(mThreadID, this);

				mThreadInitSyncEvent->wait(INFINITE);

				mThreadName = inName ? inName : L"Unnamed Air Thread";
				setThreadName(mThreadID, StringUtil::covert(mThreadName).c_str());
				setThreadPriority(inThreadPri);
			}
			PlatformProcess::returnSynchEventToPool(mThreadInitSyncEvent);
			mThreadInitSyncEvent = nullptr;
			return mThread != NULL;
		}

		virtual void setThreadPriority(EThreadPriority newPriority) override
		{

		}

		uint32 run();

		void setTLS();

		void freeTLS();

		virtual void waitForCompletion() override
		{
			WaitForSingleObject(mThread, INFINITE);
		}

	private:
		HANDLE mThread;
	};
}