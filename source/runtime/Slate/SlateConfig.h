#pragma once
#ifdef SLATE_SOURCE
#define SLATE_API DLLEXPORT
#else
#define SLATE_API DLLIMPORT
#endif