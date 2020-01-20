#include "Modules/ModuleManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Modules/ModuleInterface.h"
#include "HAL/FileManager.h"
#include "Misc/App.h"
#include "boost/algorithm/string.hpp"
#include "Containers/StringUtil.h"
#include "Logging/LogMacros.h"
#include <regex>
#include "Containers/Map.h"

namespace Air
{
#define MODULE_API_VERSION	1

	int32 ModuleManager::ModuleInfo::mCurrentLoadOrder = 0;
	ModuleManager::~ModuleManager()
	{

	}

	bool findNewestModuleFile(TArray<wstring>& filesToSearch, const time_t newerThan, const wstring& moduleFileSearchDirectory, const wstring & prefix, const wstring& suffix, wstring & outFileName)
	{
		bool bFound = false;
		time_t newestFoundFileTime = newerThan;
		for (const auto& foundFile : filesToSearch)
		{
			const wstring foundFilePath = moduleFileSearchDirectory.empty() ? foundFile : (moduleFileSearchDirectory + foundFile);

			wstring center = foundFilePath.substr(prefix.length(), foundFilePath.length() - prefix.length() - suffix.length());

			if (!StringUtil::isNumeric(center))
			{
				continue;
			}
			const time_t foundFileTime = IFileManager::get().getTimeStamp(foundFilePath.c_str());
			if (foundFileTime != -1)
			{
				if (foundFileTime > newestFoundFileTime)
				{
					bFound = true;
					newestFoundFileTime = foundFileTime;
					outFileName = Paths::getCleanFilename(foundFilePath);
				}
			}
			else
			{

			}
		}
		return bFound;

	}


	void ModuleManager::addBinariesDirectory(std::wstring_view inDirector, bool bIsGameDirectory)
	{
		if (bIsGameDirectory)
		{
			mGameBinariesDirectories.push_back(inDirector.data());
		}
		else
		{
			mEngineBinariesDirectories.push_back(inDirector.data());
		}
	}

	ModuleManager& ModuleManager::get()
	{
		static ModuleManager* mModuleManager = nullptr;
		if (mModuleManager == nullptr)
		{
			static std::mutex LockMutex;
			std::lock_guard<std::mutex> lock(LockMutex);
			if (mModuleManager == nullptr)
			{
				mModuleManager = new ModuleManager();
				const TCHAR* restrictedFolderNames[] = { L"NoRedist", L"NotForLicensees", L"CarefullyRedist" };
				wstring moduleDir = PlatformProcess::getModulesDirectory();
				for (const TCHAR * restrictedFolderName : restrictedFolderNames)
				{
					wstring restrictedFolder = moduleDir + restrictedFolderName;
					if (Paths::directoryExists(restrictedFolder))
					{
						mModuleManager->addBinariesDirectory(restrictedFolder, false);
					}
				}
			}
		}
		return *mModuleManager;
	}
	ModuleManager::ModuleInfoPtr ModuleManager::findModule(std::wstring_view name)
	{
		ModuleManager::ModuleInfoPtr result;
		std::lock_guard<std::mutex> lock(mModulesCriticalSection);

		auto it = mModules.find(name.data());
		if (it != mModules.end())
		{
			result = it->second;
		}
		return result;
	}
	std::shared_ptr<IModuleInterface> ModuleManager::getModule(const wstring name)
	{
		ModuleInfoPtr moduleInfo = findModule(name);
		if (!moduleInfo)
		{
			return std::shared_ptr<IModuleInterface>();
		}
		return moduleInfo->mModule;
	}

	bool ModuleManager::isModuleLoaded(const wstring inModuleName)
	{
		auto it = findModule(inModuleName);
		if (it)
		{
			const ModuleInfo& moduleInfo = *it;
			if (moduleInfo.mModule)
			{
				return true;
			}
		}
		return false;
	}

