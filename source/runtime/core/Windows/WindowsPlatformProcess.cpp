#include "Windows/WindowsPlatformProcess.h"

#include "Windows/WindowsRunnableThread.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/WindowsEvent.h"
#include "Template/AirTemplate.h"
#include "Misc/Paths.h"
#include "HAL/PlatformMisc.h"
#include "Logging/LogMacros.h"
#include <shellapi.h>
#include <ShlObj.h>
#include <LM.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <algorithm>
#include "boost/lexical_cast.hpp"
#include <iostream>
#include "boost/algorithm/string.hpp"
namespace Air
{

#define ID_MODULE_API_VERSION_RESOURCE 191

	TArray<wstring> WindowsPlatformProcess::mDllDirectoryStack;
	TArray<wstring> WindowsPlatformProcess::mDllDirectories;


	const void* mapRvaToPointer(const IMAGE_DOS_HEADER* header, const IMAGE_NT_HEADERS* ntHeader, size_t rva)
	{
		const IMAGE_SECTION_HEADER* sectionHeaders = (const IMAGE_SECTION_HEADER*)(ntHeader + 1);
		for (size_t sectionIdx = 0; sectionIdx < ntHeader->FileHeader.NumberOfSections; sectionIdx++)
		{
			const IMAGE_SECTION_HEADER* sectionHeader = sectionHeaders + sectionIdx;
			if (rva >= sectionHeader->VirtualAddress && rva < sectionHeader->VirtualAddress + sectionHeader->SizeOfRawData)
			{
				return (const BYTE*)header + sectionHeader->PointerToRawData + (rva - sectionHeader->VirtualAddress);
			}
		}
		return NULL;
	}

	bool readLibraryImportsFromMemory(const IMAGE_DOS_HEADER * header, TArray<string> & importNames)
	{
		bool result = false;
		if (header->e_magic == IMAGE_DOS_SIGNATURE)
		{
			IMAGE_NT_HEADERS *NTHeader = (IMAGE_NT_HEADERS*)((BYTE*)header + header->e_lfanew);
			if (NTHeader->Signature == IMAGE_NT_SIGNATURE)
			{
				IMAGE_DATA_DIRECTORY *importDirectoryEntry = &NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
				IMAGE_IMPORT_DESCRIPTOR *importDescripters = (IMAGE_IMPORT_DESCRIPTOR*)mapRvaToPointer(header, NTHeader, importDirectoryEntry->VirtualAddress);
				for (size_t importIdx = 0; importIdx * sizeof(IMAGE_IMPORT_DESCRIPTOR) < importDirectoryEntry->Size; importIdx++)
				{
					IMAGE_IMPORT_DESCRIPTOR *importDescriptor = importDescripters + importIdx;
					if (importDescriptor->Name != 0)
					{
						const char * importName = (const char*)mapRvaToPointer(header, NTHeader, importDescriptor->Name);
						importNames.push_back(importName);
					}
				}
				result = true;
			}
		}
		return result;
	}

	void WindowsPlatformProcess::sleepNoStats(float seconds)
	{
		uint32 milliseconds = (uint32)(seconds * 1000.0);
		if (milliseconds == 0)
		{
			::SwitchToThread();
		}
		::Sleep(milliseconds);
	}

	void WindowsPlatformProcess::terminateProc(ProcHandle& processHandle, bool killTree)
	{
		if (killTree)
		{
			HANDLE snapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			if (snapShot != INVALID_HANDLE_VALUE)
			{
				::DWORD processId = ::GetProcessId(processHandle.get());
				PROCESSENTRY32 entry;
				entry.dwSize = sizeof(PROCESSENTRY32);
				if (::Process32First(snapShot, &entry))
				{
					do
					{
						if (entry.th32ParentProcessID == processId)
						{
							HANDLE childProcHandle = ::OpenProcess(PROCESS_ALL_ACCESS, 0, entry.th32ProcessID);
							if (childProcHandle)
							{
								ProcHandle childHandle(childProcHandle);
								terminateProc(childHandle, killTree);
							}
						}
					} while (::Process32Next(snapShot, &entry));
				}
			}
		}
		TerminateProcess(processHandle.get(), 0);
	}

	void WindowsPlatformProcess::closeProc(ProcHandle& processHandle)
	{
		if (processHandle.isValid())
		{
			::CloseHandle(processHandle.get());
			processHandle.reset();
		}
	}

