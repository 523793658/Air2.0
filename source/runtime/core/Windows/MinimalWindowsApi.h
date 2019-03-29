#pragma once


struct HINSTANCE__;
struct HWND__;
struct HKEY__;
struct HDC__;
struct HICON__;

#define WINDOWS_MAX_PATH 260;
#define WINDOWS_PF_COMPARE_EXCHANGE128	14

#define WINAPI	__stdcall

struct HINSTANCE__;
struct HWND__;
struct HKEY__;
struct HDC__;
struct HICON__;

// Other types
struct tagPROCESSENTRY32W;
struct _EXCEPTION_POINTERS;
struct _OVERLAPPED;
struct _RTL_CRITICAL_SECTION;
union _LARGE_INTEGER;


namespace Windows
{
	using namespace Air;
	typedef int32 BOOL;
	typedef unsigned long DWORD;
	typedef DWORD* LPDWORD;
	typedef long LONG;
	typedef long* LPLONG;
	typedef int64 LONGLONG;
	typedef LONGLONG* LPLONGLONG;
	typedef void* LPVOID;
	typedef const void* LPCVOID;
	typedef const wchar_t* LPCTSTR;

	// Typedefs for standard handles
	typedef void* HANDLE;
	typedef HINSTANCE__* HINSTANCE;
	typedef HINSTANCE HMODULE;
	typedef HWND__* HWND;
	typedef HKEY__* HKEY;
	typedef HDC__* HDC;
	typedef HICON__* HICON;
	typedef HICON__* HCURSOR;

	typedef tagPROCESSENTRY32W PROCESSENTRY32;
	typedef _EXCEPTION_POINTERS* LPEXCEPTION_POINTERS;
	typedef _RTL_CRITICAL_SECTION* LPCRITICAL_SECTION;
	typedef _OVERLAPPED* LPOVERLAPPED;
	typedef _LARGE_INTEGER* LPLARGE_INTEGER;



	extern "C" __declspec(dllimport) DWORD WINAPI GetCurrentThreadId();
	extern "C" __declspec(dllimport) DWORD WINAPI TlsAlloc();

	extern "C" __declspec(dllimport) BOOL WINAPI TlsSetValue(DWORD dwTlsIndex, LPVOID lpTlsValue);

	extern "C" __declspec(dllimport) LPVOID WINAPI TlsGetValue(DWORD dwTlsIndex);

	extern "C" __declspec(dllimport) BOOL WINAPI TlsFree(DWORD dwTlsIndex);

	extern "C" __declspec(dllimport) void WINAPI InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection);

	extern "C" __declspec(dllimport) DWORD WINAPI SetCriticalSectionSpinCount(LPCRITICAL_SECTION lpCriticalSection, DWORD dwSpinCount);

	extern "C" __declspec(dllimport) void WINAPI DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection);

	extern "C" __declspec(dllimport) BOOL WINAPI TryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);

	extern "C" __declspec(dllimport) void WINAPI EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);

	extern "C" __declspec(dllimport) void WINAPI LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);

	extern "C" __declspec(dllimport) BOOL WINAPI QueryPerformanceCounter(LPLARGE_INTEGER Cycles);

	extern "C" __declspec(dllimport) BOOL WINAPI QueryPerformanceFrequency(LPLARGE_INTEGER lpFrequency);


	struct CRITICAL_SECTION { void* Opaque1[1]; long Opaque2[2]; void* Opaque3[3]; };
	struct OVERLAPPED { void *Opaque[3]; unsigned long Opaque2[2]; };
	union LARGE_INTEGER { struct { DWORD LowPart; LONG HighPart; }; LONGLONG QuadPart; };

	FORCEINLINE void WINAPI InitializeCriticalSection(CRITICAL_SECTION* lpCriticalSection)
	{
		InitializeCriticalSection((LPCRITICAL_SECTION)lpCriticalSection);
	}

	FORCEINLINE DWORD WINAPI SetCriticalSectionSpinCount(_Inout_ CRITICAL_SECTION* lpCriticalSection, DWORD dwSpinCount)
	{
		return SetCriticalSectionSpinCount((LPCRITICAL_SECTION)lpCriticalSection, dwSpinCount);
	}

	FORCEINLINE void WINAPI DeleteCriticalSection(CRITICAL_SECTION* lpCriticalSection)
	{
		DeleteCriticalSection((LPCRITICAL_SECTION)lpCriticalSection);
	}

	FORCEINLINE BOOL WINAPI TryEnterCriticalSection(CRITICAL_SECTION* lpCriticalSection)
	{
		return TryEnterCriticalSection((LPCRITICAL_SECTION)lpCriticalSection);
	}

	FORCEINLINE void WINAPI EnterCriticalSection(CRITICAL_SECTION* lpCriticalSection)
	{
		EnterCriticalSection((LPCRITICAL_SECTION)lpCriticalSection);
	}

	FORCEINLINE void WINAPI LeaveCriticalSection(CRITICAL_SECTION* lpCriticalSection)
	{
		LeaveCriticalSection((LPCRITICAL_SECTION)lpCriticalSection);
	}

	FORCEINLINE BOOL WINAPI QueryPerformanceCounter(LARGE_INTEGER* cycles)
	{
		return QueryPerformanceCounter((LPLARGE_INTEGER)cycles);
	}

	FORCEINLINE BOOL WINAPI QueryPerformanceFrequency(LARGE_INTEGER* frequency)
	{
		return QueryPerformanceFrequency((LPLARGE_INTEGER)frequency);
	}
}