#pragma once

#include "VulkanResources.h"
#include "RHIContext.h"
namespace Air
{
	class VulkanDynamicRHI;
	class VulkanQueue;
	class VulkanCommandListContext : public IRHICommandContext
	{
	public:
		inline VulkanGPUProfiler& getGPUProfiler()
		{
			return mGpuProfiler;
		}

	private:
		VulkanGPUProfiler mGpuProfiler;
	};

	class VulkanCommandListContextImmediate : public VulkanCommandListContext
	{
	public:
		VulkanCommandListContextImmediate(VulkanDynamicRHI* inRHI, VulkanDevice* inDevice, VulkanQueue* inQueue);
	};
}