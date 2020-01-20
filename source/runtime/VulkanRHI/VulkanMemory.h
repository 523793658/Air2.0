#pragma once
#include "CoreMinimal.h"
#include "Template/RefCounting.h"
#include "vulkan/vulkan.h"
#include "HAL/CriticalSection.h"
namespace Air
{
	class VulkanDevice;

	class VulkanCmdBuffer;

	class ResourceHeapManager;

	class OldResourceHeap;

	class OldResourcedAllocation;


#define VULKAN_MEMORY_TRACK_FILE_LINE	0

	enum
	{
#if 0
#else
		NUM_FRAMES_TO_WAIT_BEFORE_RELEASING_TO_OS	=10,
#endif

	};

	class RefCount
	{
	public:
		RefCount() {}

		virtual ~RefCount()
		{
			BOOST_ASSERT(mNumRefs.getValue() == 0);
		}

		inline uint32 AddRef()
		{
			int32 newValue = mNumRefs.increment();
			BOOST_ASSERT(newValue > 0);
			return (uint32)newValue;
		}

		inline uint32 Release()
		{
			int32 newValue = mNumRefs.decrement();
			if (newValue == 0)
			{
				delete this;
			}

			BOOST_ASSERT(newValue >= 0);
			return (uint32)newValue;
		}

		inline uint32 getRefCount() const
		{
			int32 value = mNumRefs.getValue();
			BOOST_ASSERT(value >= 0);
			return (uint32)value;
		}
	private: 
		ThreadSafeCounter mNumRefs;
	};

	class DeviceMemoryAllocation
	{
	public:
		DeviceMemoryAllocation()
			:mSize(0)
			,mDeviceHandle(VK_NULL_HANDLE)
			,mHandle(VK_NULL_HANDLE)
			,mMappedPointer(nullptr)
			,mMemoryTypeIndex(0)
			,bCanBeMapped(0)
			,bIsCached(0)
			,bIsCoherent(0)
			,bFreedBySystem(false)
		{

		}

		inline bool canBeMapped() const
		{
			return bCanBeMapped;
		}

		inline bool isMapped() const
		{
			return !!mMappedPointer;
		}

		inline void* getMappedPointer()
		{
			BOOST_ASSERT(isMapped());
			return mMappedPointer;
		}

		inline bool isCoherent() const
		{
			return bIsCoherent != 0;
		}

		inline VkDeviceMemory getHandle() const
		{
			return mHandle;
		}

		inline VkDeviceSize getSize() const
		{
			return mSize;
		}

		inline uint32 getMemoryTypeIndex() const
		{
			return mMemoryTypeIndex;
		}

		void* map(VkDeviceSize size, VkDeviceSize offset);

		void unmap();

		void flushMappedMemory(VkDeviceSize inOffset, VkDeviceSize inSize);

		void invalidateMappedMemory(VkDeviceSize inOffset, VkDeviceSize inSize0);

	protected:
		VkDeviceSize mSize;
		VkDevice mDeviceHandle;
		VkDeviceMemory mHandle;
		void* mMappedPointer;
		uint32 mMemoryTypeIndex : 8;
		uint32 bCanBeMapped : 1;
		uint32 bIsCoherent : 1;
		uint32 bIsCached : 1;
		uint32 bFreedBySystem : 1;
		uint32 : 0;
#if VULKAN_MEMORY_TRACK_FILE_LINE
		const char* mFile;
		uint32 mLine;
		uint32 mUID;


#endif
		~DeviceMemoryAllocation();

		friend class DeviceMemoryManager;
	};





	class DeviceMemoryManager
	{
	public:
		DeviceMemoryManager();
		~DeviceMemoryManager();

		void init(VulkanDevice* inDevice);

		void deinit();

		uint64 getTotalMemory(bool bGPU) const;

		inline uint32 getNumMemoryTypes() const
		{
			return mMemoryProperties.memoryTypeCount;
		}

		inline const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const
		{
			return mMemoryProperties;
		}

