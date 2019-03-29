#pragma once
#include "CoreType.h"
#include "Containers/String.h"
namespace Air
{
	struct ProcHandle;

	struct CORE_API GenericPlatformProcess
	{
	public:
		static bool supportsMultithreading();

		static class Event* getSynchEventFromPool(bool bIsManualReset = false);

		static class Event* createSynchEvent(bool bIsManualReset = false);

		static void returnSynchEventToPool(Event* event);

		static class RunnableThread* createRunnableThread();

		static void setThreadAffinityMask(uint64 affinityMask);

		static void sleep(float seconds);

		static ProcHandle createProc(const TCHAR* url, const TCHAR* params, bool bLaunchDetached, bool bLaunchHidden, bool bLaunchReallyHidden, uint32* outProcessId, int32 priorityModifier, const TCHAR* optionalWorkingDirectory, void* pipeWriteChild, void * pipeReadChile = nullptr);

		static const wstring getModulesDirectory();

		static wstring getCurrentWorkingDirectory();

		static const TCHAR* getModulePrefix();

		static void setupGameThread() {}

		static bool isProcRunning(ProcHandle& processHandle) { return false; }

		static void setupRenderThread(){}

		static void SetRealTimeMode() { }

		static void* getDllHandle(const TCHAR* fileName) { return nullptr; };

		static void freeDllHandle(void*);

		static void* getDllExport(void* dllHandle, const char* ProcName) { return nullptr; }

		static const TCHAR* getBinariesSubDirectory() { return nullptr; };
		static const TCHAR* getModuleExtension() { return nullptr; }

		static const TCHAR* shaderDir();
	};

	template<typename T, T InvalidHandleValue>
	struct TProcHandle
	{
		typedef T HandleType;
	public:
		FORCEINLINE TProcHandle()
			:mHandle(InvalidHandleValue)
		{}

		FORCEINLINE explicit TProcHandle(T other)
			:mHandle(other)
		{
			
		}

		FORCEINLINE TProcHandle & operator = (const TProcHandle& other)
		{
			if (this != &other)
			{
				mHandle = other.mHandle;
			}
			return *this;
		}

		FORCEINLINE T get() const
		{
			return mHandle;
		}

		FORCEINLINE void reset()
		{
			mHandle = InvalidHandleValue;
		}

		FORCEINLINE bool isValid() const
		{
			return mHandle != InvalidHandleValue;
		}

		FORCEINLINE bool close()
		{
			return isValid();
		}

	protected:
		T mHandle;
	};
}