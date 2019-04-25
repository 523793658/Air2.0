#pragma once
#include "CoreMinimal.h"
#ifdef ShaderCore_EXPORTS
#define SHADER_CORE_API DLLEXPORT
#else
#define SHADER_CORE_API DLLIMPORT
#endif