#pragma once
#include "boost/assert.hpp"
#include "CoreType.h"
template<typename To, typename From>
inline To check_cast(From p) noexcept
{
	BOOST_ASSERT(dynamic_cast<To>(p) == static_cast<To>(p));
	return static_cast<To>(p);
}

template<unsigned char ch0, unsigned char ch1, unsigned char ch2, unsigned char ch3>
struct MakeFourCC
{
	enum { value = (ch0 << 0) + (ch1 << 8) + (ch2 << 16) + (ch3 << 24) };
};

#define AIR_UNUSED(x) (void)(x)


template<int size>
CORE_API void EndianSwitch(void* p) noexcept;


template<typename T>
T Native2LE(T x) noexcept
{
#ifdef AIR_LITTLE_ENDIAN
	AIR_UNUSED(x);

#else
	EndianSwitch<sizeof(T)>(&x);

#endif
	return x;
}


template<typename T>
T LE2Native(T x) noexcept
{
	return Native2LE(x);
}
