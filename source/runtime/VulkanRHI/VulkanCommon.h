#pragma once
#include "RHIDefinitions.h"

#define VULKAN_ENABLE_SHADER_DEBUG_NAMES	1

namespace Air
{
	namespace EVulkanBindingType
	{
		enum EType : uint8
		{
			PackedConstantBuffer,
			ConstantBuffer,

			CombinedImageSampler,
			Sampler,
			Image,

			ConstantTexelBuffer,

			StorageImage,

			StorageTexelBuffer,

			StorageBuffer,

			InputAttachment,

			Count,
		};
	}
}

