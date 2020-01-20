#pragma once
#include "CoreType.h"
namespace Air
{
	struct CORE_API CommandLine
	{
		static const TCHAR* get();

		static bool set(const TCHAR* newCommandLine);

		static const uint32 MaxCommandLineSize = 16384;

	private:
#if 0
#else
#define WhitelistCommandLines()
#endif

		static bool bIsInitialized;

		static TCHAR mOriginalCmdLine[MaxCommandLineSize];

		static TCHAR mLoggingCmdLine[MaxCommandLineSize];

		static TCHAR mLoggingOriginalCmdLine[MaxCommandLineSize];

		static TCHAR mCmdLine[MaxCommandLineSize];
	};
}