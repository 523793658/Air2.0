#include "Windows/WindowsEvent.h"
namespace Air
{
	void EventWin::trigger()
	{
		//TriggerFors

		SetEvent(mEvent);
	}

	void EventWin::reset()
	{
		ResetEvent(mEvent);
	}

	bool EventWin::wait(uint32 waitTime, const bool bIgnoreThreadIdleStats )
	{
		return (WaitForSingleObject(mEvent, waitTime) == WAIT_OBJECT_0);
	}
}