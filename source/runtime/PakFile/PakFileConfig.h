#pragma once
#ifdef PAKFILE_SOURCE
#define PAKFILE_API DLLEXPORT
#else
#define PAKFILE_API DLLIMPORT
#endif