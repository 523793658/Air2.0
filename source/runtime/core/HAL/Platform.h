#pragma once

#if !defined(PLATFORM_WINDOWS)
#define PLATFORM_WINDOWS 0
#endif

#if !defined(PLATFORM_LINUX)
#define PLATFORM_LINUX 0
#endif

#include "PreprocessorHelper.h"
#include COMPILED_PLATFORM_HEADER(PlatformCompilerPreSetup.h)

#ifndef PLATFORM_COMPILER_HAS_DEFAULTED_FUNCTIONS 
#define PLATFORM_COMPILER_HAS_DEFAULTED_FUNCTIONS 1
#endif

#ifndef RESTRICT
#define RESTRICT	__restrict
#endif

#ifndef UNLIKELY
#if (defined(__clang__) || defined(__GNUC__)) && (PLATFORM_UNIX)
#define UNLIKELY(x)		__builtin_expect(!!(x), 0)
#else
#define UNLIKELY(x)		(x)
#endif
#endif


#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformCompilerPreSetup.h"
#endif


#include "GenericPlatform/GenericPlatform.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatform.h"
#endif


#ifndef GCC_PACK
#define GCC_PACK(n)
#endif

#ifndef GCC_ALIGN
#define GCC_ALIGN(n)
#endif

#ifndef MS_ALIGN
#define MS_ALIGN(n)
#endif

#ifndef PLATFORM_CAN_SUPPORT_EDITORONLY_DATA
#define PLATFORM_CAN_SUPPORT_EDITORONLY_DATA 0
#endif

#ifndef PLATFORM_SUPPORTS_GEOMETRY_SHADERS
#define PLATFORM_SUPPORTS_GEOMETRY_SHADERS	1
#endif

#ifndef PLATFORM_SUPPORTS_TESSELLATION_SHADERS
#define PLATFORM_SUPPORTS_TESSELLATION_SHADERS	1
#endif

#if PLATFORM_SUPPORTS_TESSELLATION_SHADERS && !PLATFORM_SUPPORTS_GEOMETRY_SHADERS
#error Geometry shader support is required by tessellation
#endif


typedef PlatformTypes::uint8		uint8;
typedef PlatformTypes::uint16		uint16;
typedef PlatformTypes::uint32		uint32;
typedef PlatformTypes::uint64		uint64;

typedef PlatformTypes::int8				int8;
typedef PlatformTypes::int16			int16;
typedef PlatformTypes::int32			int32;
typedef PlatformTypes::int64			int64;

typedef PlatformTypes::SIZE_T			SIZE_T;

typedef PlatformTypes::SSIZE_T			SSIZE_T;

typedef PlatformTypes::CHAR8			UTF8CHAR;
typedef PlatformTypes::CHAR16			UCS2CHAR;

typedef PlatformTypes::TCHAR			TCHAR;

typedef PlatformTypes::PTRINT			PTRINT;
typedef PlatformTypes::UPTRINT			UPTRINT;

typedef PlatformTypes::ANSICHAR			ANSICHAR;
typedef PlatformTypes::WIDECHAR			WIDECHAR;
#include <memory>
namespace Air
{

	

	template<typename T, typename... Args>
	inline std::shared_ptr<T> MakeSharedPtr(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	inline std::unique_ptr<T> makeUniquePtr(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	FORCEINLINE bool operator == (const std::weak_ptr<T>& lhs, const std::weak_ptr<T>& rhs)
	{
		return lhs._Get() == rhs._Get();
	}
}




#ifndef AIR_COMPILER_VERSION
#define AIR_COMPILER_VERSION 0
#endif

#ifdef _DEBUG
#define Suffix	"Debug"
#else
#define Suffix	""
#endif

#ifndef AIR_CXX17_LIBRARY_STRING_VIEW_SUPPORT
#define AIR_CXX17_LIBRARY_STRING_VIEW_SUPPORT 0
#endif

#ifndef LIKELY
#if (defined(__clang__) || defined(__GNUC__)) && PLATFORM_LINUX
#define LIKELY(x)		__builtin_expect(!!(x), 1)
#else
#define LIKELY(x)		(x)
#endif
#endif

#if !defined(TEXT)
#define TEXT_PASTE(x) L ## x
#define TEXT(x)	TEXT_PASTE(x)
#endif


#ifndef FORCEINLINE_DEBUGGABLE_ACTUAL
#define FORCEINLINE_DEBUGGABLE_ACTUAL inline
#endif

#ifndef FUNCTION_NON_NULL_RETURN_START
#define FUNCTION_NON_NULL_RETURN_START
#endif

#ifndef FUNCTION_NON_NULL_RETURN_END
#define FUNCTION_NON_NULL_RETURN_END
#endif

#if defined(AIR_COMPILER_MSVC)
#if defined(_M_X64)
#define AIR_CPU_X64
#define AIR_COMPILER_TARGET x64
#elif defined(_M_ARM64)
#define AIR_CPU_ARM64
#define AIR_COMPILER_TARGET arm64
#elif defined(_M_ARM)
#define AIR_CPU_ARM
#define AIR_COMPILER_TARGET	arm
#endif
#endif

#if defined(AIR_CPU_ARM) || defined(AIR_CPU_ARM64)
#elif defined(AIR_CPU_X86) || defined(AIR_CPU_X64) || defined(PLATFORM_WINDOWS)
#define AIR_LITTLE_ENDIAN
#else
#error "UnKnown CPU endian."
#endif


#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformCompilerSetup.h"
#endif
