#pragma once
#include "CoreMinimal.h"

#ifdef RHI_RESOURCE
#define RHI_API DLLEXPORT
#else
#define RHI_API DLLIMPORT
#endif