	bool WindowsPlatformProcess::isProcRunning(ProcHandle& processHandle)
	{
		bool bApplicationRunning = true;
		uint32 waitResult = ::WaitForSingleObject(processHandle.get(), 0);
		if (waitResult != WAIT_TIMEOUT)
		{
			bApplicationRunning = false;
		}
		return bApplicationRunning;
	}

	ProcHandle WindowsPlatformProcess::createProc(const TCHAR* url, const TCHAR* params, bool bLaunchDetached, bool bLaunchHidden, bool bLaunchReallyHidden, uint32* outProcessId, int32 priorityModifier, const TCHAR* optionalWorkingDirectory, void* pipeWriteChild, void * pipeReadChile /* = nullptr */)
	{
		SECURITY_ATTRIBUTES attr;
		attr.nLength = sizeof(SECURITY_ATTRIBUTES);
		attr.lpSecurityDescriptor = nullptr;
		attr.bInheritHandle = true;

		uint32 createFlags = NORMAL_PRIORITY_CLASS;
		if (priorityModifier < 0)
		{
			createFlags = (priorityModifier == -1) ? BELOW_NORMAL_PRIORITY_CLASS : IDLE_PRIORITY_CLASS;
		}
		else if (priorityModifier > 0)
		{
			createFlags = (priorityModifier == 1) ? ABOVE_NORMAL_PRIORITY_CLASS : HIGH_PRIORITY_CLASS;
		}
		if (bLaunchDetached)
		{
			createFlags |= DETACHED_PROCESS;
		}
		uint32 dwFlags = 0;
		uint16 showWindowsFlags = SW_HIDE;
		if (bLaunchReallyHidden)
		{
			dwFlags = STARTF_USESHOWWINDOW;
		}
		else if (bLaunchHidden)
		{
			dwFlags = STARTF_USESHOWWINDOW;
			showWindowsFlags = SW_SHOWMINNOACTIVE;
		}
		if (pipeWriteChild != nullptr || pipeReadChile != nullptr)
		{
			dwFlags |= STARTF_USESTDHANDLES;
		}

		STARTUPINFO startupInfo = {
			sizeof(STARTUPINFO),
			NULL, NULL, NULL,
			(::DWORD) CW_USEDEFAULT,
			(::DWORD) CW_USEDEFAULT,
			(::DWORD) CW_USEDEFAULT,
			(::DWORD) CW_USEDEFAULT,
			(::DWORD) 0, (::DWORD)0, (::DWORD)0,
			(::DWORD)dwFlags,
			showWindowsFlags,
			0, NULL,
			HANDLE(pipeReadChile),
			HANDLE(pipeWriteChild),
			HANDLE(pipeWriteChild)
		};
		wstring commandLine = String::printf(TEXT("\"%s\" %s"), url, params);
		PROCESS_INFORMATION procInfo;
		if (!CreateProcess(NULL, &commandLine[0], &attr, &attr, true, (::DWORD)createFlags, NULL, optionalWorkingDirectory, &startupInfo, &procInfo))
		{
			AIR_LOG(LogWindows, Warning, TEXT("CreateProc failed (%u) %s %s"), ::GetLastError(), url, params);
			if (outProcessId != nullptr)
			{
				*outProcessId = 0;
			}
			return ProcHandle();
		}
		if (outProcessId != nullptr)
		{
			*outProcessId = procInfo.dwProcessId;
		}
		::CloseHandle(procInfo.hThread);
		return ProcHandle(procInfo.hProcess);
	}
	RunnableThread* WindowsPlatformProcess::createRunnableThread()
	{
		return new RunnableThreadWin();
	}

	int32 WindowsPlatformProcess::getDllApiVersion(const TCHAR* filename)
	{
		int32 result = -1;
		BOOST_ASSERT(filename);
		HMODULE hModule = LoadLibraryEx(filename, NULL, LOAD_LIBRARY_AS_DATAFILE);
		if (hModule != NULL)
		{
			HRSRC hResInfo = FindResource(hModule, MAKEINTRESOURCE(ID_MODULE_API_VERSION_RESOURCE), RT_RCDATA);
			if (hResInfo)
			{
				HGLOBAL hResGlobal = LoadResource(hModule, hResInfo);
				if (hResGlobal != NULL)
				{
					void *pResData = LockResource(hResGlobal);
					if (pResData != NULL)
					{
						::DWORD length = SizeofResource(hModule, hResInfo);
						if (length > 0)
						{
							char* str = (char*)pResData;
							if (str[length - 1] == 0)
							{
								char* end = str;
								uint64 value = boost::lexical_cast<uint64>(str);
								if (*end == 0 && value <= INT_MAX)
								{
									result = (int32)value;
								}
							}
						}
						UnlockResource(pResData);
					}
				}
			}
			FreeLibrary(hModule);
		}
		return result;
	}

