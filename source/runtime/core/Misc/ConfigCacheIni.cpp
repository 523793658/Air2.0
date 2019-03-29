#include "ConfigCacheIni.h"
#include "HAL/FileManager.h"
#include "CoreGlobals.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Char.h"
#include "Containers/StringUtil.h"
namespace Air
{
	bool ConfigCacheIni::getSection(const TCHAR* section, TArray<wstring>& result, const wstring & fileName)
	{
		result.clear();
		ConfigFile* file = find(fileName, false);
		if (!file)
		{
			return false;
		}
		auto it = file->find(section);
		if (it == file->end())
		{
			return false;
		}
		ConfigSection& sec = it->second;
		result.reserve(sec.size());
		for (auto& it : sec)
		{
			result.add(String::printf(TEXT("%s=%s"), it.first.c_str(), it.second.getValue()));
		}
		return true;
	}

	ConfigFile* ConfigCacheIni::find(const wstring& filename, bool createIfNotFound)
	{
		if (filename.length() == 0)
		{
			return nullptr;
		}
		auto it = TMap<wstring, ConfigFile>::find(filename);
		if (it == TMap<wstring, ConfigFile>::end() && !bAreFileOperationsDisabled && (createIfNotFound || (IFileManager::get().fileSize(filename.c_str()) > 0)))
		{
			auto temp = emplace(filename, ConfigFile());
			if (temp.second)
			{
				it = temp.first;
				it->second.read(filename);
			}
		}
		return &it->second;
	}

	bool ConfigCacheIni::areFileOperationsDisabled()
	{
		return bAreFileOperationsDisabled;
	}

	void ConfigFile::read(const wstring& filename)
	{
		if (GConfig == nullptr || !GConfig->areFileOperationsDisabled())
		{
			clear();
			wstring text;
			if (FileHelper::loadFileToString(text, filename.c_str()))
			{
				processInputFileContents(text);
			}
		}
	}

	ConfigSection* ConfigFile::findOrAddSection(const wstring& sectionName)
	{
		auto it = find(sectionName);
		if (it == end())
		{
			auto r = emplace(sectionName, ConfigSection());
			if (r.second)
			{
				it = r.first;
			}
			else
			{
				return nullptr;
			}
		}
		return &it->second;
	}

	void ConfigFile::processInputFileContents(const wstring& contents)
	{
		const TCHAR* ptr = contents.length() > 0 ? contents.c_str() : nullptr;
		ConfigSection* currentSection = nullptr;
		bool done = false;
		while (!done && ptr != nullptr)
		{
			while (*ptr == '\r' || *ptr == '\n')
			{
				ptr++;
			}
			wstring theLine;
			int32 linesConsumed = 0;
			Parse::lineExtended(&ptr, theLine, linesConsumed, false);
			if (ptr == nullptr || *ptr == 0)
			{
				done = true;
			}
			TCHAR* start = const_cast<TCHAR*>(theLine.c_str());
			while (*start && Char::isWhitespace(start[CString::strlen(start) - 1]))
			{
				start[CString::strlen(start) - 1] = 0;
			}

			if (*start == '[' && start[CString::strlen(start) - 1] == ']')
			{
				start++;
				start[CString::strlen(start) - 1] = 0;
				currentSection = findOrAddSection(start);
			}
			else if (currentSection && *start)
			{
				TCHAR* value = 0;
				if (*start != (TCHAR)';')
				{
					value = CString::strstr(start, TEXT("="));
				}
				if (value)
				{
					*value++ = 0;
					while (*start && Char::isWhitespace(*start))
					{
						start++;
					}
					while (*start && Char::isWhitespace(start[CString::strlen(start) - 1]))
					{
						start[CString::strlen(start) - 1] = 0;
					}
					while (*value && Char::isWhitespace(*value))
					{
						value++;
					}


					while (*value && Char::isWhitespace(value[CString::strlen(value)-1]))
					{
						value[CString::strlen(value) - 1] = 0;
					}
					if (*value == '\"')
					{
						wstring preprocessedValue = StringUtil::replaceQuotesWithEscapedQuotes(StringUtil::trimQuotes(wstring(value)));
						const TCHAR* newValue = preprocessedValue.c_str();
						wstring processedValue;
						while (*newValue && *newValue != '\"')
						{
							if (*newValue != '\\')
							{
								processedValue += *newValue++;
							}
							else if (*++newValue == '\0')
							{
								break;
							}
							else if (*newValue == '\\')
							{
								processedValue += '\\';
								newValue++;
							}
							else if (*newValue == TEXT('n'))
							{
								processedValue += TEXT('\n');
								newValue++;
							}
							else if (*newValue == TEXT('u') && newValue[1] && newValue[2] && newValue[3] && newValue[4])
							{
								processedValue += (TCHAR)(Parse::hexDigit(newValue[1]) *(1 << 12) + Parse::hexDigit(newValue[2]) * (1 << 8) + Parse::hexDigit(newValue[3]) * (1 << 4) + Parse::hexDigit(newValue[4]));
								newValue += 5;
							}
							else if (newValue[1])
							{
								processedValue += (TCHAR)(Parse::hexDigit(newValue[0]) * 16 + Parse::hexDigit(newValue[1]));
								newValue += 2;
							}
						}
						currentSection->emplace(start, processedValue.c_str());
					}
					else
					{
						currentSection->emplace(start, value);
					}
				}
			}
		}
	}
}