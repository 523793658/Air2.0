#pragma once
#include "CoreMinimal.h"
#ifdef SLATE_RESOURCE
#define SLATE_API	DLLEXPORT
#else
#define SLATE_API	DLLIMPORT
#endif