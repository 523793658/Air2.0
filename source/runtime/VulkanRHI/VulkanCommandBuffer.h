#pragma once

#include "CoreMinimal.h"
#include "VulkanConfiguration.h"

namespace Air
{
	class VulkanCommanBufferPool;
	class VulkanCommandBufferManager;
	class VulkanQueue;
	class VulkanCmdBuffer
	{
	public:
		enum class EState : uint8
		{
			ReadyForBegin,
			IsInsideBegin,
			IsInsideRenderPass,
			HasEnded,
			Submitted,
			NotAllocated,
		};


		inline volatile uint64 getFenceSignaledCounter() const
		{
			return mFenceSignaledCounter;
		}

		inline volatile uint64 getFenceSignaledCounterC() const
		{
			return mFenceSignaledCounter;
		}

		inline bool isOutsideRenderPass() const
		{
			return mState == EState::IsInsideBegin;
		}

		inline bool hasBegin() const
		{
			return mState == EState::IsInsideBegin || mState == EState::IsInsideRenderPass;
		}

		inline bool hasEnded() const
		{
			return mState == EState::HasEnded;
		}

		inline bool isSubmitted() const
		{
			return mState == EState::Submitted;
		}

		inline bool isAllocated() const
		{
			return mState != EState::NotAllocated;
		}

		inline VkCommandBuffer getHandle()
		{
			return mCommandBufferHandle;
		}

		VkViewport mCurrentViewport;
		VkRect2D mCurrentScissor;
		uint32 mCurrentStencilRef;
		EState mState;
		uint8 bNeedsDynamicStateSet : 1;
		uint8 bHasPipeline : 1;
		uint8 bHasViewport : 1;
		uint8 bHasScissor : 1;
		uint8 bHasStencilRef : 1;
		uint8 bIsUploadOnly : 1;


		struct PendingQuery
		{
			uint64 mIndex;
			uint64 mCount;
			VkBuffer mBufferHandle;
			VkQueryPool mPoolHandle;
		};

	private:
		volatile uint64 mFenceSignaledCounter;

		VulkanDevice* mDevice;
		VkCommandBuffer mCommandBufferHandle;
		double mSubmittedTime = 0.0f;

		TArray<VkPipelineStageFlags> mWaitFlags;
		TArray<Semaphore*> mWaitSemaphores;
		TArray<Semaphore*> mSubmittedWaitSemaphores;

		TArray<PendingQuery> mPendingQueries;


		void markSemaphoresAsSubmitted()
		{
			mWaitFlags.reset();
			mSubmittedWaitSemaphores = mWaitSemaphores;
			mWaitSemaphores.reset();
		}

		Fence* mFence;

		volatile uint64 fenceSignaledCounter;

		volatile uint64 mSubmittedFenceCounter;

		void refreshFenceStatus();

		void initializeTimings(VulkanCommandListContext* inContext);

		VulkanCommanBufferPool* mCommanBufferPool;

		VulkanGPUTiming* mTiming;

		uint64 mLastValidTiming;

		void acquirePoolSetContainer();

		void allocMemory();

		void freeMemory();
		
	public:
		TMap<uint32, class VulkanTypedDescriptorPoolSet*> mTypedDescripterPoolSets;

		friend class VulkanDynamicRHI;

		friend class TransitionAndLayoutManager;


	};

	class VulkanCommanBufferPool
	{
		VulkanCommanBufferPool(VulkanDevice* inDevice, VulkanCommandBufferManager& inMgr);
		~VulkanCommanBufferPool();

		void refreshFenceStatus(VulkanCmdBuffer* skipCmdBuffer = nullptr);

		inline VkCommandPool getHandle() const
		{
			return mHandle;
		}

		inline CriticalSection* getCS()
		{
			return &mCS;
		}
		void freeUnusedCmdBuffers(VulkanQueue* queue);

		inline VulkanCommandBufferManager& getMgr()
		{
			return mMgr;
		}


		VkCommandPool mHandle;
		TArray<VulkanCmdBuffer*> mCmdBuffers;
		TArray<VulkanCmdBuffer*> mFreeCmdBuffers;

		CriticalSection mCS;
		VulkanDevice* mDevice;

		VulkanCommandBufferManager& mMgr;

		VulkanCmdBuffer* create(bool bIsUploadOnly);

		void create(uint32 queueFamilyIndex);

		friend class VulkanCommandBufferManager;
	};

	class VulkanCommandBufferManager
	{
	public:
		VulkanCommandBufferManager(VulkanDevice* inDevice, VulkanCommandListContext* inContext);
		~VulkanCommandBufferManager();

		VULKANRHI_API void submitUploadCmdBuffer(uint32 numSignalSemaphores = 0, VkSemaphore* signalSemaphores = nullptr);

		VULKANRHI_API VulkanCmdBuffer* getUploadCmdBuffer(uint32 numSignalSemaphores = 0, VkSemaphore* signalSemaphores = nullptr);

		inline VulkanCmdBuffer* getActiveCmdBuffer()
		{
			if (mUploadCmdBuffer)
			{
				submitUploadCmdBuffer();
			}
			return mActiveCmdBuffer;
		}

		inline VulkanCmdBuffer* getActiveCmdBufferDirect()
		{
			return mActiveCmdBuffer;
		}

		inline bool hasPendingUploadCmdBuffer() const
		{
			return mUploadCmdBuffer != nullptr;
		}

		inline bool hasPendingActiveCmdBuffer() const
		{
			return mActiveCmdBuffer != nullptr;
		}

		void submitActiveCmdBuffer(Semaphore* signalSenaphores = nullptr);

		void waitForCmdBuffer(VulkanCmdBuffer* cmdBuffer, float timeInSecondsToWait = 10.0f);

		void refreshFenceStatus(VulkanCmdBuffer* skipCmdBuffer = nullptr)
		{
			mPool.refreshFenceStatus(skipCmdBuffer);
		}

		void prepareForNewActiveCommandBuffer() const;

		inline VkCommandPool getHandle() const
		{
			return mPool.getHandle();
		}

		uint32 calculateGPUTime();

		void freeUnusedCmdBuffers();

	private:
		VulkanDevice* mDevice;
		VulkanCommanBufferPool mPool;
		VulkanQueue* mQueue;
		VulkanCmdBuffer* mActiveCmdBuffer;
		VulkanCmdBuffer* mUploadCmdBuffer;
	};
}