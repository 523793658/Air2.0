#pragma once
#include "CoreType.h"
#include "Delegates/IDelegateInstance.h"
#include <functional>
namespace Air
{
	class Ticker
	{
	public:
		CORE_API static Ticker& getCoreTicker();

		CORE_API void tick(float deltaTime);

		CORE_API DelegateHandle addTicker(const Ticker, float InDelay);
	};
}