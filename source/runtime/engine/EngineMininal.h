#pragma once
#include "CoreMinimal.h"
#include "CoreObject.h"
#include "CoreGlobals.h"
#ifdef _ENGINE_SOURCE_
#define ENGINE_API DLLEXPORT
#else
#define ENGINE_API DLLIMPORT
#endif