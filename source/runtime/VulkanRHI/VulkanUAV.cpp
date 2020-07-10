#include "VulkanRHIPrivate.h"
#include "VulkanContext.h"
namespace Air
{
	ShaderResourceViewRHIRef VulkanDynamicRHI::RHICreateShaderResourceView(RHITexture* texture2DRHI, const RHITextureSRVCreateInfo& createInfol)
	{
		VulkanShaderResourceView* srv = new VulkanShaderResourceView(mDevice, texture2DRHI, createInfol);
		return srv;
	}


	ShaderResourceViewRHIRef VulkanDynamicRHI::RHICreateShaderResourceView(RHIVertexBuffer* vertexBuffer, uint32 stride, uint8 format)
	{
		if (!vertexBuffer)
		{
			return new VulkanShaderResourceView(mDevice, nullptr, nullptr, 0, (EPixelFormat)format);
		}
	}

	ShaderResourceViewRHIRef VulkanDynamicRHI::RHICreateShaderResourceView(RHIStructuredBuffer* inStructuredBuffer)
	{
		VulkanStructuredBuffer* structuredBuffer = resourceCast(inStructuredBuffer);
		VulkanShaderResourceView* srv = new VulkanShaderResourceView(mDevice, structuredBuffer);
		return srv;
	}

	UnorderedAccessViewRHIRef VulkanDynamicRHI::RHICreateUnorderedAccessView(RHIStructuredBuffer* inStructuredBuffer, bool bUseUAVCounter, bool bAppendBuffer)
	{
		VulkanStructuredBuffer* structuredBuffer = resourceCast(inStructuredBuffer);

		VulkanUnorderedAccessView* uav = new VulkanUnorderedAccessView(mDevice);

		uav->mSourceStructuredBuffer = structuredBuffer;

		return uav;
	}

	UnorderedAccessViewRHIRef VulkanDynamicRHI::RHICreateUnorderedAccessView(RHIVertexBuffer* inVertexBuffer, uint8 bUseUAVCounter)
	{
		VulkanVertexBuffer* vertexBuffer = resourceCast(inVertexBuffer);
		VulkanUnorderedAccessView* uav = new VulkanUnorderedAccessView(mDevice);
		uav->mSourceVertexBuffer = vertexBuffer;
		return uav;
	}

	UnorderedAccessViewRHIRef VulkanDynamicRHI::RHICreateUnorderedAccessView(RHIIndexBuffer* inIndexBuffer, uint8 bUseUAVCounter)
	{
		VulkanIndexBuffer* indexBuffer = resourceCast(inIndexBuffer);
		VulkanUnorderedAccessView* uav = new VulkanUnorderedAccessView(mDevice);
		uav->mSourceIndexBuffer = indexBuffer;
		return uav;

	}

	UnorderedAccessViewRHIRef VulkanDynamicRHI::RHICreateUnorderedAccessView(RHITexture* inTexture, uint32 mipLevel)
	{
		VulkanUnorderedAccessView* uav = new VulkanUnorderedAccessView(mDevice);
		uav->mSourceTexture = inTexture;
		uav->mMipLevel = mipLevel;
		return uav;
	}


	StructuredBufferRHIRef VulkanDynamicRHI::RHICreateStructuredBuffer(uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo)
	{
		return new VulkanStructuredBuffer(mDevice, stride, size, createInfo, inUsage);
	}
}