	std::shared_ptr<IModuleInterface> ModuleManager::loadModuleWithFailureReason(const wstring inModuleName, EModuleLoadResult& reason, bool bWasReloaded)
	{
		std::shared_ptr<IModuleInterface> loadedModule;
		reason = EModuleLoadResult::Success;
		addModule(inModuleName);
		auto it = mModules.find(inModuleName);
		if (it != mModules.end())
		{
			ModuleInfoPtr moduleInfo = it->second;
			if (it->second->mModule)
			{
				loadedModule = it->second->mModule;
			}
			else
			{
				auto initIt = mStaticallyLinkedModuleInitializers.find(inModuleName);
				if (initIt != mStaticallyLinkedModuleInitializers.end())
				{
					const InitializerOfStaticLinkedModule& moduleInitializer = initIt->second;
					moduleInfo->mModule.reset(moduleInitializer());
					if (moduleInfo->mModule)
					{
						moduleInfo->mModule->startupModule();
						moduleInfo->mLoadOrder = ModuleInfo::mCurrentLoadOrder++;
						loadedModule = moduleInfo->mModule;
					}
					else
					{
						reason = EModuleLoadResult::FailedToInitialize;
					}
				}
				else
				{
					if (moduleInfo->mFileName.empty())
					{

					}
					const wstring ModuleFileToLoad = moduleInfo->mFileName;
					moduleInfo->mHandle = nullptr;
					if (Paths::fileExists(ModuleFileToLoad))
					{
						moduleInfo->mHandle = PlatformProcess::getDllHandle(ModuleFileToLoad.c_str());
						if (moduleInfo->mHandle != nullptr)
						{
							void* initPtr = PlatformProcess::getDllExport(moduleInfo->mHandle, TEXT("initializeModule"));
							if (initPtr != nullptr)
							{

								InitializerOfStaticLinkedModule initializeFunction = std::bind(static_cast<IModuleInterface*(*)()>(initPtr));

								if (moduleInfo->mModule)
								{
									loadedModule = moduleInfo->mModule;
								}
								else
								{
									moduleInfo->mModule.reset(initializeFunction());
									if (moduleInfo->mModule)
									{
										moduleInfo->mModule->startupModule();
										moduleInfo->mLoadOrder = ModuleInfo::mCurrentLoadOrder++;
										loadedModule = moduleInfo->mModule;
									}
									else
									{
										PlatformProcess::freeDllHandle(moduleInfo->mHandle);
										moduleInfo->mHandle = nullptr;
										reason = EModuleLoadResult::FailedToInitialize;
									}
								}
							}
							else
							{
								PlatformProcess::freeDllHandle(moduleInfo->mHandle);
								moduleInfo->mHandle = nullptr;
								reason = EModuleLoadResult::FailedToInitialize;
							}
						}
						else
						{
							reason = EModuleLoadResult::CouldNotBeLoadedByOS;
						}
					}
					else
					{
						reason = EModuleLoadResult::FileNotFound;
					}
				}
			}
		}
		return loadedModule;
	}

	std::shared_ptr<IModuleInterface> ModuleManager::loadModule(const wstring inModuleName, const bool bWasReleaded)
	{
		EModuleLoadResult failureReason;
		std::shared_ptr<IModuleInterface>Result = loadModuleWithFailureReason(inModuleName, failureReason, bWasReleaded);
		return Result;
	}

	std::shared_ptr<IModuleInterface> ModuleManager::loadModuleChecked(const wstring name, const bool bWasReloaded)
	{
		std::shared_ptr<IModuleInterface> module = loadModule(name, bWasReloaded);
		return module;
	}

	void ModuleManager::abandonModule(const wstring inModuleName)
	{
		ModuleInfoPtr infoPtr = findModule(inModuleName);
		if (infoPtr)
		{
			if (infoPtr->mModule)
			{
				infoPtr->mModule->shutdownModule();
				infoPtr->mModule.reset();
			}
		}
	}

	void ModuleManager::addModuleToModulesList(const wstring name, ModuleManager::ModuleInfoPtr& inModuleInfo)
	{
		{
			std::lock_guard<std::mutex> lock(mModulesCriticalSection);
			mModules.emplace(name, inModuleInfo);
		}
	}

