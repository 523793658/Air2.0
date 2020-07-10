#include "VulkanRHIPrivate.h"
#include "VulkanResources.h"
#include "VulkanDevice.h"
namespace Air
{
	static FORCEINLINE void updateVulkanBufferStats(uint64_t size, VkBufferUsageFlags usage, bool allocating)
	{

	}


	VulkanResourceMultiBuffer::VulkanResourceMultiBuffer(VulkanDevice* inDevice, VkBufferUsageFlags inBufferUsageFlags, uint32 inSize, uint32 inUsage, RHIResourceCreateInfo& createInfo, class RHICommandListImmediate* inRHICmdList /* = nullptr */)
		:DeviceChild(inDevice)
		,mUsage(inUsage)
		,mBufferUsageFlags(inBufferUsageFlags)
		,mNumBuffers(0)
		,mDynamicBufferIndex(0)
	{
		if (inSize > 0)
		{
			const bool bStatic = (inUsage & BUF_Static) != 0;
			const bool bDynamic = (inUsage & BUF_Dynamic) != 0;
			const bool bVolatile = (inUsage & BUF_Volatile) != 0;
			const bool bShaderResource = (inUsage & BUF_ShaderResource) != 0;
			const bool bIsConstantBuffer = (inBufferUsageFlags & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) != 0;
			const bool bUAV = (inUsage & BUF_UnorderedAccess) != 0;
			const bool bIndirect = (inUsage & BUF_DrawIndirect) == BUF_DrawIndirect;
			const bool bCPUReadable = (inUsage & BUF_KeepCPUAccessible) != 0;

			mBufferUsageFlags |= bVolatile ? 0 : VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			mBufferUsageFlags |= (bShaderResource && !bIsConstantBuffer) ? VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT : 0;
			mBufferUsageFlags |= bUAV ? VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT : 0;
			mBufferUsageFlags |= bIndirect ? VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT : 0;
			mBufferUsageFlags |= bCPUReadable ? (VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT) : 0;

			if (bVolatile)
			{
				bool bRenderThread = isInRenderingThread();

				void* data = lock(bRenderThread, RLM_WriteOnly, inSize, 0);

				Memory::memzero(data, inSize);

				unlock(bRenderThread);
			}
			else
			{
				VkDevice vulkanDevice = inDevice->getInstanceHandle();
				VkMemoryPropertyFlags bufferMemFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
				const bool bUnifiedMem = inDevice->hasUnifiedMemory();
				if (bUnifiedMem)
				{
					bufferMemFlags |= (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
				}

				mNumBuffers = bDynamic ? NUM_BUFFERS : 1;

				BOOST_ASSERT(mNumBuffers <= ARRAY_COUNT(mBuffers));

				for (uint32 index = 0; index < mNumBuffers; ++index)
				{
					mBuffers[index] = inDevice->getResourceHeapManager().allocateBuffer(inSize, mBufferUsageFlags, bufferMemFlags, __FILE__, __LINE__);
				}

				mCurrent.mSubAlloc = mBuffers[mDynamicBufferIndex];
				mCurrent.mBufferAllocation = mCurrent.mSubAlloc->getBufferAllocation();
				mCurrent.mHandle = mCurrent.mSubAlloc->getHandle();
				mCurrent.mOffset = mCurrent.mSubAlloc->getOffset();

				bool bRenderThread = (inRHICmdList == nullptr);
				if (bRenderThread)
				{
					BOOST_ASSERT(isInRenderingThread());
				}

				if (createInfo.mResourceArray)
				{
					uint32 copyDataSize = Math::min(inSize, createInfo.mResourceArray->getResourceDataSize());
					void* data = lock(bRenderThread, RLM_WriteOnly, copyDataSize, 0);
					Memory::memcpy(data, createInfo.mResourceArray->getResourceData(), copyDataSize);
					unlock(bRenderThread);

					createInfo.mResourceArray->discard();
				}

				updateVulkanBufferStats(inSize * (uint64_t)mNumBuffers, inBufferUsageFlags, true);

			}
		}
	}


	VulkanStructuredBuffer::VulkanStructuredBuffer(VulkanDevice* inDevice, uint32 stride, uint32 size, RHIResourceCreateInfo& createInfo, uint32 inUsage)
		:RHIStructuredBuffer(stride, size, inUsage)
		,VulkanResourceMultiBuffer(inDevice, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, size, inUsage, createInfo)
	{

	}

	VulkanStructuredBuffer::~VulkanStructuredBuffer()
	{

	}
}