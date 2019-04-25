#pragma once
#include "CoreType.h"
#include "WindowsPlatformCompilerSetup.h"
#include "Windows/MinimalWindowsApi.h"

#define SAFE_RELEASE(p) {if(p){(p)->Release(); p = NULL;}}

extern "C" CORE_API Windows::HINSTANCE hInstance;

#include <algorithm>
#include "boost/boost/assert.hpp"