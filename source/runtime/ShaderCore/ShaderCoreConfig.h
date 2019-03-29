#pragma once
#include "CoreMinimal.h"
#ifdef SHADER_CORE_RESOURCE
#define SHADER_CORE_API DLLEXPORT
#else
#define SHADER_CORE_API DLLIMPORT
#endif