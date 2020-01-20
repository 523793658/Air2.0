#pragma once

#include "CoreType.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Windows/WindowsSystemIncludes.h"
#include "Containers/Array.h"

namespace Air
{
	struct ProcHandle : public TProcHandle<Windows::HANDLE, nullptr>
	{
	public:
		FORCEINLINE ProcHandle()
			:TProcHandle()
		{}

		FORCEINLINE explicit ProcHandle(HandleType other)
			:TProcHandle(other)
		{

		}

		FORCEINLINE bool close();
	};



	struct CORE_API WindowsPlatformProcess
		: public GenericPlatformProcess
	{
	public:
		static void sleepNoStats(float seconds);

		static class Event* createSynchEvent(bool bIsManualReset = false);

		static class RunnableThread* createRunnableThread();

		static void setThreadAffinityMask(uint64 affinityMask);

		static void sleep(float seconds);

		static wstring getCurrentWorkingDirectory();

		static const TCHAR* executableName(bool bRemoveExtension = true);

		static bool isProcRunning(ProcHandle& processHandle);

		static void resolveImportsRecursive(const wstring& fileName, const TArray<wstring>& searchPaths, TArray<wstring>& importFileNames, TArray<string>& visitedImportNames);

		static bool readLibraryImports(const TCHAR* fileName, TArray<string>& importNames);

		static bool resolveImport(const string name, const TArray<wstring>& searchPaths, wstring& outFileName);

		static void* getDllHandle(const TCHAR* fileName);

		static void freeDllHandle(void*);

		static void* loadLibraryWithSearchPaths(const wstring& fileName, const TArray<wstring>& searchPaths);

		static void* getDllExport(void* dllHandle, const TCHAR* ProcName);

		static const TCHAR* getBinariesSubDirectory();

		static const TCHAR* getModuleExtension();

		static const TCHAR* baseDir();

		static int32 getDllApiVersion(const TCHAR* filename);

		static void setCurrentWorkingDirectoryToBaseDir();

		static void closeProc(ProcHandle& processHandle);

		static void terminateProc(ProcHandle& processHandle, bool killTree = false);

		static ProcHandle createProc(const TCHAR* url, const TCHAR* params, bool bLaunchDetached, bool bLaunchHidden, bool bLaunchReallyHidden, uint32* outProcessId, int32 priorityModifier, const TCHAR* optionalWorkingDirectory, void* pipeWriteChild, void * pipeReadChile = nullptr);

		static uint32 getCurrentProcessId();

		static const wstring shaderWorkingDir();

		static const TCHAR* userTempDir();

	private:
		static TArray<wstring> mDllDirectoryStack;

		static TArray<wstring> mDllDirectories; 
	};


	typedef WindowsPlatformProcess PlatformProcess;

	inline bool ProcHandle::close()
	{
		if (isValid())
		{
			PlatformProcess::closeProc(*this);
			return true;
		}
		return false;
	}
}