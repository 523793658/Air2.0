#pragma once
#include "CoreMinimal.h"
#ifdef MOVIE_PLAYER_RESOURCE
#define MOVIE_PLAYER_API	DLLEXPORT
#else
#define MOVIE_PLAYER_API	DLLIMPORT
#endif