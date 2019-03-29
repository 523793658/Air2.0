#pragma once
#include "CoreMinimal.h"
#ifdef LAUNCH_RESOURCE
#define LAUNCH_API	DLLEXPORT
#else
#define LAUNCH_API	DLLIMPORT
#endif