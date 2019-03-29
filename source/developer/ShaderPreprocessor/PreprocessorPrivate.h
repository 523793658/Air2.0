#pragma once
#include "CoreMinimal.h"
#include "ShaderCore.h"
#include "mcpp.h"
#include "boost/algorithm/string.hpp"
namespace Air
{
	inline bool filterMcppError(const wstring& errorMsg)
	{

		const TCHAR* substringsToFilter[] =
		{
			TEXT("Unknown encoding:"),
			TEXT("with no newline, supplemented newline"),
			TEXT("Converted [CR+LF] to [LF]")
		};
		const int32 filterdSubstringCount = ARRAY_COUNT(substringsToFilter);
		for (int32 substringIndex = 0; substringIndex < filterdSubstringCount; ++substringIndex)
		{
			if (boost::contains(errorMsg, substringsToFilter[substringIndex]))
			{
				return false;
			}
		}
		return true;
	}


	static bool parseMcppErrors(TArray<ShaderCompilerError>& outErrors, const wstring& McppErrors, bool bConvertFilenameToRelative)
	{
		bool bSuccess = true;
		if (McppErrors.length() > 0)
		{
			std::vector<wstring> lines;
			boost::split(lines, McppErrors, boost::is_any_of("\n"));
			for (int32 lineIndex = 0; lineIndex < lines.size(); ++lineIndex)
			{
				const wstring& line = lines[lineIndex];
				int32 sepIndex1 = line.find_first_of(TEXT(':'), 2);
				int32 sepIndex2 = line.find_first_of(TEXT(':'), sepIndex1 + 1);
				if (sepIndex1 != INDEX_NONE && sepIndex2 != INDEX_NONE)
				{
					wstring filename = line.substr(0, sepIndex1);
					wstring lineNumStr = line.substr(sepIndex1 + 1, sepIndex2 - sepIndex1 - 1);
					wstring message = line.substr(sepIndex2 + 1, line.length() - sepIndex2 - 1);
					if (filename.length() && lineNumStr.length() && CString::isNumeric(lineNumStr.c_str()) && message.length())
					{
						while (++lineIndex < lines.size() && lines[lineIndex].length() && boost::starts_with(lines[lineIndex], TEXT(" ")))
						{
							message += TEXT("\n") + lines[lineIndex];
						}
						--lineIndex;
						boost::trim(message);
						if (filterMcppError(message))
						{
							ShaderCompilerError* compilerError = new(outErrors)ShaderCompilerError;
							compilerError->mErrorFile = bConvertFilenameToRelative ? getRelativeShaderFilename(filename) : filename;
							compilerError->mErrorLineString = lineNumStr;
							compilerError->mStrippedErrorMessage = message;
						}
					}
				}
			}
		}
		return bSuccess;
	}
}