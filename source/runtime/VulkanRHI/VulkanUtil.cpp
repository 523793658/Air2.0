#include "VulkanRHIPrivate.h"
#include "VulkanUtil.h"
#include "VulkanConfiguration.h"
#include "VulkanDynamicRHI.h"
#include "VulkanDevice.h"
namespace Air
{
	VulkanDynamicRHI* GVulkanRHI = nullptr;

	extern CORE_API bool GIsGPUCrashed;

	bool GGPUCrashDebuggingEnabled = false;

	void verifyVulkanResult(VkResult result, const ANSICHAR* vkFunction, const ANSICHAR* filename, uint32 line)
	{
		bool bDumpMemory = false;
		wstring errorString;
		switch (result)
		{
#define VKERRORCASE(x) case x: errorString = TEXT(#x)
			VKERRORCASE(VK_NOT_READY); break;
			VKERRORCASE(VK_TIMEOUT); break;
			VKERRORCASE(VK_EVENT_SET); break;
			VKERRORCASE(VK_EVENT_RESET); break;
			VKERRORCASE(VK_INCOMPLETE); break;
			VKERRORCASE(VK_ERROR_OUT_OF_HOST_MEMORY); bDumpMemory = true; break;
			VKERRORCASE(VK_ERROR_OUT_OF_DEVICE_MEMORY); bDumpMemory = true; break;
			VKERRORCASE(VK_ERROR_INITIALIZATION_FAILED); break;
			VKERRORCASE(VK_ERROR_DEVICE_LOST); GIsGPUCrashed = true; break;
			VKERRORCASE(VK_ERROR_MEMORY_MAP_FAILED); break;
			VKERRORCASE(VK_ERROR_LAYER_NOT_PRESENT); break;
			VKERRORCASE(VK_ERROR_EXTENSION_NOT_PRESENT); break;
			VKERRORCASE(VK_ERROR_FEATURE_NOT_PRESENT); break;
			VKERRORCASE(VK_ERROR_INCOMPATIBLE_DRIVER); break;
			VKERRORCASE(VK_ERROR_TOO_MANY_OBJECTS); break;
			VKERRORCASE(VK_ERROR_FORMAT_NOT_SUPPORTED); break;
			VKERRORCASE(VK_ERROR_SURFACE_LOST_KHR); break;
			VKERRORCASE(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR); break;
			VKERRORCASE(VK_SUBOPTIMAL_KHR); break;
			VKERRORCASE(VK_ERROR_OUT_OF_DATE_KHR); break;
			VKERRORCASE(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR); break;
			VKERRORCASE(VK_ERROR_VALIDATION_FAILED_EXT); break;
			VKERRORCASE(VK_ERROR_INVALID_SHADER_NV); break;
			VKERRORCASE(VK_ERROR_FRAGMENTED_POOL); break;
			VKERRORCASE(VK_ERROR_OUT_OF_POOL_MEMORY_KHR); break;
			VKERRORCASE(VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR); break;
			VKERRORCASE(VK_ERROR_NOT_PERMITTED_EXT); break;
#undef VKERRORCASE
		default:
			break;
		}

		AIR_LOG(logVulkanRHI, Error, TEXT("%s failed, VkResult=%d\n at %s:%u \n with error %s"), ANSI_TO_TCHAR(vkFunction), (int32)result, ANSI_TO_TCHAR(filename), line, errorString.c_str());

#if VULKAN_SUPPORTS_GPU_CRASH_DUMPS
		if (GIsGPUCrashed && GGPUCrashDebuggingEnabled)
		{
			VulkanDevice* device = GVulkanRHI->getDevice();
			if (device->getOptionalExtensions().hasGPUCrashDumpExtensions())
			{
				//device->getImmediateContext().getGPUProfiler().dumpCrashMarkers(device->getcrash)
			}
		}
#endif
	}
}