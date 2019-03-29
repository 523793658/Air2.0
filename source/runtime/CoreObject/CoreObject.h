#pragma once
#include "CoreMinimal.h"

#ifdef CORE_OBJECT_SOURCE
#define COREOBJECT_API	DLLEXPORT
#else
#define COREOBJECT_API	DLLIMPORT
#endif
