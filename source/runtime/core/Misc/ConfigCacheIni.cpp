#include "ConfigCacheIni.h"
#include "HAL/FileManager.h"
#include "CoreGlobals.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Char.h"
#include "Misc/Paths.h"
#include "Containers/StringUtil.h"
#include "RemoteConfigIni.h"
#include "HAL/PlatformProperties.h"
#include "boost/algorithm/string.hpp"
#include "Containers/StringConv.h"
#if WITH_EDITOR
#define INI_CACHE 1
#else
#define INI_CACHE 0
#endif

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


					while (*value && Char::isWhitespace(value[CString::strlen(value) - 1]))
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

	int32 ConfigCacheIni::getArray(const TCHAR* section, const TCHAR* key, TArray<wstring>& outArr, const wstring& filename)
	{
		outArr.empty();
		ConfigFile* file = find(filename, 0);
		if (file != nullptr)
		{
			auto& secIt = file->find(section);
			if (secIt != file->end())
			{
				TArray<ConfigValue> remapArray;
				secIt->second.multiFind(key, remapArray);
				outArr.addZeroed(remapArray.size());
				for (int32 remapIndex = remapArray.size() - 1, index = 0; remapIndex >= 0; remapIndex--, index++)
				{
					outArr[index] = remapArray[remapIndex].getValue();
				}
			}
		}

		return outArr.size();
	}


	void ConfigCacheIni::loadLocalIniFile(ConfigFile& configFile, const TCHAR* iniName, bool bGenerateDestIni, const TCHAR* platform, const bool bForceReload /* = false */)
	{
		loadExternalIniFile(configFile, iniName, Paths::engineConfigDir().c_str(), Paths::sourceConfigDir().c_str(), bGenerateDestIni, platform, bForceReload);
	}


	bool ConfigFile::combine(const wstring& filename)
	{
		wstring text;
		if (FileHelper::loadFileToString(text, filename.c_str()))
		{
			combineFromBuffer(text);
			return true;
		}
		return false;
	}

	static void fixupArrayOfStructKeysForSection(ConfigSection* section, const wstring& sectionName, const TMap<wstring, TMap<wstring, wstring>>& perObjectConfigKeys)
	{
		for (const auto it : perObjectConfigKeys)
		{
			if (boost::ends_with(sectionName, it.first))
			{
				for (const auto it2 : it.second)
				{
					section->mArrayOfStructKeys.emplace(it2.first, it2.second);
				}
			}
		}
	}

	void ConfigFile::combineFromBuffer(const wstring& buffer)
	{
		const TCHAR* ptr = buffer.c_str();
		ConfigSection* currentSection = nullptr;
		wstring currentSectionName;
		bool done = false;
		while (!done)
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

				currentSectionName = start;

				fixupArrayOfStructKeysForSection(currentSection, start, mPerObjectConfigArrayOfStructKeys);
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

					if (start[0] == '~')
					{
						start++;
					}
					TCHAR cmd = start[0];
					if (cmd == '+' || cmd == '-' || cmd == '.' || cmd == '!' || cmd == '@' || cmd == '*')
					{
						start++;
					}
					else
					{
						cmd = ' ';
					}

					while (*start && Char::isWhitespace(start[CString::strlen(start) - 1]))
					{
						start[CString::strlen(start) - 1] = 0;
					}

					wstring processedValue;

					while (*value && Char::isWhitespace(*value))
					{
						value++;
					}

					while (*value && Char::isWhitespace(value[CString::strlen(value) - 1]))
					{
						value[CString::strlen(value) - 1] = 0;
					}

					if (*value == '\"')
					{
						value++;
						while (*value && *value != '\"')
						{
							if (*value != '\\')
							{
								processedValue += *value++;
							}
							else if (*++value == '\\')
							{
								processedValue += '\\';
								value++;
							}
							else if (*value == '\"')
							{
								processedValue += '\"';
								value++;
							}
							else if (*value == TEXT('n'))
							{
								processedValue += TEXT('n');
								value++;
							}
							else if (*value == TEXT('u') && value[1] && value[2] && value[3] && value[4])
							{
								processedValue += (TCHAR)(Parse::hexDigit(value[1]) * (1 << 12) + Parse::hexDigit(value[2]) * (1 << 8) + Parse::hexDigit(value[3])* (1 << 4) + Parse::hexDigit(value[4]));
								value += 5;
							}
							else if (value[1])
							{
								processedValue += (TCHAR)(Parse::hexDigit(value[0]) * 16 + Parse::hexDigit(value[1]));
								value += 2;
							}
						}
					}
					else
					{
						processedValue = value;
					}
					if (cmd == TEXT('+'))
					{
						currentSection->handleAddCommand(start, processedValue, true);
					}
					else if (cmd == TEXT('-'))
					{
						currentSection->removeSingle(start, processedValue);
					}
					else if (cmd == '.')
					{
						currentSection->handleAddCommand(start, processedValue, false);
					}
					else if (cmd == '!')
					{
						currentSection->erase(start);
					}
					else if (cmd == '@')
					{
						currentSection->mArrayOfStructKeys.emplace(start, processedValue);
					}
					else if (cmd == '*')
					{
						TMap<wstring, wstring>& pocKeys = mPerObjectConfigArrayOfStructKeys.findOrAdd(currentSectionName);
						pocKeys.emplace(start, processedValue);
					}
					else
					{
						auto& it = currentSection->find(start);
						if (it == currentSection->end())
						{
							currentSection->emplace(start, processedValue);
						}
						else
						{
							it->second = ConfigValue(processedValue);
						}
					}
					mDirty = true;
				}
			}
		}
	}

	void ConfigFile::addMissingProperties(const ConfigFile& inSourceFile)
	{
		for (auto & sourceSectionIt = inSourceFile.begin(); sourceSectionIt != inSourceFile.end(); ++sourceSectionIt)
		{
			const wstring& sourceSectionName = sourceSectionIt->first;
			const ConfigSection& sourceSection = sourceSectionIt->second;
			{
				ConfigSection* destSection = findOrAddSection(sourceSectionName);
				for (auto& sourcePropertyIt = sourceSection.begin(); sourcePropertyIt != sourceSection.end(); sourcePropertyIt++)
				{
					const wstring sourcePropertyName = sourcePropertyIt->first;
					if (destSection->find(sourcePropertyName) == destSection->end())
					{
						TArray<ConfigValue> results;
						sourceSection.multiFind(sourcePropertyName, results, true);
						for (const ConfigValue& result : results)
						{
							destSection->emplace(sourcePropertyName, result.getSavedValue());
							mDirty = true;
						}
					}
				}
			}
		}
	}

	bool ConfigFile::getString(const TCHAR* section, const TCHAR* key, wstring& value) const
	{
		const auto& it = this->find(section);
		if (it == this->end())
		{
			return false;
		}

		auto& pairString = it->second.find(key);
		if (pairString == it->second.end())
		{
			return false;
		}
		value = pairString->second.getValue();
		return true;
	}


	static void loadAnIniFile(const wstring& filenameToLoad, ConfigFile& configFile)
	{
		if (!isUsingLocalIniFile(filenameToLoad.c_str(), nullptr) || (IFileManager::get().fileSize(filenameToLoad.c_str()) >= 0))
		{
			processIniContents(filenameToLoad.c_str(), filenameToLoad.c_str(), &configFile, false, false);
		}
		else
		{

		}
	}

	static void getSourceIniHierarchyFilenames(const TCHAR* inBaseIniName, const TCHAR* inPlatformName, const TCHAR* engineConfigDir, const TCHAR* sourceConfigDir, ConfigFileHierarchy& outHierarchy, bool bRequireDefaultIni)
	{
		const wstring platformName(inPlatformName ? inPlatformName : ANSI_TO_TCHAR(PlatformProperties::iniPlatformName()));

#if IS_PROGRAM
		const bool baseIniRequired = false;
#else
		const bool baseIniRequired = true;
#endif

		outHierarchy.emplace(EConfigFileHierarchy::AbsoluteBase, IniFilename(printf(TEXT("%sBase.ini"), engineConfigDir), baseIniRequired));

		outHierarchy.emplace(EConfigFileHierarchy::EngineDirBase, IniFilename(printf(TEXT("%sBase%s.ini"), engineConfigDir, inBaseIniName), false));

	}

	static void clearHierarchyCache(const TCHAR* baseIniName)
	{
	}

	static bool loadIniFileHierarchy(const ConfigFileHierarchy& hierarchyToLoad, ConfigFile& configFile, const bool bUseCache)
	{
		if (hierarchyToLoad.size() == 0)
		{
			return true;
		}
		else
		{
			int32 numExistingOptionalInis = 0;
			for (const auto& hierarchyIt : hierarchyToLoad)
			{
				const IniFilename& iniToLoad = hierarchyIt.second;
				if (iniToLoad.bRequired == false && (!isUsingLocalIniFile(iniToLoad.mFilename.c_str(), nullptr) || IFileManager::get().fileSize(iniToLoad.mFilename.c_str()) >= 0))
				{
					numExistingOptionalInis++;
				}
			}
			if (numExistingOptionalInis == 0)
			{
				return true;
			}
		}

		EConfigFileHierarchy firstCacheIndex = EConfigFileHierarchy::AbsoluteBase;

		TArray<DateTime> timestampsOfInits;

		for (auto& hierarchyIt : hierarchyToLoad)
		{
			if (firstCacheIndex <= hierarchyIt.first)
			{
				const IniFilename& iniToLoad = hierarchyIt.second;
				const wstring& iniFileName = iniToLoad.mFilename;
				bool bDoProcess = true;

				if (bDoProcess)
				{
					if (isUsingLocalIniFile(iniFileName.c_str(), nullptr) && (IFileManager::get().fileSize(iniFileName.c_str()) < 0))
					{
						if (iniToLoad.bRequired)
						{
							return false;
						}
						else
						{
							continue;
						}
					}

					bool bDoEmptyConfig = false;
					bool bDoCombine = (hierarchyIt.first != EConfigFileHierarchy::AbsoluteBase);

					processIniContents(hierarchyIt.second.mFilename.c_str(), iniFileName.c_str(), &configFile, bDoEmptyConfig, bDoCombine);
				}
			}
		}
		configFile.mSourceIniHerarchy = hierarchyToLoad;
		return true;
	}

	void makeLocalCopy(const TCHAR* filename)
	{
		BOOST_ASSERT(filename);

		if (isUsingLocalIniFile(filename, nullptr))
		{
			return;
		}

		if (IFileManager::get().fileSize(filename) >= 0)
		{
			wstring filenameStr = filename;

			if (CString::stristr(filenameStr.c_str(), TEXT(".ini")))
			{
				filenameStr = filenameStr.substr(0, filenameStr.length() - 4);
			}
			else
			{
				BOOST_ASSERT(false);
			}

			TCHAR filenameLocal[1024];

			CString::strcpy(filenameLocal, filenameStr.c_str());
			CString::strcat(filenameLocal, TEXT("_Local.ini"));
			IFileManager::get().copy(filenameLocal, filename);
		}
	}


	static bool generateDestIniFile(ConfigFile& destConfigFile, const wstring& destIniFilename, const ConfigFileHierarchy& sourceIniHierarchy, bool bAllowGeneratedINIs, const bool bUseHierarchyCache)
	{
		bool bResult = loadIniFileHierarchy(sourceIniHierarchy, *destConfigFile.mSourceConfigFile, bUseHierarchyCache);
		if (bResult == false)
		{
			return false;
		}

		loadAnIniFile(destIniFilename, destConfigFile);

		bool bforceRegenerate = false;

		bool bShouldUpdate = PlatformProperties::requiresCookedData();

		if (!PlatformProperties::requiresCookedData() || bAllowGeneratedINIs)
		{
			bool bIsLegacyConfigSystem = false;
			for (auto& it = destConfigFile.begin(); it != destConfigFile.end(); it++)
			{
				wstring sectionName = it->first;
				if (sectionName == TEXT("IniVersion") || sectionName == TEXT("Engine.Engine"))
				{
					bIsLegacyConfigSystem = true;
					break;
				}
			}

			if (bIsLegacyConfigSystem)
			{
				bforceRegenerate = true;
			}

			else
			{
				bShouldUpdate = true;
			}
		}

		if (bforceRegenerate)
		{
			bResult = loadIniFileHierarchy(sourceIniHierarchy, destConfigFile, bUseHierarchyCache);
			destConfigFile.mSourceConfigFile = new ConfigFile(destConfigFile);

			destConfigFile.mDirty = true;
		}
		else if (bShouldUpdate)
		{
			destConfigFile.addMissingProperties(*destConfigFile.mSourceConfigFile);
			destConfigFile.mDirty = true;
		}
		if (!isUsingLocalIniFile(destIniFilename.c_str(), nullptr))
		{
			makeLocalCopy(destIniFilename.c_str());
		}
		return bResult;
	}

	static wstring getDestIniFilename(const TCHAR* baseIniName, const TCHAR* platformName, const TCHAR* generatedConfigDir)
	{
		wstring commandLineSwitch = printf(TEXT("%sINI="), baseIniName);

		wstring iniFilename;
		wstring name(platformName ? platformName : PlatformProperties::platformName());
		wstring baseIniNameString = baseIniName;
		if (boost::contains(baseIniNameString, generatedConfigDir))
		{
			iniFilename = baseIniNameString;
		}
		else
		{
			iniFilename = printf(TEXT("%s%s/%s.ini"), generatedConfigDir, name.c_str(), baseIniName);
		}

		Paths::makeStandardFilename(iniFilename);
		return iniFilename;
	}
	

	void ConfigCacheIni::loadExternalIniFile(ConfigFile& configFile, const TCHAR* iniName, const TCHAR* engineConfigDir, const TCHAR* sourceConfigDir, bool bGenerateDestIni, const TCHAR* platform /* = nullptr */, const bool bForceReload /* = false */)
	{
		if (!bGenerateDestIni)
		{
			wstring sourceIniFilename = printf(TEXT("%s/%s.ini"), sourceConfigDir, iniName);
			loadAnIniFile(sourceIniFilename, configFile);
		}
		else
		{
			getSourceIniHierarchyFilenames(iniName, platform, engineConfigDir, sourceConfigDir, configFile.mSourceIniHerarchy, false);
			if (bForceReload)
			{
				clearHierarchyCache(iniName);
			}

			configFile.mSourceConfigFile = new ConfigFile();

			const bool bAllowGenerateINIs = true;

			generateDestIniFile(configFile, getDestIniFilename(iniName, platform, Paths::generateConfigDir().c_str()), configFile.mSourceIniHerarchy, bAllowGenerateINIs, true);
		}
		configFile.mName = iniName;
	}

	static wstring extractPropertyValue(const wstring& fullStructValue, const wstring& structKeyMatch)
	{
		int32 matchLoc = fullStructValue.find(structKeyMatch);
		if (matchLoc >= 0)
		{
			matchLoc += structKeyMatch.length();
			const TCHAR* start = &fullStructValue[matchLoc];
			bool bInQuotes = false;
			if (*start == TEXT('\"'))
			{
				start++;
				bInQuotes = true;
			}
			const TCHAR* travel = start;
			while (*travel && ((bInQuotes && *travel != '\"') || (!bInQuotes && (Char::isAlnum(*travel) || *travel == '_'))))
			{
				travel++;
			}

			return fullStructValue.substr(matchLoc, travel - start);
		}
		return TEXT("");
	}

	void ConfigSection::handleAddCommand(wstring key, const wstring & value, bool bAppendValueIfNotArrayOfStructsKeyUsed)
	{
		auto & it = mArrayOfStructKeys.find(key);

		bool bHandleWithKey = false;
		if (it != mArrayOfStructKeys.end())
		{
			wstring structKeyMatch = it->first + TEXT("=");
			wstring structKeyValueToMatch = extractPropertyValue(value, structKeyMatch);
			if (structKeyValueToMatch.length() > 0)
			{
				for (auto & valueIt = this->begin(); valueIt != this->end(); valueIt++)
				{
					if (valueIt->first == key)
					{
						wstring existingStructValueKey = extractPropertyValue(valueIt->second.getValue(), structKeyMatch);
						if (existingStructValueKey == structKeyValueToMatch)
						{
							removeSingle(key, valueIt->second.getValue());
							this->emplace(key, value);

							bHandleWithKey = true;
							break;
						}
					}
				}
			}
		}
		if (!bHandleWithKey)
		{
			if (bAppendValueIfNotArrayOfStructsKeyUsed)
			{
				this->emplace(key, value);
			}
			else
			{
				this->addUnique(key, ConfigValue(value));
			}
		}
	}
}