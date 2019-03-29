#pragma once
#include "CoreType.h"
namespace Air
{
	template<typename From, typename To>
	struct TPointerIsConvertibleFromTo
	{
	private:
		static uint8	test(...);
		static uint16	test(To*);
	public:
		enum { Value = sizeof(test((From*) nullptr)) - 1 };
	};
}