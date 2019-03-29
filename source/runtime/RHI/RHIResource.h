#pragma once
#include "RHI.h"
#include "Containers/LockFreeListImpl.h"
#include "HAL/PlatformAtomics.h"
#include "Template/RefCounting.h"
#include "Template/AlignmentTemplates.h"
#include "Misc/SecureHash.h"
#include "Misc/Crc.h"
namespace Air
{
	class RHI_API RHIResource
	{
	public:
		RHIResource(bool inbDoNotDeferDelete = false)
			:mMarkedForDelete(0),
			bDoNotDeferDelete(inbDoNotDeferDelete)
		{}
		virtual ~RHIResource()
		{

		}

		FORCEINLINE uint32 AddRef() const
		{
			int32 newValue = mNumRefs.increment();
			return (uint32)newValue;
		}
		FORCEINLINE uint32 Release() const
		{
			int32 newValue = mNumRefs.decrement();
			if (newValue == 0)
			{
				if (!deferDelete())
				{
					delete this;
				}
				else
				{
					if (PlatformAtomics::interlockedCompareExchange(&mMarkedForDelete, 1, 0) == 0)
					{
						mPendingDeletes.push(const_cast<RHIResource*>(this));
					}
				}
			}
			return uint32(newValue);
		}

		FORCEINLINE uint32 GetRefCount() const
		{
			int32 currentValue = mNumRefs.getValue();
			return uint32(currentValue);
		}

		void doNotDeferDelete()
		{
			bDoNotDeferDelete = true;
		}

		static void flushPendingDeletes();

		FORCEINLINE static bool platformNeedsExtradeletionLatency()
		{
			return GRHINeedsExtraDeletionLatency && GIsRHIInitialized;
		}

		static bool bypass();


		
	private:
		mutable int32 mMarkedForDelete;
		mutable ThreadSafeCounter mNumRefs;

		bool bDoNotDeferDelete;
		static TLockFreePointerListUnordered<RHIResource, PLATFORM_CACHE_LINE_SIZE> mPendingDeletes;
		static RHIResource* mCurrentlyDeleting;



		FORCEINLINE bool deferDelete() const
		{
			return !bDoNotDeferDelete && (GRHINeedsExtraDeletionLatency || !bypass());
		}

		struct ResourceToDelete
		{
			ResourceToDelete(uint32 inFrameDeleted = 0)
				:mFrameDeleted(inFrameDeleted)
			{

			}

			TArray<RHIResource*> mResource;
			uint32 mFrameDeleted;
		};

		static TArray<ResourceToDelete> mdeferredDeletionQueue;
		static uint32 mCurrentFrame;
	};

	class RHI_API LastRenderTimeContainer
	{
	public:
		LastRenderTimeContainer()
			:mLastRenderTime(std::numeric_limits<int32>::min())
		{
		}

		double getLastRenderTime() const { return mLastRenderTime; }
		FORCEINLINE_DEBUGGABLE void setLastRenderTime(double lastRenderTime)
		{
			if (mLastRenderTime != lastRenderTime)
			{
				mLastRenderTime = lastRenderTime;
			}
		}
	private:
		double mLastRenderTime;
	};



	class RHIViewport : public RHIResource
	{
	public:
	
	};

	class RHICustomPresent : public RHIResource
	{
	public:
		explicit RHICustomPresent(RHIViewport* inViewport)
			:RHIResource(true)
			, mViewportRHI(inViewport)
		{

		}
		virtual ~RHICustomPresent() {}
		virtual void OnBackBufferResize() = 0;

		virtual bool present(int32& inOutSyncInterval) = 0;
		virtual void postPresent() {}
		virtual void onAcquireThreadOwnership() {}
		virtual void onReleaseThreadOwnership() {}

	protected:
		RHIViewport* mViewportRHI;
	};

	typedef RHICustomPresent*		CustomPresentRHIParamRef;
	typedef TRefCountPtr<RHICustomPresent>		CustomPresentRHIRef;

	class RHI_API RHITexture : public RHIResource
	{
	public:
		RHITexture(uint32 numMips, uint32 numSamples, EPixelFormat format, uint32 flags, LastRenderTimeContainer* lastRenderTime, const ClearValueBinding& clearValue)
			:mClearValue(clearValue)
			, mNumMips(numMips)
			,mNumSamples(numSamples)
			,mFormat(format)
			,mFlags(flags)
			,mLastRenderTime(lastRenderTime ? *lastRenderTime : mDefaultRenderTime)
		{}

		EPixelFormat getFormat() const { return mFormat; }

		uint32 getFlags() const { return mFlags; }

		virtual void* getTextureBaseRHI()
		{
			return nullptr;
		}

		uint32 getNumMips() const { return mNumMips; }

		uint32 getNumSamples() const { return mNumSamples; }

		virtual class RHITexture2D* getTexture2D() { return nullptr; }
		virtual class RHITexture2DArray* getTextureArray() { return nullptr; }
		virtual class RHITexture3D* getTexture3D() { return nullptr; }
		virtual class RHITextureCube* getTextureCube() { return nullptr; }
		virtual class RHITextureReference* getTextureReference() { return nullptr; }

		wstring getName() const
		{
			return mName;
		}

		void setName(const wstring& inName)
		{
			mName = inName;
		}

		bool hasClearValue() const
		{
			return mClearValue.mColorBinding != EClearBinding::ENoneBound;
		}
		const ClearValueBinding getClearBinding() const
		{
			return mClearValue;
		}

