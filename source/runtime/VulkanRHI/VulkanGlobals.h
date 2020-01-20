#pragma once
#include <atomic>
#include "CoreMinimal.h"
#include "vulkan/vulkan.h"
namespace Air
{
	extern std::atomic_uint64_t GVulkanBufferHandleIdCounter;
	extern std::atomic_uint64_t GVulkanBufferViewHandleIdCounter;
	extern std::atomic_uint64_t GVulkanImageViewHandleIdCounter;
	extern std::atomic_uint64_t GVulkanSamplerHandleIdCounter;
	extern std::atomic_uint64_t GVulkanDSetLayoutHandleIdCounter;

	template<class T>
	static FORCEINLINE void zeroVulkanStruct(T& s, VkStructureType type)
	{
		static_assert(!std::is_pointer<T>::value, "Don't use a pointer");
		static_assert(STRUCT_OFFSET(T, sType) == 0, "Assumes sType is the first member in the Vulkan type!");
		s.sType = type;
		Memory::memzero(((uint8*)& s) + sizeof(VkStructureType), sizeof(T) - sizeof(VkStructureType));
	}
}