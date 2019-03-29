#pragma once
#include "CoreType.h"
#include <unordered_set>
#include "HAL/Event.h"
#include "HAL/PlatformProcess.h"
namespace Air
{
	enum class EEventPoolTypes
	{
		AutoReset,
		ManualReset
	};

	class SafeRecyclableEvent final : public Event
	{
	public:
		Event* InnerEvent;
		SafeRecyclableEvent(Event* inInverEvent)
			:InnerEvent(inInverEvent)
		{

		}

		~SafeRecyclableEvent()
		{
			InnerEvent = nullptr;
		}

		virtual bool create(bool isManualReset = false)
		{
			return InnerEvent->create(isManualReset);
		}

		virtual bool isManualReset() override
		{
			return InnerEvent->isManualReset();
		}

		virtual void trigger()
		{
			InnerEvent->trigger();
		}

		virtual void reset()
		{
			InnerEvent->reset();
		}

		virtual bool wait(uint32 waitTime, const bool bIgnoreThreadIdleStats = false)
		{
			return InnerEvent->wait(waitTime, bIgnoreThreadIdleStats);
		}
	};



	template<EEventPoolTypes PoolType>
	class EventPool
	{
	public:
		CORE_API static EventPool& get()
		{
			static EventPool singleton;
			return singleton;
		}

		Event* getEventFromPool()
		{
			Event * result;
			if (mPool.size() > 0)
			{
				result = mPool.back();
				mPool.pop_back();
			}
			else
			{
				result = PlatformProcess::createSynchEvent((PoolType == EEventPoolTypes::ManualReset));
			}

			return new SafeRecyclableEvent(result);
		}

		void returnToPool(Event* e)
		{
			SafeRecyclableEvent* safeEvent = static_cast<SafeRecyclableEvent*>(e);
			Event* result = safeEvent->InnerEvent;
			delete safeEvent;
			result->reset();
			mPool.push_back(result);
		}

	private:
		EventPool()
		{
			mPool.reserve(PLATFORM_CACHE_LINE_SIZE);
		}

		std::vector<Event*> mPool;
	};
}