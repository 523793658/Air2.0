#pragma once
#include "VulkanMemory.h"
#include "RHIResource.h"
#include "VulkanResources.h"
#include "VulkanQueue.h"
namespace Air
{
	class VulkanSamplerState;
	class VulkanSurface;

	struct OptionalVulkanDeviceExtensions
	{
		uint32 HasKHRMaintenance1 : 1;
		uint32 HasKHRMaintenance2 : 1;

		uint32 HasKHRExternalMemoryCapabilities : 1;
		uint32 HasKHRGetPhysicalDeviceProperties2 : 1;
		uint32 HasKHRDedicatedAllocation : 1;
		uint32 HasEXTValidationCache : 1;
		uint32 HasAMDBufferMarker : 1;
		uint32 HasNVDiagnosticCheckpoints : 1;
		uint32 HasGoogleDisplayTiming : 1;
		uint32 HasYchcrSampler : 1;
		uint32 HasMemoryPriority : 1;

		inline bool hasGPUCrashDumpExtensions() const
		{
			return HasAMDBufferMarker || HasNVDiagnosticCheckpoints;
		}
	};

	class VulkanDevice
	{
	public:
		VulkanDevice(VkPhysicalDevice inGpu);

		~VulkanDevice();

		inline DeviceMemoryManager& getMemoryMananger()
		{
			return mMemoryManager;
		}

		inline const VkPhysicalDeviceProperties& getDevicePropertise() const
		{
			return mGpuProps;
		}

		inline const  VkPhysicalDeviceFeatures& getPhysicalFeatures() const
		{
			return mPhysicalFeature;
		}

		inline VkPhysicalDevice getPhysicalHandle() const
		{
			return mGPU;
		}

		inline VkDevice getInstanceHandle() const
		{
			return mDevice;
		}

		inline const VkPhysicalDeviceLimits& getLimits()const
		{
			return mGpuProps.limits;
		}

		inline const OptionalVulkanDeviceExtensions& getOptionalExtensions() const
		{
			return mOptionalDeviceExtensions;
		}

		inline VulkanCommandListContextImmediate& getImmediateContext()
		{
			return *mImmediateContext;
		}

		inline DeferredDeletionQueue& getDeferredDeletionQueue()
		{
			return mDeferredDeletionQueue;
		}

		inline VulkanQueue* getGraphicsQueue()
		{
			return mGfxQueue;
		}

		void prepareForDestroy();

		void destroy();
	private:
		DeviceMemoryManager mMemoryManager;

		ResourceHeapManager mResourceHeapManager;

		DeferredDeletionQueue mDeferredDeletionQueue;
	
		VkPhysicalDeviceProperties mGpuProps;

		VkPhysicalDeviceFeatures mPhysicalFeature;

		TMap<uint32, SamplerStateRHIRef> mSamplerMap;

		VulkanSamplerState* mDefaultSampler;

		VulkanSurface* mDefaultImage;

		VulkanTextureView mDefaultTextureView;

		VkDevice mDevice;

		VkPhysicalDevice mGPU;

		VulkanQueue* mGfxQueue;
		VulkanQueue* mComputeQueue;
		VulkanQueue* mTransferQueue;
		VulkanQueue* mPresentQueue;

		VulkanCommandListContextImmediate* mImmediateContext;
		VulkanCommandListContext* mComputeContext;
		TArray<VulkanCommandListContext*> mCommandContexts;
#if VULKAN_SUPPORTS_COLOR_CONVERSIONS
		TMap<uint32, VkSamplerYcbcrConversion> mSamplerColorConversionMap;
#endif
		class VulkanPipelineStateCacheManager* mPipelineStateCache;

#if VULKAN_ENABLE_DESKTOP_HMD_SUPPORT
		VkPhysicalDeviceIDPropertiesKHR mGpuIdProps;
#endif
		VkPhysicalDeviceFeatures mPhysicalFeatures;

		OptionalVulkanDeviceExtensions mOptionalDeviceExtensions;

		VkFormatProperties mFormatProperties[VK_FORMAT_RANGE_SIZE];

		VkComponentMapping mPixelFormatComponentMapping[PF_MAX];

		friend class VulkanDynamicRHI;
	};
}