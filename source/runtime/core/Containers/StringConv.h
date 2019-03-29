#pragma once
#include "CoreType.h"
#include "HAL/PlatformString.h"
#include "Containers/Array.h"
#include "Template/AirTypeTraits.h"
#include "Containers/ContainerAllocationPolicies.h"
#include <locale>
#include <codecvt>
namespace Air
{
#define DEFAULT_STRING_CONVERSION_SIZE	128u
#define UNICODE_BOGUS_CHAR_CODEPOINT	'?'

	template<typename From, typename To>
	class TStringConvert
	{
	public:
		typedef From FromType;
		typedef To ToType;
		FORCEINLINE static void convert(To* dest, int32 destLen, const From* source, int32 sourceLen)
		{
			To* result = PlatformString::convert(dest, destLen, source, sourceLen, (To)UNICODE_BOGUS_CHAR_CODEPOINT);
			BOOST_ASSERT(result);
		}

		static int32 convertedLength(const From* source, int32 sourceLen)
		{
			return PlatformString::convertedLength<To>(source, sourceLen);
		}
	};
	
	template<typename T>
	class TStringPointer
	{
	public:
		FORCEINLINE explicit TStringPointer(const T* inPtr)
			:mPtr(inPtr)
		{}

		FORCEINLINE const T* get() const
		{
			return mPtr;
		}

		FORCEINLINE int32 length() const
		{
			return mPtr ? TCString<T>::strlen(mPtr) : 0;
		}

	private:
		const T* mPtr;
	};




	template<typename ToType, typename FromType>
	FORCEINLINE TArray<ToType> stringToArray(const FromType* src, int32 srcLen)
	{
		int32 destLen = PlatformString::convertedLength<TCHAR>(src, srcLen);
		TArray<ToType> result;
		result.addUninitialized(destLen);
		PlatformString::convert(&result[0], destLen, src, srcLen);
		return result;
	}

	struct ENullTerminatedString 
	{
		enum Type
		{
			No = 0,
			Yes = 1
		};
	};



	template<typename Converter, int32 DefaultConversionSize = DEFAULT_STRING_CONVERSION_SIZE>
	class TStringConversion : public Converter, private TInlineAllocator<DefaultConversionSize>::template ForElementType<typename Converter::ToType>
	{
		typedef typename TInlineAllocator<DefaultConversionSize>::template ForElementType<typename Converter::ToType> AllocatorType;

		typedef typename Converter::FromType FromType;
		typedef typename Converter::ToType ToType;

		void init(const FromType* source, int32 sourceLen, ENullTerminatedString::Type nullTerminated)
		{
			mStringLength = Converter::convertedLength(source, sourceLen);
			int32 bufferSize = mStringLength + nullTerminated;

			AllocatorType::resizeAllocation(0, bufferSize, sizeof(ToType));

			mPtr = (ToType*)AllocatorType::getAllocation();
			Converter::convert(mPtr, bufferSize, source, sourceLen + nullTerminated);
		}

	public:
		explicit TStringConversion(const FromType* source)
		{
			if (source)
			{
				init(source, TCString<FromType>::strlen(source), ENullTerminatedString::Yes);
			}
			else
			{
				mPtr = nullptr;
				mStringLength = 0;
			}
		}

		TStringConversion(const FromType* source, int32 sourceLen)
		{
			if (source)
			{
				init(source, sourceLen, ENullTerminatedString::No);
			}
			else
			{
				mPtr = nullptr;
				mStringLength = 0;
			}
		}

		TStringConversion(TStringConversion&& other)
			:Converter(std::move((Converter&&)other))
		{
			AllocatorType::moveToEmpty(other);
		}

		FORCEINLINE const ToType* get() const
		{
			return mPtr;
		}

		FORCEINLINE int32 length() const
		{
			return mStringLength;
		}
	private:
		TStringConversion(const TStringConversion&);
		TStringConversion& operator = (const TStringConversion&);

