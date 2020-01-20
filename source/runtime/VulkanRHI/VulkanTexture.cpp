#include "VulkanRHIPrivate.h"
#include "VulkanDynamicRHI.h"
#include "VulkanResources.h"
namespace Air
{
	Texture2DRHIRef VulkanDynamicRHI::RHICreateTexture2D(uint32 width, uint32 height, uint8 format, uint32 numMips, uint32 numSamplers, uint32 flags, RHIResourceCreateInfo& createInfo)
	{
		return new VulkanTexture2D(*mDevice, (EPixelFormat)format, width, height, numMips, numSamplers, flags, createInfo);
	}

	TextureCubeRHIRef VulkanDynamicRHI::RHICreateTextureCube(uint32 size, uint8 format, uint32 numMips, uint32 flags, RHIResourceCreateInfo& createInfo)
	{
		return new VulkanTextureCube(*mDevice, (EPixelFormat)format, size, false, 1, numMips, flags, createInfo.mBulkData, createInfo.mClearValueBinding);
	}

	
}