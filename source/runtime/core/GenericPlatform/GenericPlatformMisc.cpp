#include "GenericPlatform/GenericPlatformMisc.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "GenericPlatform/GenericApplication.h"
#include "Misc/Paths.h"
#include "boost/algorithm/string.hpp"
#include "CoreGlobals.h"
#include "HAL/PlatformTime.h"
#include "Misc/Guid.h"
#include <algorithm>
namespace Air
{
	
	int32 GenericPlatformMisc::numberOfCoresIncludingHyperthreads()
	{
		return PlatformMisc::numberOfCores();
	}

	int32 GenericPlatformMisc::numberOfWorkerThreadsToSpawn()
	{
		static int32 MaxGameThreads = 4;
		static int32 MaxThreads = 16;
		int32 numberOfCores = PlatformMisc::numberOfCores();
		int32 maxworkerThreadsWanted = (MaxGameThreads);

		return std::max(std::min(numberOfCores - 1, maxworkerThreadsWanted), 1);
	}

	void GenericPlatformMisc::setEnvironmentVar(const TCHAR* variableName, const TCHAR* value)
	{

	}

	const TCHAR* GenericPlatformMisc::getPathVarDelimiter()
	{
		return TEXT(";");
	}

	const TCHAR* GenericPlatformMisc::engineDir()
	{
		static wstring engineDirectory = L"";
		if (engineDirectory.length() == 0)
		{
			wstring defaultEngineDir = TEXT("../../");
#if PLATFORM_DESKTOP
			PlatformProcess::setCurrentWorkingDirectoryToBaseDir();
			if (Paths::directoryExists(defaultEngineDir + TEXT("bin")))
			{
				engineDirectory = defaultEngineDir;
			}
			if (engineDirectory.length() == 0)
			{
				engineDirectory = defaultEngineDir;

			}
		
#else
			engineDirectory = defaultEngineDir;
#endif
		}
		return engineDirectory.c_str();
	}

	void GenericPlatformMisc::cacheLaunchDir()
	{
		static bool bOneTime = false;
		if (bOneTime)
		{
			return;
		}
		bOneTime = true;
		getWrappedLaunchDir() = PlatformProcess::getCurrentWorkingDirectory() + TEXT("/");
	}

	const TCHAR* GenericPlatformMisc::rootDir()
	{
		static wstring path;
		if (path.length() == 0)
		{
			path = Paths::engineDir();
			
		}
		return path.c_str();
	}

	void GenericPlatformMisc::requestExit(bool force)
	{
		if (force)
		{
			abort();
		}
		else
		{
			GIsRequestingExit = 1;
		}
	}

	void GenericPlatformMisc::memoryBarrier()
	{

	}

	void GenericPlatformMisc::createGuid(Guid& result)
	{
		static uint16 incrementCounter = 0;
		int32 year = 0, month = 0, dayOffWeek = 0, day = 0, hour = 0, min = 0, sec = 0, msec = 0;
		uint32 sequentialBits = static_cast<uint32>(incrementCounter++);
		uint32 randBits = Math::rand() & 0xffff;
		PlatformTime::systemTime(year, month, dayOffWeek, day, hour, min, sec, msec);

		result = Guid(randBits | (sequentialBits << 16), day | (hour << 8) | (hour << 8) | (month << 16) | (sec << 24), msec | (min << 16), year ^PlatformTime::cycles());

	}