	uint32 WindowsPlatformProcess::getCurrentProcessId()
	{
		return ::GetCurrentProcessId();
	}

	const TCHAR* WindowsPlatformProcess::userTempDir()
	{
		static wstring windowsUserTempDir;
		if (!windowsUserTempDir.length())
		{
			TCHAR tempPath[MAX_PATH];
			ZeroMemory(tempPath, sizeof(TCHAR) * MAX_PATH);
			::GetTempPath(MAX_PATH, tempPath);
			TCHAR fullTempPath[MAX_PATH];
			ZeroMemory(fullTempPath, sizeof(TCHAR) * MAX_PATH);
			::GetLongPathName(tempPath, fullTempPath, MAX_PATH);

			windowsUserTempDir = wstring(fullTempPath);
			boost::replace_all(windowsUserTempDir, TEXT("\\"), TEXT("/"));
		}
		return windowsUserTempDir.c_str();
	}


	const wstring WindowsPlatformProcess::shaderWorkingDir()
	{
		return wstring(PlatformProcess::userTempDir()) + TEXT("AirShaderWorkingDir/");
	}

	class Event* WindowsPlatformProcess::createSynchEvent(bool bIsManualReset)
	{
		Event* e = NULL;
		if (PlatformProcess::supportsMultithreading())
		{
			e = new EventWin();
		}
		if (!e->create(bIsManualReset))
		{
			delete e;
			e = nullptr;
		}
		return e;
	}

	void WindowsPlatformProcess::setThreadAffinityMask(uint64 affinityMask)
	{
		if (affinityMask != PlatformAffinity::getNoAffinityMask())
		{
			::SetThreadAffinityMask(::GetCurrentThread(), (DWORD_PTR)affinityMask);
		}
	}

	void WindowsPlatformProcess::sleep(float seconds)
	{
		sleepNoStats(seconds);
	}

