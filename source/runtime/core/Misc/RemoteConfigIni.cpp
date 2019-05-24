#include "RemoteConfigIni.h"
#include "ConfigCacheIni.h"
namespace Air
{
	RemoteConfig GRemoteConfig;

	RemoteConfig::RemoteConfig()
		:mTimeout(-1.0f)
		, bIsEnabled(true)
		, bHasCachedFilenames(false)
	{}

	RemoteConfigAsyncIOInfo::RemoteConfigAsyncIOInfo(const TCHAR* inDefaultIniFile)
		:bReadIOFailed(false)
		, bWasProcessed(false)
	{
		CString::strcpy(mDefaultIniFile, inDefaultIniFile);
	}

	RemoteConfigAsyncIOInfo& RemoteConfigAsyncIOInfo::operator=(const RemoteConfigAsyncIOInfo& other)
	{
		mBuffer = other.mBuffer;
		mTimeStamp = other.mTimeStamp;
		mStartReadTime = other.mStartReadTime;
		mStartWriteTime = other.mStartWriteTime;
		bReadIOFailed = other.bReadIOFailed;
		bWasProcessed = other.bWasProcessed;
		Memory::memcpy(mDefaultIniFile, other.mDefaultIniFile, 1024);
		return *this;
	}

	RemoteConfigAsyncIOInfo* RemoteConfig::findConfig(const TCHAR* filename)
	{
		auto &it = mConfigBuffers.find(filename);
		if (it == mConfigBuffers.end())
		{
			return nullptr;
		}
		return &it->second;
	}

	bool isUsingLocalIniFile(const TCHAR* filenameToLoad, const TCHAR* iniFileName)
	{
		BOOST_ASSERT(filenameToLoad);
		RemoteConfigAsyncIOInfo* remoteInfo = GRemoteConfig.findConfig(filenameToLoad);
		bool bIsGeneratedFile = iniFileName ? !CString::stricmp(filenameToLoad, iniFileName) : true;
		return !remoteInfo || !bIsGeneratedFile || (remoteInfo && remoteInfo->bReadIOFailed);
	}

	void processIniContents(const TCHAR* filenameToLoad, const TCHAR* iniFilename, ConfigFile* config, bool bDoEmptyConfig, bool bDoCombine)
	{
		BOOST_ASSERT(filenameToLoad);
		BOOST_ASSERT(iniFilename);
		BOOST_ASSERT(config);

		if (isUsingLocalIniFile(filenameToLoad, iniFilename))
		{
			if (bDoCombine)
			{
				config->combine(iniFilename);
			}
			else
			{
				config->read(iniFilename);
			}
		}
		else
		{
			RemoteConfigAsyncIOInfo* remoteInfo = GRemoteConfig.findConfig(filenameToLoad);
			if (bDoEmptyConfig)
			{
				config->empty();
			}

			if (bDoCombine)
			{
				config->combineFromBuffer(remoteInfo->mBuffer);
			}
			else
			{
				config->processInputFileContents(remoteInfo->mBuffer);
			}
		}
	}
}