#include "Misc/Parse.h"
namespace Air
{
	bool Parse::lineExtended(const TCHAR** stream, wstring& result, int32& linesConsumed, bool exact /* = 0 */)
	{
		bool gotStream = 0;
		bool isQuoted = 0;
		bool ignore = 0;
		int32 bracketDepth = 0;
		result = TEXT("");
		linesConsumed = 0;
		while (**stream != 0 && ((**stream != 10 && **stream != 13) || bracketDepth > 0))
		{
			if (!isQuoted && !exact && (*stream)[0] == '/' && (*stream)[1] == '/')
			{
				ignore = 1;
			}

			if (!isQuoted && !exact && **stream == '|')
			{
				break;
			}
			gotStream = 1;
			if (**stream == 10 || **stream == 13)
			{
				BOOST_ASSERT(bracketDepth > 0);
				result.append(1, TEXT(' '));
				linesConsumed++;
				(*stream)++;
				if (**stream == 10 || **stream == 13)
				{
					(*stream)++;
				}
			}
			else if(!isQuoted && (*stream)[0] == '\\' && ((*stream)[1] == 10 || (*stream)[1] == 13))
			{
				result.append(1, TEXT(' '));
				linesConsumed++;
				(*stream) += 2;
				if (**stream == 10 || **stream == 13)
				{
					(*stream)++;
				}
			}
			else if (!isQuoted && **stream == '{')
			{
				bracketDepth++;
				(*stream)++;
			}
			else if (!isQuoted && **stream == '}' && bracketDepth > 0)
			{
				bracketDepth--;
				(*stream)++;
			}
			else
			{
				isQuoted = isQuoted ^ (**stream == 34);
				if (!ignore)
				{
					result.append(1, *((*stream)++));
				}
				else
				{
					(*stream)++;
				}
			}
		}
		if (**stream == 0)
		{
			if (gotStream)
			{
				linesConsumed++;
			}
		}
		else if (exact)
		{
			if (**stream == 13 || **stream == 10)
			{
				linesConsumed++;
				if (**stream == 13)
				{
					(*stream)++;
				}
				if (**stream == 10)
				{
					(*stream)++;
				}
			}
		}
		else
		{
			while (**stream == 10 || **stream == 13 || **stream == '|')
			{
				if (**stream != '|')
				{
					linesConsumed++;
				}
				if (((*stream)[0] == 10 && (*stream)[1] == 13) || ((*stream)[0] == 13 && (*stream)[1] == 10))
				{
					(*stream)++;
				}
				(*stream)++;
			}
		}
		return **stream != 0 || gotStream;
	}
}