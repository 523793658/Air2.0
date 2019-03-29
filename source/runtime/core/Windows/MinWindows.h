#pragma once
#include "CoreType.h"


#include "HAL/PlatformCrt.h"



#if defined(_WINDOWS_) && !defined(AIR_MINIMAL_WINDOWS_INCLUDE)
#pragma message( " ")
#pragma message( "You have include windows.h before MinWindows.h")
#pragma message( "All useless stuff from the windows headers won't be excluded")
#pragma  message( " " )
#endif

#define AIR_MINIMAL_WINDOWS_INCLUDE

#define WIN32_LEAN_AND_MEAN

#define NOGDICAPMASKS

#undef TEXT

#include <windows.h>
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif