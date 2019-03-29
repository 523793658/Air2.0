#pragma once
#include "CoreMinimal.h"

#ifdef SLATE_CORE_RESOURCE
#define SLATE_CORE_API DLLEXPORT
#else
#define SLATE_CORE_API DLLIMPORT
#endif // SLATE_CORE_RESOURCE
