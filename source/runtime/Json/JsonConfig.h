#pragma once
#ifdef JSON_SOURCE
#define JSON_API DLLEXPORT
#else
#define JSON_API DLLIMPORT
#endif