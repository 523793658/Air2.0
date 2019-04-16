#pragma once
#include "CoreType.h"
namespace Air
{
	class float32
	{
	public:
		union
		{
			struct 
			{
#if PLATFORM_LITTLE_ENDIAN
				uint32 mMantissa : 23;
				uint32 mExponent : 8;
				uint32 mSign : 1;
#else
				uint32 mSign : 1;
				uint32 mExponent : 8;
				uint32 mMantissa : 23;
#endif
			}mComponents;
			float mFloatValue;

		};

		float32(float inValue = 0.0f);
	};

	FORCEINLINE float32::float32(float inValue /* = 0.0f */)
		:mFloatValue(inValue)
	{

	}
}