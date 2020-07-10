#pragma once

#include "BoundShaderStateCache.h"
#include "CrossCompilerCommon.h"
namespace Air
{
	static inline VkDescriptorType bindingToDescriptorType(EVulkanBindingType::EType type)
	{
		switch (type)
		{
		case Air::EVulkanBindingType::PackedConstantBuffer:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case Air::EVulkanBindingType::ConstantBuffer:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		case Air::EVulkanBindingType::CombinedImageSampler:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		case Air::EVulkanBindingType::Sampler:
			return VK_DESCRIPTOR_TYPE_SAMPLER;
		case Air::EVulkanBindingType::Image:
			return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

		case Air::EVulkanBindingType::ConstantTexelBuffer:
			return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;

		case Air::EVulkanBindingType::StorageImage:
			return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

		case Air::EVulkanBindingType::StorageTexelBuffer:
			return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		case Air::EVulkanBindingType::StorageBuffer:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case Air::EVulkanBindingType::InputAttachment:
			return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		default:
			BOOST_ASSERT(false);
			break;
		}
		return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}

	static inline EVulkanBindingType::EType descriptorTypeToBinding(VkDescriptorType type, bool bUsePacked = false)
	{
		switch (type)
		{
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:		return bUsePacked ? EVulkanBindingType::PackedConstantBuffer : EVulkanBindingType::ConstantBuffer;
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return EVulkanBindingType::CombinedImageSampler;
		case VK_DESCRIPTOR_TYPE_SAMPLER:			return EVulkanBindingType::Sampler;
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:		return EVulkanBindingType::Image;
		case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:	return EVulkanBindingType::ConstantTexelBuffer;
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:		return EVulkanBindingType::StorageImage;
		case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:	return EVulkanBindingType::StorageTexelBuffer;
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:		return EVulkanBindingType::StorageBuffer;
		case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:	return EVulkanBindingType::InputAttachment;
		default:
			BOOST_ASSERT(false);
			break;
		}
		return EVulkanBindingType::Count;
	}

	struct VulkanShaderHeader
	{
		enum EType
		{
			PackedGlobal,
			Global,
			ConstantBuffer,

			Count,
		};

		struct SpirvInfo
		{
			SpirvInfo() = default;

			SpirvInfo(uint32 inDescriptorSetOffset, uint32 inBindingIndexOffset)
				:mDescriptorSetOffset(inDescriptorSetOffset)
				,mBindingIndexOffset(inBindingIndexOffset)
			{}


			uint32 mDescriptorSetOffset = UINT32_MAX;
			uint32 mBindingIndexOffset = UINT32_MAX;
		};

		struct CBResourceInfo
		{
			uint16 mSourceCBResourceIndex;
			uint16 mOriginalBindingIndex;
			uint16 mGlobalIndex;
			TEnumAsByte<EConstantBufferBaseType> mCBBaseType;
			uint8 mmPad0 = 0;
#if VULKAN_ENABLE_SHADER_DEBUG_NAMES
			wstring mDebugName;
#endif
		};

		struct ConstantBufferInfo
		{
			uint32 mLayoutHash;
			uint16 mConstantDataOriginalBindingIndex;
			uint8 bOnlyHasResources;
			uint8 mPad0 = 0;
			TArray<CBResourceInfo> mResourceEntries;
#if VULKAN_ENABLE_SHADER_DEBUG_NAMES
			wstring mDebugName;
#endif
		};

		TArray<ConstantBufferInfo> mConstantBuffers;

		struct GlobalInfo
		{
			uint16 mOriginalBindingIndex;
			uint16 mCombinedSamplerStateAliasIndex;
			uint16 mTypeIndex;
			uint8 bImmutableSampler = 0;
			uint8 mPad0 = 0;
#if VULKAN_ENABLE_SHADER_DEBUG_NAMES
			wstring mDebugName;
#endif
		};

		TArray<GlobalInfo>		mGlobals;
		TArray<TEnumAsByte<VkDescriptorType>>	mGlobalDescriptorTypes;

		struct PackedGlobalInfo
		{
			uint16 mConstantDataSizeInFloats;
			EPackedTypeIndex mPackedTypeIndex;
			uint8 mPackedCBIndex;
#if VULKAN_ENABLE_SHADER_DEBUG_NAMES
			wstring mDebugName;
#endif
		};
		TArray<PackedGlobalInfo> mPackedGlobals;

		struct PackedCBInfo
		{
			uint32 mSizeInBytes;
			uint16 mOriginalBindingIndex;
			EPackedTypeIndex mPackedTypeIndex;
			uint8 mPad0 = 0;
			uint32 mSPIRVDescriptorSetOffset;
			uint32 mSPIRVBindingIndexOffset;
		};

		TArray<PackedCBInfo>	mPackedCBs;