	uint32 GenericPlatformMisc::getStandardPrintableKeyMap(uint32* keyCodes, wstring* keyNames, uint32 maxMappings, bool bMapUppercaseKeys, bool bMapLowercaseKeys)
	{
		uint32 numMappings = 0;
#define ADDKEYMAP(keyCode, keyName) if(numMappings < maxMappings){keyCodes[numMappings] = keyCode; keyNames[numMappings] = keyName; ++numMappings;};

		ADDKEYMAP('0', TEXT("Zero"));
		ADDKEYMAP('1', TEXT("One"));
		ADDKEYMAP('2', TEXT("Two"));
		ADDKEYMAP('3', TEXT("Three"));
		ADDKEYMAP('4', TEXT("Four"));
		ADDKEYMAP('5', TEXT("Five"));
		ADDKEYMAP('6', TEXT("Six"));
		ADDKEYMAP('7', TEXT("Seven"));
		ADDKEYMAP('8', TEXT("Eight"));
		ADDKEYMAP('9', TEXT("Nine"));

		if (bMapUppercaseKeys)
		{
			ADDKEYMAP('A', TEXT("A"));
			ADDKEYMAP('B', TEXT("B"));
			ADDKEYMAP('C', TEXT("C"));
			ADDKEYMAP('D', TEXT("D"));
			ADDKEYMAP('E', TEXT("E"));
			ADDKEYMAP('F', TEXT("F"));
			ADDKEYMAP('G', TEXT("G"));
			ADDKEYMAP('H', TEXT("H"));
			ADDKEYMAP('I', TEXT("I"));
			ADDKEYMAP('J', TEXT("J"));
			ADDKEYMAP('K', TEXT("K"));
			ADDKEYMAP('L', TEXT("L"));
			ADDKEYMAP('M', TEXT("M"));
			ADDKEYMAP('N', TEXT("N"));
			ADDKEYMAP('O', TEXT("O"));
			ADDKEYMAP('P', TEXT("P"));
			ADDKEYMAP('Q', TEXT("Q"));
			ADDKEYMAP('R', TEXT("R"));
			ADDKEYMAP('S', TEXT("S"));
			ADDKEYMAP('T', TEXT("T"));
			ADDKEYMAP('U', TEXT("U"));
			ADDKEYMAP('V', TEXT("V"));
			ADDKEYMAP('W', TEXT("W"));
			ADDKEYMAP('X', TEXT("X"));
			ADDKEYMAP('Y', TEXT("Y"));
			ADDKEYMAP('Z', TEXT("Z"));

		}


		if (bMapLowercaseKeys)
		{
			ADDKEYMAP('a', TEXT("A"));
			ADDKEYMAP('b', TEXT("B"));
			ADDKEYMAP('c', TEXT("C"));
			ADDKEYMAP('d', TEXT("D"));
			ADDKEYMAP('e', TEXT("E"));
			ADDKEYMAP('f', TEXT("F"));
			ADDKEYMAP('g', TEXT("G"));
			ADDKEYMAP('h', TEXT("H"));
			ADDKEYMAP('i', TEXT("I"));
			ADDKEYMAP('j', TEXT("J"));
			ADDKEYMAP('k', TEXT("K"));
			ADDKEYMAP('l', TEXT("L"));
			ADDKEYMAP('m', TEXT("M"));
			ADDKEYMAP('n', TEXT("N"));
			ADDKEYMAP('o', TEXT("O"));
			ADDKEYMAP('p', TEXT("P"));
			ADDKEYMAP('q', TEXT("Q"));
			ADDKEYMAP('r', TEXT("R"));
			ADDKEYMAP('s', TEXT("S"));
			ADDKEYMAP('t', TEXT("T"));
			ADDKEYMAP('u', TEXT("U"));
			ADDKEYMAP('v', TEXT("V"));
			ADDKEYMAP('w', TEXT("W"));
			ADDKEYMAP('x', TEXT("X"));
			ADDKEYMAP('y', TEXT("Y"));
			ADDKEYMAP('z', TEXT("Z"));

		}

		ADDKEYMAP(';', TEXT("Semicolon"));
		ADDKEYMAP('=', TEXT("Equals"));
		ADDKEYMAP(',', TEXT("Comma"));
		ADDKEYMAP('-', TEXT("Hyphen"));
		ADDKEYMAP('.', TEXT("Period"));
		ADDKEYMAP('/', TEXT("Slash"));
		ADDKEYMAP('`', TEXT("Tilde"));
		ADDKEYMAP('[', TEXT("LeftBracket"));
		ADDKEYMAP(']', TEXT("RightBracket"));
		ADDKEYMAP('\\', TEXT("Backslash"));
		ADDKEYMAP('\'', TEXT("Apostrophe"));
		ADDKEYMAP(' ', TEXT("SpaceBar"));


		ADDKEYMAP('&', TEXT("Ampersand"));
		ADDKEYMAP('*', TEXT("Asterix"));
		ADDKEYMAP('^', TEXT("Caret"));
		ADDKEYMAP(':', TEXT("Colon"));
		ADDKEYMAP('$', TEXT("Dollar"));
		ADDKEYMAP('!', TEXT("Exclamation"));
		ADDKEYMAP('(', TEXT("LeftParantheses"));
		ADDKEYMAP(')', TEXT("RightParantheses"));
		ADDKEYMAP('"', TEXT("Quote"));
		ADDKEYMAP('_', TEXT("Underscore"));
		ADDKEYMAP(224, TEXT("A_AccentGrave"));
		ADDKEYMAP(231, TEXT("C_Cedille"));
		ADDKEYMAP(233, TEXT("E_AccentAigu"));
		ADDKEYMAP(232, TEXT("E_AccentGrave"));
		ADDKEYMAP(167, TEXT("Section"));
		return numMappings;
	}
}