		LinearColor getClearColor() const
		{
			return mClearValue.getClearColor();
		}

		FORCEINLINE_DEBUGGABLE void setLastRenderTime(float inLastRenderTime)
		{
			mLastRenderTime.setLastRenderTime(inLastRenderTime);
		}

		bool isMultisampled() const { return mNumSamples > 1; }

		LastRenderTimeContainer* getLastRenderTimeContainer()
		{
			if (&mLastRenderTime == &mDefaultRenderTime)
			{
				return nullptr;
			}
			return &mLastRenderTime;
		}
	private:

		ClearValueBinding mClearValue;
		uint32 mNumMips;
		uint32 mNumSamples;
		EPixelFormat mFormat;
		uint32 mFlags;
		LastRenderTimeContainer& mLastRenderTime;
		LastRenderTimeContainer mDefaultRenderTime;
		wstring mName;
	public:
		RHIResourceInfo mResourceInfo;
	};

	class RHI_API RHITexture2D : public RHITexture
	{
	public:
		RHITexture2D(uint32 width, uint32 height, uint32 numMips, uint32 numSamples, EPixelFormat format, uint32 flags, const ClearValueBinding& clearValue)
			:RHITexture(numMips, numSamples, format, flags, nullptr, clearValue)
			,mWidth(width),mHeight(height)
		{

		}

		virtual RHITexture2D* getTexture2D() { return this; }
		uint32 getWidth() const { return mWidth; }
		uint32 getHeight() const { return mHeight; }
	private:
		uint32 mWidth;
		uint32 mHeight;


	};



	

	class RHI_API RHITexture2DArray : public RHITexture
	{

	};


	class RHI_API RHITexture3D : public RHITexture
	{

	};

	class RHI_API RHITextureCube : public RHITexture
	{
	public:
		RHITextureCube(uint32 inSize, uint32 inNumMips, EPixelFormat inFormat, uint32 inflags, const ClearValueBinding& inClearValue)
			:RHITexture(inNumMips, 1, inFormat, inflags, nullptr, inClearValue)
			,mSize(inSize)
		{}
		virtual RHITextureCube* getTextureCube() { return this; }

		uint32 getSize()const { return mSize; }
	private:
		uint32 mSize;
	};

	class RHI_API RHITextureReference : public RHITexture
	{
	public:
		explicit RHITextureReference(LastRenderTimeContainer* inLastRenderTime)
			:RHITexture(0, 0, PF_Unknown, 0, inLastRenderTime, ClearValueBinding())
		{}

		virtual RHITextureReference* getTextureReference() override { return this; }

		inline RHITexture* getReferencedTexture() const { return mReferencedTexture.getReference(); }

		void setReferencedTexture(RHITexture* inTexture)
		{
			mReferencedTexture = inTexture;
		}
	private:
		TRefCountPtr<RHITexture> mReferencedTexture;
	};

	class RHI_API RHITextureReferenceNullImpl : public RHITextureReference
	{
		RHITextureReferenceNullImpl()
			:RHITextureReference(nullptr)
		{}
		void setReferencedTexture(RHITexture* inTexture)
		{
			RHITextureReference::setReferencedTexture(inTexture);
		}
	};

	class RHIIndexBuffer : public RHIResource
	{
	public:
		RHIIndexBuffer(uint32 inStride, uint32 inSize, uint32 inUsage)
			:mStride(inStride)
			,mSize(inSize)
			,mUsage(inUsage)
		{

		}

		uint32 getStride() const { return mStride; }

		uint32 getSize() const { return mSize; }

		uint32 getUsage() const { return mUsage; }

	private:
		uint32 mStride;
		uint32 mSize;
		uint32 mUsage;
	};

	class RHIVertexBuffer : public RHIResource
	{
	public:
		RHIVertexBuffer(uint32 inSize, uint32 inUsage)
			:mSize(inSize),
			mUsage(inUsage)
		{

		}
		uint32 getSize()const { return mSize; }
		uint32 getUsage() const { return mUsage; }
	private:
		uint32 mSize;
		uint32 mUsage;
	};

	class RHIComputeFence : public RHIResource
	{
	public:
		RHIComputeFence(wstring inName)
			:mName(inName),
			mWriteEnqueued(false)
		{}

		FORCEINLINE wstring getName() const
		{
			return mName;
		}

		FORCEINLINE bool getWriteEnqueued() const
		{
			return mWriteEnqueued;
		}

		virtual void reset()
		{
			mWriteEnqueued = false;
		}

		virtual void writeFence()
		{
			mWriteEnqueued = true;
		}


	private:
		wstring mName;
		bool mWriteEnqueued;
	};



	class RHIUnorderedAccessView : public RHIResource {};

	class RHIShaderResourceView : public RHIResource {};

	class RHIShader : public RHIResource
	{
	public:
		RHIShader(bool inbDonotDeferDelete = false) : RHIResource(inbDonotDeferDelete) {}

		void setHash(SHAHash inHash) { mHash = inHash; }
		SHAHash getHash() const { return mHash; }

		string mShaderName;
	private:
		SHAHash mHash;
	};


	struct RHIConstantBufferLayout
	{
		uint32 mConstantBufferSize;
		uint32 mResourceOffset;
		TArray<uint8> mResource;

