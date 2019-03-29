#pragma once
#include "CoreType.h"
#include <string>
#include "Template/AirTypeTraits.h"
#include "HAL/AirMemory.h"
namespace Air
{

	struct GenericPlatformString 
	{
	public:
		template<bool Dummy, typename T>
		struct TIsFixedWidthEncoding_Helper
		{
			enum { value = false };
		};
		template<bool Dummy> struct TIsFixedWidthEncoding_Helper<Dummy, ANSICHAR>
		{
			enum { value = true };
		};

		template<bool Dummy> struct TIsFixedWidthEncoding_Helper<Dummy, WIDECHAR>
		{
			enum { value = true };
		};

		template<bool Dummy> struct TIsFixedWidthEncoding_Helper<Dummy, UCS2CHAR>
		{
			enum { value = true };
		};


		template<typename T>
		struct TIsFixedWidthEncoding : TIsFixedWidthEncoding_Helper<false, T>
		{

		};

		template <typename Encoding>
		static bool isValidChar(Encoding ch)
		{
			return true;
		}


		template<typename DestEncoding, typename SourceEncoding>
		static bool canConvertChar(SourceEncoding ch)
		{
			return isValidChar(ch) && (SourceEncoding)(DestEncoding)ch == ch && isValidChar((DestEncoding)ch);
		}


		template<typename EncodingA, typename EncodingB>
		struct TAreEncodingsCompatible
		{
			enum {
				value = TIsFixedWidthEncoding
			<EncodingA>::value && TIsFixedWidthEncoding<EncodingB>::value && sizeof(EncodingA) == sizeof(EncodingB)};
		};



		template <typename DestEncoding, typename SourceEncoding>
		static typename std::enable_if<TIsFixedWidthEncoding<SourceEncoding>::value && TIsFixedWidthEncoding<DestEncoding>::value, int32>::type convertedLength(const SourceEncoding* src, int32 SrcSize)
		{
			return SrcSize;
		}


		template<typename SourceEncoding, typename DestEncoding>
		static FORCEINLINE typename std::enable_if<TAreEncodingsCompatible<SourceEncoding, DestEncoding>::value, DestEncoding*>::type convert(DestEncoding* dest, int32 destSize, const  SourceEncoding* src, int32 srcSize, DestEncoding bogusChar = (DestEncoding)'?')
		{
			if (destSize < srcSize)
			{
				return nullptr;
			}
			return (DestEncoding*)Memory::memcpy(dest, src, srcSize * sizeof(SourceEncoding)) + srcSize;
		}

		template<typename SourceEncoding, typename DestEncoding>
		static typename std::enable_if<!TAreEncodingsCompatible<SourceEncoding, DestEncoding>::value && TIsFixedWidthEncoding<SourceEncoding>::value, DestEncoding*>::type convert(DestEncoding* dest, int32 destSize, const SourceEncoding* src, int32 srcSize, DestEncoding bogusChar = (DestEncoding)'?')
		{
			const SourceEncoding* inSrc = src;
			int32 InSrcSize = srcSize;
			bool bInvalidChars = false;
			while (srcSize)
			{
				if (!destSize)
				{
					return nullptr;
				}
				SourceEncoding srcCh = *src++;
				if (canConvertChar<DestEncoding>(srcCh))
				{
					*dest++ = (DestEncoding)srcCh;
				}
				else
				{
					*dest++ = bogusChar;
					bInvalidChars = true;
				}
				--srcSize;
				--destSize;
			}
			if (bInvalidChars)
			{

			}
			return dest;
		}

	};

	template<>
	inline bool GenericPlatformString::isValidChar<ANSICHAR>(ANSICHAR ch)
	{
		return ch >= 0x00 && ch <= 0x7f;
	}
}