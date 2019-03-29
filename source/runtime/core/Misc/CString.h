#pragma once
#include "CoreType.h"
#include "HAL/PlatformString.h"
#include <vadefs.h>
#include "Misc/Char.h"
namespace Air
{
	template<typename T>
	struct TCString
	{
		typedef T CharType;
		static FORCEINLINE int32 getVarArgs(CharType* dest, SIZE_T destSize, int32 count, const CharType*& fmt, va_list argprt);

		static FORCEINLINE int32 strlen(const CharType* string);
		
		static FORCEINLINE int32 strcmp(const CharType* string1, const CharType* string2);

		static FORCEINLINE int32 strncmp(const CharType* string1, const CharType* string2, SIZE_T count);

		static FORCEINLINE int32 strnicmp(const CharType* string1, const CharType* string2, SIZE_T count);

		static FORCEINLINE CharType* strcpy(CharType* dest, SIZE_T destCount, const CharType* src);

		static FORCEINLINE CharType* strncpy(CharType* dest, const CharType* src, int32 maxLen);

		template<SIZE_T destCount>
		static FORCEINLINE CharType* strcpy(CharType(&dest)[destCount], const CharType* src)
		{
			return strcpy(dest, destCount, src);
		}

		static FORCEINLINE const CharType* strchr(const CharType* string, CharType c);
		static FORCEINLINE CharType* strchr(CharType* string, CharType c);

		static FORCEINLINE bool isPureAnsi(const CharType* str);

		static FORCEINLINE const CharType* strstr(const CharType* string, const CharType* find);

		static FORCEINLINE CharType* strstr(CharType* string, const CharType* find);

		static const CharType* stristr(const CharType* str, const CharType* find);

		static CharType* stristr(CharType* str, const CharType* find)
		{
			return (CharType*)stristr((const CharType*)str, find);
		}

		static bool isNumeric(const CharType* str)
		{
			if (*str == '-' || *str == '+')
			{
				str++;
			}
			bool bHasDot = false;
			while (*str != '\0')
			{
				if (*str == '.')
				{
					if (bHasDot)
					{
						return false;
					}
					bHasDot = true;
				}
				else if (!Char::isDigit(*str))
				{
					return false;
				}
				++str;
			}
			return true;
		}

		static FORCEINLINE int vsprintf(CharType* const _Buffer, CharType const* const _Format, va_list argPtr);
	};




	typedef TCString<TCHAR> CString;
	typedef TCString<ANSICHAR> CStringAnsi;
	typedef TCString<WIDECHAR> CStringWide;

	template<typename T> FORCEINLINE
		int32 TCString<T>::getVarArgs(CharType* dest, SIZE_T destSize, int32 count, const CharType*& fmt, va_list argprt)
	{
		return PlatformString::getVarArgs(dest, destSize, count, fmt, argprt);
	}

	template<typename T> FORCEINLINE
		int32 TCString<T>::strlen(const CharType* string)
	{
		return PlatformString::strlen(string);
	}

	template<typename T> FORCEINLINE
		int32 TCString<T>::strncmp(const CharType* string1, const CharType* string2, SIZE_T count)
	{
		return PlatformString::strncmp(string1, string2, count);
	}

	template<typename T> FORCEINLINE
		int32 TCString<T>::strnicmp(const CharType* string1, const CharType* string2, SIZE_T count)
	{
		return PlatformString::strnicmp(string1, string2, count);
	}

	template<typename T> FORCEINLINE
		typename TCString<T>::CharType* TCString<T>::strcpy(CharType* dest, SIZE_T destCount, const CharType* src)
	{
		return PlatformString::strcpy(dest, destCount, src);
	}



	template<> FORCEINLINE bool TCString<ANSICHAR>::isPureAnsi(const ANSICHAR* str)
	{
		return true;
	}

	template<> FORCEINLINE bool TCString<WIDECHAR>::isPureAnsi(const WIDECHAR* str)
	{
		for ( ; *str; str++)
		{
			if (*str > 0x7f)
			{
				return false;
			}
		}
		return true;
	}


	template<typename T> FORCEINLINE
		const typename TCString<T>::CharType* TCString<T>::strstr(const CharType* string, const CharType* find)
	{
		return PlatformString::strstr(string, find);
	}


	template<typename T> FORCEINLINE
		typename TCString<T>::CharType* TCString<T>::strstr(CharType* string, const CharType* find)
	{
		return (CharType*)PlatformString::strstr(string, find);
	}

	template<typename T>
	const typename TCString<T>::CharType* TCString<T>::stristr(const CharType* str, const CharType* find)
	{
		if (find == nullptr || str == nullptr)
		{
			return nullptr;
		}
		CharType findInitial = TChar<CharType>::toUpper(*find);
		int32 length = strlen(find++) - 1;
		CharType strChar = *str++;
		while (strChar)
		{
			strChar = TChar<CharType>::toUpper(strChar);
			if (strChar == findInitial && !strnicmp(str, find, length))
			{
				return str - 1;
			}
			strChar = *str++;
		}
		return nullptr;
	}
	template<typename T> FORCEINLINE
		int32 TCString<T>::strcmp(const CharType* string1, const CharType* string2)
	{
		return PlatformString::strcmp(string1, string2);
	}


	template<typename T> FORCEINLINE
		const typename TCString<T>::CharType* TCString<T>::strchr(const CharType* string, CharType c)
	{
		return PlatformString::strchr(string, c);
	}

	template<typename T> FORCEINLINE
		typename TCString<T>::CharType* TCString<T>::strchr(CharType* string, CharType c)
	{
		return (CharType*)PlatformString::strchr(string, c);
	}
	template<typename T> FORCEINLINE 
		int TCString<T>::vsprintf(T* const _Buffer, T const* const _Format, va_list argPtr)
	{
		return PlatformString::vsprintf(_Buffer, _Format, argPtr);
	}
}