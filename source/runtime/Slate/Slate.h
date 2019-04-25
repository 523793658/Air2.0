#pragma once
#include "CoreMinimal.h"
#ifdef Slate_EXPORTS
#define SLATE_API	DLLEXPORT
#else
#define SLATE_API	DLLIMPORT
#endif