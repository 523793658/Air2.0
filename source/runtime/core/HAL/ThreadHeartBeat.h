
#pragma once


#include "CoreType.h"

#include "Runnable.h"

#include "HAL/ThreadSafeCounter.h"

#include <mutex>

#include <map>
namespace Air
{
	class CORE_API ThreadHeartBeat : public Runnable 
	{
		static ThreadHeartBeat* mSingleton;

		struct HeartBeatInfo
		{
			HeartBeatInfo()
				: mLastHeartBeatTime(0.0)
				, mSuspendedCount(0)
			{

			}

			double mLastHeartBeatTime;
			int32 mSuspendedCount;
		};


		std::mutex mHeartBeatCritical;

		std::map<uint32, HeartBeatInfo> mThreadHeartBeat;

		ThreadSafeCounter mStopTaskCounter;

	public:

		enum EConstants
		{
			InvalidThreadId = (uint32)-1
		};

		static ThreadHeartBeat& get();
		static ThreadHeartBeat* getNoInit();

		void start();

		void heartBeat();

		uint32 checkHeartBeat();

		void killHeartBeat();

		virtual bool init();

		virtual uint32 run();

		virtual void stop();

		bool isBeating();

		virtual ~ThreadHeartBeat();
	};
}