#pragma once
#include "CoreMinimal.h"
#ifdef MOVIE_SCENE_CAPTURE_SOURCE
#define MOVIE_SCENE_CAPTURE_API DLLEXPORT
#else
#define MOVIE_SCENE_CAPTURE_API DLLIMPORT
#endif