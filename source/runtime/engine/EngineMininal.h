#pragma once
#include "CoreMinimal.h"
#include "CoreUObject.h"
#include "CoreGlobals.h"
#ifdef engine_EXPORTS
#define ENGINE_API DLLEXPORT
#else
#define ENGINE_API DLLIMPORT
#endif