	void ModuleManager::findModulePaths(std::wstring_view namePattern, TMap<wstring, wstring> & outModulePaths, bool bCanUseCache /* = true */) const
	{
		if (!mModulePathsCache)
		{
			mModulePathsCache = makeUniquePtr<TMap<wstring, wstring>>();
			const bool bCanUseCacheWhileGeneratingIt = false;
			findModulePaths(L"*", *mModulePathsCache, bCanUseCacheWhileGeneratingIt);
		}

		if (bCanUseCache)
		{
			wstring pattern = namePattern.data();
			for (const auto& pair : *mModulePathsCache)
			{
				if (StringUtil::matchesWildcard(pair.first, pattern, ESearchCase::IgnoreCase))
				{
					outModulePaths.emplace(pair.first, pair.second);
				}
			}
			if (outModulePaths.size() > 0)
			{
				return;
			}
		}
		findModulePathsInDirectory(PlatformProcess::getModulesDirectory(), false, namePattern, outModulePaths);

		for (int idx = 0; idx < mEngineBinariesDirectories.size(); idx++)
		{
			findModulePathsInDirectory(mEngineBinariesDirectories[idx], false, namePattern, outModulePaths);
		}

		for (int idx = 0; idx < mGameBinariesDirectories.size(); ++idx)
		{
			findModulePathsInDirectory(mGameBinariesDirectories[idx], true, namePattern, outModulePaths);
		}

	}

	void ModuleManager::getModuleFilenameFormat(bool bGameModule, wstring & outPrefix, wstring & outSuffix)
	{
		const TCHAR* configSuffix = nullptr;
		switch (App::getBuildConfiguration())
		{
		case EBuildConfiguaration::Debug:
			configSuffix = TEXT("-Debug");
			break;
		case EBuildConfiguaration::DebugGame:
			configSuffix = bGameModule ? TEXT("-DebugGame") : NULL;
			break;
		case EBuildConfiguaration::Development:
			configSuffix = NULL;
			break;
		case EBuildConfiguaration::Test:
			configSuffix = TEXT("-Test");
			break;
		case EBuildConfiguaration::Shipping:
			configSuffix = TEXT("-Shipping");
			break;
		default:
			break;
		}

		outPrefix = PlatformProcess::getModulePrefix() + Paths::getBaseFilename(PlatformProcess::executableName());
		if (outPrefix.find('-') >= 0)
		{
			outPrefix = outSuffix.substr(0, outPrefix.find('-') + 1);
		}
		else
		{
			outPrefix += TEXT("-");
		}

		outSuffix.clear();
		if (configSuffix != nullptr)
		{
			outSuffix += TEXT("-");
			outSuffix += PlatformProcess::getBinariesSubDirectory();
			outSuffix += configSuffix;
		}
		outSuffix += TEXT(".");
		outSuffix += PlatformProcess::getModuleExtension();
	}


	void ModuleManager::findModulePathsInDirectory(std::wstring_view inDirectoryName, bool bIsGameDirectory, std::wstring_view NamePattern, TMap<wstring, wstring> & outModulePaths) const
	{
		wstring modulePrefix, moduleSuffix;
		getModuleFilenameFormat(bIsGameDirectory, modulePrefix, moduleSuffix);

		TArray<wstring> fullFileNames;
		IFileManager::get().findFilesRecursive(fullFileNames, inDirectoryName.data(), (modulePrefix + NamePattern.data() + moduleSuffix).c_str(), true, false);

		for (int32 idx = 0; idx < fullFileNames.size(); idx++)
		{
			const wstring & fullFileName = fullFileNames[idx];
			wstring fileName = Paths::getFilename(fullFileName);
			if (boost::algorithm::starts_with(fileName, modulePrefix) && boost::algorithm::ends_with(fileName, moduleSuffix))
			{
				wstring moduleName = fileName.substr(modulePrefix.length(), fileName.length() - (modulePrefix.length() + moduleSuffix.length()));
				if (!boost::algorithm::ends_with(moduleName, "-Debug") && !boost::algorithm::ends_with(moduleName, "-Shipping") && !boost::algorithm::ends_with(moduleName, "-Test") && !boost::algorithm::ends_with(moduleName, "-DebugGame"))
				{
					outModulePaths.emplace(moduleName, fullFileName);
				}
			}
		}
	}