		uint32 getHash() const
		{
			if (!bComputeHash)
			{
				uint32 tmpHash = mConstantBufferSize << 16;
				tmpHash ^= align(mResourceOffset, 8);
				uint32 N = mResource.size();
				while (N >= 4)
				{
					tmpHash ^= (mResource[--N] << 0);
					tmpHash ^= (mResource[--N] << 8);
					tmpHash ^= (mResource[--N] << 16);
					tmpHash ^= (mResource[--N] << 24);
				}
				while (N >= 2)
				{
					tmpHash ^= mResource[--N] << 0;
					tmpHash ^= mResource[--N] << 16;
				}
				while (N > 0)
				{
					tmpHash ^= mResource[--N];
				}
				mHash = tmpHash;
				bComputeHash = true;
			}
			return mHash;
		}

		explicit RHIConstantBufferLayout(wstring inName) :
			mConstantBufferSize(0),
			mResourceOffset(0),
			mName(inName),
			mHash(0),
			bComputeHash(false)
		{}
		enum EInit
		{
			Zero
		};
		explicit RHIConstantBufferLayout(EInit) :
			mConstantBufferSize(0),
			mResourceOffset(0),
			mName(TEXT("")),
			mHash(0),
			bComputeHash(false)
		{}

		void copyFrom(const RHIConstantBufferLayout& source)
		{
			mConstantBufferSize = source.mConstantBufferSize;
			mResourceOffset = source.mResourceOffset;
			mResource = source.mResource;
			mName = source.mName;
			mHash = source.mHash;
			bComputeHash = source.bComputeHash;
		}

		const wstring getDebugName() const { return mName; }
	private:
		wstring mName;
		mutable uint32 mHash;
		mutable bool bComputeHash;
	};

	class RHIConstantBuffer : public RHIResource
	{
	public:
		RHIConstantBuffer(const RHIConstantBufferLayout& inLayout)
			:mLayout(&inLayout)
		{}
		uint32 getSize() const { return mLayout->mConstantBufferSize; }
		const RHIConstantBufferLayout& getLayout() const { return *mLayout; }
	private:
		const RHIConstantBufferLayout* mLayout;
	};

	class RHIBoundShaderState : public RHIResource {};
	class RHIVertexDeclaration : public RHIResource { };

	class RHISamplerState : public RHIResource {};

	class RHIBlendState : public RHIResource {};

	class RHIRasterizerState : public RHIResource {};

	class RHIDepthStencilState : public RHIResource{};

	typedef RHIDepthStencilState* DepthStencilStateRHIParamRef;
	typedef TRefCountPtr<RHIDepthStencilState> DepthStencilStateRHIRef;

	typedef RHIRasterizerState* RasterizerStateRHIParamRef;
	typedef TRefCountPtr<RHIRasterizerState> RasterizerStateRHIRef;

	typedef RHIViewport*	ViewportRHIParamRef;
	typedef TRefCountPtr<RHIViewport>	ViewportRHIRef;

	typedef RHIVertexDeclaration*		VertexDeclarationRHIParamRef;
	typedef TRefCountPtr<RHIVertexDeclaration>	VertexDeclarationRHIRef;


	typedef RHITexture*		TextureRHIParamRef;
	typedef TRefCountPtr<RHITexture> TextureRHIRef;

	typedef RHITexture2D*	Texture2DRHIParamRef;
	typedef TRefCountPtr<RHITexture2D>	Texture2DRHIRef;

	typedef RHITextureCube* TextureCubeRHIParamRef;
	typedef TRefCountPtr<RHITextureCube> TextureCubeRHIRef;

	typedef RHITextureReference*		TextureReferenceRHIParamRef;
	typedef TRefCountPtr<RHITextureReference>		TextureReferenceRHIRef;

	typedef RHIUnorderedAccessView* UnorderedAccessViewRHIParamRef;

	typedef TRefCountPtr<RHIUnorderedAccessView> UnorderedAccessViewRHIRef;

	typedef RHIComputeFence*		ComputeFenceRHIParamRef;
	typedef TRefCountPtr<RHIComputeFence>	ComputeFenceRHIRef;

	typedef RHIShaderResourceView* ShaderResourceViewRHIParamRef;
	typedef TRefCountPtr<RHIShaderResourceView> ShaderResourceViewRHIRef;

	typedef RHIConstantBuffer*				ConstantBufferRHIParamRef;
	typedef TRefCountPtr<RHIConstantBuffer>			ConstantBufferRHIRef;

	typedef RHISamplerState*			SamplerStateRHIParamRef;
	typedef TRefCountPtr<RHISamplerState> SamplerStateRHIRef;

	typedef RHIBoundShaderState*		BoundShaderStateRHIParamRef;
	typedef TRefCountPtr<RHIBoundShaderState> BoundShaderStateRHIRef;

	typedef RHIBlendState*			BlendStateRHIParamRef;
	typedef TRefCountPtr<RHIBlendState>	BlendStateRHIRef;


	class RHIVertexShader : public RHIShader {};

	class RHIHullShader : public RHIShader {};
	class RHIDomainShader : public RHIShader {};
	class RHIGeometryShader : public RHIShader {};
	class RHIPixelShader : public RHIShader {};
	class RHIComputeShader : public RHIShader {};
	class RHIGraphicsPipelineState : public RHIResource {};
	class RHIComputePipelineState : public RHIResource {};


	typedef RHIVertexShader*				VertexShaderRHIParamRef;
	typedef TRefCountPtr<RHIVertexShader>	VertexShaderRHIRef;

