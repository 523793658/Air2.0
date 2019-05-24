#include "CoreGlobals.h"
#include "HAL/PlatformTime.h"
namespace Air
{
	static void appNoop()
	{

	}


	bool			GIsRequestingExit = false;

	bool GIsBuildMachine = false;

	CORE_API Malloc*			GMalloc = nullptr;

	float GNearClippingPlane = 0.1f;

	uint32 GGameThreadId = 0;
	uint32 GRenderThreadId = 0;
	uint32 GSlateLoadingThreadId = 0;
	bool GIsRunning = false;

	bool GIsClient = false;

#if WITH_EDITORONLY_DATA
	bool			GIsEditor = false;
#endif

	bool			GIsDemo = false;

	wstring		GGameIni;

	wstring		GEngineIni;

	bool GIntraFrameDebuggingGameThread = false;

	bool GIsGameThreadIdInitialized = false;

	bool GPumpingMessagesOutsideOfMainLoop = false;

	bool GIsCriticalError = false;

	bool GDiscardUnusedQualityLevels = true;

	std::mutex mMutex_GameIdleTime;
	std::mutex mMutex_PumpMessages;
	std::mutex mMutex_FrameSyncTime;
	ConfigCacheIni*				GConfig = nullptr;


	uint32 GFrameNumber = 1;
	uint32 GFrameNumberRenderThread = 1;

	uint64 GFrameCounter = 0;

	double GStartTime = PlatformTime::initTiming();

	void(*GFlushStreamingFunc)(void) = &appNoop;
	void(*resumeAsyncLoading)() = &appNoop;
	void(*suspendAsyncLoading)() = &appNoop;

	static bool IsInAsyncLoadingThreadCoreInternal()
	{
		return false;
	}



	bool(*isInAsyncLoadingThread)() = &IsInAsyncLoadingThreadCoreInternal;



	static bool IsAsyncLoadingCoreInternal()
	{
		return false;
	}

	bool(*isAsyncLoadingMultithreaded)() = &IsAsyncLoadingCoreInternal;

	bool(*isAsyncLoading)() = &IsAsyncLoadingCoreInternal;
}