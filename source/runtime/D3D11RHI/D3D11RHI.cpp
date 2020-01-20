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

	void D3D11DynamicRHI::trackResourceBoundAsVB(D3D11BaseShaderResource* resource, int32 streamIndex)
	{
		BOOST_ASSERT(streamIndex >= 0 && streamIndex < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT);

		if (resource)
		{
			mMaxBoundVertexBufferIndex = Math::max(mMaxBoundVertexBufferIndex, streamIndex);
			mCurrentResourceBoundAsVBs[streamIndex] = resource;
		}
		else if(mCurrentResourceBoundAsVBs[streamIndex] != nullptr)
		{
			mCurrentResourceBoundAsVBs[streamIndex] = nullptr;

			if (mMaxBoundVertexBufferIndex == streamIndex)
			{
				do 
				{
					mMaxBoundVertexBufferIndex--;
				} while (mMaxBoundVertexBufferIndex >= 0 && mCurrentResourceBoundAsVBs[mMaxBoundVertexBufferIndex] == nullptr);
			}
		}
	}

	void D3D11DynamicRHI::trackResourceBoundAsIB(D3D11BaseShaderResource* resource)
	{
		mCurrentResourceBoundAsIB = resource;
	}
	

	IMPLEMENT_MODULE(D3D11DynamicRHIModule, D3D11RHI);
}