	typedef RHIHullShader*					HullShaderRHIParamRef;
	typedef TRefCountPtr<RHIHullShader>		HullShaderRHIRef;

	typedef RHIDomainShader*				DomainShaderRHIParamRef;
	typedef TRefCountPtr<RHIDomainShader>	DomainShaderRHIRef;

	typedef RHIGeometryShader*				GeometryShaderRHIParamRef;
	typedef TRefCountPtr<RHIGeometryShader>	GeometryShaderRHIRef;

	typedef RHIPixelShader*					PixelShaderRHIParamRef;
	typedef TRefCountPtr<RHIPixelShader>	PixelShaderRHIRef;

	typedef RHIComputeShader*				ComputeShaderRHIParamRef;
	typedef TRefCountPtr<RHIComputeShader>	ComputeShaderRHIRef;

	typedef RHIIndexBuffer*					IndexBufferRHIParamRef;
	typedef TRefCountPtr<RHIIndexBuffer>	IndexBufferRHIRef;

	typedef RHIVertexBuffer*				VertexBufferRHIParamRef;
	typedef TRefCountPtr<RHIVertexBuffer>	VertexBufferRHIRef;


	class FExclusiveDepthStencil
	{
	public:
		enum Type
		{
			// don't use those directly, use the combined versions below
			// 4 bits are used for depth and 4 for stencil to make the hex value readable and non overlapping
			DepthNop = 0x00,
			DepthRead = 0x01,
			DepthWrite = 0x02,
			DepthMask = 0x0f,
			StencilNop = 0x00,
			StencilRead = 0x10,
			StencilWrite = 0x20,
			StencilMask = 0xf0,

			// use those:
			DepthNop_StencilNop = DepthNop + StencilNop,
			DepthRead_StencilNop = DepthRead + StencilNop,
			DepthWrite_StencilNop = DepthWrite + StencilNop,
			DepthNop_StencilRead = DepthNop + StencilRead,
			DepthRead_StencilRead = DepthRead + StencilRead,
			DepthWrite_StencilRead = DepthWrite + StencilRead,
			DepthNop_StencilWrite = DepthNop + StencilWrite,
			DepthRead_StencilWrite = DepthRead + StencilWrite,
			DepthWrite_StencilWrite = DepthWrite + StencilWrite,
		};

	private:
		Type Value;

	public:
		// constructor
		FExclusiveDepthStencil(Type InValue = DepthNop_StencilNop)
			: Value(InValue)
		{
		}

		inline bool IsUsingDepthStencil() const
		{
			return Value != DepthNop_StencilNop;
		}
		inline bool IsDepthWrite() const
		{
			return ExtractDepth() == DepthWrite;
		}
		inline bool IsStencilWrite() const
		{
			return ExtractStencil() == StencilWrite;
		}

		inline bool IsAnyWrite() const
		{
			return IsDepthWrite() || IsStencilWrite();
		}

		inline void SetDepthWrite()
		{
			Value = (Type)(ExtractStencil() | DepthWrite);
		}
		inline void SetStencilWrite()
		{
			Value = (Type)(ExtractDepth() | StencilWrite);
		}
		inline void SetDepthStencilWrite(bool bDepth, bool bStencil)
		{
			Value = DepthNop_StencilNop;

			if (bDepth)
			{
				SetDepthWrite();
			}
			if (bStencil)
			{
				SetStencilWrite();
			}
		}
		bool operator==(const FExclusiveDepthStencil& rhs) const
		{
			return Value == rhs.Value;
		}
		inline bool IsValid(FExclusiveDepthStencil& Current) const
		{
			Type Depth = ExtractDepth();

			if (Depth != DepthNop && Depth != Current.ExtractDepth())
			{
				return false;
			}

			Type Stencil = ExtractStencil();

			if (Stencil != StencilNop && Stencil != Current.ExtractStencil())
			{
				return false;
			}

			return true;
		}

		uint32 GetIndex() const
		{
			// Note: The array to index has views created in that specific order.

			// we don't care about the Nop versions so less views are needed
			// we combine Nop and Write
			switch (Value)
			{
			case DepthWrite_StencilNop:
			case DepthNop_StencilWrite:
			case DepthWrite_StencilWrite:
			case DepthNop_StencilNop:
				return 0; // old DSAT_Writable

			case DepthRead_StencilNop:
			case DepthRead_StencilWrite:
				return 1; // old DSAT_ReadOnlyDepth

			case DepthNop_StencilRead:
			case DepthWrite_StencilRead:
				return 2; // old DSAT_ReadOnlyStencil

			case DepthRead_StencilRead:
				return 3; // old DSAT_ReadOnlyDepthAndStencil
			}
			// should never happen
			return static_cast<uint32>(-1);
		}
		static const uint32 MaxIndex = 4;

	private:
		inline Type ExtractDepth() const
		{
			return (Type)(Value & DepthMask);
		}
		inline Type ExtractStencil() const
		{
			return (Type)(Value & StencilMask);
		}
	};

	class RHIRenderTargetView
	{
	public:
		TextureRHIParamRef mTexture;
		uint32 mMipIndex;
		uint32 mArraySliceIndex;
		ERenderTargetLoadAction mLoadAction;
		ERenderTargetStoreAction mStoreAction;

