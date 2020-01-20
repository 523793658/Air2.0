#include "VulkanRHIPrivate.h"
#include "VulkanGenericPlatform.h"
namespace Air
{
	void VulkanGenericPlatform::setupFeatureLevels()
	{
		GShaderPlatformForFeatureLevel[ERHIFeatureLevel::ES2] = SP_VULKAN_PCES3_1;
		GShaderPlatformForFeatureLevel[ERHIFeatureLevel::ES3_1] = SP_VULKAN_PCES3_1;
		GShaderPlatformForFeatureLevel[ERHIFeatureLevel::SM4] = SP_VULKAN_SM4;
		GShaderPlatformForFeatureLevel[ERHIFeatureLevel::SM5] = SP_VULKAN_SM5;
	}
}