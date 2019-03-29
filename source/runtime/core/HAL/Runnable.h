#pragma once

#include "CoreType.h"

namespace Air
{
	class CORE_API Runnable
	{
	public:
		virtual bool init()
		{
			return true;
		}

		virtual uint32 run() = 0;

		virtual void stop() {}

		virtual void exit(){}

		virtual ~Runnable() {}
	};
}