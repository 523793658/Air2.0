#pragma once
#include "RHIResource.h"
#include "VulkanGlobals.h"
namespace Air
{
	class VulkanDevice;

	class VulkanSamplerState : public RHISamplerState
	{
	public:
		VulkanSamplerState(const VkSamplerCreateInfo& inInfo, VulkanDevice& inDevice, const bool bInIsImmutable = false);

		virtual bool isImmutable() const final override { return bIsImmutable; }

		VkSampler mSampler;

		uint32 mSamplerId;

		static void setupSamplerCreateInfo(const SamplerStateInitializerRHI& initializer, VulkanDevice& inDevice, VkSamplerCreateInfo& outSamplerInfo);

	private:
		bool bIsImmutable;
	};

	class VulkanRasterizerState : public RHIRasterizerState
	{
	public:
		VulkanRasterizerState(const RasterizerStateInitializerRHI& inInitializer);

		static void resetCreateInfo(VkPipelineRasterizationStateCreateInfo& outInfo)
		{
			zeroVulkanStruct(outInfo, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
			outInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
			outInfo.lineWidth = 1.0f;
		}

		virtual bool getInitializer(RasterizerStateInitializerRHI& out) override final
		{
			out = mInitializer;
			return true;
		}
		VkPipelineRasterizationStateCreateInfo mRasterizerState;
		RasterizerStateInitializerRHI mInitializer;
	};

	class VulkanDepthStencilState : public RHIDepthStencilState
	{
	public:
		VulkanDepthStencilState(const DepthStencilStateInitializerRHI& inInitializer)
		{
			mInitializer = inInitializer;
		}

		virtual bool getInitializer(DepthStencilStateInitializerRHI& out) override final
		{
			out = mInitializer;
			return true;
		}

		void setupCreateInfo(const GraphicsPipelineStateInitializer& gfxPOSInit, VkPipelineDepthStencilStateCreateInfo& outDepthStencilState);

		DepthStencilStateInitializerRHI mInitializer;
	};

	class VulkanBlendState : public RHIBlendState
	{
	public:
		VulkanBlendState(const BlendStateInitializerRHI& inInitializer);

		virtual bool getInitializer(BlendStateInitializerRHI& out) override final
		{
			out = mInitializer;
			return true;
		}

		VkPipelineColorBlendAttachmentState mBlendStates[MaxSimultaneousRenderTargets];

		BlendStateInitializerRHI mInitializer;
	};
}