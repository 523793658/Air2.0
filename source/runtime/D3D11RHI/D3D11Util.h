#pragma once
#include "CoreMinimal.h"
#include "D3D11RHI.h"
#include "RHIDefinitions.h"
#include "D3D11Typedefs.h"
#include "Template/TypeHash.h"
#include "WindowsD3D11UniformBuffer.h"
#include <map>
#include "D3D11StateCache.h"
namespace Air
{


	void verifyD3D11Result(HRESULT result, const ANSICHAR* code, const ANSICHAR* filename, uint32 line, ID3D11Device* device);

	extern D3D11RHI_API void verifyD3D11CreateTextureResult(HRESULT d3dResult, const ANSICHAR* code, const ANSICHAR* filename, uint32 line, uint32 width, uint32 height, uint32 depth, uint8 d3dFormat, uint32 numMips, uint32 flags, ID3D11Device* device);

#define VERIFYD3D11RESULT_EX(x, Device)	{HRESULT hr = x; if (FAILED(hr)) { verifyD3D11Result(hr,#x,__FILE__,__LINE__, Device); }}

#define VERIFYD3d11RESULT(x)		{HRESULT hr = x; if(FAILED(hr)){verifyD3D11Result(hr, #x, __FILE__, __LINE__, 0);}}

#define VERIFYD3D11CEATETEXTURERESULT(x, width, height, depth, format, numMips, flags, device){HRESULT hr = x;if(FAILED(hr)){verifyD3D11CreateTextureResult(hr, #x, __FILE__, __LINE__, width, height, depth, format, numMips, flags, device);}}



	class D3D11BoundRenderTargets
	{
	public:
		explicit D3D11BoundRenderTargets(ID3D11DeviceContext* inDeviceContext);

		~D3D11BoundRenderTargets();

		FORCEINLINE int32 getNumActiveTargets()
		{
			return mNumActiveTargets;
		}


		FORCEINLINE ID3D11RenderTargetView* getRenderTargetView(int32 targetIndex) { return mRenderTargetViews[targetIndex]; }
		FORCEINLINE ID3D11DepthStencilView* getDepthStencilView()
		{
			return mDepthStencilView;
		}
	private:
		ID3D11RenderTargetView* mRenderTargetViews[MaxSimultaneousRenderTargets];
		ID3D11DepthStencilView* mDepthStencilView;
		int32 mNumActiveTargets;
	};

	struct D3D11LockedData
	{
		TRefCountPtr<ID3D11Resource> mStagingResource;
		uint32 mPitch;
		uint32 mDepthPitch;
		D3D11LockedData()
			:bAllocDataWasUsed(false)
		{}
		void allocData(uint32 size)
		{
			mData = (uint8*)Memory::malloc(size, 16);
			bAllocDataWasUsed = true;
		}

		void setData(void* inData)
		{
			BOOST_ASSERT(!bAllocDataWasUsed);
			mData = (uint8*)inData;
		}
		uint8* getData() const
		{
			return mData;
		}

		void freeData()
		{
			BOOST_ASSERT(bAllocDataWasUsed);
			Memory::free(mData);
			mData = 0;
		}


	private:
		uint8* mData;
		bool bAllocDataWasUsed;
	};


	class D3D11LockedKey
	{
	public:
		void * mSourceObject;
		uint32 mSubresource;

	public:
		D3D11LockedKey() : mSourceObject(nullptr)
			,mSubresource(0)
		{}
		D3D11LockedKey(ID3D11Texture2D* source, uint32 subresource = 0):
			mSourceObject((void*)source),
			mSubresource(subresource)
		{}
		D3D11LockedKey(ID3D11Texture3D* source, uint32 subresource = 0)
			:mSourceObject((void*)source)
			,mSubresource(subresource)
		{}
		D3D11LockedKey(ID3D11Buffer* source, uint32 subresource = 0)
			:mSourceObject((void*)source),
			mSubresource(subresource)
		{}

		bool operator == (const D3D11LockedKey& rhs) const
		{
			return mSourceObject == rhs.mSourceObject && mSubresource == rhs.mSubresource;
		}

		bool operator !=(const D3D11LockedKey & rhs) const
		{
			return mSubresource != rhs.mSubresource || mSourceObject != rhs.mSourceObject;
		}

		D3D11LockedKey& operator = (const D3D11LockedKey& rhs)
		{
			mSourceObject = rhs.mSourceObject;
			mSubresource = rhs.mSubresource;
			return *this;
		}

		size_t getHash() const
		{
			return pointerHash(mSourceObject);
		}

		friend uint32 getTypeHash(const D3D11LockedKey& k)
		{
			return k.getHash();
		}
	};

	struct  D3DRHIUtil
	{
		template<EShaderFrequency shaderFrequencyT>
		static FORCEINLINE void commitUniforms(D3D11UniformBuffer* inUniformBuffer, D3D11StateCache& stateCache, uint32 index, bool bDiscardSharedUniforms)
		{
			auto * uniformBuffer = ((WinD3D11UniformBuffer*)inUniformBuffer);
			if (uniformBuffer && uniformBuffer->commitUniformsToDevice(bDiscardSharedUniforms))
			{
				ID3D11Buffer* deviceBuffer = uniformBuffer->getUniformBuffer();
				stateCache.setConstantBuffer<shaderFrequencyT>(deviceBuffer, index);
			}
		}
	};

	FORCEINLINE uint32 getD3D11CubeFace(ECubeFace face)
	{
		switch (face)
		{
		case Air::CubeFace_PosX:
			return 0;
		case Air::CubeFace_NegX:
			return 1;
		case Air::CubeFace_PosY:
			return 2;
		case Air::CubeFace_NegY:
			return 3;
		case Air::CubeFace_PosZ:
			return 4;
		case Air::CubeFace_NegZ:
			return 5;
		case Air::CubeFace_MAX:
			break;
		default:
			break;
		}
	}
}
