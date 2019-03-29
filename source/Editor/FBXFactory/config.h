#pragma once
#include "CoreMinimal.h"

#ifdef FbxFactory_SOURCE
#define Fbx_API DLLEXPORT
#else
#define Fbx_API DLLIMPORT
#endif