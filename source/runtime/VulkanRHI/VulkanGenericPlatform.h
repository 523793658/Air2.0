#pragma once
#include "PixelFormat.h"
#include "RHI.h"
namespace Air
{
	class VulkanGenericPlatform
	{
	public:
		static bool isSupported() { return true; }

		static bool requiresMobileRenderer() {
			return false;
		}

		static bool loadVulkanLibrary() { return true; }

		static void freeVulkanLibrary() {}

		static void overridePlatformHandlers(bool bInit){}
	
		static bool supportsDynamicResolution() { return false; }

		static bool supportsTimestampRenderQueries() { return true; }

		static bool supportParallelRenderingTask() { return true; }

		static bool supportsDepthFetchDuringDepthTest() { return true; }

		static void setupFeatureLevels();

		static bool hasUnifiedMemory() { return false; }

		static bool loadVulkanInstanceFunctions(VkInstance inInstance) { return true; }
	};
}