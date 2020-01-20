#pragma once
#include "CoreType.h"
#include "Containers/String.h"
namespace Air
{
	class GenericApplication;

	namespace EBuildConfiguaration
	{
		enum Type
		{
			Unknown,
			Debug,
			DebugGame,
			Development,
			Shipping,
			Test
		};
	}

	namespace EAppReturnType
	{
		enum Type
		{
			No,
			Yes,
			YesAll,
			NoAll,
			Cancel,
			Ok,
			Retry,
			Continue,
		};
	}

	namespace EAppMsgType
	{
		enum Type
		{
			Ok,
			YesNo,
			OkCancel,
			YesNoCancel,
			CancelRetryContinue,
			YesNoYesAllNoAll,
			YesNoYesAllNoAllCancel,
			YesNoYesAll,
		};
	}


	struct CORE_API GenericPlatformMisc
	{
	public:
		static int32 numberOfWorkerThreadsToSpawn();

		static int32 numberOfCoresIncludingHyperthreads();

		static int32 numberOfCores()
		{
			return 1;
		}

		static bool allowRenderThread()
		{
			return true;
		}

		static const TCHAR* engineDir();

		static GenericApplication* createApplication();

		static void* getHardwareWindow()
		{
			return nullptr;
		}

		static void cacheLaunchDir();

		static void pumpMessages(bool bFromMainLoop);

		static wstring& getWrappedLaunchDir()
		{
			static wstring LaunchDir;
			return LaunchDir;
		}

		static void getEnvironmentVariable(const TCHAR* variableName, TCHAR* result, int32 resultLength)
		{
			*result = 0;
		}


		static void setEnvironmentVar(const TCHAR* variableName, const TCHAR* value);

		static const TCHAR* getPathVarDelimiter();
		static bool isDebuggerPresent() 
		{ 
			return 1;
		};

		static void normalizePath(wstring& inPath)
		{}

		static void createGuid(struct Guid& result);

		static const TCHAR* rootDir();

		static void requestExit(bool force);

		FORCEINLINE static void memoryBarrier();

		static uint32 getStandardPrintableKeyMap(uint32* keyCodes, wstring* keyNames, uint32 maxMappings, bool bMapUppercaseKeys, bool bMapLowercaseKeys);

		static EAppReturnType::Type messageBoxExt(EAppMsgType::Type msgType, const TCHAR* text, const TCHAR* caption);
	};
}