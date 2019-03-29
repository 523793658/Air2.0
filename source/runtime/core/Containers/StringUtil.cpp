#include "StringUtil.h"
#include <algorithm>
#include "boost/algorithm/string.hpp"
namespace Air
{
	bool StringUtil::matchesWildcard(const wstring& inTarget, const wstring& inWildCard, ESearchCase::Type searchCase)
	{
		wstring widlcard(inWildCard);
		wstring target(inTarget);
		int32 indexOfStar = widlcard.find_last_of('*');
		int32 indexOfQuestion = widlcard.find_last_of('?');
		int32 suffix = std::max(indexOfStar, indexOfQuestion);
		if (suffix == INDEX_NONE)
		{
			if (searchCase == ESearchCase::IgnoreCase)
			{
				return boost::algorithm::iequals(target, widlcard);
			}
			else
			{
				return boost::algorithm::equals(target, widlcard);
			}
		}
		else
		{
			if (suffix + 1 < widlcard.length())
			{
				wstring suffixString = widlcard.substr(suffix + 1);
				if (searchCase == ESearchCase::IgnoreCase)
				{
					if (!boost::algorithm::iends_with(target, suffixString))
					{
						return false;
					}
				}
				else
				{
					if (!boost::algorithm::ends_with(target, suffixString))
					{
						return false;
					}
				}
				widlcard = widlcard.substr(0, suffix + 1);
				target = target.substr(0, target.length() - suffixString.length());
			}
			int32 prefixIndexOfStar = widlcard.find_first_of('*');
			int32 prefixIndexOfQuestion = widlcard.find_first_of('?');
			int32 prefix = std::min<int32>(prefixIndexOfStar < 0 ? std::numeric_limits<int32>::max() : prefixIndexOfStar, prefixIndexOfQuestion < 0 ? std::numeric_limits<int32>::max() : prefixIndexOfQuestion);

			if (prefix > 0)
			{
				wstring prefexString = widlcard.substr(0, prefix);
				if (searchCase == ESearchCase::IgnoreCase)
				{
					if (!boost::algorithm::istarts_with(target, prefexString))
					{
						return false;
					}
				}
				else
				{
					if (!boost::algorithm::starts_with(target, prefexString))
					{
						return false;
					}
				}
				widlcard = widlcard.substr(prefix);
				target = target.substr(prefix);
			}
		}
		TCHAR firstWild = widlcard[0];
		widlcard = widlcard.substr(1);
		if (firstWild == TEXT('*') || firstWild == TEXT('?'))
		{
			if (!widlcard.length())
			{
				if (firstWild == TEXT('*') || target.length() < 2)
				{
					return true;
				}
			}
			int32 maxNum = std::min<int32>(target.length(), firstWild == TEXT('?') ? 1 : std::numeric_limits<int32>::max());
			for (int32 index = 0; index <= maxNum; index++)
			{
				if (matchesWildcard(target.substr(index), widlcard, searchCase))
				{
					return true;
				}
			}
			return false;
		}
		else
		{
			return false;
		}
	}

	int32 StringUtil::find(wstring& input, const TCHAR* substr, ESearchCase::Type searchCase, ESearchDir::Type searchDir, int32 startPos)
	{
		if (substr == nullptr)
		{
			return INDEX_NONE;
		}
		if (searchDir == ESearchDir::FromStart)
		{
			const TCHAR* start = &input[0];
			if (startPos != INDEX_NONE)
			{
				start += Math::clamp<int32>(startPos, 0, input.length() - 1);
			}
			const TCHAR* tmp = searchCase == ESearchCase::IgnoreCase ? CString::stristr(start, substr) : CString::strstr(start, substr);
			return tmp ? (tmp - input.c_str()) : INDEX_NONE;
		}
		else
		{
			if (searchCase == ESearchCase::IgnoreCase)
			{
				find(boost::to_upper_copy(input), boost::to_upper_copy(wstring(substr)).c_str(), ESearchCase::CaseSensitive, searchDir, startPos);
			}
			else
			{
				const int32 serachStringLength = Math::max(1, CString::strlen(substr));
				if (startPos == INDEX_NONE)
				{
					startPos = input.length();
				}
				for (int32 i = startPos - serachStringLength; i >= 0; i--)
				{
					int32 j;
					for (j = 0; substr[j]; j++)
					{
						if (input[i + j] != substr[j])
						{
							break;
						}
					}
					if (!substr[j])
					{
						return i;
					}
				}
				return INDEX_NONE;
			}
		}
	}
}