		RHIRenderTargetView()
			:mTexture(nullptr)
			, mMipIndex(0)
			, mArraySliceIndex(-1)
			, mLoadAction(ERenderTargetLoadAction::ELoad)
			, mStoreAction(ERenderTargetStoreAction::EStore)
		{}

		RHIRenderTargetView(Texture2DRHIParamRef inTexture, uint32 inMapIndex, uint32 inArraySliceIndex, ERenderTargetLoadAction inLoadAction, ERenderTargetStoreAction inStoreAction):
			mTexture(inTexture),
			mMipIndex(inMapIndex),
			mArraySliceIndex(inArraySliceIndex),
			mLoadAction(inLoadAction),
			mStoreAction(inStoreAction)
		{}

		RHIRenderTargetView(TextureRHIParamRef inTexture, uint32 inMipIndex, uint32 inArraySliceInded, ERenderTargetLoadAction inLoadAction, ERenderTargetStoreAction inStoreAction)
			:mTexture(inTexture)
			, mMipIndex(inMipIndex)
			, mArraySliceIndex(inArraySliceInded)
			, mLoadAction(inLoadAction)
			, mStoreAction(inStoreAction)
		{}

		RHIRenderTargetView(TextureRHIParamRef inTexture)
			:mTexture(inTexture)
			,mMipIndex(0)
			,mArraySliceIndex(-1)
			,mLoadAction(ERenderTargetLoadAction::ELoad)
			,mStoreAction(ERenderTargetStoreAction::EStore)
		{}
	};

	class RHIDepthRenderTargetView
	{
	public:
		TextureRHIParamRef mTexture;
		ERenderTargetLoadAction mDepthLoadAction;
		ERenderTargetStoreAction mDepthStoreAction;
		ERenderTargetLoadAction	mStencialLoadAction;

	public:
		RHIDepthRenderTargetView() :
			mTexture(nullptr),
			mDepthLoadAction(ERenderTargetLoadAction::EClear),
			mDepthStoreAction(ERenderTargetStoreAction::EStore),
			mStencialLoadAction(ERenderTargetLoadAction::EClear),
			mStencialStoreAction(ERenderTargetStoreAction::EStore),
			mDepthStencilAccess(FExclusiveDepthStencil::DepthWrite_StencilWrite)
		{

		}

		RHIDepthRenderTargetView(TextureRHIParamRef inTexture)
			:mTexture(inTexture),
			mDepthLoadAction(ERenderTargetLoadAction::EClear),
			mDepthStoreAction(ERenderTargetStoreAction::EStore),
			mStencialLoadAction(ERenderTargetLoadAction::EClear),
			mStencialStoreAction(ERenderTargetStoreAction::EStore),
			mDepthStencilAccess(FExclusiveDepthStencil::DepthWrite_StencilWrite)
		{
			validate();
		}
		RHIDepthRenderTargetView(TextureRHIParamRef inTexture, ERenderTargetLoadAction inLoadAction, ERenderTargetStoreAction inStoreAction)
			:mTexture(inTexture),
			mDepthLoadAction(inLoadAction),
			mDepthStoreAction(inStoreAction),
			mStencialLoadAction(inLoadAction),
			mStencialStoreAction(inStoreAction),
			mDepthStencilAccess(FExclusiveDepthStencil::DepthWrite_StencilWrite)
		{
			validate();
		}

		RHIDepthRenderTargetView(TextureRHIParamRef inTexture, ERenderTargetLoadAction inLoadAction, ERenderTargetStoreAction inStoreAction, FExclusiveDepthStencil inDepthStencilAccess)
			:mTexture(inTexture)
			, mDepthLoadAction(inLoadAction)
			, mDepthStoreAction(inStoreAction)
			, mStencialLoadAction(inLoadAction)
			, mStencialStoreAction(inStoreAction)
			, mDepthStencilAccess(inDepthStencilAccess)
		{
			validate();
		}

		void validate() const
		{
			BOOST_ASSERT(mDepthStencilAccess.IsDepthWrite() || mDepthStoreAction == ERenderTargetStoreAction::ENoAction);
			BOOST_ASSERT(mDepthStencilAccess.IsStencilWrite() || mStencialStoreAction == ERenderTargetStoreAction::ENoAction);
		}

		ERenderTargetStoreAction	mStencialStoreAction;
		FExclusiveDepthStencil		mDepthStencilAccess;

	public:
		FExclusiveDepthStencil getDepthStencilAccess() const { return mDepthStencilAccess; }
	};


	struct BoundShaderStateInput
	{
		VertexDeclarationRHIParamRef	mVertexDeclarationRHI;
		VertexShaderRHIParamRef			mVertexShaderRHI;
		HullShaderRHIParamRef			mHullShaderRHI;
		DomainShaderRHIParamRef			mDomainShaderRHI;
		GeometryShaderRHIParamRef		mGeometryShaderRHI;
		PixelShaderRHIParamRef			mPixelShaderRHI;
		FORCEINLINE BoundShaderStateInput()
			:mVertexDeclarationRHI(nullptr)
			,mVertexShaderRHI(nullptr)
			,mHullShaderRHI(nullptr)
			,mDomainShaderRHI(nullptr)
			,mGeometryShaderRHI(nullptr)
			,mPixelShaderRHI(nullptr)
		{}

