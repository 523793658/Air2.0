#pragma once
#include "CoreType.h"
namespace Air
{
	class NoopCounter
	{
	public:
		NoopCounter() {};
		NoopCounter(const NoopCounter& other) {}
		NoopCounter(int32 value) {}
		int32 increment()
		{
			return 0;
		}

		int32 add(int32 amount)
		{
			return 0;
		}

		int32 decrement()
		{
			return 0;
		}

		int32 subtract(int32 amount)
		{
			return 0;
		}

		int32 set(int32 value)
		{
			return 0;
		}

		int32 reset()
		{
			return 0;
		}

		int32 getValue()
		{
			return 0;
		}
	};
}