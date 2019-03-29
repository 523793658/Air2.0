#pragma once
#include "CoreType.h"
#include "Containers/String.h"
namespace Air
{
	struct CORE_API Parse
	{
		static bool lineExtended(const TCHAR** stream, wstring& result, int32& linesConsumed, bool exact = 0);

		static FORCEINLINE int32 hexDigit(TCHAR c)
		{
			int32 result = 0;
			if (c >= '0' && c <= '9')
			{
				result = c - '0';
			}
			else if (c >= 'a' && c <= 'f')
			{
				result = c + 10 - 'a';
			}
			else if (c >= 'A' && c <= 'F')
			{
				result = c + 10 - 'A';
			}
			else
			{
				result = 0;
			}
			return result;
		}
	};
}