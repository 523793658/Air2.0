#include "CommandLine.h"
#include "Misc/CString.h"
#include "Template/AirTemplate.h"
#include "Misc/CoreMisc.h"
namespace Air
{
	bool CommandLine::bIsInitialized = false;

	TCHAR CommandLine::mCmdLine[CommandLine::MaxCommandLineSize] = TEXT("");
	TCHAR CommandLine::mOriginalCmdLine[CommandLine::MaxCommandLineSize] = TEXT("");
	TCHAR CommandLine::mLoggingCmdLine[CommandLine::MaxCommandLineSize] = TEXT("");
	TCHAR CommandLine::mLoggingOriginalCmdLine[CommandLine::MaxCommandLineSize] = TEXT("");

	const TCHAR* CommandLine::get()
	{
		return mCmdLine;
	}

	bool CommandLine::set(const TCHAR* newCommandLine)
	{
		if (!bIsInitialized)
		{
			CString::strncpy(mOriginalCmdLine, newCommandLine, ARRAY_COUNT(mOriginalCmdLine));
			CString::strncpy(mLoggingOriginalCmdLine, newCommandLine, ARRAY_COUNT(mLoggingOriginalCmdLine));
		}

		CString::strncpy(mCmdLine, newCommandLine, ARRAY_COUNT(mCmdLine));
		CString::strncpy(mLoggingCmdLine, newCommandLine, ARRAY_COUNT(mLoggingCmdLine));
		WhitelistCommandLines();

		bIsInitialized = true;

		if (stringHasBadDashes(newCommandLine))
		{
			return false;
		}
		return true;
	}
}