		FORCEINLINE BoundShaderStateInput(
			VertexDeclarationRHIParamRef inVertexDeclarationRHI,
			VertexShaderRHIParamRef inVertexShaderRHI,
			HullShaderRHIParamRef inHullShaderRHI,
			DomainShaderRHIParamRef inDomainShaderRHI,
			GeometryShaderRHIParamRef inGeometryShaderRHI,
			PixelShaderRHIParamRef inPixelShaderRHI
		)
			:mVertexDeclarationRHI(inVertexDeclarationRHI)
			,mVertexShaderRHI(inVertexShaderRHI)
			,mHullShaderRHI(inHullShaderRHI)
			,mDomainShaderRHI(inDomainShaderRHI)
			,mGeometryShaderRHI(inGeometryShaderRHI)
			,mPixelShaderRHI(inPixelShaderRHI)
		{}
	};

	class GraphicsPipelineStateInitializer
	{
	public:
		enum OptionalState : uint32
		{
			OS_SetStencilRef = 1 << 0,
			OS_SetBlendFactor = 1 << 1,
		};
		using TRenderTargetFormats = TStaticArray<EPixelFormat, MaxSimultaneousRenderTargets>;
		using TRenderTargetFlags = TStaticArray<uint32, MaxSimultaneousRenderTargets>;
		using TRenderTargetLoadActions = TStaticArray<ERenderTargetLoadAction, MaxSimultaneousRenderTargets>;
		using TRenderTargetStoreActions = TStaticArray<ERenderTargetStoreAction, MaxSimultaneousRenderTargets>;

		GraphicsPipelineStateInitializer()
			:mBlendState(nullptr)
			, mRasterizerState(nullptr)
			, mDepthStencilState(nullptr)
			, mPrimitiveType(PT_Num)
			, mRenderTargetsEnabled(0)
			, mDepthStencilTargetFormat(PF_Unknown)
			, mDepthStencilTargetFlags(0)
			, mDepthStencilTargetLoadAction(ERenderTargetLoadAction::ENoAction)
			, mDepthStencilTargetStoreAction(ERenderTargetStoreAction::ENoAction)
			, mNumSamples(0)
			, mOptState(0)
			, mStencilRef(0)
			, mBlendFactor(1.0f, 1.0f, 1.0f, 1.0f)
		{
			for (uint32 i = 0; i < MaxSimultaneousRenderTargets; ++i)
			{
				mRenderTargetFormats[i] = PF_Unknown;
				mRenderTargetFlags[i] = 0;
				mRenderTargetLoadActions[i] = ERenderTargetLoadAction::ENoAction;
				mRenderTargetStoreActions[i] = ERenderTargetStoreAction::ENoAction;
			}
		}

		GraphicsPipelineStateInitializer(
			BoundShaderStateInput			inBoundShaderState,
			BlendStateRHIParamRef			inBlendState,
			RasterizerStateRHIParamRef		inRasterizerState,
			DepthStencilStateRHIParamRef	inDepthStencilState,
			uint32							inStencilRef,
			LinearColor						inBlendFactor,
			EPrimitiveType					inPrimitiveType,
			uint32							inRenderTargetEnabled,
			const TRenderTargetFormats&		inRenderTargetFormsts,
			const TRenderTargetFlags&		inRenderTargetFlags,
			const TRenderTargetLoadActions&	inRenderTargetLoadActions,
			const TRenderTargetStoreActions& inRenderTargetStoreActions,
			EPixelFormat					inDepthStencilTargetFormat,
			uint32							inDepthStencilTargetFlag,
			ERenderTargetLoadAction			inDepthStencilTargetLoadAction,
			ERenderTargetStoreAction		inDepthStencilTargetStoreAction,
			uint32							inNumSamples
		)
			:mBoundShaderState(inBoundShaderState)
			,mBlendState(inBlendState)
			,mRasterizerState(inRasterizerState)
			,mDepthStencilState(inDepthStencilState)
			,mPrimitiveType(inPrimitiveType)
			, mRenderTargetsEnabled(inRenderTargetEnabled)
			,mRenderTargetFormats(inRenderTargetFormsts)
			,mRenderTargetFlags(inRenderTargetFlags)
			,mRenderTargetLoadActions(inRenderTargetLoadActions)
			,mRenderTargetStoreActions(inRenderTargetStoreActions)
			,mDepthStencilTargetFormat(inDepthStencilTargetFormat)
			,mDepthStencilTargetFlags(inDepthStencilTargetFlag)
			,mDepthStencilTargetLoadAction(inDepthStencilTargetLoadAction)
			,mDepthStencilTargetStoreAction(inDepthStencilTargetStoreAction)
			,mNumSamples(inNumSamples)
			,mOptState(OptionalState::OS_SetStencilRef | OptionalState::OS_SetBlendFactor)
			,mStencilRef(inStencilRef)
			,mBlendFactor(inBlendFactor)
		{}

