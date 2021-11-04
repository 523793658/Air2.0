#pragma once
#include "HAL/Platform.h"
#ifdef TARGETPLATFORM_SOURCE
#define TARGETPLATFORM_API DLLEXPORT
#else
#define TARGETPLATFORM_API DLLIMPORT
#endif