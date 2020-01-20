#pragma once
#include "CoreType.h"
#include "HAL/PlatformMath.h"
#include "Math/VectorRegister.h"
#if USE_KLAYGE_MATH
#include "Math/KlaygeMath/KlaygeMath.h"
#else
#endif

#include <algorithm>

namespace Air
{
	struct Math : public PlatformMath
	{
		template <class T>
		static FORCEINLINE T clamp(const T x, const T min, const T max)
		{
			return x < min ? min : x < max ? x : max;
		}

		template<class T>
		static FORCEINLINE T square(const T a)
		{
			return a * a;
		}

		static FORCEINLINE float fastAsin(float value)
		{
			bool nonnegative = (value >= 0.0f);
			float x = Math::abs(value);
			float omx = 1.0f - x;
			if (omx < 0.0f)
			{
				omx = 0.0f;
			}
			float root = Math::sqrt(omx);
			float result = ((((((-0.0012624911f * x + 0.0066700901f) * x - 0.0170881256f) * x + 0.0308918810f) * x - 0.0501743046f) * x + 0.0889789874f) * x - 0.2145988016f) * x + FASTASIN_HALF_PI;
			result *= root;
			return (nonnegative ? FASTASIN_HALF_PI - result : result - FASTASIN_HALF_PI);
		}

		template<class T>
		static FORCEINLINE T divideAndRoundUp(T dividend, T divisor)
		{
			return (dividend + divisor - 1) / divisor;
		}

		template<class T, class U>
		static FORCEINLINE_DEBUGGABLE T lerp(const T& A, const T& B, const U& alpha)
		{
			return (T)(A + alpha*(B - A));
		}

		template<class T>
		static FORCEINLINE auto degreesToRadians(T const& degVal) -> decltype(degVal * (PI / 180.f))
		{
			return degVal * (PI / 180.f);
		}

		static FORCEINLINE void sinCos(float* scalarSin, float* scalarCos, float value)
		{
			float quotient = (INV_PI * 0.5f) * value;
			if (value >= 0.0f)
			{
				quotient = (float)((int)(quotient + 0.5f));
			}
			else
			{
				quotient = (float)((int)(quotient - 0.5f));
			}
			float y = value - (2.0f * PI) * quotient;

			float sign;
			if (y > HALF_PI)
			{
				y = PI - y;
				sign = -1.f;
			}
			else if (y < -HALF_PI)
			{
				y = -PI - y;
				sign = -1.f;
			}
			else
			{
				sign = 1.0f;
			}
			float y2 = y * y;

			*scalarSin = (((((-2.3889859e-08f* y2 + 2.7525562e-06f) * y2 - 0.00019840874) * y2 + 0.0083333310f) * y2 - 0.1666666667f) * y2 + 1.0f) * y;
			float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05) * y2 - 0.0013888378) * y2 + 0.041666638) * y2 - 0.5f) * y2 + 1.0f;

			*scalarCos = sign * p;
		}

		static FORCEINLINE bool isNearlyEqual(float a, float b, float erroTolerance = SMALL_NUMBER)
		{
			return abs<float>(a - b) <= erroTolerance;
		}

		template<typename T>
		static FORCEINLINE bool isPowerOfTwo(T value)
		{
			return ((value & (value - 1)) == (T)0);
		}

		static float CORE_API clampAngle(float angleDegrees, float minAngleDegrees, float maxAngledegress);

		static FORCEINLINE float loge(float value) { return logf(value); }

		static FORCEINLINE float log2(float value)
		{
			static const float LogToLog2 = 1.f / loge(2.f);
			return loge(value) * LogToLog2;
		}
	};

	
}

