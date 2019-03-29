#pragma once
#include "CoreType.h"
#include <limits>
namespace Air
{
	class Event
	{
	public:
		virtual bool create(bool bIsManualReset = false) = 0;

		virtual bool isManualReset() = 0;

		virtual void trigger() = 0;

		virtual void reset() = 0;

		virtual bool wait(uint32 waitTime, const bool bIgnoreThreadIdleStats = false) = 0;

		bool wait()
		{
			return wait(INT_MAX);
		}

		Event()
			:mEventId(0)
			, mEventStartCycles(0)
		{

		}

		virtual ~Event()
		{

		}

		void advanceStats();

	protected:
		void waitForStats();

		void triggerForStats();

		void resetForStats();


		uint32 mEventId;

		uint32 mEventStartCycles;

		static uint32 mEventUniqueId;
	};
}