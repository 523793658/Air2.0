#pragma once
#include "CoreType.h"
using namespace Air;
template<typename T>
inline CONSTEXPR T align(const T ptr, int32 alignment)
{
	return (T)(((PTRINT)ptr + alignment - 1) & ~(alignment - 1));
}

template <class T> inline T alignArbitrary(const T Ptr, uint32 aligment)
{
	return (T)((((UPTRINT)Ptr + aligment - 1) / aligment) * aligment);
}