		inline VkResult getMemoryTypeFromProperties(uint32 typeBits, VkMemoryPropertyFlags properties, uint32* outTypeIndex)
		{
			for (uint32 i = 0; i < mMemoryProperties.memoryTypeCount && typeBits; i++)
			{
				if ((typeBits & 1) == 1)
				{
					if ((mMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
					{
						*outTypeIndex = i;
						return VK_SUCCESS;
					}
				}
				typeBits >>= 1;
			}

			return VK_ERROR_FEATURE_NOT_PRESENT;
		}

		void free(DeviceMemoryAllocation*& allocation);

	protected:
		VkDevice mDeviceHandle;
		bool bHasUnifiedMemory;
		VulkanDevice* mDevice;
		uint32 mNumAllocations;
		uint32 mPeakNumAllocations;
		VkPhysicalDeviceMemoryProperties mMemoryProperties;
		struct HeapInfo
		{
			VkDeviceSize mTotalSize;
			VkDeviceSize mUsedSize;
			VkDeviceSize mPeakSize;
			TArray<DeviceMemoryAllocation*> mAllocations;
			HeapInfo():
				mTotalSize(0)
				,mUsedSize(0)
				,mPeakSize(0)
			{}
		};

		TArray<HeapInfo> mHeapInfos;
		void setupAndPrintMemInfo();
	};

	class DeviceChild
	{
	public:
		DeviceChild(VulkanDevice* inDevice = nullptr)
			:mDevice(inDevice)
		{}

		inline VulkanDevice* getParent() const
		{
			BOOST_ASSERT(mDevice);
			return mDevice;
		}

		inline void setParent(VulkanDevice* inDevice)
		{
			BOOST_ASSERT(!mDevice);
			mDevice = inDevice;
		}
	protected:
		VulkanDevice* mDevice;
	};

	class Semaphore : public RefCount
	{
	public:
		Semaphore(VulkanDevice& inDevice);

		virtual ~Semaphore();

		inline VkSemaphore getHandle() const
		{
			return mSemaphoreHandle;
		}

	private:
		VulkanDevice& mDevice;
		VkSemaphore mSemaphoreHandle;
	};

	class ResourceSuballocation : public RefCount
	{
	public:
		ResourceSuballocation(uint32 inRequestedSize, uint32 inAlignedOffset, uint32 inAllocationSize, uint32 inAllocationOffset)
			:mRequestedSize(inRequestedSize)
			,mAlignedOffset(inAlignedOffset)
			,mAllocationSize(inAllocationSize)
			,mAllocationOffset(inAllocationOffset)
		{}

		inline uint32 getOffset() const
		{
			return mAlignedOffset;
		}

		inline uint32 getSize() const
		{
			return mRequestedSize;
		}

#if VULKAN_USE_LLM
		void setLLMTrackerID(uint64 inTrackerId) { mLLMTrackerId = inTrackerId; }

		uint64 getLLMTrackerID() { return mLLMTrackerId; }
#endif
	protected:
		uint32 mRequestedSize;
		uint32 mAlignedOffset;
		uint32 mAllocationSize;
		uint32 mAllocationOffset;
#if VULKAN_MEMORY_TRACK_FILE_LINE
		const char* mFile;
		uint32 mLine;
#endif

#if VULKAN_USE_LLM
		uint64 mLLMTrackerId;
#endif
	};

	struct Range
	{
		uint32 mOffset;
		uint32 mSize;
		inline bool operator < (const Range& rhs) const
		{
			return mOffset < rhs.mOffset;
		}

		static void joinConsecutiveRanges(TArray<Range>& ranges);

		static int32 insertAndTryToMerge(TArray<Range>& ranges, const Range& item, int32 proposedIndex);

		static int32 appendAndTryToMerge(TArray<Range>& ranges, const Range& item);

		static void allocateFromEntry(TArray<Range>& ranges, int32 index, uint32 sizeToAllocate);

		static void sanityCheck(TArray<Range>& ranges);

		static int32 add(TArray<Range>& ranges, const Range& item);
	};

	class SubresourceAllocator
	{
	public:

		virtual void destroy(VulkanDevice* device) = 0;
	protected:
		ResourceHeapManager* mOwner;
		uint32 mMemoryTypeIndex;
		VkMemoryPropertyFlags mMemoryPropertyFlags;
		DeviceMemoryAllocation* mMemoryAllocation;
		uint32 mMaxSize;
		uint32 mAlignment;
		uint32 mFrameFreed;
		uint32 mUsedSize;
		static CriticalSection mCS;
		TArray<Range> mFreeList;
		TArray<ResourceSuballocation*> mSuballocations;
		bool jointFreeBlocks();
	};

	class BufferSuballocation;


	class BufferAllocation : public SubresourceAllocator
	{
	public:
		uint32 getMaxSize() { return mMaxSize; }

		virtual void destroy(VulkanDevice* device) override;

		void release(BufferSuballocation* suballocation);
	protected:
		VkBufferUsageFlags mBufferUsageFlags;
		VkBuffer mBuffer;
		uint32 mBufferId;
		int32 mPoolSizeIndex;
		friend class ResourceHeapManager;
	};

	static inline uint32 getMaxSize(BufferAllocation* allocation)
	{
		return allocation->getMaxSize();
	}
	class BufferSuballocation;

	class ResourceHeapManager : public DeviceChild
	{
	public:
		ResourceHeapManager(VulkanDevice* inDevice);
		~ResourceHeapManager();

		void init();
		void deinit();

		void processPendingUBFreesNoLock(bool bForce);
		
		void processPendingUBFrees(bool bForce);

		void destroyResourceAllocations();

		void releaseBuffer(BufferAllocation* bufferAllocation);

		struct UBPendingFree
		{
			BufferSuballocation* mAllocation = nullptr;
			uint64 mFrame = 0;
		};

		struct
		{
			CriticalSection mCS;
			TArray<UBPendingFree> mPendingFree;
			uint32 mPeak = 0;
		} UBAllocations;
	protected:
		void releaseFreedResources(bool bImmediately);


	protected:
		DeviceMemoryManager* mDeviceMemoryManager;

		TArray<OldResourceHeap*> mResourceTypeHeaps;
		enum
		{
			BufferAllocationSize = 1 * 1024 * 1024,
			UniformBufferAllocationSize = 2 * 1024 * 1024
		};

		enum class EPoolSizes : uint8
		{
			E128,
			E256,
			E512,
			E1k,
			E2k,
			E8k,
			E16k,
			SizesCount,
		};

		TArray<BufferAllocation*> mUsedBufferAllocations[(int32)EPoolSizes::SizesCount + 1];
		TArray<BufferAllocation*> mFreeBufferAllocations[(int32)EPoolSizes::SizesCount + 1];
	};

	class BufferSuballocation : public ResourceSuballocation
	{
	public:
		BufferSuballocation(BufferAllocation* inOwner, VkBuffer inHandle, uint32 inRequestedSize, uint32 inAlignedOffset, uint32 inAllocationSize, uint32 inAllocationOffset)
			:ResourceSuballocation(inRequestedSize, inAlignedOffset, inAllocationSize, inAllocationOffset)
			,mOwner(inOwner)
			,mHandle(inHandle)
		{
			uint32 size = getMaxSize(inOwner);
			BOOST_ASSERT(inAlignedOffset <= size);
			BOOST_ASSERT(inAllocationOffset + inAllocationSize <= size);
			BOOST_ASSERT(inAlignedOffset + inRequestedSize <= size);
		}

		virtual ~BufferSuballocation();

		inline VkBuffer getHandle() const
		{
			return mHandle;
		}

		inline BufferAllocation* getBufferAllocation()
		{
			return mOwner;
		}

		void* getMappedPointer();

		void flush();

	protected:
		friend class BufferAllocation;
		BufferAllocation* mOwner;
		VkBuffer mHandle;
	};

	class DeferredDeletionQueue : public DeviceChild
	{
	public:
		DeferredDeletionQueue(VulkanDevice* inDevice);
		~DeferredDeletionQueue();

		enum class EType
		{
			RenderPass,
			Buffer,
			BufferView,
			Image,
			ImageView,
			Pipeline,
			PipelineLayout,
			Framebuffer,
			DescriptorSetLayout,
			Sampler,
			Semaphore,
			ShaderModule,
			Event,
		};

		template<typename T>
		inline void enqueueResource(EType type, T handle)
		{
			static_assert(sizeof(T) <= sizeof(uint64), "Vulkan resource handle type size too large.");
			enqueueGenericResource(type, (uint64)handle);

		}

		void releaseResources(bool bDeleteImmediately = false);

		inline void clear()
		{
			releaseResources(true);
		}

		void onCmdBufferDeleted(VulkanCmdBuffer* cmdBuffer);

	private:
		void enqueueGenericResource(EType type, uint64 handle);

		struct Entry
		{
			uint64 mFenceCounter;
			uint64 mHandle;
			VulkanCmdBuffer* mCmdBuffer;
			EType mStructureType;
			uint32 mFrameNumber;
		};
		CriticalSection mCS;
		TArray<Entry> mEntries;
	};

	class OldResourceHeapPage
	{
	
	protected:
		friend class ResourceHeapManager;
	};

	class OldResourceHeap
	{
	public:
		enum class EType
		{
			Image,
			Buffer,
		};

		OldResourceHeap(ResourceHeapManager* inOwner, uint32 inMemoryTypeIndex, uint32 inPageSize);

	protected:
		ResourceHeapManager* mOwner;

		uint32 mMemoryTypeIndex;
		bool bIsHostCacheSupports;
		bool bIsLazilyAllocatedSupported;

#if VULKAN_FREEPAGE_FOR_TYPE
		uint32 mDefaultPageSizeForImage;
		uint32 mDefaultPageSizeForBuffer;
#endif
		uint32 mDefaultPageSize;
		uint32 mPeakPageSize;
		uint64 mUsedMemory;
		uint32 mPageIDCounter;

		TArray<OldResourceHeapPage*> mUsedBufferPage;
		TArray<OldResourceHeapPage*> mUsedImagePages;

#if VULKAN_FREEPAGE_FOR_TYPE
		TArray<OldResourceHeapPage*> mFreeBufferPages;
		TArray<OldResourceHeapPage*> mFreeImagePages;
#else
		TArray<OldResourceHeapPage*> mFreePages;
#endif

		OldResourcedAllocation* allocateResource(EType type, uint32 size, uint32 alignmemt, bool bMapAllocation, const char* file, uint32 line);

#if VULKAN_SUPPORTS_DEDICATED_ALLOCATION
		TArray<OldResourceHeapPage*> mUsedDedicatedImagePages;
		TArray<OldResourceHeapPage*> mUsedDedicatedBufferPages;

		OldResourcedAllocation* allocateDedicatedImage(VkImage image, uint32 size, uint32 alignment, const char* file, uint32 line);
#endif
		friend class ResourceHeapManager;

	};

	

	class OldResourcedAllocation : public RefCount
	{
	public:
	private:
		
	};

	class StagingBuffer : public RefCount
	{
	protected:
		//TRefCountPtr<OldResource
	};
}