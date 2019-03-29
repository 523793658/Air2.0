#pragma once
#include "CoreType.h"
#include "Math/Math.h"
namespace Air
{
	class ScalarRegister
	{
	public:
		VectorRegister mValue;

		explicit FORCEINLINE ScalarRegister(VectorRegister vectorValue);

		explicit FORCEINLINE ScalarRegister(const float& scalarValue);
	};

	FORCEINLINE ScalarRegister::ScalarRegister(VectorRegister vectorValue)
	{
		mValue = vectorValue;
	}

	FORCEINLINE ScalarRegister::ScalarRegister(const float& scalarValue)
	{
		mValue = VectorLoadFloat1(&scalarValue);
	}
}