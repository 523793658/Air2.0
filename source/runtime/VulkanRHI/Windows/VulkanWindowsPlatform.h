#pragma once
#include "windows/WindowsHWrapper.h"
#include "RHI.h"

#define VK_USE_PLATFORM_WIN32_KHR			1
#define VK_USE_PLATFORM_WIN32_KHX			1

#define AIR_VK_API_VERSION	VK_API_VERSION_1_1

#define VULKAN_SUPPORTS_COLOR_CONVERSIONS 1

#define VULKAN_ENABLE_DESKTOP_HMD_SUPPORT	1

#define VULKAN_SUPPORTS_AMD_BUFFER_MARKER	1

#define VULKAN_SUPPORTS_NV_DIAGNOSTIC_CHECHPOINT	1

#define ENUM_VK_ENTRYPOINTS_PLATFORM_BASE(EnumMacro)

#define ENUM_VK_ENTRYPOINTS_PLATFORM_INSTANCE(EnumMacro) \
	EnumMacro(PFN_vkCreateWin32SurfaceKHR, vkCreateWin32SurfaceKHR) \
	EnumMacro(PFN_vkGetPhysicalDeviceProperties2KHR, vkGetPhysicalDeviceProperties2KHR) \
	EnumMacro(PFN_vkGetImageMemoryRequirements2KHR, vkGetImageMemoryRequirements2KHR) \
	EnumMacro(PFN_vkCmdWriteBufferMarkerAMD, vkCmdWriteBufferMarkerAMD) \
	EnumMacro(PFN_vkCmdSetCheckpointNV, vkCmdSetCheckpointNV) \
	EnumMacro(PFN_vkGetQueueCheckpointDataNV, vkGetQueueCheckpointDataNV) \
	EnumMacro(PFN_vkGetBufferMemoryRequirements2KHR, vkGetBufferMemoryRequirements2KHR)

#define ENUM_VK_ENTRYPOINTS_OPTIONAL_PLATFORM_INSTANCE(EnumMacro) \
	EnumMacro(PFN_vkCreateSamplerYcbcrConversionKHR, vkCreateSamplerYcbcrConversionKHR) \
	EnumMacro(PFN_vkDestroySamplerYcbcrConversionKHR, vkDestroySamplerYcbcrConversionKHR)	

#include "../VulkanLoader.h"

#include "../VulkanGenericPlatform.h"

namespace Air
{


	class  VulkanWindowsPlatform : public VulkanGenericPlatform
	{
	public:
		static bool loadVulkanLibrary();

		static void freeVulkanLibrary();

		static bool loadVulkanInstanceFunctions(VkInstance inInstance);
	};

	typedef VulkanWindowsPlatform VulkanPlatform;
}