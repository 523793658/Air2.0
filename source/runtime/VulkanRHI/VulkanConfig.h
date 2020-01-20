#pragma once
#include "CoreType.h"
#ifdef VulkanRHI_EXPORTS
#define VULKANRHI_API DLLEXPORT
#else
#define VULKANRHI_API DLLIMPORT
#endif