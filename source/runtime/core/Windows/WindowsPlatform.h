#pragma once

#include <sal.h>
#include "GenericPlatform/GenericPlatform.h"
#include <stdint.h>
struct WindowsPlatformTypes : public GenericPlatformTypes
{
	typedef uint8_t			uint8;
	typedef uint16_t		uint16;
	typedef uint32_t		uint32;
	typedef uint64_t		uint64;

	typedef int8_t			int8;
	typedef int16_t			int16;
	typedef int32_t			int32;
	typedef int64_t			int64;


	typedef uint8			CHAR8;
	typedef uint16			CHAR16;
	typedef uint32			CHAR32;

	typedef WIDECHAR		TCHAR;

	typedef SelectIntPointerType<uint32, uint64, sizeof(void*)>::TIntPointer UPTRINT;

	typedef SelectIntPointerType<int32, int64, sizeof(void*)>::TIntPointer PTRINT;
#ifdef _WIN64
	typedef unsigned __int64	SIZE_T;
	typedef	__int64				SSIZE_T;
#else
	typedef unsigned long		SIZE_T;
	typedef long				SSIZE_T;
#endif
};

typedef WindowsPlatformTypes PlatformTypes;

#define PLATFORM_DESKTOP				1
#if defined(_WIN64)
#define PLATFORM_64BITS					1
#else
#define PLATFORM_64BITS					0
#endif

#if !PLATFORM_64BITS
#error 32bit platform is not support!
#endif

#if defined(_DEBUG) || defined(DEBUG)
#define AIR_DEBUG	1
#else
#define AIR_DEBUG	0
#endif

#define PLATFORM_CAN_SUPPORT_EDITORONLY_DATA	1


#if defined(_MSC_VER) && _MSC_VER < 1900
#define PLATFORM_COMPILER_HAS_DEFAULTED_FUNCTIONS 0
#endif

#if _MSVC_LANG > 201402
#if _MSC_VER >= 1910
#define AIR_CXX17_CORE_STATIC_ASSERT_V2_SUPPORT	1
#endif
#define AIR_CXX17_LIBRARY_ANY_SUPPORT	1

#if _MSC_VER >=1914
#define	AIR_CXX17_LIBRARY_FILESYSTEM_SUPPORT	1
#endif // _MSC_VER >=1914

#define AIR_CXX17_LIBRARY_OPTIONAL_SUPPORT	1
#define AIR_CXX17_LIBRARY_STRING_VIEW_SUPPORT	1


#endif

#define VARARGS		__cdecl
#define CDECL		__cdecl
#define STDCALL		__stdcall
#define FORCEINLINE	__forceinline
#define FORCENOINLINE	__declspec(noinline)
#define FUNCTION_NO_RETURE_START	_declspec(noreturn)
#define FUNCTION_NON_NULL_RETURN_START _Ret_notnull_

#if !defined(__clang__) || defined(_MSC_VER)
#define ASSUME(expr) __assume(expr)
#endif

#define DECALRE_UINT64(x)	x

#if _MSC_VER >= 1910
#define AIR_COMPILER_VERSION 141
#elif _MSC_VER >= 1900
#define AIR_COMPILER_VERSION 140
#elif _MSC_VER >= 1800
#define AIR_COMPILER_VERSION 120
#else
#error	"Unsupported compiler version."
#endif

#ifdef __clang__
#define  A 1
#endif

#if !defined(__clang__)
#define PRAGMA_DISABLE_OPTIMIZATION_ACTUAL __pragma(optimize("",off))
#define PRAGMA_ENABLE_OPTIMIZATION_ACTUAL  __pragma(optimize("",on))
#elif defined(_MSC_VER)		// Clang only supports __pragma with -fms-extensions
#define PRAGMA_DISABLE_OPTIMIZATION_ACTUAL __pragma(clang optimize off)
#define PRAGMA_ENABLE_OPTIMIZATION_ACTUAL  __pragma(clang optimize on)
#endif

#if defined(__clang__) || _MSC_VER >= 1900
#define CONSTEXPR	constexpr
#else
#define CONSTEXPR	
#endif

#define ABSTRACT	abstract

#if defined(__clang__)
#define GCC_PACK(n) __attribute__((packed,aligned(n)))
#define GCC_ALIGN(n) __attribute__((aligned(n)))
#if defined(_MSC_VER)
#define MS_ALIGN(n) __declspec(align(n)) // With -fms-extensions, Clang will accept either alignment attribute
#endif
#else
#define MS_ALIGN(n) __declspec(align(n))
#endif

#if AIR_COMPILER_VERSION >= 140
#define AIR_CPP11_CORE_CONSTEXPR_SUPPORT	1
#define AIR_CXX11_CORE_NOEXCEPT_SUPPORT		1
#define AIR_TS_LIBRARY_FILESYSTEM_V3_SUPPORT 1
#else
#define AIR_CPP11_CORE_CONSTEXPR_SUPPORT	0
#define AIR_CXX11_CORE_NOEXCEPT_SUPPORT		0
#define AIR_TS_LIBRARY_FILESYSTEM_V3_SUPPORT 0
#define AIR_TS_LIBRARY_FILESYSTEM_V2_SUPPORT 1
#endif

// Pragmas
#define MSVC_PRAGMA(Pragma) __pragma(Pragma)

// Prefetch
#define PLATFORM_CACHE_LINE_SIZE	128

// DLL export and import definitions
#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)

#pragma warning(disable : 4100) // unreferenced formal parameter


#ifndef ENABLE_WIN_ALLOC_TRACKING
#define ENABLE_WIN_ALLOC_TRACKING 0
#endif

#ifndef USE_SECURE_CRT
#define USE_SECURE_CRT 0

#define PLATFORM_CAN_SUPPORT_EDITORONLY_DATA	1

#define PLATFORM_HAS_64BIT_ATOMICS				(WINVER >= 0X600)

#if AIR_CXX11_CORE_NOEXCEPT_SUPPORT
#define AIR_NOEXCEPT	noexcept
#else
#define AIR_NOEXCEPT
#endif

#if defined(__clang__)
#define PLATFORM_ENABLE_VECTORINTRINSICS	0
#else
#define PLATFORM_ENABLE_VECTORINTRINSICS	1
#endif


#define PLATFORM_LITTLE_ENDIAN				1
#define PLATFORM_SUPPORTS_UNALIGNED_INT_LOADS	1
#if defined(__clang__)
#define PLATFORM_SEH_EXCEPTIONS_DISABLED				1
#define PLATFORM_EXCEPTIONS_DISABLED					1
#endif

#define LINE_TERMINATOR TEXT("\r\n")
#define LINE_TERMINATOR_ANSI	"\r\n"




#pragma warning( disable: 4251 )

#endif

#if defined(_MSC_VER)
#define AIR_COMPILER_MSVC

#endif // 
