#pragma once
#include "RHIResource.h"
#include "D3D11ShaderResource.h"
namespace Air
{
	class D3D11TextureBase;
	class D3D11DynamicRHI;
	FORCEINLINE D3D11TextureBase* getD3D11TextureFromRHITexture(RHITexture* texture)
	{
		if (!texture)
		{
			return nullptr;
		}
		D3D11TextureBase* result((D3D11TextureBase*)texture->getTextureBaseRHI());
		return result;
	}

	class D3D11RHI_API D3D11TextureBase : public D3D11BaseShaderResource
	{
	public:
		D3D11TextureBase(D3D11DynamicRHI* inD3DRHI, 
			ID3D11Resource* inResource, 
			ID3D11ShaderResourceView* inShaderResourceView,
			int32 inRTVArraySize,
			bool bInCreatedRTVsPerSlice,
			const TArray<TRefCountPtr<ID3D11RenderTargetView>>& inRenderTargetVews,
			TRefCountPtr<ID3D11DepthStencilView>* inDepthStencileViews)
			:mD3DRHI(inD3DRHI)
			,IHVResourceHandle(nullptr)
			,mMemorySize(0)
			,mBaseShaderResource(this)
			,mResource(inResource)
			,mShaderResourceView(inShaderResourceView)
			,mRenderTargetViews(inRenderTargetVews)
			,mCreatedRTVsPerSlice(bInCreatedRTVsPerSlice)
			,mNumDepthStencilViews(0)
		{
			if (inDepthStencileViews != nullptr)
			{
				for (uint32 index = 0; index < FExclusiveDepthStencil::MaxIndex; index++)
				{
					mDepthStencilViews[index] = inDepthStencileViews[index];
					if (mDepthStencilViews[index] != NULL)
					{
						mNumDepthStencilViews++;
					}
				}
			}
		}



		ID3D11DepthStencilView* getDepthStencilView(FExclusiveDepthStencil accessType) const
		{
			return mDepthStencilViews[accessType.GetIndex()];
		}

		ID3D11RenderTargetView* getRenderTargetView(int32 mipIndex, int32 arraySliceIndex) const
		{
			int32 arrayIndex = mipIndex;
			if (mCreatedRTVsPerSlice)
			{
				arrayIndex = mipIndex * mRTVArraySize + arraySliceIndex;
			}
			else
			{

			}
			if ((uint32)arrayIndex < (uint32)mRenderTargetViews.size())
			{
				return mRenderTargetViews[arrayIndex];
			}
			return 0;
		}

		ID3D11Resource* getResource() const { return mResource; }

		ID3D11ShaderResourceView* getShaderResourceView() const { return mShaderResourceView; }

		D3D11BaseShaderResource* getBaseShaderResource() const { return mBaseShaderResource; }

		void setMemorySize(int32 inMemorySize)
		{
			mMemorySize = inMemorySize;
		}

		int32 getMemorySize() const
		{
			return mMemorySize;
		}

		bool hasDepthStencilView()
		{
			return (mNumDepthStencilViews > 0);
		}
		

	protected:
		D3D11DynamicRHI* mD3DRHI;
		void* IHVResourceHandle;
		int32 mMemorySize;
		D3D11BaseShaderResource* mBaseShaderResource;

		TRefCountPtr<ID3D11Resource> mResource;

		TRefCountPtr<ID3D11ShaderResourceView> mShaderResourceView;

		TRefCountPtr<ID3D11DepthStencilView>	mDepthStencilViews[FExclusiveDepthStencil::MaxIndex];

		TArray<TRefCountPtr<ID3D11RenderTargetView>> mRenderTargetViews;

		uint32 mNumDepthStencilViews;
		bool mCreatedRTVsPerSlice;
		int32 mRTVArraySize;
	};

	class D3D11BaseTexture2D : public RHITexture2D
	{
	public:
		D3D11BaseTexture2D(uint32 width, uint32 height, uint32 depth, uint32 numMips, uint32 numSamples, EPixelFormat inFormat, uint32 flags, const ClearValueBinding& clearValue)
			:RHITexture2D(width, height, numMips, numSamples, inFormat, flags, clearValue)
		{

		}

		uint32 getDepth()const { return 0; }
	};


	template<typename BaseResourceType>
	class D3D11RHI_API TD3D11Texture2D : public BaseResourceType, public D3D11TextureBase
	{
	public:
		uint32 mFlags;

		TD3D11Texture2D(
			class D3D11DynamicRHI* inD3DRHI,
			ID3D11Texture2D*	inResource,
			ID3D11ShaderResourceView* inShaderResourceView,
			bool bInCreatedRTVsPerSlice,
			int32 inRTVArraySize,
			const TArray < TRefCountPtr<ID3D11RenderTargetView>>& inRenderTargetViews,
			TRefCountPtr<ID3D11DepthStencilView>* inDepthStencilViews,
			uint32 inSizeX,
			uint32 inSizeY,
			uint32 inSizeZ,
			uint32 inNumMips,
			uint32 inNumSamples,
			EPixelFormat inFormat,
			bool bInCubemap,
			uint32 inFlags,
			bool inPooled,
			const ClearValueBinding& inClearValue
		)
			:BaseResourceType(
				inSizeX,
				inSizeY,
				inSizeZ,
				inNumMips,
				inNumSamples,
				inFormat,
				inFlags,
				inClearValue
			),
			D3D11TextureBase(
				inD3DRHI,
				inResource,
				inShaderResourceView,
				inRTVArraySize,
				bInCreatedRTVsPerSlice,
				inRenderTargetViews,
				inDepthStencilViews
			),
			mFlags(inFlags)
			,mCubeMap(bInCubemap)
			,mPooled(inPooled)
		{}


		ID3D11Texture2D* getResource() const { return (ID3D11Texture2D*)D3D11TextureBase::getResource(); }


		virtual void* getTextureBaseRHI() override final
		{
			return static_cast<D3D11TextureBase*>(this);
		}

		void* lock(uint32 mipIndex, uint32 arrayIndex, EResourceLockMode lockMode, uint32 & destStride);

		void unlock(uint32 mipIndex, uint32 arrayIndex);

	public:
		virtual uint32 AddRef() const
		{
			return RHIResource::AddRef();
		}
		virtual uint32 Release() const
		{
			return RHIResource::Release();
		}

		virtual uint32 GetRefCount() const
		{
			return RHIResource::GetRefCount();
		}

	private:
		const uint32 mCubeMap : 1;
		const uint32 mPooled : 1;
	};



	class D3D11BaseTexture2DArray : public RHITexture2DArray
	{

	};

	class D3D11BaseTextureCube : public RHITextureCube
	{
	public:
		D3D11BaseTextureCube(uint32 inWidth, uint32 inHeight, uint32 inDepth, uint32 inNumMips, uint32 inNumSamplers, EPixelFormat inFormat, uint32 inFlags, const ClearValueBinding& inClearValue)
			:RHITextureCube(inWidth, inNumMips, inFormat, inFlags, inClearValue)
		{
			BOOST_ASSERT(inNumSamplers == 1);
		}
		uint32 getWidth() const { return getSize(); }
		uint32 getHeight() const { return getSize(); }
		uint32 getDepth() const { return 0; }
	};

	typedef TD3D11Texture2D<RHITexture>				D3D11Texture;
	typedef TD3D11Texture2D<D3D11BaseTexture2D>		D3D11Texture2D;
	typedef TD3D11Texture2D<D3D11BaseTexture2DArray> D3D11Texture2DArray;
	typedef TD3D11Texture2D<D3D11BaseTextureCube>	D3D11TextureCube;

	class D3D11TextureReference : public RHITextureReference, public D3D11TextureBase
	{
	public:
		D3D11TextureReference(class D3D11DynamicRHI* inD3DRHI, LastRenderTimeContainer* lastRenderTime)
			:RHITextureReference(lastRenderTime)
			, D3D11TextureBase(inD3DRHI, nullptr, nullptr, 0, false, TArray<TRefCountPtr<ID3D11RenderTargetView>>(), nullptr)
		{
			mBaseShaderResource = nullptr;
		}

		void setReferencedTexture(RHITexture* inTexture, D3D11BaseShaderResource* inBaseShaderResource, ID3D11ShaderResourceView* inSRV)
		{
			mShaderResourceView = inSRV;
			mBaseShaderResource = inBaseShaderResource;
			RHITextureReference::setReferencedTexture(inTexture);
		}

		virtual void* getTextureBaseRHI() override final
		{
			return static_cast<D3D11TextureBase*>(this);
		}

		virtual uint32 AddRef() const
		{
			return RHIResource::AddRef();
		}

		virtual uint32 Release() const
		{
			return RHIResource::Release();
		}

		virtual uint32 GetRefCount() const
		{
			return RHIResource::GetRefCount();
		}

	};

	
	


	template<>
	struct TD3D11ResourceTraits<RHITexture2D>
	{
		typedef D3D11Texture2D TConcreteType;
	};

	DECL_D3D11_RESOURCE_TRAITS(D3D11TextureCube, RHITextureCube);

}