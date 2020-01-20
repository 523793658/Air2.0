#include "VulkanRHIPrivate.h"
#include "VulkanMemory.h"
#include "VulkanDevice.h"
namespace Air
{

	uint32 GVulkanRHIDeletionFrameNumber = 0;

	const uint32 NUM_FRAMES_TO_WAIT_FOR_RESOURCE_DELETE = 2;

	static CriticalSection GDeviceMemLock;
	static CriticalSection GResourceHeapLock;

#define VULKAN_FAKE_MEMORY_LIMIT	011u

#if (_DEBUG)
	static int32 GForceCoherent = 0;
#else
	constexpr int32 GForceCoherent = 0;
#endif

#if VULKAN_FAKE_MEMORY_LIMIT
	static uint64 GDeviceMemAllocated = 0;
#endif

	enum
	{
		GPU_ONLY_HEAP_PAGE_SIZE = 256 * 1024 * 1024,
		STAGING_HEAP_PAGE_SIZE = 32 * 1024 * 1024,
		ANDROID_MAX_HEAP_PAGE_SIZE = 16 * 1024 * 1024
	};


	void* DeviceMemoryAllocation::map(VkDeviceSize size, VkDeviceSize offset)
	{
		BOOST_ASSERT(bCanBeMapped);
		BOOST_ASSERT(!mMappedPointer);
		BOOST_ASSERT(size == VK_WHOLE_SIZE || size + offset <= mSize);
		VERIFYVULKANRESULT(vkMapMemory(mDeviceHandle, mHandle, offset, size, 0, &mMappedPointer));
		return mMappedPointer;
	}

	void DeviceMemoryAllocation::unmap()
	{
		BOOST_ASSERT(mMappedPointer);
		vkUnmapMemory(mDeviceHandle, mHandle);
		mMappedPointer = nullptr;
	}

	void DeviceMemoryAllocation::flushMappedMemory(VkDeviceSize inOffset, VkDeviceSize inSize)
	{
		if (!isCoherent() || GForceCoherent != 0)
		{
			BOOST_ASSERT(isMapped());
			BOOST_ASSERT(inOffset + inSize <= mSize);
			VkMappedMemoryRange range;
			zeroVulkanStruct(range, VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE);
			range.memory = mHandle;
			range.offset = inOffset;
			range.size = inSize;
			VERIFYVULKANRESULT(vkFlushMappedMemoryRanges(mDeviceHandle, 1, &range));
		}
	}

	void DeviceMemoryAllocation::invalidateMappedMemory(VkDeviceSize inOffset, VkDeviceSize inSize)
	{
		if (!isCoherent() || GForceCoherent != 0)
		{
			BOOST_ASSERT(isMapped());
			BOOST_ASSERT(inOffset + inSize <= mSize);
			VkMappedMemoryRange range;
			zeroVulkanStruct(range, VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE);
			range.memory = mHandle;
			range.offset = inOffset;
			range.size = inSize;
			VERIFYVULKANRESULT(vkInvalidateMappedMemoryRanges(mDeviceHandle, 1, &range));
		}
	}


	DeviceMemoryManager::DeviceMemoryManager()
		:mDeviceHandle(VK_NULL_HANDLE),
		bHasUnifiedMemory(false),
		mDevice(nullptr),
		mNumAllocations(0),
		mPeakNumAllocations(0)
	{
		Memory::memzero(mMemoryProperties);
	}

	void DeviceMemoryManager::init(VulkanDevice* inDevice)
	{
		BOOST_ASSERT(mDevice == nullptr);
		mDevice = inDevice;
		mNumAllocations = 0;
		mPeakNumAllocations = 0;

		mDeviceHandle = mDevice->getInstanceHandle();
		vkGetPhysicalDeviceMemoryProperties(inDevice->getPhysicalHandle(), &mMemoryProperties);
		mHeapInfos.addDefaulted(mMemoryProperties.memoryHeapCount);
		setupAndPrintMemInfo();
	}

