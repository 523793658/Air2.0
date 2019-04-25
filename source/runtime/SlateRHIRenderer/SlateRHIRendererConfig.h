#pragma once
#include "CoreMinimal.h"

#ifdef SlateRHIRenderer_EXPORTS
#define RHI_RENDERER_API DLLEXPORT
#else
#define RHI_RENDERER_API DLLIMPORT
#endif