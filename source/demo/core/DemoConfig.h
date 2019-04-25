#pragma once
#include "CoreMinimal.h"
#ifdef Demo_EXPORTS
#define DEMO_API	DLLEXPORT
#else
#define DEMO_API	DLLIMPORT
#endif