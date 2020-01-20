#pragma once
#include "VulkanConfiguration.h"
#include "HAL/CriticalSection.h"
#include "VulkanCommandBuffer.h"
#include "Misc/ScopeLock.h"
namespace Air
{
	class VulkanDevice;

	class VulkanQueue
	{
	public:
		VulkanQueue(VulkanDevice* inDevice, uint32 inFamilyIndex);
		~VulkanQueue();
		inline uint32 GetFamilyIndex() const
		{
			return mFamilyIndex;
		}

		void submit(VulkanCmdBuffer* cmdBuffer, uint32 numSignalSemaphores = 0, VkSemaphore * signalSemaphores = nullptr);

		inline void submit(VulkanCmdBuffer* cmdBuffer, VkSemaphore signalSemaphore)
		{
			submit(cmdBuffer, 1, &signalSemaphore);
		}

		inline VkQueue getHandle() const
		{
			return mQueue;
		}

		void getLastSubmittedInfo(VulkanCmdBuffer*& outCmdBuffer, uint64& outFenceCounter) const
		{
			ScopeLock scopeLock(&mCS);
			outCmdBuffer = mLastSubmittedCmdBuffer;
			outFenceCounter = mLastSubmittedCmdBufferFenceCounter;
		}

		inline uint64 getSubmitCount() const
		{
			return mSubmitCounter;
		}

	private:
		VkQueue mQueue;
		uint32 mFamilyIndex;
		uint32 mQueueIndex;
		VulkanDevice* mDevice;

		mutable CriticalSection mCS;
		VulkanCmdBuffer* mLastSubmittedCmdBuffer;
		uint64 mLastSubmittedCmdBufferFenceCounter;
		uint64 mSubmitCounter;

		void updateLastSubmittedCommandBuffer(VulkanCmdBuffer* cmdBuffer);
	};
}