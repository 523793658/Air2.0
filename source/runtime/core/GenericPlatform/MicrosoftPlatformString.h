#pragma once
#include "GenericPlatform/GenericPlatformString.h"
#include <tchar.h>
#include <stdio.h>
#include <stdarg.h>
#if !USE_SECURE_CRT
#pragma warning(push)
#pragma warning(disable : 4996)
#pragma warning(disable : 4995)
#endif


namespace Air
{
	struct MicrosoftPlatformString : public GenericPlatformString
	{
		static FORCEINLINE int32 getVarArgs(WIDECHAR* dest, SIZE_T destSize, int32 count, const WIDECHAR*& fmt, va_list argPtr)
		{
#if USE_SECURE_CRT
			int32 result = _vsntprintf_s(dest, destSize, count, fmt, argPtr);
#else
			int32 result = _vsntprintf(dest, count, fmt, argPtr);
#endif
			va_end(argPtr);
			return result;
		}

		static FORCEINLINE int32 getVarArgs(ANSICHAR* dest, SIZE_T destSize, int32 count, const ANSICHAR*& fmt, va_list argPtr)
		{
#if USE_SECURE_CRT
			int32 result = _vsnprintf_s(dest, destSize, count, fmt, argPtr);
#else
			int32 result = _vsnprintf(dest, count, fmt, argPtr);
#endif
			va_end(argPtr);
			return result;
		}

		static FORCEINLINE int vsprintf(WIDECHAR* const _Buffer, WIDECHAR const* const _Format, va_list argPtr)
		{
			return std::vswprintf(_Buffer, _Format, argPtr);
		}

		static FORCEINLINE int vsprintf(ANSICHAR* const _Buffer, ANSICHAR const * const _Format, va_list argPtr)
		{
			return std::vsprintf(_Buffer, _Format, argPtr);
		}

		static FORCEINLINE int32 strlen(const WIDECHAR* string)
		{
			return _tcslen(string);
		}

		static FORCEINLINE int32 strlen(const ANSICHAR* string)
		{
			return ::strlen(string);
		}

		static FORCEINLINE int32 strlen(const UCS2CHAR* string)
		{
			return _tcslen((const WIDECHAR*)string);
		}
		static FORCEINLINE int32 strncmp(const WIDECHAR* string1, const WIDECHAR* string2, SIZE_T count)
		{
			return _tcsncmp(string1, string2, count);
		}

		static FORCEINLINE int32 strncmp(const ANSICHAR* string1, const ANSICHAR* string2, SIZE_T count)
		{
			return ::strncmp(string1, string2, count);
		}

		static FORCEINLINE int32 strnicmp(const WIDECHAR* string1, const WIDECHAR* strint2, SIZE_T count)
		{
			return _tcsnicmp(string1, strint2, count);
		}
		static FORCEINLINE int32 strnicmp(const ANSICHAR* string1, const ANSICHAR* string2, SIZE_T count)
		{
			return ::strnicmp(string1, string2, count);
		}
		static FORCEINLINE const WIDECHAR* strstr(const WIDECHAR* string, const WIDECHAR* find)
		{
			return _tcsstr(string, find);
		}

		static FORCEINLINE const ANSICHAR* strstr(const ANSICHAR* string, const ANSICHAR* find)
		{
			return ::strstr(string, find);
		}

		static FORCEINLINE int32 strcmp(const WIDECHAR* string1, const WIDECHAR* string2)
		{
			return ::_tcscmp(string1, string2);
		}
		static FORCEINLINE const WIDECHAR* strchr(const WIDECHAR* string, WIDECHAR c)
		{
			return _tcschr(string, c);
		}

		static FORCEINLINE int32 strcmp(const ANSICHAR* string1, const ANSICHAR* string2)
		{
			return ::strcmp(string1, string2);
		}

		static FORCEINLINE WIDECHAR* strcpy(WIDECHAR* dest, SIZE_T destCount, const WIDECHAR* src)
		{
#if USE_SECURE_CRT
			_tcscpy_s(dest, destCount, src);
			return dest;
#else
			return (WIDECHAR*)_tcscpy(dest, src);
#endif
		}

		static FORCEINLINE WIDECHAR* strncpy(WIDECHAR* dest, const WIDECHAR* src, SIZE_T maxLen)
		{
#if USE_SECURE_CRT
			_tcsnccpy_s(dest, maxLen, src, maxLen - 1);
#else
			_tcsncpy(dest, src, maxLen - 1);
			dest[maxLen - 1] = 0;
#endif
			return dest;
		}

		static FORCEINLINE ANSICHAR* strcpy(ANSICHAR* dest, SIZE_T destCount, const ANSICHAR* src)
		{
#if USE_SECURE_CRT
			strcpy_s(dest, destCount, src);
			return dest;
#else
			return (ANSICHAR*)::strcpy(dest, src);
#endif
		}
		static FORCEINLINE void strncpy(ANSICHAR* dest, const ANSICHAR* src, SIZE_T maxLen)
		{
#if USE_SECURE_CRT
			strncpy_s(dest, maxLen, src, maxLen - 1);
#else
			::strncpy(dest, src, maxLen);
			dest[maxLen - 1] = 0;
#endif
		}

		static FORCEINLINE const ANSICHAR* strchr(const ANSICHAR* string, ANSICHAR c)
		{
			return ::strchr(string, c);
		}
	};
}