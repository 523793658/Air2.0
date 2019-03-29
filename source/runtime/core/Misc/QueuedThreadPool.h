#pragma once
#include "CoreType.h"
#include "GenericPlatform/GenericPlatformAffinity.h"
#include "Misc/IQueuedWork.h"
namespace Air
{
	class CORE_API QueuedThreadPool
	{
	public:
		virtual bool create(uint32 inNumQueuedThreads, uint32 statckSize = (32 * 1024), EThreadPriority threadPriority = TPri_Normal) = 0;

		virtual void destroy() = 0;

		virtual void addQueuedWork(IQueuedWork* inQueuedWork) = 0;

		virtual bool retractQueuedWork(IQueuedWork* inQueuedWork) = 0;

		virtual IQueuedWork* returnToPoolOrGetNextJob(class QuquedThread* inQueuedThread) = 0;

		virtual int32 getNumThreads() const = 0;

	public:
		static QueuedThreadPool* allocate();

		static uint32 mOverrideStackSize;
	};

	extern CORE_API QueuedThreadPool* GThreadPool;
}