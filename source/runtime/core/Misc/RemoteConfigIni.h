#pragma once
#include "CoreType.h"
#include "DateTime.h"
#include "Containers/String.h"
#include "Containers/Map.h"
#include "Containers/Array.h"
namespace Air
{
	class ConfigFile;

	struct RemoteConfigAsyncIOInfo
	{
		RemoteConfigAsyncIOInfo() {}

		RemoteConfigAsyncIOInfo(const TCHAR* inDefaultIniFile);

		RemoteConfigAsyncIOInfo& operator=(const RemoteConfigAsyncIOInfo& other);

		wstring mBuffer;
		DateTime mTimeStamp;

		double mStartReadTime;
		double mStartWriteTime;
		bool bReadIOFailed;
		bool bWasProcessed;

		TCHAR mDefaultIniFile[1024];
	};

	class RemoteConfig
	{
	public:
		RemoteConfig();

		RemoteConfigAsyncIOInfo* findConfig(const TCHAR* filename);
	private:
		wstring generateRemotePath(const TCHAR* filename);

		TMap<wstring, RemoteConfigAsyncIOInfo> mConfigBuffers;

		float mTimeout;

		bool bIsEnabled;

		bool bHasCachedFilenames;

		TArray<wstring> mCachedFilenames;
	};


	bool isUsingLocalIniFile(const TCHAR* filenameToLoad, const TCHAR* iniFileName);

	void processIniContents(const TCHAR* filenameToLoad, const TCHAR* iniFilename, ConfigFile* config, bool bDoEmptyConfig, bool bDoCombine);


}