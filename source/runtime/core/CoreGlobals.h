#pragma once
#include "CoreType.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformTLS.h"
#include "Containers/String.h"
#include "Misc/ConfigCacheIni.h"
#include <mutex>
namespace Air
{
	class RunnableThread;

	extern CORE_API bool GIsRequestingExit;

	extern CORE_API bool GIsBuildMachine;
	
	extern CORE_API float GNearClippingPlane;

	extern CORE_API uint32 GGameThreadId;
	extern CORE_API uint32 GRenderThreadId;
	extern CORE_API uint32 GSlateLoadingThreadId;



	extern CORE_API int32 GIsRenderingThreadSuspended;

	extern CORE_API RunnableThread* GRenderingThread;

	extern CORE_API RunnableThread* GRHIThread;

	extern CORE_API bool GIsGameThreadIdInitialized;

	extern CORE_API bool GIsRunning;

	extern CORE_API bool GIsClient;

	extern CORE_API bool GIsCriticalError;

	extern CORE_API bool GDiscardUnusedQualityLevels;

	extern CORE_API bool GPumpingMessagesOutsideOfMainLoop;

	extern CORE_API uint32 GFrameNumber;
	extern CORE_API uint32 GFrameNumberRenderThread;

	extern CORE_API uint64 GFrameCounter;

	extern CORE_API bool GIsEditor;

	extern CORE_API bool GIntraFrameDebuggingGameThread;

	extern CORE_API bool GIsDemo;

	extern CORE_API wstring GGameIni;

	extern CORE_API wstring GEngineIni;

	extern CORE_API ConfigCacheIni*		GConfig;

	extern CORE_API bool isInRenderingThread();

	extern CORE_API bool isInSlateThread();

	extern CORE_API bool isInParallelRenderingThread();

	extern CORE_API bool isInRHIThread();

	extern CORE_API bool isInActualRenderinThread();

	extern CORE_API bool(*isAsyncLoadingMultithreaded)();

	extern CORE_API void(*suspendAsyncLoading)();

	extern CORE_API bool(*isInAsyncLoadingThread)();

	extern CORE_API void(*resumeAsyncLoading)();

	extern CORE_API void(*GFlushStreamingFunc)(void);

	extern CORE_API bool(*isAsyncLoading)();

	extern CORE_API std::mutex mMutex_GameIdleTime;
	extern CORE_API std::mutex mMutex_PumpMessages;
	extern CORE_API std::mutex mMutex_FrameSyncTime;
	FORCEINLINE bool isInGameThread()
	{
		if (GIsGameThreadIdInitialized)
		{
			const uint32 currentThreadId = PlatformTLS::getCurrentThreadId();
			return currentThreadId == GGameThreadId;
		}
		return true;
	}


}