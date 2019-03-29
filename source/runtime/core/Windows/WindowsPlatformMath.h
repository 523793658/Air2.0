#pragma once
#include "CoreType.h"
#include "GenericPlatform/GenericPlatformMath.h"
#if PLATFORM_WINDOWS
#include <intrin.h>
#endif
#include "Math/PlatformMathSSE.h"
namespace Air
{
	struct WindowsPlatformMath : public GenericPlatformMath
	{
#if PLATFORM_ENABLE_VECTORINTRINSICS

		static FORCEINLINE int32 truncToInt(float F)
		{
			return _mm_cvtt_ss2si(_mm_set_ss(F));
		}



		static FORCEINLINE float InvSqrt(float f)
		{
			return PlatformMathSSE::InvSqrt(f);
		}

#pragma intrinsic(_BitScanReverse)
		static FORCEINLINE uint32 floorLog2(uint32 value)
		{
			unsigned long log2;
			if (_BitScanReverse(&log2, value) != 0)
			{
				return log2;
			}
			return 0;
		}

		static FORCEINLINE uint32 countTrainlingZeros(uint32 value)
		{
			if (value == 0)
			{
				return 32;
			}
			unsigned long bitIndex;
			_BitScanForward(&bitIndex, value);
			return bitIndex;
		}

		static FORCEINLINE int32 floorToInt(float f)
		{
			return _mm_cvt_ss2si(_mm_set_ss(f + f - 0.5f)) >> 1;
		}

		static FORCEINLINE bool isNan(float A) { return _isnan(A) != 0; }
		static FORCEINLINE bool isFinite(float A) { return _finite(A) != 0; }

		static FORCEINLINE uint32 countLeadingZeros(uint32 value)
		{
			unsigned long log2;
			if (_BitScanReverse(&log2, value) != 0)
			{
				return 31 - log2;
			}
			return 32;
		}

		static FORCEINLINE uint32 ceilLogTwo(uint32 arg)
		{
			int32 bitmask = ((int32)(countLeadingZeros(arg) << 26)) >> 31;
			return (32 - countLeadingZeros(arg - 1)) & (~bitmask);
		}

		static FORCEINLINE uint32 roundUpToPowerOfTwo(uint32 arg)
		{
			return 1 << ceilLogTwo(arg);
		}

#endif
	};

	typedef WindowsPlatformMath PlatformMath;
}