		bool operator == (GraphicsPipelineStateInitializer& rhs)
		{
			if (mBoundShaderState.mVertexDeclarationRHI != rhs.mBoundShaderState.mVertexDeclarationRHI || 
				mBoundShaderState.mVertexShaderRHI != rhs.mBoundShaderState.mVertexShaderRHI || 
				mBoundShaderState.mHullShaderRHI != rhs.mBoundShaderState.mHullShaderRHI || 
				mBoundShaderState.mDomainShaderRHI != rhs.mBoundShaderState.mDomainShaderRHI || 
				mBoundShaderState.mGeometryShaderRHI != rhs.mBoundShaderState.mGeometryShaderRHI || 
				mBoundShaderState.mPixelShaderRHI != rhs.mBoundShaderState.mPixelShaderRHI ||
				mBlendState != rhs.mBlendState ||
				mRasterizerState != rhs.mRasterizerState || 
				mDepthStencilState != rhs.mDepthStencilState || 
				mStencilRef != rhs.mStencilRef || 
				mBlendFactor != rhs.mBlendFactor || 
				mPrimitiveType != rhs.mPrimitiveType || mRenderTargetsEnabled != rhs.mRenderTargetsEnabled ||
				mRenderTargetFormats != rhs.mRenderTargetFormats ||
				mRenderTargetFlags != rhs.mRenderTargetFlags || 
				mRenderTargetLoadActions != rhs.mRenderTargetLoadActions ||
				mRenderTargetStoreActions != rhs.mRenderTargetStoreActions || 
				mDepthStencilTargetFormat != rhs.mDepthStencilTargetFormat || 
				mDepthStencilTargetFlags != rhs.mDepthStencilTargetFlags ||
				mDepthStencilTargetLoadAction != rhs.mDepthStencilTargetLoadAction || 
				mDepthStencilTargetStoreAction != rhs.mDepthStencilTargetStoreAction || 
				mNumSamples != rhs.mNumSamples || 
				mOptState != rhs.mOptState)
			{
				return false;
			}
			return true;
		}

#define COMPARE_FIELD_BEGIN(Field)	\
		if(Field != rhs.Field)	\
		{return Field COMPARE_OP rhs.Field;\
		}

#define COMPARE_FIELD(Field)	\
		else if(Field != rhs.Field)	\
		{\
			return Field COMPARE_OP rhs.Field;\
		}

#define  COMPARE__FIELD_END	\
		else{return false;}

		bool operator < (GraphicsPipelineStateInitializer& rhs) const
		{
#define COMPARE_OP <
			COMPARE_FIELD_BEGIN(mBoundShaderState.mVertexDeclarationRHI)
				COMPARE_FIELD(mBoundShaderState.mVertexShaderRHI)
				COMPARE_FIELD(mBoundShaderState.mHullShaderRHI)
				COMPARE_FIELD(mBoundShaderState.mDomainShaderRHI)
				COMPARE_FIELD(mBoundShaderState.mGeometryShaderRHI)
				COMPARE_FIELD(mBoundShaderState.mPixelShaderRHI)
				COMPARE_FIELD(mBlendState)
				COMPARE_FIELD(mRasterizerState)
				COMPARE_FIELD(mDepthStencilState)
				COMPARE_FIELD(mPrimitiveType)
				COMPARE__FIELD_END;
#undef  COMPARE_OP
		}

		bool operator>(GraphicsPipelineStateInitializer& rhs) const
		{
#define COMPARE_OP >
			COMPARE_FIELD_BEGIN(mBoundShaderState.mVertexDeclarationRHI)
				COMPARE_FIELD(mBoundShaderState.mVertexShaderRHI)
				COMPARE_FIELD(mBoundShaderState.mHullShaderRHI)
				COMPARE_FIELD(mBoundShaderState.mDomainShaderRHI)
				COMPARE_FIELD(mBoundShaderState.mGeometryShaderRHI)
				COMPARE_FIELD(mBoundShaderState.mPixelShaderRHI)
				COMPARE_FIELD(mBlendState)
				COMPARE_FIELD(mRasterizerState)
				COMPARE_FIELD(mDepthStencilState)
				COMPARE_FIELD(mPrimitiveType)
				COMPARE__FIELD_END;
#undef COMPARE_OP
		}

#undef COMPARE_FIELD_BEGIN
#undef COMPARE_FIELD
#undef COMPARE__FIELD_END

		uint32 getOptionalSetState()
		{
			return mOptState;
		}

		uint32 getStencilRef() const
		{
			return mStencilRef;
		}
		void setStencilRef(uint32 InStencilRef)
		{
			mOptState |= OptionalState::OS_SetStencilRef;
			mStencilRef = InStencilRef;
		}

		void clearSetStencilRef()
		{
			mOptState &= ~OptionalState::OS_SetStencilRef;
		}

		LinearColor getBlendFactor() const
		{
			return mBlendFactor;
		}

		void setBlendFactor(LinearColor inBlendFactor)
		{
			mOptState |= OptionalState::OS_SetBlendFactor;
			mBlendFactor = inBlendFactor;
		}
		void clearSetBlendFactor()
		{
			mOptState &= ~OptionalState::OS_SetBlendFactor;
		}

		BoundShaderStateInput			mBoundShaderState;
		BlendStateRHIParamRef			mBlendState;
		RasterizerStateRHIParamRef		mRasterizerState;
		DepthStencilStateRHIParamRef	mDepthStencilState;
		EPrimitiveType					mPrimitiveType;
		uint32							mRenderTargetsEnabled;
		TRenderTargetFormats			mRenderTargetFormats;
		TRenderTargetFlags				mRenderTargetFlags;
		TRenderTargetLoadActions		mRenderTargetLoadActions;
		TRenderTargetStoreActions		mRenderTargetStoreActions;
		EPixelFormat					mDepthStencilTargetFormat;
		uint32							mDepthStencilTargetFlags;
		ERenderTargetLoadAction			mDepthStencilTargetLoadAction;
		ERenderTargetStoreAction		mDepthStencilTargetStoreAction;
		uint32							mNumSamples;
	private:
		uint32							mOptState;
		uint32							mStencilRef;
		LinearColor						mBlendFactor;
		friend class MeshDrawingPolicy;
	};

