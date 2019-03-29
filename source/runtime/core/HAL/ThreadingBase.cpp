#include "RunnableThread.h"
#include "PlatformProcess.h"
#include "HAL/Event.h"
#include "HAL/PlatformTLS.h"
#include "HAL/ThreadManager.h"
#include "Misc/QueuedThreadPool.h"
#include "CoreGlobals.h"
#include "HAL/ThreadSingleton.h"
namespace Air
{
	uint32 Event::mEventUniqueId = 0;
	uint32 RunnableThread::mRunnableTLSSlot = RunnableThread::getTLSSlot();

	CORE_API RunnableThread* GRenderingThread = nullptr;

	CORE_API int32 GIsRenderingThreadSuspended = 0;

	CORE_API RunnableThread* GRHIThread = nullptr;

	QueuedThreadPool* GThreadPool = nullptr;

	void Event::advanceStats()
	{
	}

	void Event::waitForStats()
	{

	}

	void Event::triggerForStats()
	{

	}

	void Event::resetForStats()
	{

	}

	CORE_API bool isInSlateThread()
	{
		return GSlateLoadingThreadId != 0 && PlatformTLS::getCurrentThreadId() == GSlateLoadingThreadId;
	}

	CORE_API bool isInParallelRenderingThread()
	{
		return !GRenderingThread || GIsRenderingThreadSuspended || (PlatformTLS::getCurrentThreadId() != GGameThreadId);
	}

	CORE_API bool isInRHIThread()
	{
		return GRHIThread && PlatformTLS::getCurrentThreadId() == GRHIThread->getID();
	}

	RunnableThread* RunnableThread::create(class Runnable* inRunnable, const TCHAR* threadName, uint32 inStackSize /* = 0 */, EThreadPriority inThreadPri /* = TPri_Normal */, uint64 inThreadAffinityMask /* = PlatformAffinity::GetNoAffinityMask() */)
	{
		RunnableThread* newThread = nullptr;
		if (PlatformProcess::supportsMultithreading())
		{
			newThread = PlatformProcess::createRunnableThread();
			if (newThread)
			{
				if (newThread->createInternal(inRunnable, threadName, inStackSize, inThreadPri, inThreadAffinityMask) == false)
				{
					delete newThread;
					newThread = nullptr;
				}
			}
		}
		else
		{
			BOOST_ASSERT(false);
		}

		return newThread;
	}

	uint32 RunnableThread::getTLSSlot()
	{
		BOOST_ASSERT(isInGameThread());
		uint32 tlsSlot = PlatformTLS::allocTLSSlot();
		BOOST_ASSERT(PlatformTLS::isValidTLSSlot(tlsSlot));
		return tlsSlot;
	}

	void ThreadManager::addThread(uint32 threadId, class RunnableThread* thread)
	{
		std::lock_guard<std::mutex> lock(mThreadManagerSingleton);
		auto it = mThreads.find(threadId);
		if (it == mThreads.end())
		{
			mThreads.emplace(threadId, thread);
		}
	}

	void ThreadManager::removeThread(class RunnableThread* thread)
	{
		std::lock_guard<std::mutex> lock(mThreadManagerSingleton);
		auto it = mThreads.find(thread->getID());
		if (it != mThreads.end())
		{
			mThreads.erase(thread->getID());
		}
	}

	void ThreadManager::tick()
	{
		if (!PlatformProcess::supportsMultithreading())
		{
			std::lock_guard<std::mutex> lock(mThreadManagerSingleton);
			for (auto& it : mThreads)
			{
				it.second->tick();
			}
		}
	}

	const wstring& ThreadManager::getThreadName(uint32 threadId)
	{
		std::lock_guard<std::mutex> lock(mThreadManagerSingleton);

		auto it = mThreads.find(threadId);
		if (it != mThreads.end())
		{
			return it->second->getName();
		}
	}

	ThreadManager& ThreadManager::get()
	{
		static ThreadManager manger;
		return manger;
	}


	CORE_API bool isInRenderingThread()
	{
		return !GRenderingThread || GIsRenderingThreadSuspended ||
			(PlatformTLS::getCurrentThreadId() == GRenderingThread->getID());
	}

	CORE_API bool isInActualRenderinThread()
	{
		return GRenderingThread && PlatformTLS::getCurrentThreadId() == GRenderingThread->getID();
	}


	class QueuedThread
		:public Runnable
	{

	};



	void TLSAutoCleanup::Register()
	{
		RunnableThread* runnableThread = RunnableThread::getRunnableThread();
		if (runnableThread)
		{
			runnableThread->mTLSInstances.push_back(this);
		}
	}

	TLSAutoCleanup* ThreadSingletonInitializer::get(std::function<TLSAutoCleanup*()> createInstance, uint32& tlsSlot)
	{
		if (tlsSlot == 0xffffffff)
		{
			const uint32 thisTLSSlot = PlatformTLS::allocTLSSlot();
			BOOST_ASSERT(PlatformTLS::isValidTLSSlot(thisTLSSlot));
			const uint32 prevTlsSlot = PlatformAtomics::interlockedCompareExchange((int32*)&tlsSlot, (int32)thisTLSSlot, 0xffffffff);
			if (prevTlsSlot != 0xffffffff)
			{
				PlatformTLS::freeTlsSlot(thisTLSSlot);
			}
		}
		TLSAutoCleanup* threadSingleton = (TLSAutoCleanup*)PlatformTLS::getTLSValue(tlsSlot);
		if (!threadSingleton)
		{
			threadSingleton = createInstance();
			threadSingleton->Register();
			PlatformTLS::setTlsValue(tlsSlot, threadSingleton);
		}
		return threadSingleton;
	}
}