#pragma once
#include "HAL/Event.h"
#include "WindowsHWrapper.h"
#include "windows/AllowWindowsPlatformTypes.h"
namespace Air
{
	class EventWin : public Event
	{
	public:
		EventWin() : mEvent(nullptr)
		{

		}

		virtual ~EventWin()
		{
			if (mEvent != nullptr)
			{
				CloseHandle(mEvent);
			}
		}

		virtual bool create(bool bIsManualReset = false)
		{
			mEvent = CreateEvent(nullptr, bIsManualReset, 0, nullptr);
			mManualReset = bIsManualReset;
			return mEvent != nullptr;
		}

		virtual bool isManualReset() override
		{
			return mManualReset;
		}

		virtual void trigger() override;

		virtual void reset() override;

		virtual bool wait(uint32 waitTime, const bool bIgnoreThreadIdleStats = false) override;

	private:
		HANDLE mEvent;
		bool mManualReset;
	};
}