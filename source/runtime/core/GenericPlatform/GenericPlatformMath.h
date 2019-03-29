#pragma once
#include "CoreType.h"
#include "HAL/PlatformCrt.h"
#include "Math/MathUtility.h"
namespace Air
{
	struct GenericPlatformMath
	{
		static FORCEINLINE float InvSqrt(float f)
		{
			return 1.0f / sqrtf(f);
		}

		static FORCEINLINE float sin(float value) { return sinf(value); }

		static FORCEINLINE float cos(float value) { return cosf(value); }

		static FORCEINLINE float tan(float value) { return tanf(value); }

		static FORCEINLINE float atan2(float Y, float X);

		static FORCEINLINE bool isNegativeFloat(const float& a)
		{
			return ((*(uint32*)&a) >= (uint32)0x80000000);
		}
		static FORCEINLINE bool isNegativeDouble(const double& a)
		{
			return ((*(uint64*)&a) >= (uint64)0x8000000000000000);
		}

		static FORCEINLINE float fmod(float x, float y)
		{
			if (fabsf(y) <= 1.e-8f)
			{
				return 0.f;
			}
			const float quotient = std::truncf(x / y);
			float intPortion = y * quotient;
			if (fabsf(intPortion) > fabsf(x))
			{
				intPortion = x;
			}
			const float result = x - intPortion;
			return result;
		}



		template<class T>
		static CONSTEXPR FORCEINLINE T abs(const T A)
		{
			return (A >= (T)0) ? A : -A;
		}

		static FORCEINLINE float sqrt(float value) { return sqrtf(value); }


		static CONSTEXPR FORCEINLINE float floatSelect(float comparand, float valueGEZero, float valueLTZero)
		{
			return comparand >= 0.0f ? valueGEZero : valueLTZero;
		}

		template<class T>
		static CONSTEXPR FORCEINLINE T min(const T A, const T B)
		{
			return (A <= B) ? A : B;
		}

		template<class T>
		static CONSTEXPR FORCEINLINE T max(const T A, const T B)
		{
			return (B <= A) ? A : B;
		}

		template<typename T>
		static CONSTEXPR FORCEINLINE T sign(const T A)
		{
			return (A > (T)0) ? (T)1 : ((A < (T)0) ? (T)-1 : (T)0);
		}

		static FORCEINLINE float pow(float A, float B) { return powf(A, B); }

		static FORCEINLINE uint32 floorLog2(uint32 value)
		{
			uint32 pos = 0;
			if (value >= 1 << 16) { value >>= 16; pos += 16; }
			if (value >= 1 << 8) { value >>= 8; pos += 8; }
			if (value >= 1 << 4) { value >>= 4; pos += 4; }
			if (value >= 1 << 2) { value >>= 2; pos += 2; }
			if (value >= 1 << 1) { pos += 1; }
			return (value == 0) ? 0 : pos;
		}


		static FORCEINLINE uint32 countLeadingZeros(uint32 value)
		{
			if (value == 0) return 32;
			return 31 - floorLog2(value);
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

		static FORCEINLINE int32 rand() { return ::rand(); }

	};

	template<> 
	FORCEINLINE float GenericPlatformMath::abs(const float a)
	{
		return fabsf(a);
	}

	FORCEINLINE float GenericPlatformMath::atan2(float Y, float X)
	{
		const float absX = GenericPlatformMath::abs(X);
		const float absY = GenericPlatformMath::abs(Y);
		const bool yAbsBigger = (absY > absX);
		float t0 = yAbsBigger ? absY : absX; // Max(absY, absX)
		float t1 = yAbsBigger ? absX : absY; // Min(absX, absY)

		if (t0 == 0.f)
			return 0.f;

		float t3 = t1 / t0;
		float t4 = t3 * t3;

		static const float c[7] = {
			+7.2128853633444123e-03f,
			-3.5059680836411644e-02f,
			+8.1675882859940430e-02f,
			-1.3374657325451267e-01f,
			+1.9856563505717162e-01f,
			-3.3324998579202170e-01f,
			+1.0f
		};

		t0 = c[0];
		t0 = t0 * t4 + c[1];
		t0 = t0 * t4 + c[2];
		t0 = t0 * t4 + c[3];
		t0 = t0 * t4 + c[4];
		t0 = t0 * t4 + c[5];
		t0 = t0 * t4 + c[6];
		t3 = t0 * t3;

		t3 = yAbsBigger ? (0.5f * PI) - t3 : t3;
		t3 = (X < 0.0f) ? PI - t3 : t3;
		t3 = (Y < 0.0f) ? -t3 : t3;

		return t3;
	}

	
}