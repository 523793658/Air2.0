#pragma once
#include "CoreMinimal.h"
#include "CoreObject.h"
#include "CoreGlobals.h"
#ifdef engine_EXPORTS
#define ENGINE_API DLLEXPORT
#else
#define ENGINE_API DLLIMPORT
#endif