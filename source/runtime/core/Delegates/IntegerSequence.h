#pragma once
#include "CoreType.h"

	template<typename T, T... Indices>
	struct TIntegerSequence
	{

	};

#ifdef _MSC_VER
	template<typename T, T N>
	using TMakeIntegerSequence = __make_integer_seq<TIntegerSequence, T, N>;

#elif __has_builtin(__make_integer_seq)
	template <typename T, T N>
	using TMakeIntegerSequence = __make_integer_seq<TIntegerSequence, T, N > ;
#else


#endif
