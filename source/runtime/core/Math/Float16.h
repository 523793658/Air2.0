#pragma once
#include "CoreType.h"
#include "Serialization/Archive.h"
#include "Math/Math.h"
#include "Float32.h"
namespace Air
{
	class float16
	{
	public:
		union
		{
			struct
			{
#if PLATFORM_LITTLE_ENDIAN
				uint16 mMantissa : 10;
				uint16 mExponent : 5;
				uint16 mSign : 1;
#else
				uint16 mSign : 1;
				uint16 mExponent : 5;
				uint16 mMantissa : 10;
#endif
			}mComponents;
			uint16 mEncoded;

		};
		float16();
		float16(const float16& p16Value);
		float16(float p32Value);
		float16& operator =(float p32Value);
		float16& operator=(const float16& p16Value);
		operator float() const;

		void set(float p32Value);

		void setWithoutBoundsCheches(const float p32Value);

		float getFloat() const;

		friend Archive& operator <<(Archive& ar, float16& v)
		{
			return ar << v.mEncoded;
		}
	};

	FORCEINLINE float16::float16()
		:mEncoded(0)
	{}

	FORCEINLINE float16::float16(const float16& p16Value)
	{
		mEncoded = p16Value.mEncoded;
	}

	FORCEINLINE float16::float16(float p32Value)
	{
		set(p32Value);
	}

	FORCEINLINE float16& float16::operator =(float p32Value)
	{
		set(p32Value);
		return *this;
	}

	FORCEINLINE float16& float16::operator=(const float16& p16Value)
	{
		mEncoded = p16Value.mEncoded;
		return *this;
	}

	FORCEINLINE float16::operator float() const
	{
		return getFloat();
	}

	FORCEINLINE void float16::set(float p32Value)
	{
		float32 p32(p32Value);
		mComponents.mSign = p32.mComponents.mSign;
		if (p32.mComponents.mExponent <= 112)
		{
			mComponents.mExponent = 0;
			mComponents.mMantissa = 0;
		}
		else if (p32.mComponents.mExponent >= 143)
		{
			mComponents.mExponent = 30;
			mComponents.mMantissa = 1023;
		}
		else
		{
			mComponents.mExponent = int32(p32.mComponents.mExponent) - 127 + 15;
			mComponents.mMantissa = uint16(p32.mComponents.mMantissa >> 13);
		}
	}

	FORCEINLINE void float16::setWithoutBoundsCheches(const float p32Value)
	{
		const float32 p32(p32Value);
		mComponents.mMantissa = uint16(p32.mComponents.mMantissa >> 13);
		mComponents.mSign = p32.mComponents.mSign;
		mComponents.mExponent = int32(p32.mComponents.mExponent) - 127 + 15;
	}

	FORCEINLINE float float16::getFloat() const
	{
		float32 result;
		result.mComponents.mSign = mComponents.mSign;
		if (mComponents.mExponent == 0)
		{
			uint32 mantissa = mComponents.mMantissa;
			if (mantissa == 0)
			{
				result.mComponents.mExponent = 0;
				result.mComponents.mMantissa = 0;
			}
			else
			{
				uint32 matissaShift = 10 - (uint32)Math::truncToInt(Math::log2(mantissa));
				result.mComponents.mExponent = 127 - (15 - 1) - matissaShift;
				result.mComponents.mMantissa = mantissa << (matissaShift + 23 - 10);
			}
		}
		else if (mComponents.mExponent == 31)
		{
			result.mComponents.mExponent = 142;
			result.mComponents.mMantissa = 8380416;
		}
		else
		{
			result.mComponents.mExponent = int32(mComponents.mExponent) - 15 + 127;
			result.mComponents.mMantissa = uint32(mComponents.mMantissa) << 13;
		}
		return result.mFloatValue;
	}
}