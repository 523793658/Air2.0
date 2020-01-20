#include "VulkanRHIPrivate.h"

#include "VulkanDevice.h"

#include "HAL/AirMemory.h"

namespace Air
{
	VulkanDevice::VulkanDevice(VkPhysicalDevice inGpu)
		:mDevice(VK_NULL_HANDLE)
		, mResourceHeapManager(this)
		, mDeferredDeletionQueue(this)
		, mDefaultSampler(nullptr)
		, mDefaultImage(nullptr)
		, mGPU(inGpu)
		, mGfxQueue(nullptr)
		, mComputeQueue(nullptr)
		, mTransferQueue(nullptr)
		, mPresentQueue(nullptr)
		, mImmediateContext(nullptr)
		, mComputeContext(nullptr)
		, mPipelineStateCache(nullptr)
	{
		Memory::memzero(mGpuProps);
#if VULKAN_ENABLE_DESKTOP_HMD_SUPPORT
		Memory::memzero(mGpuIdProps);
#endif
		Memory::memzero(mPhysicalFeatures);
		Memory::memzero(mFormatProperties);
		Memory::memzero(mPixelFormatComponentMapping);
	}

	VulkanDevice::~VulkanDevice()
	{
		if (mDevice != VK_NULL_HANDLE)
		{
			destroy();
			mDevice = VK_NULL_HANDLE;
		}
	}
}