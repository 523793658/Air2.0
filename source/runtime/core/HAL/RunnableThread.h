#pragma once

#include "CoreType.h"
#include "PlatformAffinity.h"
#include "Runnable.h"
#include "Event.h"
#include "Containers/Array.h"
#include "Containers/String.h"
#include "HAL/TIsAutoCleanup.h"
#include "HAL/PlatformTLS.h"
namespace Air
{
	class Runnable;
	class CORE_API RunnableThread
	{
		friend class ThreadManager;


		static uint32 mRunnableTLSSlot;

	public:
		static uint32 getTLSSlot();

		static RunnableThread* create(class Runnable* inRunnable, const TCHAR* threadName, uint32 inStackSize = 0,
			EThreadPriority inThreadPri = TPri_Normal,
			uint64 inThreadAffinityMask = PlatformAffinity::getNoAffinityMask());

		virtual bool kill(bool shouldWait = false) = 0;

		virtual bool createInternal(Runnable* inRunnable, const TCHAR *  inName, uint32 inStackSize = 0, EThreadPriority inThreadPri = TPri_Normal, uint64 inThreadAffinityMask = 0) = 0;

		virtual void setThreadPriority(EThreadPriority newPriority) = 0;

		uint32 getID() const 
		{
			return mThreadID;
		}

		const wstring& getName() const
		{
			return mThreadName;
		}

		virtual void waitForCompletion() = 0;

		static RunnableThread* getRunnableThread()
		{
			RunnableThread* runnableThread = (RunnableThread*)PlatformTLS::getTLSValue(mRunnableTLSSlot);
			return runnableThread;
		}

	private:
		virtual void tick(){}
	protected:
		Runnable* mRunnable;
		Event* mThreadInitSyncEvent;
		wstring mThreadName;
		uint64 mThreadAffinityMask;
		uint32 mThreadID;
	public:
		TArray<TLSAutoCleanup*> mTLSInstances;
	};
}