#pragma once
#include "VulkanPlatform.h"
#include "VulkanDynamicRHI.h"

namespace Air
{
	extern class VulkanDynamicRHI* GVulkanRHI;
	class StagingBuffer;

	struct PendingBufferLock
	{
		StagingBuffer* mStagingBuffer;
		uint32 mOffset;
		uint32 mSize;
		EResourceLockMode mLockMode;
	};
}