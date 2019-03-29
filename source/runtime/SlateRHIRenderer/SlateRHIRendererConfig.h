#pragma once
#include "CoreMinimal.h"

#ifdef SLATE_RHI_RENDERER_RESOURCE
#define RHI_RENDERER_API DLLEXPORT
#else
#define RHI_RENDERER_API DLLIMPORT
#endif