	class RHIGraphicsPipelineStateFallBack : public RHIGraphicsPipelineState
	{
	public:
		RHIGraphicsPipelineStateFallBack() {}
		RHIGraphicsPipelineStateFallBack(GraphicsPipelineStateInitializer& init)
			:mInitializer(init)
		{}
		GraphicsPipelineStateInitializer mInitializer;
	};

	class RHISetRenderTargetsInfo
	{
	public:
		RHIRenderTargetView mColorRenderTarget[MaxSimultaneousRenderTargets];
		int32 mNumColorRenderTargets{ 0 };
		bool bClearColor{ false };
		RHIDepthRenderTargetView mDepthStencilRenderTarget;
		bool bClearDepth{ false };
		bool bClearStencil{ false };
		UnorderedAccessViewRHIRef mUnorderedAccessView[MaxSimultaneousUAVs];
		int32 mNumUAVs{ 0 };
	public:

		RHISetRenderTargetsInfo()
		{
		}

		RHISetRenderTargetsInfo(int32 inNumColorRenderTargets, const RHIRenderTargetView* inColorRenderTargets, const RHIDepthRenderTargetView& inDepthStencilRenderTarget) :
			mNumColorRenderTargets(inNumColorRenderTargets),
			bClearColor(inNumColorRenderTargets > 0 && inColorRenderTargets[0].mLoadAction == ERenderTargetLoadAction::EClear),
			mDepthStencilRenderTarget(inDepthStencilRenderTarget),
			bClearDepth(inDepthStencilRenderTarget.mTexture && inDepthStencilRenderTarget.mDepthLoadAction == ERenderTargetLoadAction::EClear),
			bClearStencil(inDepthStencilRenderTarget.mTexture &&inDepthStencilRenderTarget.mStencialLoadAction == ERenderTargetLoadAction::EClear),
			mNumUAVs(0)
		{
			for (int32 index = 0; index < inNumColorRenderTargets; ++index)
			{
				mColorRenderTarget[index] = inColorRenderTargets[index];
			}
		}

		void setClearDepthStencil(bool binClearDepth, bool bInClearStencil = false)
		{
			if (binClearDepth)
			{
				mDepthStencilRenderTarget.mDepthLoadAction = ERenderTargetLoadAction::EClear;
			}
			if (bInClearStencil)
			{
				mDepthStencilRenderTarget.mStencialLoadAction = ERenderTargetLoadAction::EClear;
			}
			bClearDepth = binClearDepth;
			bClearStencil = bInClearStencil;
		}
		uint32 calculateHash() const
		{
			struct HashableStruct 
			{
				RHITexture* mTexture[MaxSimultaneousRenderTargets + 1];
				uint32 mMipIndex[MaxSimultaneousRenderTargets];
				uint32 mArraySliceIndex[MaxSimultaneousRenderTargets];
				ERenderTargetLoadAction mLoadAction[MaxSimultaneousRenderTargets];
				ERenderTargetStoreAction mStoreAction[MaxSimultaneousRenderTargets];
				ERenderTargetLoadAction mDepthLoadAction;
				ERenderTargetStoreAction mDepthStoreAction;
				ERenderTargetLoadAction mStencilLoadAction;
				ERenderTargetStoreAction mStencilStoreAction;
				FExclusiveDepthStencil mDepthStencilAccess;
				bool bClearDepth;
				bool bClearStencil;
				bool bClearColor;
				RHIUnorderedAccessView* mUnorderedAccessView[MaxSimultaneousUAVs];
				void set(const RHISetRenderTargetsInfo& info)
				{
					Memory::memzero(*this);
					for (int32 index = 0; index < info.mNumColorRenderTargets; index++)
					{
						mTexture[index] = info.mColorRenderTarget[index].mTexture;
						mMipIndex[index] = info.mColorRenderTarget[index].mMipIndex;
						mArraySliceIndex[index] = info.mColorRenderTarget[index].mArraySliceIndex;
						mLoadAction[index] = info.mColorRenderTarget[index].mLoadAction;
						mStoreAction[index] = info.mColorRenderTarget[index].mStoreAction;
					}
					mTexture[MaxSimultaneousRenderTargets] = info.mDepthStencilRenderTarget.mTexture;
					mDepthLoadAction = info.mDepthStencilRenderTarget.mDepthLoadAction;
					mDepthStoreAction = info.mDepthStencilRenderTarget.mDepthStoreAction;
					mStencilLoadAction = info.mDepthStencilRenderTarget.mStencialLoadAction;
					mStencilStoreAction = info.mDepthStencilRenderTarget.mStencialStoreAction;
					mDepthStencilAccess = info.mDepthStencilRenderTarget.mDepthStencilAccess;

					bClearColor = info.bClearColor;
					bClearDepth = info.bClearDepth;
					bClearStencil = info.bClearStencil;
					for (int32 index = 0; index < MaxSimultaneousUAVs; ++index)
					{
						mUnorderedAccessView[index] = info.mUnorderedAccessView[index];
					}
				}
			};
			HashableStruct RTHash;
			Memory::memzero(RTHash);
			RTHash.set(*this);
			return Crc::memCrc32(&RTHash, sizeof(RTHash));
		}

	};

	
}