	void ModuleManager::addModule(const wstring inModuleName)
	{
		auto it = mModules.find(inModuleName);
		if (it != mModules.end())
		{
			return;
		}
		ModuleInfoPtr infoPtr = std::make_shared<ModuleInfo>();
		ModuleManager::get().addModuleToModulesList(inModuleName, infoPtr);
		TMap<wstring, wstring> modulePathMap;
		findModulePaths(inModuleName, modulePathMap);
		if (modulePathMap.size() != 1)
		{
			return;
		}
		wstring moduleFileName = modulePathMap.begin()->second;
		const auto matchPos = boost::algorithm::ifind_first(moduleFileName, inModuleName).begin() - moduleFileName.begin();
		if (matchPos == INDEX_NONE)
		{
			return;
		}

		const auto suffixStart = matchPos + inModuleName.length();
		auto suffixEnd = suffixStart;
		if (moduleFileName[suffixEnd] == TEXT('-'))
		{
			wstring substr = moduleFileName.substr(suffixEnd + 1);
			std::wsmatch m;
			std::wregex re(TEXT("^\\d*"));
			regex_search(substr, m, re);
			if (m.size() > 0)
			{
				suffixEnd += m[0].second - m[0].first;
			}
		}
		const wstring prefix = moduleFileName.substr(0, suffixStart);
		const wstring suffix = moduleFileName.substr(suffixEnd);
		infoPtr->mOriginalFilename = prefix + suffix;
		infoPtr->mFileName = infoPtr->mOriginalFilename;

		const time_t originalModuleFileTime = IFileManager::get().getTimeStamp(infoPtr->mOriginalFilename.c_str());
		if (originalModuleFileTime == 0)
		{
			return;
		}
		const wstring moduleFileSearchString = prefix + TEXT("-*") + suffix;
		TArray<wstring> foundFiles;
		IFileManager::get().findFiles(foundFiles, moduleFileSearchString.c_str(), true, false);
		if (foundFiles.size() == 0)
		{
			return;
		}

		const wstring moduleFileSearchDirectory = Paths::getPath(moduleFileSearchString);
		wstring newestModuleFilename;
		bool bFoundNewestFile = findNewestModuleFile(foundFiles, originalModuleFileTime, moduleFileSearchDirectory, prefix, suffix, newestModuleFilename);
		if (!bFoundNewestFile)
		{
			return;
		}
		const wstring newestModuleFilePath = moduleFileSearchDirectory.empty() ? newestModuleFilename : (moduleFileSearchDirectory + newestModuleFilename);
		infoPtr->mFileName = newestModuleFilePath;
	}

	void ModuleManager::findModules(const TCHAR* wildCardWithoutExtension, TArray<wstring>& outModules) const
	{
#if !IS_MONOLITHIC
		TMap<wstring, wstring> modulePaths;
		findModulePaths(wildCardWithoutExtension, modulePaths);
		for (auto& iter : modulePaths)
		{
			if (checkModuleCompatibility(iter.second.c_str()))
			{
				outModules.add(iter.first);
			}
		}
#endif
	}

	bool ModuleManager::checkModuleCompatibility(const TCHAR* filename)
	{
		int32 moduleApiVersion = PlatformProcess::getDllApiVersion(filename);
		int32 compiledInApiVersion = MODULE_API_VERSION;
		if (moduleApiVersion != compiledInApiVersion)
		{
			AIR_LOG(LogModuleManager, Warning, TEXT("Found Module file"));
			return true;
		}
		return true;
	}
}