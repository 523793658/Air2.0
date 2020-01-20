#include "VulkanWindowsPlatform.h"
#include "HAL/PlatformProcess.h"
namespace Air
{
	static HMODULE GVulkanDllModule = nullptr;

	static PFN_vkGetInstanceProcAddr GGetInstanceProcAddr = nullptr;

	void VulkanWindowsPlatform::freeVulkanLibrary()
	{
		if (GVulkanDllModule != nullptr)
		{
			::FreeLibrary(GVulkanDllModule);
			GVulkanDllModule = nullptr;
		}
	}

#define CHECK_VK_ENTRYPOINTS(Type, Func) if(VulkanDynamicAPI::Func == NULL) {bFoundAllEntryPoints = false; AIR_LOG(LogRHI, Warning, TEXT("Failed to find entry point for %s"), TEXT(#Func));}

	bool VulkanWindowsPlatform::loadVulkanLibrary()
	{
		GVulkanDllModule = ::LoadLibraryW(TEXT("vulkan-1.dll"));

		if (GVulkanDllModule)
		{
#define GET_VK_ENTRYPOINTS(Type, Func) VulkanDynamicAPI::Func = (Type)PlatformProcess::getDllExport(GVulkanDllModule, L#Func);
			ENUM_VK_ENTRYPOINTS_BASE(GET_VK_ENTRYPOINTS);

			bool bFoundAllEntryPoints = true;
			ENUM_VK_ENTRYPOINTS_BASE(CHECK_VK_ENTRYPOINTS);
			if (!bFoundAllEntryPoints)
			{
				freeVulkanLibrary();
				return false;
			}

			ENUM_VK_ENTRYPOINTS_OPTIONAL_BASE(GET_VK_ENTRYPOINTS);
			
			ENUM_VK_ENTRYPOINTS_PLATFORM_BASE(GET_VK_ENTRYPOINTS);
			ENUM_VK_ENTRYPOINTS_PLATFORM_BASE(CHECK_VK_ENTRYPOINTS);

#undef GET_VK_ENTRYPOINTS

			return true;

		}
		return false;
	}

	bool VulkanWindowsPlatform::loadVulkanInstanceFunctions(VkInstance inInstance)
	{
		if (!GVulkanDllModule)
		{
			return false;
		}

		GGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)PlatformProcess::getDllExport(GVulkanDllModule, TEXT("vkGetInstanceProcAddr"));
		if (!GGetInstanceProcAddr)
		{
			return false;
		}

		bool bFoundAllEntryPoints = true;
#define CHECK_VK_ENTRYPOINTS(Type, Func) if(VulkanDynamicAPI::Func == NULL) {bFoundAllEntryPoints = false; AIR_LOG(LogRHI, Warning, TEXT("Failed to find entry point for %s"), TEXT(#Func));}

#define GETINSTANCE_VK_ENTRYPOINTS(Type, Func) VulkanDynamicAPI::Func = (Type)VulkanDynamicAPI::vkGetInstanceProcAddr(inInstance, #Func);
		ENUM_VK_ENTRYPOINTS_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS);
		ENUM_VK_ENTRYPOINTS_INSTANCE(CHECK_VK_ENTRYPOINTS);
		ENUM_VK_ENTRYPOINTS_SURFACE_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS);
		ENUM_VK_ENTRYPOINTS_SURFACE_INSTANCE(CHECK_VK_ENTRYPOINTS);

		if (!bFoundAllEntryPoints)
		{
			freeVulkanLibrary();
			return false;
		}

		ENUM_VK_ENTRYPOINTS_OPTIONAL_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS);
		ENUM_VK_ENTRYPOINTS_OPTIONAL_PLATFORM_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS);

		ENUM_VK_ENTRYPOINTS_PLATFORM_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS);
		ENUM_VK_ENTRYPOINTS_PLATFORM_INSTANCE(CHECK_VK_ENTRYPOINTS);


#undef GET_VK_ENTRYPOINTS
		return true;
	}
}