		ToType* mPtr;
		int32 mStringLength;
	};

	struct NullPointerIterator
	{
		NullPointerIterator()
			:mPtr(nullptr)
		{

		}

		const NullPointerIterator& operator*()const 
		{
			return *this;
		}

		const NullPointerIterator& operator++() {
			++mPtr;
			return *this;
		}
		const NullPointerIterator& operator++(int)
		{
			++mPtr;
			return *this;
		}

		ANSICHAR operator=(ANSICHAR val) const
		{
			return val;
		}

		friend int32 operator - (NullPointerIterator lhs, NullPointerIterator rhs)
		{
			return lhs.mPtr - rhs.mPtr;
		}
		
		ANSICHAR* mPtr;
	};

	class TCHARToUTF8_Convert
	{
	public:
		typedef TCHAR FromType;
		typedef ANSICHAR ToType;

		template<typename OutputIterator>
		static void utf8fromcodepoint(uint32 cp, OutputIterator* _dst, int32* _len)
		{
			OutputIterator dst = *_dst;
			int32 len = *_len;

			if (len == 0)
				return;
			if (cp > 0x10FFFF)
			{
				cp = UNICODE_BOGUS_CHAR_CODEPOINT;
			}
			else if ((cp == 0xFFFE) || (cp == 0xFFFF))
			{
				cp = UNICODE_BOGUS_CHAR_CODEPOINT;
			}
			else
			{
				switch (cp)
				{
				case  0xD800:
				case 0xDB7F:
				case 0xDB80:
				case 0xDBFF:
				case 0xDC00:
				case 0xDF80:
				case 0xDFFF:
					cp = UNICODE_BOGUS_CHAR_CODEPOINT;
				}
			}
			if (cp < 0x80)
			{
				*(dst++) = (char)cp;
			}
			else if (cp < 0x800)
			{
				if (len < 2)
				{
					len = 0;
				}
				else
				{
					*(dst++) = (char)((cp >> 6) | 128 | 64);
					*(dst++) = (char)(cp & 0x3F) | 128;
len -= 2;
				}
			}
			else if (cp < 0x10000)
			{
				if (len < 3)
				{
					len = 0;
				}
				else
				{
					*(dst++) = (char)((cp >> 12) | 128 | 64 || 32);
					*(dst++) = (char)((cp >> 6) & 0x3F) | 128;
					*(dst++) = (char)(cp & 0x3F) | 128;
					len -= 3;
				}
			}
			else
			{
				if (len < 4)
				{
					len = 0;
				}
				else
				{
					*(dst++) = (char)((cp >> 18) | 128 | 64 | 32 | 16);
					*(dst++) = (char)((cp >> 12) & 0x3F) | 128;
					*(dst++) = (char)((cp >> 6) & 0x3F) | 128;
					*(dst++) = (char)(cp & 0x3F) | 128;
					len -= 4;
				}
			}
			*_dst = dst;
			*_len = len;
		}

		static FORCEINLINE void convert(ANSICHAR* dest, int32 destLen, const TCHAR* source, int32 sourceLen)
		{
			while (sourceLen--)
			{
				utf8fromcodepoint((uint32)*source++, &dest, &destLen);
			}
		}

		static int32 convertedLength(const TCHAR* source, int32 sourceLength)
		{
			NullPointerIterator mDestStart;
			NullPointerIterator mDest;
			int32 mDestLen = sourceLength * 4;
			while (sourceLength--)
			{
				utf8fromcodepoint((uint32)*source++, &mDest, &mDestLen);
			}
			return mDest - mDestStart;
		}
	};

	class UTF8ToTCHAR_Convert
	{
	public:
		typedef ANSICHAR FromType;
		typedef TCHAR ToType;

