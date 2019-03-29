#pragma once
#include "CoreMinimal.h"

#ifdef RENDERER_SOURCE
#define RENDERER_API	DLLEXPORT
#else
#define RENDERER_API	DLLIMPORT
#endif