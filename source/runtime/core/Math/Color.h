#pragma once
#include "CoreType.h"
#include "Containers/String.h"
#include "Math.h"
#include "boost/lexical_cast.hpp"
#include <algorithm>
#include <regex>
namespace Air
{
	struct Color;
	class Float16Color;
	struct CORE_API LinearColor
	{
		float R, G, B, A;
	public:
		FORCEINLINE LinearColor(float inR, float inG, float inB, float inA)
			:R(inR)
			,G(inG)
			,B(inB)
			,A(inA)
		{

		}
		FORCEINLINE explicit LinearColor(EForceInit)
			:R(0), G(0), B(0), A(0)
		{}

		LinearColor(const Color& Color);

		explicit LinearColor(const Float16Color& c);

		FORCEINLINE LinearColor()
		{}

		Color toColor(const bool bSRGB) const;

		FORCEINLINE LinearColor operator / (float scalar)  const
		{
			const float invScale = 1.0f / scalar;
			return LinearColor(this->R * invScale, G * invScale, B * invScale, A * invScale);
		}

		FORCEINLINE bool operator != (const LinearColor & rhs) const
		{
			return this->R != rhs.R || G != rhs.G || B != rhs.B || A != rhs.A;
		}

		FORCEINLINE LinearColor& operator*= (const LinearColor& colorB)
		{
			R *= colorB.R;
			G *= colorB.G;
			B *= colorB.B;
			A *= colorB.A;
			return *this;
		}

		FORCEINLINE LinearColor operator *(float scalar) const
		{
			return LinearColor(
				this->R * scalar,
				this->G * scalar,
				this->B * scalar,
				this->A * scalar
			);
		}

		FORCEINLINE LinearColor& operator*= (float scalar)
		{
			R *= scalar;
			G *= scalar;
			B *= scalar;
			A *= scalar;
			return *this;
		}

		FORCEINLINE LinearColor operator*(const LinearColor& other) const
		{
			return LinearColor(
				this->R * other.R,
				this->G * other.G,
				this->B * other.B,
				this->A * other.A
			);
		}

		FORCEINLINE bool operator == (const LinearColor& other) const
		{
			return other.R == R && other.G == this->G && other.B == this->B && other.A == this->A;
		}

		FORCEINLINE LinearColor operator + (const LinearColor& other) const
		{
			return LinearColor(
				this->R + other.R,
				this->G + other.G,
				this->B + other.B,
				this->A + other.A
			);
		}
		FORCEINLINE LinearColor operator - (const LinearColor& other) const
		{
			return LinearColor(
				this->R - other.R,
				this->G - other.G,
				this->B - other.B,
				this->A - other.A
			);
		}

		FORCEINLINE float& component(int32 index)
		{
			return (&R)[index];
		}
		float computeLuminance() const;

		bool isAlmostBlack() const
		{
			return Math::square(R) < DELTA && Math::square(G) < DELTA && Math::square(B) < DELTA;
		}

		static LinearColor fromString(wstring s)
		{
			std::wregex reg(TEXT("^R=(\\d+(.\\d+)?),G=(\\d+(.\\d+)?),B=(\\d+(.\\d+)?),A=(\\d+(.\\d+)?)$"));
			std::wsmatch result;
			if (std::regex_match(s, result, reg))
			{
				return LinearColor(
					boost::lexical_cast<float>(result[1].str()),
					boost::lexical_cast<float>(result[3].str()),
					boost::lexical_cast<float>(result[5].str()),
					boost::lexical_cast<float>(result[7].str())
				);
			}
			else
			{
				return Black;
			}
		}

		static const LinearColor White;
		static const LinearColor Gray;
		static const LinearColor Black;
		static const LinearColor Transparent;
		static const LinearColor Red;
		static const LinearColor Green;
		static const LinearColor Blue;
		static const LinearColor Yellow;

		static float sRGBToLinearTable[256];
	};

	struct Color 
	{
	public:
#if PLATFORM_LITTLE_ENDIAN
#ifdef _MSC_VER
		union 
		{
			struct
			{
				uint8 B, G, R, A;
			};
			uint32 mAlignmentDummy;
		};
#else
		uint8 B GCC_ALIGN(4);
		uint8 G, R, A;
#endif // _MSC_VER
#else
		union 
		{
			struct { uint8 A, R, G, B; };
			uint32 mAlignmentDummy;
		};
#endif
		uint32 & DWColor(void) { return *((uint32*) this); }
		const uint32 & DWColor(void) const { return *((uint32*)this); }
		
		FORCEINLINE Color() {}
		FORCEINLINE explicit Color(EForceInit)
		{
			R = G = B = A = 0;
		}

		FORCEINLINE Color(uint8 inR, uint8 inG, uint8 inB, uint8 inA = 255)
		{
			R = inR;
			G = inG;
			B = inB;
			A = inA;
		}

		FORCEINLINE explicit Color(uint32 inColor)
		{
			DWColor() = inColor;
		}

		FORCEINLINE bool operator == (const Color & rhs) const
		{
			return DWColor() == rhs.DWColor();
		}

		FORCEINLINE bool operator != (const Color & rhs) const
		{
			return DWColor() != rhs.DWColor();
		}


		FORCEINLINE void operator += (const Color & rhs)
		{
			R = (uint8)std::min<int32>((int32)R + (int32)rhs.R, 255);
			G = (uint8)std::min<int32>((int32)G + (int32)rhs.G, 255);
			B = (uint8)std::min<int32>((int32)B + (int32)rhs.B, 255);
			A = (uint8)std::min<int32>((int32)A + (int32)rhs.A, 255);
		}
		
		CORE_API LinearColor fromRGBE() const;

		static CORE_API Color fromHex(const string& hexString);

		static CORE_API Color makeRandomColor();

		static CORE_API const Color White;

		static CORE_API const Color Black;
	};
}