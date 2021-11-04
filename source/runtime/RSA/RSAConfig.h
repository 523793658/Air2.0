#pragma once
#ifdef RSA_SOURCE
#define RSA_API DLLEXPORT
#else 
#define RSA_API DLLIMPORT
#endif