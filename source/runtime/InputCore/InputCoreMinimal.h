#pragma once
#include "CoreMinimal.h"
#ifdef INPUT_CORE_RESOURCE
#define INPUT_CORE_API DLLEXPORT
#else
#define INPUT_CORE_API DLLIMPORT
#endif