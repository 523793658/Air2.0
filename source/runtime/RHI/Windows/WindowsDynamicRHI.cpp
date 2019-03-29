#include "RHI.h"
#include "Modules/ModuleManager.h"
#include "DynamicRHI.h"
#if PLATFORM_WINDOWS
namespace Air
{
	DynamicRHI* platformCreateDynamicRHI()
	{
		DynamicRHI* dynamicRHI = nullptr;

		IDynamicRHIModule* dynamicRHIModule = nullptr;
		ERHIFeatureLevel::Type requestedFeatureLevel = ERHIFeatureLevel::Num;
		EShaderPlatform TargetedPlatform = SP_PCD3D_SM5;

		requestedFeatureLevel = getMaxSupportedFeatureLevel(TargetedPlatform);


		dynamicRHIModule = &ModuleManager::loadModuleChecked<IDynamicRHIModule>(L"D3D11RHI");
		if (!dynamicRHIModule->isSupported())
		{
			//ÍÆ³ö³ÌÐò
			dynamicRHIModule = nullptr;
		}
		if (dynamicRHIModule)
		{
			dynamicRHI = dynamicRHIModule->createRHI(requestedFeatureLevel);
		}
		return dynamicRHI;
	}
}
#endif
