#pragma once
#include "CoreType.h"
#include "GenericPlatform/GenericPlatformMisc.h"

namespace Air
{
	struct CORE_API WindowsPlatformMisc : public GenericPlatformMisc
	{
	public:
		static int32 numberOfCores();

		static int32 numberOfCoresIncludingHyperthreads();

		static bool isDebuggerPresent();

		static GenericApplication* createApplication();

		static void pumpMessages(bool bFromMainLoop);

		static bool getDiskTotalAndFreeSpace(const wstring& inPath, uint64 & totalNumOfBytes, uint64& numberOfFreeBytes);

		FORCEINLINE static void debugBreak()
		{
			if (isDebuggerPresent())
			{
				__debugbreak();
			}
		}

		FORCEINLINE static void prefetch(void const* x, int32 offset = 0)
		{
			_mm_prefetch((char const*)(x)+offset, _MM_HINT_T0);
		}

		static struct GPUDriverInfo getGPUDriverInfo(const wstring & deviceDescription);

		static void requestExit(bool force);
	
		static uint32 getCharKeyMap(uint32* keyCodes, wstring* keyNames, uint32 maxMappings);
		static uint32 getKeyMap(uint32* keyCodes, wstring* keyNames, uint32 maxMappings);

		static void getEnvironmentVariable(const TCHAR* variableName, TCHAR* result, int32 resultLength);

		static void setEnvironmentVar(const TCHAR* variableName, const TCHAR* value);

		static bool verifyWindowsVersion(uint32 majorVersion, uint32 minorVersion);

		static EAppReturnType::Type WindowsPlatformMisc::messageBoxExt(EAppMsgType::Type msgType, const TCHAR* text, const TCHAR* caption);
	};

	typedef WindowsPlatformMisc PlatformMisc;
}
