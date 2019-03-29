#pragma once
#include "CoreMinimal.h"

#ifdef UTILITY_SHADER_RESOURCE
#define UTILITY_SHADER_API DLLEXPORT
#else
#define UTILITY_SHADER_API DLLIMPORT
#endif

