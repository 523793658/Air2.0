#pragma once
#include "RHIResource.h"
#include "VulkanResources.h"
namespace Air
{
	class VulkanRHIGraphicsPipelineState : public RHIGraphicsPipelineState
	{

	};


	template<>
	struct TVulkanResourceTraits<class RHIComputePipelineState>
	{
		typedef class VulkanComputePipeline TConcreteType;
	};

	template<>
	struct TVulkanResourceTraits<RHIGraphicsPipelineState>
	{
		typedef VulkanRHIGraphicsPipelineState TConcreteType;
	};
}