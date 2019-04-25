#pragma once
#include "CoreMinimal.h"

#ifdef Renderer_EXPORTS
#define RENDERER_API	DLLEXPORT
#else
#define RENDERER_API	DLLIMPORT
#endif