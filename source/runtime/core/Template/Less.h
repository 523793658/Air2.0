#pragma once
#include "CoreType.h"
namespace Air
{

	template<typename T = void>
	struct TLess
	{
		FORCEINLINE bool operator()(const T& A, const T& B) const
		{
			return A < B;
		}
	};

	template<>
	struct TLess<void>
	{
		template<typename T>
		FORCEINLINE bool operator()(const T& A, const T& B)const
		{
			return A < B;
		}
	};
}