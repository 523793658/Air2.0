#include "D3D11RHI.h"
#include "Modules/ModuleManager.h"
#include "D3D11DynamicRHI.h"
#include "D3D11Typedefs.h"
#include "IHeadMountedDisplayModule.h"
#include "D3D11Util.h"
#include "GenericPlatform/genericPlatformDriver.h"
#include <iostream>
namespace Air
{

	void updateBufferStats(TRefCountPtr<ID3D11Buffer> buffer, bool bAllocating)
	{

	}



	bool D3D11DynamicRHIModule::isSupported()
	{
		if (!mChosenAdapter.isValid())
		{
			findAdapter();
		}
		int x = 0; 
		std::cout << x;
		return mChosenAdapter.isValid()
			&& mChosenAdapter.mMaxSupportedFeatureLevel != D3D_FEATURE_LEVEL_9_1
			&& mChosenAdapter.mMaxSupportedFeatureLevel != D3D_FEATURE_LEVEL_9_2
			&& mChosenAdapter.mMaxSupportedFeatureLevel != D3D_FEATURE_LEVEL_9_3;
	}

#ifndef PLATFORM_IMPLEMENTS_FASTVRAMALLOCATOR
#define PLATFORM_IMPLEMENTS_FASTVRAMALLOCATOR 0
#endif
#if !PLATFORM_IMPLEMENTS_FASTVRAMALLOCATOR
	FastVRAMAllocator* FastVRAMAllocator::getFastVRAMAllocator()
	{
		static FastVRAMAllocator fastVRAMAllocatorSingleton;
		return &fastVRAMAllocatorSingleton;
	}
#endif
	

	IMPLEMENT_MODULE(D3D11DynamicRHIModule, D3D11RHI);
}