	void DeviceMemoryManager::setupAndPrintMemInfo()
	{
		const uint32 maxAllocations = mDevice->getLimits().maxMemoryAllocationCount;
		AIR_LOG(logVulkanRHI, Display, TEXT("%d Device Memory Heaps; Max memory allocations %d"), mMemoryProperties.memoryHeapCount, maxAllocations);
		for (uint32 index = 0; index < mMemoryProperties.memoryHeapCount; ++index)
		{
			bool bIsGPUHeap = ((mMemoryProperties.memoryHeaps[index].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
			AIR_LOG(LobVulkanRHI, Display, TEXT("%d: Flags 0x%x Size %llu (%.2f MB) %s"), index, mMemoryProperties.memoryHeaps[index].flags, mMemoryProperties.memoryHeaps[index].size,
				(float)((double)mMemoryProperties.memoryHeaps[index].size / 1024.0 / 1024.0), bIsGPUHeap ? TEXT("GPU") : TEXT(""));
			mHeapInfos[index].mTotalSize = mMemoryProperties.memoryHeaps[index].size;
		}

		bHasUnifiedMemory = VulkanPlatform::hasUnifiedMemory();
		AIR_LOG(logVulkanRHI, Display, TEXT("%d Device Memory Types (%sunified)"), mMemoryProperties.memoryTypeCount, bHasUnifiedMemory ? TEXT("") : TEXT("Not "));
		for (uint32 index = 0; index < mMemoryProperties.memoryTypeCount; ++index)
		{
			auto getFlagsString = [](VkMemoryPropertyFlags flags)
			{
				wstring str;
				if ((flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
				{
					str += TEXT(" Local");
				}
				if ((flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
				{
					str += TEXT(" HostVisible");
				}
				if ((flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
				{
					str += TEXT("HostCoherent");
				}
				if ((flags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) == VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
				{
					str += TEXT("HostCached");
				}
				if ((flags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) == VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
				{
					str += TEXT("Lazy");
				}
				return str;
			};
			AIR_LOG(logVulkanRHI, Display, TEXT("%d: Flags 0x%x Heap %d %s"), index, mMemoryProperties.memoryTypes[index].propertyFlags,
				mMemoryProperties.memoryTypes[index].heapIndex,
				getFlagsString(mMemoryProperties.memoryTypes[index].propertyFlags).c_str());
		}
		for (uint32 index = 0; index < mMemoryProperties.memoryHeapCount; ++index)
		{
			const bool bIsGPUHeap = ((mMemoryProperties.memoryHeaps[index].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
			if (bIsGPUHeap)
			{
				mHeapInfos[index].mTotalSize = (uint64)((float)mHeapInfos[index].mTotalSize * 0.95f);
			}
		}
	}

	void DeviceMemoryManager::deinit()
	{
		for (int32 index = 0; index < mHeapInfos.size(); ++index)
		{
			if (mHeapInfos[index].mAllocations.size())
			{
				AIR_LOG(LogVulkanRHI, Warning, TEXT("Found %d unfreed allocations!"), mHeapInfos[index].mAllocations.size());
#if 0
				
#endif
			}
		}
		mNumAllocations = 0;
	}
	DeviceMemoryManager::~DeviceMemoryManager()
	{
		deinit();
	}

	uint64 DeviceMemoryManager::getTotalMemory(bool bGPU) const
	{
		uint64 totalMemory = 0;
		for (uint32 index = 0; index < mMemoryProperties.memoryHeapCount; ++index)
		{
			const bool bIsGPUHeap = ((mMemoryProperties.memoryHeaps[index].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
			if (bIsGPUHeap == bGPU)
			{
				totalMemory += mHeapInfos[index].mTotalSize;
			}
		}
		return totalMemory;
	}

	void DeviceMemoryManager::free(DeviceMemoryAllocation*& allocation)
	{
		ScopeLock lock(&GDeviceMemLock);
		BOOST_ASSERT(allocation);
		BOOST_ASSERT(allocation->mHandle != VK_NULL_HANDLE);
		BOOST_ASSERT(!allocation->bFreedBySystem);
#if VULKAN_FAKE_MEMORY_LIMIT
		GDeviceMemAllocated -= allocation->mSize;
#endif

		vkFreeMemory(mDeviceHandle, allocation->mHandle, VULKAN_CPU_ALLOCATOR);

#if VULKAN_USE_LLM
		//LLM(LowLevelMemTracker)
#endif
		--mNumAllocations;
		
		uint32 heapIndex = mMemoryProperties.memoryTypes[allocation->mMemoryTypeIndex].heapIndex;

		mHeapInfos[heapIndex].mUsedSize -= allocation->mSize;
		mHeapInfos[heapIndex].mAllocations.removeSwap(allocation);
		allocation->bFreedBySystem = true;
		delete allocation;
		allocation = nullptr;
	}

	DeviceMemoryAllocation::~DeviceMemoryAllocation()
	{
		BOOST_ASSERT(bFreedBySystem);
	}
	

	Semaphore::Semaphore(VulkanDevice& inDevice)
		:mDevice(inDevice)
		, mSemaphoreHandle(VK_NULL_HANDLE)
	{
		VkSemaphoreCreateInfo createInfo;
		zeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
		VERIFYVULKANRESULT(vkCreateSemaphore(mDevice.getInstanceHandle(), &createInfo, VULKAN_CPU_ALLOCATOR, &mSemaphoreHandle));
	}

	Semaphore::~Semaphore()
	{
		BOOST_ASSERT(mSemaphoreHandle != VK_NULL_HANDLE);
		mDevice.getDeferredDeletionQueue().enqueueResource(DeferredDeletionQueue::EType::Semaphore, mSemaphoreHandle);
		mSemaphoreHandle = VK_NULL_HANDLE;
	}

	DeferredDeletionQueue::DeferredDeletionQueue(VulkanDevice* inDevice)
		:DeviceChild(inDevice)
	{}

	DeferredDeletionQueue::~DeferredDeletionQueue()
	{
		BOOST_ASSERT(mEntries.size() == 0);
	}

	void DeferredDeletionQueue::enqueueGenericResource(EType type, uint64 handle)
	{
		VulkanQueue* queue = mDevice->getGraphicsQueue();
		Entry entry;
		queue->getLastSubmittedInfo(entry.mCmdBuffer, entry.mFenceCounter);
		entry.mHandle = handle;
		entry.mStructureType = type;
		entry.mFrameNumber = GVulkanRHIDeletionFrameNumber;
		{
			ScopeLock scopeLock(&mCS);

#if VULKAN_HAS_DEBUGGING_ENABLED
			Entry* existingEntry = mEntries.findByPredicate([&](const Entry& inEntry)
				{
					return inEntry.mHandle == entry.mHandle;
				});
			BOOST_ASSERT(existingEntry == nullptr);
#endif
			mEntries.add(entry);
		}
	}

	void DeferredDeletionQueue::releaseResources(bool bDeleteImmediately /* = false */)
	{
		ScopeLock scopeLock(&mCS);

		VkDevice deviceHandle = mDevice->getInstanceHandle();
		for (int32 index = mEntries.size() - 1; index >= 0; --index)
		{
			Entry* entry = &mEntries[index];
			if (bDeleteImmediately || (GVulkanRHIDeletionFrameNumber > entry->mFrameNumber + NUM_FRAMES_TO_WAIT_FOR_RESOURCE_DELETE && (entry->mCmdBuffer == nullptr || entry->mFenceCounter < entry->mCmdBuffer->getFenceSignaledCounterC())))
			{
				switch (entry->mStructureType)
				{
#define VKSWITCH(Type, ...) case EType::Type: __VA_ARGS__; vkDestroy##Type(deviceHandle, (Vk##Type)entry->mHandle, VULKAN_CPU_ALLOCATOR); break
					VKSWITCH(RenderPass);
					VKSWITCH(Buffer);
					VKSWITCH(BufferView);
					VKSWITCH(Image);
					VKSWITCH(ImageView);
					VKSWITCH(Pipeline);
					VKSWITCH(PipelineLayout);
					VKSWITCH(Framebuffer);
					VKSWITCH(DescriptorSetLayout);
					VKSWITCH(Sampler);
					VKSWITCH(Semaphore);
					VKSWITCH(ShaderModule);
					VKSWITCH(Event);
#undef VKSWITCH
				default:
					BOOST_ASSERT(false);
					break;
				}
				mEntries.removeAtSwap(index, 1, false);
			}
		}
	}

	void DeferredDeletionQueue::onCmdBufferDeleted(VulkanCmdBuffer* cmdBuffer)
	{
		ScopeLock scopeLock(&mCS);
		for (int32 index = 0; index < mEntries.size(); ++index)
		{
			Entry& entry = mEntries[index];
			if (entry.mCmdBuffer == cmdBuffer)
			{
				entry.mCmdBuffer = nullptr;
			}
		}
	}

	ResourceHeapManager::ResourceHeapManager(VulkanDevice* inDevice)
		:DeviceChild(inDevice)
		,mDeviceMemoryManager(&inDevice->getMemoryMananger())
	{}

	ResourceHeapManager::~ResourceHeapManager()
	{
		deinit();
	}

	void ResourceHeapManager::init()
	{
		DeviceMemoryManager& memoryManager = mDevice->getMemoryMananger();
		const uint32 typeBits = (1 << memoryManager.getNumMemoryTypes()) - 1;

		const VkPhysicalDeviceMemoryProperties& memoryProperties = memoryManager.getMemoryProperties();

		mResourceTypeHeaps.addZeroed(memoryProperties.memoryTypeCount);

		auto getMemoryTypesFromProperties = [memoryProperties](uint32 inTypeBits, VkMemoryPropertyFlags properties, TArray<uint32>& outTypeIndices)
		{
			for (uint32 i = 0; i < memoryProperties.memoryTypeCount && inTypeBits; i++)
			{
				if ((inTypeBits & 1) == 1)
				{
					if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
					{
						outTypeIndices.add(i);
					}
				}
				inTypeBits >>= 1;
			}

			for (uint32 index = outTypeIndices.size() - 1; index >= 1; --index)
			{
				if (memoryProperties.memoryTypes[index].propertyFlags != memoryProperties.memoryTypes[0].propertyFlags)
				{
					outTypeIndices.removeAtSwap(index, 1, false);
				}
			}

			return outTypeIndices.size() > 0;
		};

		{
			TArray<uint32> typeIndices;
			getMemoryTypesFromProperties(typeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, typeIndices);
			BOOST_ASSERT(typeIndices.size() > 0);

			for (int32 index = 0; index < typeIndices.size(); ++index)
			{
				int32 heapIndex = memoryProperties.memoryTypes[typeIndices[index]].heapIndex;
				VkDeviceSize heapSize = memoryProperties.memoryHeaps[heapIndex].size;
				if (VULKAN_FAKE_MEMORY_LIMIT)
				{
					heapSize = Math::min<VkDeviceSize>(VULKAN_FAKE_MEMORY_LIMIT << 30llu, heapSize);
				}

				VkDeviceSize pageSize = Math::min<VkDeviceSize>(heapSize / 8, GPU_ONLY_HEAP_PAGE_SIZE);
				mResourceTypeHeaps[typeIndices[index]] = new OldResourceHeap(this, typeIndices[index], pageSize);
				mResourceTypeHeaps[typeIndices[index]]->bIsHostCacheSupports = ((memoryProperties.memoryTypes[index].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) == VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
				mResourceTypeHeaps[typeIndices[index]]->bIsLazilyAllocatedSupported = ((memoryProperties.memoryTypes[index].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) == VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
			}
		}
		{
			uint32 typeIndex = 0;
			VERIFYVULKANRESULT(memoryManager.getMemoryTypeFromProperties(typeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &typeIndex));
			uint64 heapSize = memoryProperties.memoryHeaps[memoryProperties.memoryTypes[typeIndex].heapIndex].size;
			mResourceTypeHeaps[typeIndex] = new OldResourceHeap(this, typeIndex, STAGING_HEAP_PAGE_SIZE);
		}

		{
			uint32 typeIndex = 0;
			{
				uint32 hostVisCachedIndex = 0;
				VkResult hostCachedResult = memoryManager.getMemoryTypeFromProperties(typeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, &hostVisCachedIndex);
				uint32 hostVisIndex = 0;
				VkResult hostResult = memoryManager.getMemoryTypeFromProperties(typeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &hostVisIndex);
				if (hostCachedResult == VK_SUCCESS)
				{
					typeIndex = hostVisCachedIndex;
				}
				else if (hostResult == VK_SUCCESS)
				{
					typeIndex = hostVisIndex;
				}
				else
				{
					AIR_LOG(logVulkanRHI, Fatal, TEXT("No memory type found supporting VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT!"));
				}
			}

			uint64 heapSize = memoryProperties.memoryHeaps[memoryProperties.memoryTypes[typeIndex].heapIndex].size;
			mResourceTypeHeaps[typeIndex] = new OldResourceHeap(this, typeIndex, STAGING_HEAP_PAGE_SIZE);
		}
	}

	void ResourceHeapManager::deinit()
	{
		{
			processPendingUBFreesNoLock(true);
			BOOST_ASSERT(UBAllocations.mPendingFree.size() == 0);
		}

		destroyResourceAllocations();

		for (int32 index = 0; index < mResourceTypeHeaps.size(); ++index)
		{
			delete mResourceTypeHeaps[index];
			mResourceTypeHeaps[index] = nullptr;
		}

		mResourceTypeHeaps.empty(0);
	}

	void ResourceHeapManager::processPendingUBFreesNoLock(bool bForce)
	{
		static uint32 GFrameNumberRenderThread_WhenWeCanDelete = 0;
		if (UNLIKELY(bForce))
		{
			int32 numAlloc = UBAllocations.mPendingFree.size();
			for (int32 index = 0; index < numAlloc; ++index)
			{
				UBPendingFree& alloc = UBAllocations.mPendingFree[index];
				delete alloc.mAllocation;
			}
			UBAllocations.mPendingFree.empty();

			GFrameNumberRenderThread_WhenWeCanDelete = 0;
		}
		else
		{
			if (LIKELY(GFrameNumberRenderThread < GFrameNumberRenderThread_WhenWeCanDelete))
			{
				return;
			}

			int32 oldestFrameToKeep = GFrameNumberRenderThread - NUM_FRAMES_TO_WAIT_BEFORE_RELEASING_TO_OS;
			int32 numAlloc = UBAllocations.mPendingFree.size();
			int32 index = 0;
			for (; index < numAlloc; ++index)
			{
				UBPendingFree& alloc = UBAllocations.mPendingFree[index];
				if (LIKELY(alloc.mFrame < oldestFrameToKeep))
				{
					delete alloc.mAllocation;

				}
				else
				{
					GFrameNumberRenderThread_WhenWeCanDelete = alloc.mFrame + NUM_FRAMES_TO_WAIT_BEFORE_RELEASING_TO_OS + 1;
					break;
				}
			}
			int32 elementsLeft = numAlloc - index;
			if (elementsLeft > 0 && elementsLeft != numAlloc)
			{
				Memory::memmove(UBAllocations.mPendingFree.getData(), UBAllocations.mPendingFree.getData() + index, elementsLeft * sizeof(UBPendingFree));
			}
			UBAllocations.mPendingFree.setNum(numAlloc - index, false);
		}
	}

	void ResourceHeapManager::processPendingUBFrees(bool bForce)
	{
		ScopeLock scopeLock(&UBAllocations.mCS);
		processPendingUBFreesNoLock(bForce);
	}

	void ResourceHeapManager::destroyResourceAllocations()
	{
		releaseFreedResources(true);
		for (auto& usedAllocations : mUsedBufferAllocations)
		{
			for (int32 index = usedAllocations.size() - 1; index >= 0; --index)
			{
				BufferAllocation* bufferAllocation = usedAllocations[index];
				if (!bufferAllocation->jointFreeBlocks())
				{
					AIR_LOG(logVulkanRHI, Warning, TEXT("suballocation(s) for buffer %p were not released."), (void*)bufferAllocation->mBuffer);
				}
				bufferAllocation->destroy(getParent());
				getParent()->getMemoryMananger().free(bufferAllocation->mMemoryAllocation);
				delete bufferAllocation;
			}
			usedAllocations.empty(0);
		}
		for (auto& freeAllocations : mFreeBufferAllocations)
		{
			for (int32 index = 0; index < freeAllocations.size(); ++index)
			{
				BufferAllocation* bufferAllocation = freeAllocations[index];
				bufferAllocation->destroy(getParent());
				getParent()->getMemoryMananger().free(bufferAllocation->mMemoryAllocation);
				delete bufferAllocation;
			}
			freeAllocations.empty(0);
		}
	}

	void ResourceHeapManager::releaseFreedResources(bool bImmediately)
	{
		BufferAllocation* bufferAllocationToRelase = nullptr;
		{
			ScopeLock scopeLock(&GResourceHeapLock);
			for (auto& freeAllocations : mFreeBufferAllocations)
			{
				for (int32 index = 0; index < freeAllocations.size(); ++index)
				{
					BufferAllocation* bufferAllocation = freeAllocations[index];
					if (bImmediately || bufferAllocation->mFrameFreed + NUM_FRAMES_TO_WAIT_BEFORE_RELEASING_TO_OS < GFrameNumberRenderThread)
					{
						bufferAllocationToRelase = bufferAllocation;
						freeAllocations.removeAtSwap(index, 1, false);
						break;
					}
				}
			}
		}
		if (bufferAllocationToRelase)
		{
			bufferAllocationToRelase->destroy(getParent());
			getParent()->getMemoryMananger().free(bufferAllocationToRelase->mMemoryAllocation);
			delete bufferAllocationToRelase;
		}
	}


#define AIR_VK_MEMORY_KEEP_FREELIST_SORTED			1
#define AIR_VK_MEMORY_JOIN_FREELIST_ON_THE_FLY			(AIR_VK_MEMORY_KEEP_FREELIST_SORTED && 1)

#define AIR_VK_MEMORY_KEEP_FREELIST_SORTED_CATCHBUGS	0

	int32 Range::insertAndTryToMerge(TArray<Range>& ranges, const Range& item, int32 proposedIndex)
	{
#if !AIR_VK_MEMORY_JOIN_FREELIST_ON_THE_FLY
		int32 ret = ranges.insert(item, proposedIndex);
#else
		BOOST_ASSERT(item.mOffset < ranges[proposedIndex].mOffset);
		int32 ret = proposedIndex;
		if (UNLIKELY(proposedIndex == 0))
		{
			Range& nextRange = ranges[ret];
			if (UNLIKELY(nextRange.mOffset == item.mOffset + item.mSize))
			{
				nextRange.mOffset = item.mOffset;
				nextRange.mSize += item.mSize;
			}
			else
			{
				ret = ranges.insert(item, proposedIndex);
			}
		}
		else
		{
			Range& nextRange = ranges[proposedIndex];
			Range& prevRange = ranges[proposedIndex - 1];
			if (UNLIKELY(prevRange.mOffset + prevRange.mSize == item.mOffset))
			{
				prevRange.mSize += item.mSize;
				if (UNLIKELY(prevRange.mOffset + prevRange.mSize == nextRange.mOffset))
				{
					prevRange.mSize += nextRange.mSize;
					ranges.removeAt(proposedIndex);
					ret = proposedIndex - 1;
				}
			}
			else if (UNLIKELY(item.mOffset + item.mSize == nextRange.mOffset))
			{
				nextRange.mOffset = item.mOffset;
				nextRange.mSize += item.mSize;
			}
			else
			{
				ret = ranges.insert(item, proposedIndex);
			}
		}
#endif
#if AIR_VK_MEMORY_KEEP_FREELIST_SORTED_CATCHBUGS
		sanityCheck(ranges);
#endif
		return ret;
	}

	int32 Range::appendAndTryToMerge(TArray<Range>& ranges, const Range& item)
	{
#if !AIR_VK_MEMORY_JOIN_FREELIST_ON_THE_FLY
		int32 ret = ranges.add(item);
#else
		int32 ret = ranges.size() - 1;
		BOOST_ASSERT(ret >= 0);
		Range& prevRangs = ranges[ret];
		if (UNLIKELY(prevRangs.mOffset + prevRangs.mSize == item.mOffset))
		{
			prevRangs.mSize += item.mSize;
		}
		else
		{
			ret = ranges.add(item);
		}
#endif
#if AIR_VK_MEMORY_KEEP_FREELIST_SORTED_CATCHBUGS
		sanityCheck(ranges);
#endif
		return ret;
	}

	void Range::allocateFromEntry(TArray<Range>& ranges, int32 index, uint32 sizeToAllocate)
	{
		Range& entry = ranges[index];
		if (sizeToAllocate < entry.mSize)
		{
			entry.mSize -= sizeToAllocate;
			entry.mOffset += sizeToAllocate;
		}
		else
		{
			ranges.removeAt(index, 1, false);
#if AIR_VK_MEMORY_KEEP_FREELIST_SORTED_CATCHBUGS
			sanityCheck(ranges);
#endif
		}
	}

	void Range::sanityCheck(TArray<Range>& ranges)
	{
		if (AIR_VK_MEMORY_KEEP_FREELIST_SORTED_CATCHBUGS)
		{
			int32 num = ranges.size();
			if (num > 1)
			{
				for (int32 chkIndex = 0; chkIndex < num - 1; ++chkIndex)
				{
					BOOST_ASSERT(ranges[chkIndex].mOffset < ranges[chkIndex + 1].mOffset);
#if AIR_VK_MEMORY_JOIN_FREELIST_ON_THE_FLY
					BOOST_ASSERT(ranges[chkIndex].mOffset + ranges[chkIndex].mSize < ranges[chkIndex + 1].mOffset);
#else
					BOOST_ASSERT(ranges[chkIndex].mOffset + ranges[chkIndex].mSize<= ranges[chkIndex + 1].mOffset);
#endif
				}
			}
		}
	}


	int32 Range::add(TArray<Range>& ranges, const Range& item)
	{
#if AIR_VK_MEMORY_KEEP_FREELIST_SORTED
		int32 numRanges = ranges.size();
		if (LIKELY(numRanges <= 0))
		{
			return ranges.add(item);
		}
		Range* data = ranges.getData();
		for (int32 index = 0; index < numRanges; ++index)
		{
			if (UNLIKELY(data[index].mOffset > item.mOffset))
			{
				return insertAndTryToMerge(ranges, item, index);
			}
		}
		return appendAndTryToMerge(ranges, item);
#else
		return ranges.add(item);
#endif

	}






	void Range::joinConsecutiveRanges(TArray<Range>& ranges)
	{
		if (ranges.size() > 1)
		{
#if !AIR_VK_MEMORY_KEEP_FREELIST_SORTED
			ranges.sort();
#else
#if AIR_VK_MEMORY_KEEP_FREELIST_SORTED_CATCHBUGS
			sanityCheck(ranges);
#endif
#endif
#if !AIR_VK_MEMORY_JOIN_FREELIST_ON_THE_FLY
			for (int32 index = ranges.size() - 1; index > 0; --index)
			{
				Range& current = ranges[index];
				Range& prev = ranges[index - 1];
				if (prev.mOffset + prev.mSize == current.mOffset)
				{
					prev.mSize += current.mSize;
					ranges.removeAt(index, 1, false);
				}
			}
#endif
		}
	}

	bool SubresourceAllocator::jointFreeBlocks()
	{
		ScopeLock scopeLock(&mCS);
#if !AIR_VK_MEMORY_JOIN_FREELIST_ON_THE_FLY
		Range::joinConsecutiveRanges(mFreeList);
#endif
		if (mFreeList.size() == 1)
		{
			if (mSuballocations.size() == 0)
			{
				BOOST_ASSERT(mUsedSize == 0);
				BOOST_ASSERT(mFreeList[0].mOffset == 0 && mFreeList[0].mSize == mMaxSize);
				return true;
			}
		}
		return false;
	}

	void BufferAllocation::destroy(VulkanDevice* device)
	{
		vkDestroyBuffer(device->getInstanceHandle(), mBuffer, VULKAN_CPU_ALLOCATOR);
		mBuffer = VK_NULL_HANDLE;
	}

	void BufferAllocation::release(BufferSuballocation* suballocation)
	{
		{
			ScopeLock scopeLock(&mCS);
			mSuballocations.removeSingleSwap(suballocation, false);
			Range newFree;
			newFree.mOffset = suballocation->mAllocationOffset;
			newFree.mSize = suballocation->mAllocationSize;
			BOOST_ASSERT(newFree.mOffset <= getMaxSize());
			BOOST_ASSERT(newFree.mOffset + newFree.mSize <= getMaxSize());

			Range::add(mFreeList, newFree);

			mUsedSize -= suballocation->mAllocationSize;
			BOOST_ASSERT(mUsedSize >= 0);
		}
		mOwner->releaseBuffer(this);
	}

	BufferSuballocation::~BufferSuballocation()
	{
		mOwner->release(this);
	}
}