#include "ThreadHeartBeat.h"
#include "HAL/PlatformTime.h"
#include "HAL/PlatformTLS.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformProcess.h"
#include "boost/boost/assert.hpp"

namespace Air
{
	ThreadHeartBeat* ThreadHeartBeat::mSingleton = nullptr;

	ThreadHeartBeat::~ThreadHeartBeat()
	{

	}

	bool ThreadHeartBeat::init()
	{
		return true;
	}

	uint32 ThreadHeartBeat::run()
	{
		bool InHungState = false;
		while (mStopTaskCounter.getValue() == 0)
		{
			uint32 threadThatHung = checkHeartBeat();

			if (threadThatHung == ThreadHeartBeat::InvalidThreadId)
			{
				InHungState = false;
			}
			else if (InHungState == false)
			{
				InHungState = true;

				const SIZE_T StackTraceSize = 65535;
				ANSICHAR* stackTrace = (ANSICHAR*)GMalloc->malloc(StackTraceSize);
				stackTrace[0] = 0;
				GMalloc->free(stackTrace);
			}
			PlatformProcess::sleepNoStats(0.5f);
		}
		return 0;
	}

	void ThreadHeartBeat::stop()
	{

	}
	uint32 ThreadHeartBeat::checkHeartBeat()
	{
		return static_cast<uint32>(InvalidThreadId);
	}



	ThreadHeartBeat& ThreadHeartBeat::get()
	{
		struct FInitHelper
		{
			ThreadHeartBeat* mInstance;
			FInitHelper()
			{
				mInstance = new ThreadHeartBeat();
				mSingleton = mInstance;
			}

			~FInitHelper()
			{
				mSingleton = nullptr;
				delete mInstance;
				mInstance = nullptr;
			}
		};
		static FInitHelper mHelper;
		return *mHelper.mInstance;
	}

	ThreadHeartBeat* ThreadHeartBeat::getNoInit()
	{
		return mSingleton;
	}

	void ThreadHeartBeat::heartBeat()
	{
		uint32 threadId = PlatformTLS::getCurrentThreadId();
		std::lock_guard<std::mutex> lock(mHeartBeatCritical);
		auto rs = mThreadHeartBeat.find(threadId);
		if (rs == mThreadHeartBeat.end())
		{
			rs = mThreadHeartBeat.emplace(threadId, HeartBeatInfo()).first;
		}
		rs->second.mLastHeartBeatTime = PlatformTime::seconds();
	}

	bool ThreadHeartBeat::isBeating()
	{
		uint32 threadId = PlatformTLS::getCurrentThreadId();
		std::lock_guard<std::mutex> lock(mHeartBeatCritical);
		mThreadHeartBeat.find(threadId);
		auto it = mThreadHeartBeat.find(threadId);
		if (it != mThreadHeartBeat.end() && it->second.mSuspendedCount == 0)
		{
			return true;
		}
		return false;
	}
}