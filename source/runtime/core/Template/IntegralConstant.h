#pragma once
#include "CoreType.h"

template <typename T, T Val>
struct TIntegralConstant
{
	static const T Value = Val;
};