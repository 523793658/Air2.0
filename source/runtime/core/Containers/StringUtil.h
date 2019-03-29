#pragma once
#include "CoreType.h"
#include <regex>
#include <xstring>
#include "boost/algorithm/string.hpp"
#include "string.h"
#include <codecvt>
#include "Math/Math.h"
#include "HAL/StringView.h"
namespace Air
{
#define MAX_SPRINTF 1024

	static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;

	class CORE_API StringUtil
	{
	public:
		static bool matchesWildcard(const wstring& inTarget, const wstring& inWildCard, ESearchCase::Type searchCase);

		static bool isNumeric(wstring str)
		{
			if (str.empty())
			{
				return false;
			}
			std::wregex re(L"^[+|-]?\\d+");
			return regex_match(str, re);
		}

		static wstring covert(string const & src)
		{
			return conv.from_bytes(src);
		}

		static string covert(wstring const & src)
		{
			return conv.to_bytes(src);
		}

		static wstring trimQuotes(const wstring& src, bool* bQuotesRemoved = nullptr)
		{
			bool bQuotesWereRemoved = false;
			int32 start = 0, count = src.length();
			if (count > 0)
			{
				if (src[0] == TCHAR('"'))
				{
					start++;
					count--;
					bQuotesWereRemoved = true;
				}
				if (src.length() > 1 && src[src.length() - 1] == TCHAR('"'))
				{
					count--;
					bQuotesWereRemoved = true;
				}
			}
			if (bQuotesRemoved != nullptr)
			{
				*bQuotesRemoved = bQuotesWereRemoved;
			}
			return src.substr(start, count);
		}

		static wstring replaceQuotesWithEscapedQuotes(const wstring& src)
		{
			if (boost::algorithm::contains(src, TEXT("\"")))
			{
				wstring result;
				const TCHAR* pChar = src.c_str();
				bool bEscaped = false;
				while (*pChar != 0)
				{
					if (bEscaped)
					{
						bEscaped = false;
					}
					else if (*pChar == TCHAR('\\'))
					{
						bEscaped = true;
					}
					else if (*pChar == TCHAR('"'))
					{
						result.append(TCHAR('\\'),1);
					}
					result.append(*pChar++, 1);
				}
				return result;
			}
			return src.c_str();
		}

		static int32 find(wstring& input, const TCHAR* substr, ESearchCase::Type searchCase = ESearchCase::
			CaseSensitive, ESearchDir::Type searchDir = ESearchDir::FromStart, int32 startPos = INDEX_NONE);
	};
}