#include "VulkanRHIPrivate.h"
#include "VulkanResources.h"
#include "RHICommandList.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "VulkanMemory.h"
namespace Air
{
	static int32 GVulkanAllowConstantUpload = 0;

#if AIR_DEBUG
	constexpr EConstantBufferValidation ConstantBufferValidation = EConstantBufferValidation::ValidateResources;
#else
	constexpr EConstantBufferValidation ConstantBufferValidation = EConstantBufferValidation::None;
#endif


	static void validateConstantBufferResource(const RHIConstantBufferLayout& inLayout, int32 index, RHIResource* resource, EConstantBufferValidation validation)
	{
		if (!(GMaxRHIFeatureLevel <= ERHIFeatureLevel::ES3_1 && (inLayout.mResources[index].mMemberType == CBMT_SRV ||
			inLayout.mResources[index].mMemberType == CBMT_RDG_TEXTURE_SRV ||
			inLayout.mResources[index].mMemberType == CBMT_RDG_BUFFER_SRV))
			&& validation == EConstantBufferValidation::ValidateResources)
		{
			BOOST_ASSERT(resource);
		}
	}

	template<bool RealCBs>
	inline void VulkanDynamicRHI::updateConstantBuffer(VulkanConstantBuffer* inConstantBuffer, const void* contents)
	{
		const RHIConstantBufferLayout& layout = inConstantBuffer->getLayout();

		const int32 constantBufferSize = layout.mConstantBufferSize;
		const int32 numResources = layout.mResources.size();

		RHICommandListImmediate& RHICmdList = RHICommandListExecutor::getImmediateCommandList();

		VulkanRealConstantBuffer* realConstantBuffer = RealCBs ? (VulkanRealConstantBuffer*)inConstantBuffer : nullptr;
		VulkanEmulatedConstantBuffer* emulatedConstantBuffer = RealCBs ? nullptr : (VulkanEmulatedConstantBuffer*)inConstantBuffer;

		BufferSuballocation* newCBAlloc = nullptr;

		bool bIsInRenderPass = RHICmdList.isInsideRenderPass();
		bool bUseUpload = GVulkanAllowConstantUpload && !bIsInRenderPass;

		if (RealCBs && !bUseUpload)
		{
			newCBAlloc = nullptr;
			if (constantBufferSize > 0)
			{
				newCBAlloc = mDevice->getResourceHeapManager().allocConstantBuffer(constantBufferSize, contents);
			}
		}
		auto updateConstantBufferHelper = [](VulkanCommandListContext& context, VulkanRealConstantBuffer* vulkanConstantBuffer, int32 dataSize, const void* data)
		{
			VulkanCmdBuffer* cmdBuffer = context.getCommandBufferManager()->getActiveCmdBufferDirect();

			BOOST_ASSERT(cmdBuffer->isOutsideRenderPass());
			TempFrameAllocationBuffer::TempAllocInfo lockInfo;
			context.getTempFrameAllocationBuffer().alloc(dataSize, 16, lockInfo);
			Memory::memcpy(lockInfo.mData, data, dataSize);
			VkBufferCopy region;
			region.size = dataSize;
			region.srcOffset = lockInfo.getBindOffset();
			region.dstOffset = vulkanConstantBuffer->getOffset();
			VkBuffer cb = vulkanConstantBuffer->getBufferAllocation()->getHandle();
			vkCmdCopyBuffer(cmdBuffer->getHandle(), lockInfo.getHandle(), cb, 1, &region);
		};

		bool bRHIBypass = RHICmdList.bypass();

		if (bRHIBypass)
		{
			if (RealCBs)
			{
				if (constantBufferSize > 0)
				{
					if (bUseUpload)
					{
						VulkanCommandListContext& context = (VulkanCommandListContext&)*mDevice->mImmediateContext;
						updateConstantBufferHelper(context, realConstantBuffer, constantBufferSize, contents);
					}
					else
					{
						BufferSuballocation* prevAlloc = realConstantBuffer->updateCBAllocation(newCBAlloc);
						mDevice->getResourceHeapManager().releaseConstantBuffer(newCBAlloc);
					}
				}
			}
			else
			{
				emulatedConstantBuffer->udpateConstantData(contents, constantBufferSize);
			}
			inConstantBuffer->updateResourceTable(layout, contents, numResources);
		}
		else
		{
			RHIResource** cmdListResources = nullptr;
			if (numResources > 0)
			{
				cmdListResources = (RHIResource**)RHICmdList.alloc(sizeof(RHIResource*) * numResources, alignof(RHIResource*));
				for (int32 resourceIndex = 0; resourceIndex < numResources; ++resourceIndex)
				{
					RHIResource* resource = *(RHIResource**)((uint8*)contents + layout.mResources[resourceIndex].mMemberOffset);
					validateConstantBufferResource(layout, resourceIndex, resource, ConstantBufferValidation);
				}
			}
			if (RealCBs)
			{
				if (bUseUpload)
				{
					void* cmdListConstantBufferData = RHICmdList.alloc(constantBufferSize, 16);
					Memory::memcpy(cmdListConstantBufferData, contents, constantBufferSize);
					RHICmdList.enqueueLambda([updateConstantBufferHelper, realConstantBuffer, cmdListResources, numResources, constantBufferSize, cmdListConstantBufferData](RHICommandList& cmdList)
						{
							VulkanCommandListContext& context = (VulkanCommandListContext&)cmdList.getContext();
							updateConstantBufferHelper(context, realConstantBuffer, constantBufferSize, cmdListConstantBufferData);
							realConstantBuffer->updateResourceTable(cmdListResources, numResources);
						});
				}
				else
				{
					RHICmdList.enqueueLambda([realConstantBuffer, newCBAlloc, cmdListResources, numResources](RHICommandList& cmdList)
						{
							BufferSuballocation* prevAlloc = realConstantBuffer->updateCBAllocation(newCBAlloc);
							realConstantBuffer->mDevice->getResourceHeapManager().releaseConstantBuffer(newCBAlloc);
							realConstantBuffer->updateResourceTable(cmdListResources, numResources);
						});
				}
			}
			else
			{
				void* cmdListConstantBufferData = RHICmdList.alloc(constantBufferSize, 16);
				Memory::memcpy(cmdListConstantBufferData, contents, constantBufferSize);
				RHICmdList.enqueueLambda([emulatedConstantBuffer, cmdListResources, numResources, cmdListConstantBufferData, constantBufferSize](RHICommandList&)
					{
						emulatedConstantBuffer->udpateConstantData(cmdListConstantBufferData, constantBufferSize);
						emulatedConstantBuffer->updateResourceTable(cmdListResources, numResources);
					});
			}
			RHICmdList.RHIThreadFence(true);
		}
	}

	void VulkanDynamicRHI::RHIUpdateConstantBuffer(RHIConstantBuffer* constantBufferRHI, const void* contents)
	{
		const bool bHasRealCBs = true;
		VulkanConstantBuffer* constantBuffer = resourceCast(constantBufferRHI);
		if (bHasRealCBs)
		{
			updateConstantBuffer<true>(constantBuffer, contents);
		}
		else
		{
			updateConstantBuffer<false>(constantBuffer, contents);
		}
	}

	
}