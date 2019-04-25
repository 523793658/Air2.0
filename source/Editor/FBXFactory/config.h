#pragma once
#include "CoreMinimal.h"

#ifdef FBXFactory_EXPORTS
#define Fbx_API DLLEXPORT
#else
#define Fbx_API DLLIMPORT
#endif