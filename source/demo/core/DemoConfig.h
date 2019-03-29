#pragma once
#include "CoreMinimal.h"
#ifdef _DEMO_
#define DEMO_API	DLLEXPORT
#else
#define DEMO_API	DLLIMPORT
#endif