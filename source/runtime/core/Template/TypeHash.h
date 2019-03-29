#pragma once
#include "CoreType.h"
#include "boost/functional/hash.hpp"
namespace Air
{
	inline size_t pointerHash(const void* key, size_t c = 0)
	{
#if PLATFORM_64BITS
		auto ptrInt = reinterpret_cast<UPTRINT>(key) >> 4;
#else
		auto ptrInt = reinterpret_cast<UPTRINT>(key);
#endif
		boost::hash_combine(ptrInt, c);
		return c;
	}
}