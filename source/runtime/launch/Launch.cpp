#include "CoreMinimal.h"

#include "LaunchEngineLoop.h"
#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#endif
namespace Air
{

	EngineLoop GEngineLoop;

	void engineExit(void)
	{
		GIsRequestingExit = true;
		GEngineLoop.exit();
	}

	int32 enginePreInit(const TCHAR* cmdLine)
	{
		int32 errorLevel = GEngineLoop.preInit(cmdLine);
		return errorLevel;
	}

	int32 engineInit()
	{
		int32 errorLevel = GEngineLoop.init();
		return errorLevel;
	}

	void engineTick()
	{
		GEngineLoop.tick();
	}


#if PLATFORM_WINDOWS
	int32 guardedMain(const TCHAR* cmdLine, HINSTANCE hInInstance, HINSTANCE hPrevInstatance, int32 nCmdShow)
#else
	int32 guardedMain(const TCHAR* cmdLine)
#endif
	{
		struct EngineLoopCleanupGuard
		{
			~EngineLoopCleanupGuard()
			{
				engineExit();
			}
		}CleanupGuard;

		int32 ErrorLevel = enginePreInit(cmdLine);
		if (ErrorLevel != 0)
		{
			return ErrorLevel;
		}

		{
			ErrorLevel = engineInit();
		}



		while (!GIsRequestingExit)
		{
			engineTick();
		}
		return 0;
	}

}