#pragma once
#include "CoreType.h"
#include "Misc/CString.h"
#include "boost/assert.hpp"
#include "Misc/VarArgs.h"
#include <string>

namespace Air
{
	using std::wstring;
	using std::string;

#define Name_None	TEXT("")

	namespace ESearchCase
	{
		enum Type
		{
			CaseSensitive,

			IgnoreCase,
		};
	}

	namespace ESearchDir
	{
		enum Type
		{
			FromStart,
			FromEnd,
		};
	}

	inline TCHAR nibbleToChar(uint8 num)
	{
		if (num > 9)
		{
			return TEXT('A') + TCHAR(num - 10);
		}
		return TEXT('0') + TCHAR(num);
	}

	inline void byteToHex(uint8 In, wstring& result)
	{
		result += nibbleToChar(In >> 4);
		result += nibbleToChar(In & 15);
	}


	inline wstring bytesToHex(const uint8* In, int32 count)
	{
		wstring result;
		result.resize(count * 2);
		while (count)
		{
			byteToHex(*In++, result);
			count--;
		}
		return result;
	}

	void pathAppend(wstring& basePath, const TCHAR* str, int32 strLength);

	FORCEINLINE wstring& operator /=(wstring& src, const TCHAR* str)
	{
		BOOST_ASSERT(str != nullptr);
		pathAppend(src, str, CString::strlen(str));
		return src;
	}
	

	FORCEINLINE wstring operator /= (const wstring& lhs, const wstring& rhs)
	{
		return lhs + rhs;
	}

#define STARTING_BUFFER_SIZE	512

	namespace String
	{

		FORCEINLINE wstring printf(const TCHAR* _Format, ...)
		{
			/*TCHAR temp[512];
			va_list args;
			va_start(args, _Format);
			CString::printf(temp, _Format, args);
			va_end(args);
			return wstring(temp);*/
			int32 bufferSize = STARTING_BUFFER_SIZE;
			TCHAR startingBuffer[STARTING_BUFFER_SIZE];
			TCHAR* buffer = startingBuffer;
			int32 result = -1;
			GET_VARARGS_RESULT(buffer, bufferSize, bufferSize - 1, _Format, _Format, result);
			if (result == -1)
			{
				buffer = nullptr;
				while (result == -1)
				{
					bufferSize *= 2;
					buffer = (TCHAR*)Memory::realloc(buffer, bufferSize * sizeof(TCHAR));
					GET_VARARGS_RESULT(buffer, bufferSize, bufferSize - 1, _Format, _Format, result);
				};
			}
			buffer[result] = 0;
			wstring resultString(buffer);
			if (bufferSize != STARTING_BUFFER_SIZE)
			{
				Memory::free(buffer);
			}
			return resultString;
		}

		FORCEINLINE string printf(const char* _Format, ...)
		{
			int32 bufferSize = STARTING_BUFFER_SIZE;
			char startingBuffer[STARTING_BUFFER_SIZE];
			char* buffer = startingBuffer;
			int32 result = -1;
			GET_VARARGS_RESULT_ANSI(buffer, bufferSize, bufferSize - 1, _Format, _Format, result);
			if (result == -1)
			{
				buffer = nullptr;
				while (result == -1)
				{
					bufferSize *= 2;
					buffer = (char*)Memory::realloc(buffer, bufferSize * sizeof(TCHAR));
					GET_VARARGS_RESULT_ANSI(buffer, bufferSize, bufferSize - 1, _Format, _Format, result);
				};
			}
			buffer[result] = 0;
			string resultString(buffer);
			if (bufferSize != STARTING_BUFFER_SIZE)
			{
				Memory::free(buffer);
			}
			return resultString;
		}

		FORCEINLINE void printf_b(TCHAR buffer[], const TCHAR* format, ...)
		{
			va_list args;
			va_start(args, format);
			CString::vsprintf(buffer, format, args);
			va_end(args);
		}

		FORCEINLINE void printf_b(char buffer[], const char* format, ...)
		{
			va_list args;
			va_start(args, format);
			CStringAnsi::vsprintf(buffer, format, args);
			va_end(args);
		}
	}
	using namespace String;
}