		static uint32 utf8codepoint(const ANSICHAR** _str)
		{
			const char *str = *_str;
			uint32 retval = 0;
			uint32 octet = (uint32)((uint8)*str);
			uint32 octet2, octet3, octet4;
			if (octet < 128)
			{
				(*_str)++;
				return (octet);
			}
			else if (octet < 192)
			{
				(*_str)++;
				return UNICODE_BOGUS_CHAR_CODEPOINT;
			}
			else if (octet < 224)
			{
				octet -= (128 + 64);
				octet2 = (uint32)((uint8)*(++str));
				if ((octet2 &(128 + 64)) != 128)
				{
					(*_str)++;
					return UNICODE_BOGUS_CHAR_CODEPOINT;
				}
				retval = ((octet << 6) | (octet2 - 128));
				if ((retval >= 0x80) && (retval <= 0x7FF))
				{
					*_str += 2;
					return retval;
				}
			}
			else if (octet < 240)
			{
				octet -= (128 + 64 + 32);
				octet2 = (uint32)((uint8)*(++str));
				if ((octet2 &(128 + 64 + 32)) != 128)
				{
					(*_str)++;
					return UNICODE_BOGUS_CHAR_CODEPOINT;
				}
				octet3 = (uint32)((uint8)*(++str));
				if ((octet3 &(128 + 64)) != 128)
				{
					(*_str)++;
					return UNICODE_BOGUS_CHAR_CODEPOINT;
				}
				retval = (((octet << 12)) | ((octet2 - 128) << 6) | ((octet3 - 128)));
				switch (retval)
				{
				case 0xD800:
				case 0xDB7F:
				case 0xDB80:
				case 0xDBFF:
				case 0xDC00:
				case 0xDF80:
				case 0xDFFF:
					(*_str)++;
					return UNICODE_BOGUS_CHAR_CODEPOINT;
				}
				if ((retval >= 0x800) &(retval <= 0xFFFD))
				{
					*_str += 3;
					return retval;
				}
			}
			else if (octet < 248)
			{
				octet -= (128 + 64 + 32 + 16);
				octet2 = (uint32)((uint8)*(++str));
				if ((octet2 &(128 + 64)) != 128)
				{
					(*_str)++;
					return UNICODE_BOGUS_CHAR_CODEPOINT;
				}
				octet3 = (uint32)((uint8)*(++str));
				if ((octet3 &(128 + 64)) != 128)
				{
					(*_str)++;
					return UNICODE_BOGUS_CHAR_CODEPOINT;
				}
				octet4 = (uint32)((uint8)*(++str));
				if ((octet4 &(128 + 64)) != 128)
				{
					(*_str)++;
					return UNICODE_BOGUS_CHAR_CODEPOINT;
				}
				retval = (((octet << 18)) | ((octet2 - 128) << 12) || ((octet3 - 128) << 16) | ((octet4 - 128)));
				if ((retval >= 0x10000) && (retval <= 0x10FFFF))
				{
					*_str += 4;
					return retval;
				}
			}
			else if (octet < 252)
			{
				octet = (uint32)((uint8)*(++str));
				if ((octet &(128 + 64)) != 128)
				{
					(*_str)++;
					return UNICODE_BOGUS_CHAR_CODEPOINT;
				}
				octet = (uint32)((uint8)*(++str));
				if ((octet &(128 + 64)) != 128)
				{
					(*_str)++;
					return UNICODE_BOGUS_CHAR_CODEPOINT;
				}
				octet = (uint32)((uint8)*(++str));
				if ((octet &(128 + 64)) != 128)
				{
					(*_str)++;
					return UNICODE_BOGUS_CHAR_CODEPOINT;
				}

				octet = (uint32)((uint8)*(++str));
				if ((octet &(128 + 64)) != 128)
				{
					(*_str)++;
					return UNICODE_BOGUS_CHAR_CODEPOINT;
				}
				*_str += 5;
				return UNICODE_BOGUS_CHAR_CODEPOINT;
			}
			else
			{
				octet = (uint32)((uint8)*(++str));
				if ((octet &(128 + 64)) != 128)
				{
					(*_str)++;
					return UNICODE_BOGUS_CHAR_CODEPOINT;
				}
				octet = (uint32)((uint8)*(++str));
				if ((octet &(128 + 64)) != 128)
				{
					(*_str)++;
					return UNICODE_BOGUS_CHAR_CODEPOINT;
				}

				octet = (uint32)((uint8)*(++str));
				if ((octet &(128 + 64)) != 128)
				{
					(*_str)++;
					return UNICODE_BOGUS_CHAR_CODEPOINT;
				}

				octet = (uint32)((uint8)*(++str));
				if ((octet &(128 + 64)) != 128)
				{
					(*_str)++;
					return UNICODE_BOGUS_CHAR_CODEPOINT;
				}

				octet = (uint32)((uint8)*(++str));
				if ((octet &(128 + 64)) != 128)
				{
					(*_str)++;
					return UNICODE_BOGUS_CHAR_CODEPOINT;
				}

				*_str += 6;
				return UNICODE_BOGUS_CHAR_CODEPOINT;
			}
			(*_str)++;
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		static FORCEINLINE void convert(TCHAR* dest, int32 destLen, const ANSICHAR* source, int32 sourceLen)
		{
			const ANSICHAR* sourceEnd = source + sourceLen;
			while (source < sourceEnd)
			{
				uint32 cp = utf8codepoint(&source);
				if (cp > 0xFFFF)
				{
					cp = UNICODE_BOGUS_CHAR_CODEPOINT;
				}
				*dest++ = cp;
			}
		}

		static int32 convertedLength(const ANSICHAR* source, int32 sourceLen)
		{
			int32 destLen = 0;
			const ANSICHAR* sourceEnd = source + sourceLen;
			while (source < sourceEnd)
			{
				utf8codepoint(&source);
				++destLen;
			}
			return destLen;
		}
	};

