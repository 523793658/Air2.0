#pragma once
#include "CoreType.h"

#if PLATFORM_WINDOWS
#include "windows/WindowsPlatformProperties.h"

namespace Air
{
	typedef WindowsPlatformProperties<WITH_EDITORONLY_DATA, false, true> PlatformProperties;
}

#endif