		enum class EAttachementType : uint8
		{
			Color,
			Depth,
			Count,
		};

		struct InputAttachment
		{
			uint16 mGlobalIndex;
			EAttachementType mType;
			uint8	mPad = 0;
		};

		TArray<InputAttachment> mInputAttachments;

		TArray<uint32>	mEmulatedCBCopyRanges;
		TArray<ConstantBufferCopyInfo>mEmulatedCBsCopyInfo;

		uint32 mInOutMask;
		bool bHasRealCBs;
		uint8 mPad0 = 0;
		uint16 mPad1 = 0;

		SHAHash	mSourceHash;
		uint32 mSpirvCRC = 0;

		TArray<SpirvInfo> mConstantBufferSpirvInfos;
		TArray<SpirvInfo> mGlobalSpirvInfos;

#if VULKAN_ENABLE_SHADER_DEBUG_NAMES
		wstring mDebugName;
#endif

		VulkanShaderHeader() = default;

		enum EInit
		{
			EZero,
		};

		VulkanShaderHeader(EInit)
			:mInOutMask(0)
			,bHasRealCBs(0)
		{

		}

	};

	inline Archive& operator <<(Archive& ar, VulkanShaderHeader::SpirvInfo& info)
	{
		ar << info.mDescriptorSetOffset;
		ar << info.mBindingIndexOffset;
		return ar;
	}

	inline Archive& operator<<(Archive& ar, VulkanShaderHeader::CBResourceInfo& entry)
	{
		ar << entry.mSourceCBResourceIndex;
		ar << entry.mOriginalBindingIndex;
		ar << entry.mGlobalIndex;
		ar << entry.mCBBaseType;
#if VULKAN_ENABLE_SHADER_DEBUG_NAMES
		ar << entry.mDebugName;
#endif
	}

	inline Archive& operator << (Archive& ar, VulkanShaderHeader::ConstantBufferInfo& cbInfo)
	{
		ar << cbInfo.mLayoutHash;
		ar << cbInfo.mConstantDataOriginalBindingIndex << cbInfo.bOnlyHasResources << cbInfo.mResourceEntries;
#if VULKAN_ENABLE_SHADER_DEBUG_NAMES
		ar << cbInfo.mDebugName;
#endif
		return ar;
	}

	inline Archive& operator<<(Archive& ar, VulkanShaderHeader::PackedGlobalInfo& packedGlobalInfo)
	{
		ar << packedGlobalInfo.mConstantDataSizeInFloats;
		ar << packedGlobalInfo.mPackedTypeIndex;
		ar << packedGlobalInfo.mPackedCBIndex;
#if VULKAN_ENABLE_SHADER_DEBUG_NAMES
		ar << packedGlobalInfo.mDebugName;
#endif
		return ar;
	}

	inline Archive& operator << (Archive& ar, VulkanShaderHeader::PackedCBInfo& packedCBInfo)
	{
		ar << packedCBInfo.mSizeInBytes;
		ar << packedCBInfo.mOriginalBindingIndex;
		ar << packedCBInfo.mPackedTypeIndex;
		ar << packedCBInfo.mSPIRVDescriptorSetOffset;
		ar << packedCBInfo.mSPIRVBindingIndexOffset;
		return ar;
	}

	inline Archive& operator <<(Archive& ar, VulkanShaderHeader::GlobalInfo& globalInfo)
	{
		ar << globalInfo.mOriginalBindingIndex;
		ar << globalInfo.mCombinedSamplerStateAliasIndex;
		ar << globalInfo.mTypeIndex;
		ar << globalInfo.bImmutableSampler;
#if VULKAN_ENABLE_SHADER_DEBUG_NAMES
		ar << globalInfo.mDebugName;
#endif
		return ar;
	}

	inline Archive& operator<<(Archive& ar, VulkanShaderHeader::InputAttachment& attchementInfo)
	{
		ar << attchementInfo.mGlobalIndex;
		ar << attchementInfo.mType;
		return ar;
	}

	inline Archive& operator<<(Archive& ar, VulkanShaderHeader& header)
	{
		ar << header.mConstantBuffers;
		ar << header.mGlobals;
		ar << header.mGlobalDescriptorTypes;
		ar << header.mPackedGlobals;
		ar << header.mPackedCBs;
		ar << header.mInputAttachments;
		ar << header.mEmulatedCBCopyRanges;
		ar << header.mEmulatedCBsCopyInfo;
		ar << header.mInOutMask;
		ar << header.bHasRealCBs;
		ar << header.mSourceHash;
		ar << header.mSpirvCRC;
		ar << header.mConstantBufferSpirvInfos;
		ar << header.mGlobalSpirvInfos;
#if VULKAN_ENABLE_SHADER_DEBUG_NAMES
		ar << header.mDebugName;
#endif
		return ar;
	}
}