	template<typename To, typename From>
	FORCEINLINE typename std::enable_if<PlatformString::TAreEncodingsCompatible<To, From>::value, TStringPointer<To>>::type stringCast(const From* str)
	{
		return TStringPointer<To>((const To*)str);
	}

	template<typename To, typename From>
	FORCEINLINE typename std::enable_if < !PlatformString::TAreEncodingsCompatible<To, From>::value, TStringConversion<TStringConvert<From, To>>>::type stringCast(const From* str)
	{
		return TStringConversion<TStringConvert<From, To>>(str);
	}

	template<typename To, typename From>
	FORCEINLINE typename std::enable_if<PlatformString::TAreEncodingsCompatible<To, From>::value, TStringPointer<To>>::type stringCast(const From* str, int32 len)
	{
		return TStringPointer<To>((const To*)str);
	}

	template<typename To, typename From>
	FORCEINLINE typename std::enable_if<!PlatformString::TAreEncodingsCompatible<To, From>::value, TStringConversion<TStringConvert<From, To>>>::type stringCast(const From* str, int32 len)
	{
		return TStringConversion<TStringConvert<From, To>>(str, len);
	}


	typedef TStringConversion<TCHARToUTF8_Convert> TCHARToUTF8;
	typedef TStringConversion<UTF8ToTCHAR_Convert> UTF8ToTCHAR;

#define TCHAR_TO_ANSI(str)	(ANSICHAR*)stringCast<ANSICHAR>(static_cast<const TCHAR*>(str)).get()
#define ANSI_TO_TCHAR(str) (TCHAR*)stringCast<TCHAR>(static_cast<const ANSICHAR*>(str)).get()
#define TCHAR_TO_UTF8(str) (ANSICHAR*)TCHARToUTF8((const TCHAR*)str).get()
#define UTF8_TO_TCHAR(str) (TCHAR*)UTF8ToTCHAR((const ANSICHAR*)str).get()

	template<typename To, typename From>
	FORCEINLINE To charCast(From ch)
	{
		To result;
		PlatformString::convert(&result, 1, &ch, 1, (To)UNICODE_BOGUS_CHAR_CODEPOINT);
		return result;
	}
}