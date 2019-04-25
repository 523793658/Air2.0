#pragma once
#include "CoreMinimal.h"
#ifdef launch_EXPORTS
#define LAUNCH_API	DLLEXPORT
#else
#define LAUNCH_API	DLLIMPORT
#endif