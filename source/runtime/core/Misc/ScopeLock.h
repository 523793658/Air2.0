#pragma once
#include "CoreType.h"
#include "HAL/CriticalSection.h"
#include "boost/assert.hpp"
namespace Air
{
	class ScopeLock
	{
	public:
		ScopeLock(CriticalSection* inSynchObject)
			:mSynchObject(inSynchObject)
		{
			BOOST_ASSERT(mSynchObject);
			mSynchObject->lock();
		}

		~ScopeLock()
		{
			BOOST_ASSERT(mSynchObject);
			mSynchObject->unlock();
		}

	private:
		ScopeLock();

		ScopeLock(const ScopeLock& inScopedLock);

		ScopeLock& operator = (ScopeLock& inScopeLock)
		{
			return *this;
		}

	private:
		CriticalSection* mSynchObject;
	};
}