#pragma once
#include "CoreType.h"
namespace Air
{
	class IQueuedWork
	{
	public:
		virtual void doThreadedWork() = 0;

		virtual void abandon() = 0;

	public:
		virtual ~IQueuedWork(){}
	};

	typedef IQueuedWork QuquedWork;
}