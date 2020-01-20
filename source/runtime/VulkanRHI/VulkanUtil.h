#pragma once
#include "VulkanConfig.h"
#include "GPUProfiler.h"
#include "Containers/IndirectArray.h"
namespace Air
{
	extern VULKANRHI_API void verifyVulkanResult(VkResult result, const ANSICHAR* vkFunction, const ANSICHAR* filename, uint32 line);
}

#define VERIFYVULKANRESULT(VkFunction)	{const VkResult scopedResult = VkFunction; if(scopedResult != VK_SUCCESS) {verifyVulkanResult(scopedResult, #VkFunction, __FILE__, __LINE__);}}

#define VERIFYVULKANRESULT_EXPANDED(VkFunction)	{const VkResult scopedResult = VkkFunction; if(scopedResult < VK_SUCCESS) {verifyVulkanResult(scopedResult, #VkFunction, __FILE__, __LINE__);}}

namespace Air
{
	class VulkanCommandListContext;
	class VulkanTimingQueryPool;
	class VulkanDevice;

	class VulkanGPUTiming : public GPUTiming
	{
	private:
		VulkanDevice* mDevice;

		bool bIsTiming;

		bool bEndTimestampIssued;

		VulkanCommandListContext* mCmdContext;

		VulkanTimingQueryPool* mPool = nullptr;
	};


	class VulkanEventNodeFrame : public GPUProfilerEventNodeFrame
	{
	public:
		VulkanEventNodeFrame(VulkanCommandListContext* inCmd, VulkanDevice* inDevice)
		{

		}

		VulkanGPUTiming mRootEventTiming;
	};

	struct VulkanGPUProfiler : public GPUProfiler
	{
#if VULKAN_SUPPORTS_GPU_CRASH_DUMPS
		void dumpCrashMarkers(void* bufferData);
#endif

		TindirectArray<VulkanEventNodeFrame> mGPUHitchEventNodeFrames;

		bool bCommandListSubmitted;

		VulkanDevice* mDevice;
		VulkanCommandListContext* mCmdContext;

		TMap<uint32, wstring> mCachedStrings;
		TArray<uint32> mPushPopStack;
	};
}