#pragma once

#include "VulkanResources.h"
#include "RHIContext.h"
namespace Air
{
	class VulkanDynamicRHI;
	class VulkanQueue;
	class VulkanCommandBufferManager;
	class TempFrameAllocationBuffer;





	class VulkanCommandListContext : public IRHICommandContext
	{
	public:
		inline VulkanGPUProfiler& getGPUProfiler()
		{
			return mGpuProfiler;
		}

		inline VulkanCommandBufferManager* getCommandBufferManager()
		{
			return mCommandBufferManager;
		}

		inline TempFrameAllocationBuffer& getTempFrameAllocationBuffer()
		{
			return mTempFrameAllocationBuffer;
		}

	private:
		VulkanGPUProfiler mGpuProfiler;

		VulkanCommandBufferManager* mCommandBufferManager;

		TempFrameAllocationBuffer mTempFrameAllocationBuffer;
	};

	class VulkanCommandListContextImmediate : public VulkanCommandListContext
	{
	public:
		VulkanCommandListContextImmediate(VulkanDynamicRHI* inRHI, VulkanDevice* inDevice, VulkanQueue* inQueue);
	};
}