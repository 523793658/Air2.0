#pragma once


#include "Misc/Build.h"
#include "HAL/Platform.h"
#include "Misc/CoreMiscDefines.h"
#ifdef core_EXPORTS
#define CORE_API DLLEXPORT
#else
#define CORE_API DLLIMPORT
#endif
