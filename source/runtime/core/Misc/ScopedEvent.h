#pragma once
#include "HAL/Event.h"
namespace Air
{
	class ScopedEvent
	{
	public:
		CORE_API ScopedEvent();

		CORE_API ~ScopedEvent();

		void trigger()
		{
			mEvent->trigger();
		}

		Event* get()
		{
			return mEvent;
		}
	private:
		Event* mEvent;
	};
}