	const TCHAR* WindowsPlatformProcess::baseDir()
	{
		static TCHAR result[512] = TEXT("");
		if (!result[0])
		{
			wstring baseArg;
			HMODULE hCurrentModule;
			if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)&baseDir, &hCurrentModule) == 0)
			{
				hCurrentModule = hInstance;
			}
			GetModuleFileName(hCurrentModule, result, ARRAY_COUNT(result));
			wstring tempResult(result);
			
			tempResult = Paths::getPath(tempResult);
			boost::algorithm::replace_all(tempResult, TEXT("\\"), TEXT("/"));

			tempResult.copy(result, tempResult.length());
			result[tempResult.length()] = 0;
		}
		return result;
	}

	wstring WindowsPlatformProcess::getCurrentWorkingDirectory()
	{
		TCHAR currentDirectory[MAX_PATH];
		GetCurrentDirectoryW(MAX_PATH, currentDirectory);
		return currentDirectory;
	}

	void WindowsPlatformProcess::setCurrentWorkingDirectoryToBaseDir()
	{
		PlatformMisc::cacheLaunchDir();
		SetCurrentDirectoryW(baseDir());
	}

	const TCHAR* WindowsPlatformProcess::executableName(bool bRemoveExtension /* = true */)
	{
		static TCHAR result[512] = TEXT("");
		static TCHAR resultWithExt[512] = TEXT("");
		if (!result[0])
		{
			if (GetModuleFileName(hInstance, result, ARRAY_COUNT(result)) != 0)
			{
				wstring fileName = result;
				wstring fileNameWithExt = result;
				fileName = Paths::getBaseFilename(fileName);
				fileNameWithExt = Paths::getCleanFilename(fileNameWithExt);
				memcpy(result, fileName.c_str(), std::min<size_t>(sizeof(result), fileName.size() * sizeof(fileName[0])));
				memcpy(resultWithExt, fileNameWithExt.c_str(), std::min<size_t>(sizeof(resultWithExt), fileNameWithExt.size() * sizeof(fileNameWithExt[0])));
			}
		}
		return (bRemoveExtension ? result : resultWithExt);
	}

	void* WindowsPlatformProcess::loadLibraryWithSearchPaths(const wstring& fileName, const TArray<wstring>& searchPaths)
	{
		wstring fullFileName = fileName;
		if (Paths::fileExists(fullFileName))
		{
			fullFileName = Paths::convertRelativePathToFull(fullFileName);
			TArray<string> visitedImportNames;
			TArray<wstring> importFileNames;
			resolveImportsRecursive(fullFileName, searchPaths, importFileNames, visitedImportNames);

			for (int32 idx = 0; idx < importFileNames.size(); idx++)
			{
				if (GetModuleHandle(importFileNames[idx].c_str()) == nullptr)
				{
					LoadLibrary(importFileNames[idx].c_str());
				}
			}
		}
		
		std::wcout << fullFileName << std::endl;
		return LoadLibrary(fullFileName.c_str());
	}


	void* WindowsPlatformProcess::getDllHandle(const TCHAR* fileName)
	{
		TArray<wstring> searchPath;
		searchPath.push_back(PlatformProcess::getModulesDirectory());
		if (mDllDirectoryStack.size() > 0)
		{
			searchPath.push_back(mDllDirectoryStack.back());
		}
		for (int32 idx = 0; idx < mDllDirectories.size(); ++idx)
		{
			searchPath.push_back(mDllDirectories[idx]);
		}
		int32 PrevErrorMode = ::SetErrorMode(SEM_NOOPENFILEERRORBOX);
		void* handle = loadLibraryWithSearchPaths(fileName, searchPath);
		::SetErrorMode(PrevErrorMode);
		return handle;
	}

	void WindowsPlatformProcess::freeDllHandle(void* dllHandle)
	{
		::FreeLibrary((HMODULE)dllHandle);
	}

	void* WindowsPlatformProcess::getDllExport(void* dllHandle, const char* ProcName)
	{
		return (void*)::GetProcAddress((HMODULE)dllHandle, ProcName);
	}

	void WindowsPlatformProcess::resolveImportsRecursive(const wstring& fileName, const TArray<wstring>& searchPaths, TArray<wstring>& importFileNames, TArray<string>& visitedImportNames)
	{
		TArray<string> importNames;
		if (readLibraryImports(fileName.c_str(), importNames))
		{
			for (int idx = 0; idx < importNames.size(); ++idx)
			{
				const string& importName = importNames[idx];
				if(visitedImportNames.contains(importName))
				{
					visitedImportNames.push_back(importName);
					wstring importFileName;
					if (resolveImport(importName, searchPaths, importFileName))
					{
						resolveImportsRecursive(importFileName, searchPaths, importFileNames, visitedImportNames);
						importFileNames.push_back(importFileName);
					}
				}
			}
		}
	}

	bool WindowsPlatformProcess::readLibraryImports(const TCHAR* fileName, TArray<string>& importNames)
	{
		bool result = false;
		HANDLE newFileHandle = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (newFileHandle != INVALID_HANDLE_VALUE)
		{
			HANDLE newFileMappingHandle = CreateFileMapping(newFileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
			if (newFileMappingHandle != NULL)
			{
				void* newData = MapViewOfFile(newFileMappingHandle, FILE_MAP_READ, 0, 0, 0);
				if (newData != NULL)
				{
					const IMAGE_DOS_HEADER* Header = (const IMAGE_DOS_HEADER*)newData;
					result = readLibraryImportsFromMemory(Header, importNames);
					UnmapViewOfFile(newData);
				}
				CloseHandle(newFileMappingHandle);
			}
			CloseHandle(newFileHandle);
		}
		return result;
	}

	bool WindowsPlatformProcess::resolveImport(const string name, const TArray<wstring>& searchPaths, wstring& outFileName)
	{
		for (int idx = 0; idx < searchPaths.size(); idx++)
		{
			wstring x(name.begin(), name.end());
			wstring fileName = searchPaths[idx] + x;
			if (Paths::fileExists(fileName))
			{
				outFileName = Paths::convertRelativePathToFull(fileName);
				return true;
			}
		}
		return false;
	}

	const TCHAR* WindowsPlatformProcess::getBinariesSubDirectory()
	{
#if PLATFORM_64BITS
		return TEXT("win_x64");
#else
		return TEXT("win_x86");
#endif

	}
	const TCHAR* WindowsPlatformProcess::getModuleExtension()
	{
		return TEXT("dll");
	}
}