#include "ScopedEvent.h"
#include "Misc/EventPool.h"
namespace Air
{
	ScopedEvent::ScopedEvent()
		:mEvent(EventPool<EEventPoolTypes::AutoReset>::get().getEventFromPool())
	{

	}

	ScopedEvent::~ScopedEvent()
	{
		mEvent->wait();
		EventPool<EEventPoolTypes::AutoReset>::get().returnToPool(mEvent);
		mEvent = nullptr;
	}
}