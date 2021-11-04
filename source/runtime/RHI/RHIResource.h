#pragma once
#include "Containers/LockFreeListImpl.h"
#include "HAL/PlatformAtomics.h"
#include "Template/RefCounting.h"
#include "Template/AlignmentTemplates.h"
#include "Misc/SecureHash.h"
#include "Misc/Crc.h"
#include "RHIConfig.h"
#include "RHIResource.h"
#include "RHI.h"

namespace Air
{
	extern RHI_API bool GRHINeedsExtraDeletionLatency;


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

		bool isValid() const
		{
			return !mMarkedForDelete && mNumRefs.getValue() > 0;
		}
		

		bool isCommitted() const
		{
			BOOST_ASSERT(isInRenderingThread());
			return bCommitted;
		}

		void setCommitted(bool bInCommitted)
		{
			BOOST_ASSERT(isInRenderingThread());
			bCommitted = bInCommitted;
		}

	private:
		mutable int32 mMarkedForDelete;
		mutable ThreadSafeCounter mNumRefs;
		bool bCommitted;
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

		inline int2 getSizeXY() const { return int2(mWidth, mHeight); }
		virtual int3 getSizeXYZ() const { return int3(mWidth, mHeight, 1); }
	private:
		uint32 mWidth;
		uint32 mHeight;


	};



	

	class RHI_API RHITexture2DArray : public RHITexture2D
	{
	public:
		RHITexture2DArray(uint32 inSizeX, uint32 inSizeY, uint32 inSizeZ, uint32 inNumMips, uint32 numSamples, EPixelFormat inFormat, uint32 inFlags, const ClearValueBinding& inClearValue)
			:RHITexture2D(inSizeX, inSizeY, inNumMips, numSamples, inFormat, inFlags, inClearValue)
			, mDepth(inSizeZ)
		{}


		virtual RHITexture2DArray* getTexture2DArray(){return this; }

		virtual RHITexture2D* getTexture2D() { return nullptr; }

		uint32 getDepth() const { return mDepth; }

		virtual int3 getSizeXYZ() const final override
		{
			return int3(getWidth(), getHeight(), getDepth());
		}

	private:
		uint32 mDepth;
	};


	class RHI_API RHITexture3D : public RHITexture
	{
	public:
		RHITexture3D(uint32 width, uint32 height, uint32 depth, uint32 inNumMips, EPixelFormat inFormat, uint32 inFlags, const ClearValueBinding& inClearValue)
			:RHITexture(inNumMips, 1, inFormat, inFlags, nullptr, inClearValue)
			, mWidth(width)
			, mHeight(height)
			, mDepth(depth)
		{

		}

		virtual RHITexture3D* getTexture3D() { return this; }

		uint32 getWidth() const { return mWidth; }
		
		uint32 getHeight() const { return mHeight; }

		uint32 getDepth() const { return mDepth; }
	private:
		uint32 mWidth;
		uint32 mHeight;
		uint32 mDepth;
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
			:mName(inName)
		{}

		FORCEINLINE wstring getName() const
		{
			return mName;
		}

		FORCEINLINE bool getWriteEnqueued() const
		{
			return mTransition != nullptr;
		}

	private:
		wstring mName;

	public:
		const RHITransition* mTransition = nullptr;
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
		struct ResourceParameter
		{
			uint16 mMemberOffset;
			EConstantBufferBaseType mMemberType;
		};

		uint32 mConstantBufferSize;
		TArray<ResourceParameter> mResources;

		inline uint32 getHash() const
		{
			return mHash;
		}

		void computeHash()
		{
			uint32 tmpHash = mConstantBufferSize << 16;
			for (int32 resourceIndex = 0; resourceIndex < mResources.size(); resourceIndex++)
			{
				BOOST_ASSERT(mResources[resourceIndex].mMemberOffset = align(mResources[resourceIndex].mMemberOffset, SHADER_PARAMETER_POINTER_ALIGNMENT));
				tmpHash ^= mResources[resourceIndex].mMemberOffset;
			}

			uint32 n = mResources.size();
			while (n >= 4)
			{
				tmpHash ^= (mResources[--n].mMemberType << 0);
				tmpHash ^= (mResources[--n].mMemberType << 8);
				tmpHash ^= (mResources[--n].mMemberType << 16);
				tmpHash ^= (mResources[--n].mMemberType << 24);
			}

			while (n >= 2)
			{
				tmpHash ^= mResources[--n].mMemberType << 0;
				tmpHash ^= mResources[--n].mMemberType << 16;
			}

			while (n > 0)
			{
				tmpHash ^= mResources[--n].mMemberType;
			}
			mHash = tmpHash;
		}

		explicit RHIConstantBufferLayout(wstring inName)
			:mConstantBufferSize(0)
			, mName(inName)
			, mHash(0)
		{}

		enum EInit
		{
			Zero
		};

		explicit RHIConstantBufferLayout(EInit)
			: mConstantBufferSize(0)
			, mName()
			, mHash(0)
		{}

		void copyFrom(const RHIConstantBufferLayout& source)
		{
			mConstantBufferSize = source.mConstantBufferSize;
			mResources = source.mResources;
			mName = source.mName;
			mHash = source.mHash;
		}

		const wstring getDebugName() const { return mName; }

		uint32 numRenderTargets() const { return 0; }

		uint32 numTextures() const { return 0; }

		uint32 numUAVs() const { return 0; }
	private:
		wstring mName;
		uint32 mHash;
	};


	inline bool operator == (const RHIConstantBufferLayout::ResourceParameter& a, const RHIConstantBufferLayout::ResourceParameter& b)
	{
		return a.mMemberOffset == b.mMemberOffset
			&& a.mMemberType == b.mMemberType;
	}

	inline bool operator == (const RHIConstantBufferLayout& a, const RHIConstantBufferLayout& b)
	{
		return a.mConstantBufferSize == b.mConstantBufferSize
			&& a.mResources == b.mResources;
	}

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

	class RHIStructuredBuffer : public RHIResource
	{
	public:
		RHIStructuredBuffer(uint32 inStride, uint32 inSize, uint32 inUsage)
			:mStride(inStride)
			,mSize(inSize)
			,mUsage(inUsage)
		{}

		uint32 getStride() const { return mStride; }

		uint32 getSize() const { return mSize; }

		uint32 getUsage() const { return mUsage; }
	private:
		uint32 mStride;
		uint32 mSize;
		uint32 mUsage;
	};

	class RHIBoundShaderState : public RHIResource {};
	class RHIVertexDeclaration : public RHIResource 
	{ 
	public:

		virtual bool getInitializer(VertexDeclarationElementList& init) { return false; }
	};

	class RHISamplerState : public RHIResource 
	{
	public:
		virtual bool isImmutable()const { return false; }
	};

	class RHIBlendState : public RHIResource 
	{
	public:
		virtual bool getInitializer(BlendStateInitializerRHI& init)
		{
			return false;
		}
	};

	class RHIRasterizerState : public RHIResource 
	{
	public:
		virtual bool getInitializer(struct RasterizerStateInitializerRHI& init) { return false; }
	};

	class RHIDepthStencilState : public RHIResource
	{
	public:
		virtual bool getInitializer(struct DepthStencilStateInitializerRHI& init)
		{
			return false;
		}
	};

	typedef TRefCountPtr<RHIDepthStencilState> DepthStencilStateRHIRef;

	typedef TRefCountPtr<RHIRasterizerState> RasterizerStateRHIRef;

	typedef TRefCountPtr<RHIViewport>	ViewportRHIRef;

	typedef TRefCountPtr<RHIVertexDeclaration>	VertexDeclarationRHIRef;

	typedef TRefCountPtr<RHITexture> TextureRHIRef;

	typedef TRefCountPtr<RHITexture2D>	Texture2DRHIRef;

	typedef TRefCountPtr<RHITextureCube> TextureCubeRHIRef;

	typedef TRefCountPtr<RHITextureReference>		TextureReferenceRHIRef;

	typedef TRefCountPtr<RHIUnorderedAccessView> UnorderedAccessViewRHIRef;

	typedef TRefCountPtr<RHIComputeFence>	ComputeFenceRHIRef;

	typedef TRefCountPtr<RHIShaderResourceView> ShaderResourceViewRHIRef;

	typedef TRefCountPtr<RHIConstantBuffer>			ConstantBufferRHIRef;

	typedef TRefCountPtr<RHISamplerState> SamplerStateRHIRef;

	typedef TRefCountPtr<RHIBoundShaderState> BoundShaderStateRHIRef;

	typedef TRefCountPtr<RHIBlendState>	BlendStateRHIRef;


	class RHIVertexShader : public RHIShader {};

	class RHIHullShader : public RHIShader {};
	class RHIDomainShader : public RHIShader {};
	class RHIGeometryShader : public RHIShader {};
	class RHIPixelShader : public RHIShader {};
	class RHIComputeShader : public RHIShader 
	{
	public:
		RHIComputeShader() : mStats(nullptr) {}

		inline void setStats(struct PipelineStateStats* ptr) { mStats = ptr; }

		void updateStats();
	private:
		struct PipelineStateStats* mStats;
	};
	class RHIGraphicsPipelineState : public RHIResource {};
	class RHIComputePipelineState : public RHIResource {};


	typedef TRefCountPtr<RHIVertexShader>	VertexShaderRHIRef;

	typedef TRefCountPtr<RHIHullShader>		HullShaderRHIRef;

	typedef TRefCountPtr<RHIDomainShader>	DomainShaderRHIRef;

	typedef TRefCountPtr<RHIGeometryShader>	GeometryShaderRHIRef;

	typedef TRefCountPtr<RHIPixelShader>	PixelShaderRHIRef;

	typedef TRefCountPtr<RHIComputeShader>	ComputeShaderRHIRef;

	typedef TRefCountPtr<RHIIndexBuffer>	IndexBufferRHIRef;

	typedef TRefCountPtr<RHIVertexBuffer>	VertexBufferRHIRef;

	typedef TRefCountPtr<RHIStructuredBuffer> StructuredBufferRHIRef;

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

		inline bool isUsingDepth() const
		{
			return (ExtractDepth() != DepthNop);
		}

		inline bool isUsingStencil() const
		{
			return (ExtractStencil() != StencilNop);
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

		bool operator !=(const FExclusiveDepthStencil& rhs) const
		{
			return Value != rhs.Value;
		}

		inline void getAccess(ERHIAccess& depthAccess, ERHIAccess& stencilAccess) const 
		{
			depthAccess = ERHIAccess::None;
			constexpr ERHIAccess dsvReadOnlyMask = ERHIAccess::DSVRead | ERHIAccess::SRVGraphics | ERHIAccess::SRVCompute;

			constexpr ERHIAccess dsvReadWriteMask = ERHIAccess::DSVRead | ERHIAccess::DSVWrite;

			if (isUsingDepth())
			{
				depthAccess = IsDepthWrite() ? dsvReadWriteMask : dsvReadOnlyMask;
			}
			stencilAccess = ERHIAccess::None;
			if (isUsingStencil())
			{
				stencilAccess = IsStencilWrite() ? dsvReadWriteMask : dsvReadOnlyMask;
			}
		}

		template<typename TFunction>
		inline void enumerateSubresource(TFunction function) const
		{
			if (!IsUsingDepthStencil())
			{
				return;
			}
			ERHIAccess depthAccess = ERHIAccess::None;
			ERHIAccess stencilAccess = ERHIAccess::None;
			getAccess(depthAccess, stencilAccess);
			if (depthAccess == stencilAccess)
			{
				function(depthAccess, RHITransitionInfo::kAllSubresources);
			}
			else
			{
				if (depthAccess != ERHIAccess::None)
				{
					function(depthAccess, RHITransitionInfo::kDepthPlaneSlice);
				}
				if (stencilAccess != ERHIAccess::None)
				{
					function(stencilAccess, RHITransitionInfo::kStencilPlaneSlice);
				}
			}
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
		RHITexture* mTexture;
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

		RHIRenderTargetView(RHITexture* inTexture, uint32 inMipIndex, uint32 inArraySliceIndex):
			mTexture(inTexture)
			,mMipIndex(inMipIndex)
			,mArraySliceIndex(inArraySliceIndex)
			,mLoadAction(ERenderTargetLoadAction::ELoad)
			,mStoreAction(ERenderTargetStoreAction::EStore)
		{}

		RHIRenderTargetView(RHITexture2D* inTexture, uint32 inMapIndex, uint32 inArraySliceIndex, ERenderTargetLoadAction inLoadAction, ERenderTargetStoreAction inStoreAction):
			mTexture(inTexture),
			mMipIndex(inMapIndex),
			mArraySliceIndex(inArraySliceIndex),
			mLoadAction(inLoadAction),
			mStoreAction(inStoreAction)
		{}

		RHIRenderTargetView(RHITexture* inTexture, uint32 inMipIndex, uint32 inArraySliceInded, ERenderTargetLoadAction inLoadAction, ERenderTargetStoreAction inStoreAction)
			:mTexture(inTexture)
			, mMipIndex(inMipIndex)
			, mArraySliceIndex(inArraySliceInded)
			, mLoadAction(inLoadAction)
			, mStoreAction(inStoreAction)
		{}

		RHIRenderTargetView(RHITexture* inTexture)
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
		RHITexture* mTexture;
		ERenderTargetLoadAction mDepthLoadAction;
		ERenderTargetStoreAction mDepthStoreAction;
		ERenderTargetLoadAction	mStencialLoadAction;

	public:
		explicit RHIDepthRenderTargetView() :
			mTexture(nullptr),
			mDepthLoadAction(ERenderTargetLoadAction::EClear),
			mDepthStoreAction(ERenderTargetStoreAction::EStore),
			mStencialLoadAction(ERenderTargetLoadAction::EClear),
			mStencialStoreAction(ERenderTargetStoreAction::EStore),
			mDepthStencilAccess(FExclusiveDepthStencil::DepthWrite_StencilWrite)
		{

		}

		explicit RHIDepthRenderTargetView(RHITexture* inTexture)
			:mTexture(inTexture),
			mDepthLoadAction(ERenderTargetLoadAction::EClear),
			mDepthStoreAction(ERenderTargetStoreAction::EStore),
			mStencialLoadAction(ERenderTargetLoadAction::EClear),
			mStencialStoreAction(ERenderTargetStoreAction::EStore),
			mDepthStencilAccess(FExclusiveDepthStencil::DepthWrite_StencilWrite)
		{
			validate();
		}
		explicit RHIDepthRenderTargetView(RHITexture* inTexture, ERenderTargetLoadAction inLoadAction, ERenderTargetStoreAction inStoreAction)
			:mTexture(inTexture),
			mDepthLoadAction(inLoadAction),
			mDepthStoreAction(inStoreAction),
			mStencialLoadAction(inLoadAction),
			mStencialStoreAction(inStoreAction),
			mDepthStencilAccess(FExclusiveDepthStencil::DepthWrite_StencilWrite)
		{
			validate();
		}

		explicit RHIDepthRenderTargetView(RHITexture* inTexture, ERenderTargetLoadAction inLoadAction, ERenderTargetStoreAction inStoreAction, FExclusiveDepthStencil inDepthStencilAccess)
			:mTexture(inTexture)
			, mDepthLoadAction(inLoadAction)
			, mDepthStoreAction(inStoreAction)
			, mStencialLoadAction(inLoadAction)
			, mStencialStoreAction(inStoreAction)
			, mDepthStencilAccess(inDepthStencilAccess)
		{
			validate();
		}

		explicit RHIDepthRenderTargetView(RHITexture* inTexture, ERenderTargetLoadAction inDepthLoad, ERenderTargetStoreAction inDepthStore, ERenderTargetLoadAction inStencilLoad, ERenderTargetStoreAction inStencilStore, FExclusiveDepthStencil inDepthStencilAccess)
			:mTexture(inTexture)
			,mDepthLoadAction(inDepthLoad)
			,mDepthStoreAction(inDepthStore)
			,mStencialLoadAction(inStencilLoad)
			,mStencialStoreAction(inStencilStore)
			,mDepthStencilAccess(inDepthStencilAccess)
		{

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
		RHIVertexDeclaration*	mVertexDeclarationRHI;
		RHIVertexShader*			mVertexShaderRHI;
		RHIHullShader*			mHullShaderRHI;
		RHIDomainShader*			mDomainShaderRHI;
		RHIGeometryShader*		mGeometryShaderRHI;
		RHIPixelShader*			mPixelShaderRHI;
		FORCEINLINE BoundShaderStateInput()
			:mVertexDeclarationRHI(nullptr)
			,mVertexShaderRHI(nullptr)
			,mHullShaderRHI(nullptr)
			,mDomainShaderRHI(nullptr)
			,mGeometryShaderRHI(nullptr)
			,mPixelShaderRHI(nullptr)
		{}

		FORCEINLINE BoundShaderStateInput(
			RHIVertexDeclaration* inVertexDeclarationRHI,
			RHIVertexShader* inVertexShaderRHI,
			RHIHullShader* inHullShaderRHI,
			RHIDomainShader* inDomainShaderRHI,
			RHIGeometryShader* inGeometryShaderRHI,
			RHIPixelShader* inPixelShaderRHI
		)
			:mVertexDeclarationRHI(inVertexDeclarationRHI)
			,mVertexShaderRHI(inVertexShaderRHI)
			,mHullShaderRHI(inHullShaderRHI)
			,mDomainShaderRHI(inDomainShaderRHI)
			,mGeometryShaderRHI(inGeometryShaderRHI)
			,mPixelShaderRHI(inPixelShaderRHI)
		{}
	};

	enum class ESubpassHint : uint8
	{
		None,
		DepthReadSubpass,
	};

	

	

	class RHISetRenderTargetsInfo
	{
	public:
		RHIRenderTargetView mColorRenderTarget[MaxSimultaneousRenderTargets];
		int32 mNumColorRenderTargets{ 0 };
		bool bClearColor{ false };

		RHIRenderTargetView mColorResolveRenderTarget[MaxSimultaneousRenderTargets];

		RHIDepthRenderTargetView mDepthStencilRenderTarget;
		bool bClearDepth{ false };
		bool bClearStencil{ false };
		bool bHasResolveAttachments{ false };
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
				bool bHasResolveAttachments;
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
					bHasResolveAttachments = info.bHasResolveAttachments;
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
	enum class ERenderTargetActions : uint8
	{
		LoadOpMask = 2,
#define RTACTION_MAKE_MASK(Load, Store) (((uint8)ERenderTargetLoadAction::ELoad << (uint8)LoadOpMask) | (uint8)ERenderTargetStoreAction::EStore)

		DontLoad_DontStore = RTACTION_MAKE_MASK(ENoAction, ENoAction),

		DontLoad_Store = RTACTION_MAKE_MASK(ENoAction, EStore),
		Clear_Store = RTACTION_MAKE_MASK(EClear, EStore),
		Load_Store = RTACTION_MAKE_MASK(ELoad, EStore),
		
		Clear_DontStore = RTACTION_MAKE_MASK(EClear, ENoAction),
		Load_DontStore = RTACTION_MAKE_MASK(ELoad, ENoAction),
		Clear_Resolve = RTACTION_MAKE_MASK(EClear, EMultisampleResolve),
		Load_Resolve = RTACTION_MAKE_MASK(ELoad, EMultisampleResolve),

#undef RTACTION_MAKE_MASK
	};

	enum class EDepthStencilTargetActions :uint8
	{
		DepthMask = 4,
#define RTACTION_MAKE_MASK(Depth, Stencil) (((uint8)ERenderTargetActions::Depth << (uint8)DepthMask) | (uint8)ERenderTargetActions::Stencil)

		DontLoad_DontStore =					RTACTION_MAKE_MASK(DontLoad_DontStore, DontLoad_DontStore),

		DontLoad_StoreDepthStencil =					RTACTION_MAKE_MASK(DontLoad_Store, DontLoad_Store),
		
		DontLoad_StoreStencilNotDepth =					RTACTION_MAKE_MASK(DontLoad_DontStore, DontLoad_Store),

		ClearDepthStencil_StoreDepthStencil =					RTACTION_MAKE_MASK(Clear_Store, Clear_Store),
		LoadDepthStencil_StoreDepthStencil =					RTACTION_MAKE_MASK(Load_Store, Load_Store),
		LoadDepthNotStencil_DontStore =					RTACTION_MAKE_MASK(Load_DontStore, DontLoad_DontStore),
		LoadDepthStencil_StoreStencilNotDepth = RTACTION_MAKE_MASK(Load_DontStore, Load_Store),

		ClearDepthStencil_DontStoreDepthStencil = RTACTION_MAKE_MASK(Clear_DontStore, Clear_DontStore),
		LoadDepthStencil_DontStoreDepthStencil = RTACTION_MAKE_MASK(Load_DontStore, Load_DontStore),

		ClearDepthStencil_StoreDepthNotStencil = RTACTION_MAKE_MASK(Clear_Store, Clear_DontStore),

		ClearDepthStencil_StoreStencilNotDepth = RTACTION_MAKE_MASK(Clear_DontStore, Clear_Store),

		ClearDepthStencil_ResolveDepthNotStencil = RTACTION_MAKE_MASK(Clear_Resolve, Clear_DontStore),

		ClearDepthStencil_ResolveStencilNotDepth = RTACTION_MAKE_MASK(Clear_DontStore, Clear_Resolve),

		ClearStencilDontLoadDepth_StoreStencilNotDepth = RTACTION_MAKE_MASK(DontLoad_DontStore, Clear_Store),
#undef RTACTION_MAKE_MASK
	};

	inline constexpr EDepthStencilTargetActions makeDepthStencilTargetActions(const ERenderTargetActions depth, ERenderTargetActions stencil)
	{
		return (EDepthStencilTargetActions)(((uint8)depth << (uint8)EDepthStencilTargetActions::DepthMask) | (uint8)stencil);
	}

	inline ERenderTargetActions makeRenderTargetActions(ERenderTargetLoadAction load, ERenderTargetStoreAction store)
	{
		return (ERenderTargetActions)(((uint8)load << (uint8)ERenderTargetActions::LoadOpMask) | (uint8)store);
	}

	inline ERenderTargetActions getDepthActions(EDepthStencilTargetActions action)
	{
		return (ERenderTargetActions)((uint8)action >> (uint8)EDepthStencilTargetActions::DepthMask);
	}

	inline ERenderTargetActions getStencilActions(EDepthStencilTargetActions action)
	{
		return (ERenderTargetActions)((uint8)action & ((1 << (uint8)EDepthStencilTargetActions::DepthMask) - 1));
	}

	inline ERenderTargetLoadAction getLoadAction(ERenderTargetActions action)
	{
		return (ERenderTargetLoadAction)((uint8)action >> (uint8)ERenderTargetActions::LoadOpMask);
	}

	inline ERenderTargetStoreAction getStoreAction(ERenderTargetActions action)
	{
		return (ERenderTargetStoreAction)((uint8)action & ((1 < (uint8)ERenderTargetActions::LoadOpMask) - 1));
	}

	struct RHIRenderPassInfo
	{
		struct ColorEntry
		{
			RHITexture* mRenderTarget;
			RHITexture* mResolveTarget;
			int32 mArraySlice;
			uint8 mMipIndex;
			ERenderTargetActions mAction;
		};

		ColorEntry mColorRenderTargets[MaxSimultaneousRenderTargets];


		struct DepthStencilEntry
		{
			RHITexture* mDepthStencilTarget;
			RHITexture* mResolveTarget;
			EDepthStencilTargetActions mActions;
			FExclusiveDepthStencil mExculusiveDepthStencil;
		};

		DepthStencilEntry mDepthStencilRenderTarget;

		ResolveParams mResolveParameters;

		uint32 mNumOcclusionQueries = 0;
		bool bOcclusionQueries = false;

		bool bGeneratingMips = false;

		bool bMultiviewPass = false;

		ESubpassHint mSubpassHint = ESubpassHint::None;

		bool bTooManyUAVs = false;

		bool bIsMSAA = false;

		int32 mUAVIndex = -1;

		int32 mNumUAVs = 0;


		UnorderedAccessViewRHIRef mUAVs[MaxSimultaneousUAVs];

		explicit RHIRenderPassInfo(RHITexture* colorRT, ERenderTargetActions colorAction, RHITexture* resolveRT = nullptr, uint32 inMipIndex = 0, int32 inArraySlice = -1)
		{
			BOOST_ASSERT(colorRT);
			mColorRenderTargets[0].mRenderTarget = colorRT;
			mColorRenderTargets[0].mResolveTarget = resolveRT;
			mColorRenderTargets[0].mArraySlice = inArraySlice;
			mColorRenderTargets[0].mMipIndex = inMipIndex;
			mColorRenderTargets[0].mAction = colorAction;
			mDepthStencilRenderTarget.mDepthStencilTarget = nullptr;
			mDepthStencilRenderTarget.mActions = EDepthStencilTargetActions::DontLoad_DontStore;
			mDepthStencilRenderTarget.mExculusiveDepthStencil = FExclusiveDepthStencil::DepthNop_StencilNop;
			mDepthStencilRenderTarget.mResolveTarget = nullptr;
			bIsMSAA = colorRT->getNumSamples() > 1;
			Memory::memzero(&mColorRenderTargets[1], sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - 1));
		}

		explicit RHIRenderPassInfo(int32 numColorRTs, RHITexture* colorRTs[], ERenderTargetActions colorAction)
		{
			BOOST_ASSERT(numColorRTs > 0);
			for (int32 index = 0; index < numColorRTs; ++index)
			{
				BOOST_ASSERT(colorRTs[index]);
				mColorRenderTargets[index].mRenderTarget = colorRTs[index];
				mColorRenderTargets[index].mAction = colorAction;
				mColorRenderTargets[index].mResolveTarget = nullptr;
				mColorRenderTargets[index].mArraySlice = -1;
				mColorRenderTargets[index].mMipIndex = 0;
			}

			mDepthStencilRenderTarget.mDepthStencilTarget = nullptr;
			mDepthStencilRenderTarget.mActions = EDepthStencilTargetActions::DontLoad_DontStore;
			mDepthStencilRenderTarget.mExculusiveDepthStencil = FExclusiveDepthStencil::DepthNop_StencilNop;
			mDepthStencilRenderTarget.mResolveTarget = nullptr;
			if (numColorRTs < MaxSimultaneousRenderTargets)
			{
				Memory::memzero(&mColorRenderTargets[numColorRTs], sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - numColorRTs));
			}
		}

		explicit RHIRenderPassInfo(int32 numColorRTs, RHITexture* colorRTs[], ERenderTargetActions colorAction, RHITexture* resolveTargets[])
		{
			BOOST_ASSERT(numColorRTs > 0);
			for (int32 index = 0; index < numColorRTs; ++index)
			{
				BOOST_ASSERT(colorRTs[index]);
				mColorRenderTargets[index].mRenderTarget = colorRTs[index];
				mColorRenderTargets[index].mResolveTarget = resolveTargets[index];
				mColorRenderTargets[index].mArraySlice = -1;
				mColorRenderTargets[index].mMipIndex = 0;
				mColorRenderTargets[index].mAction = colorAction;
			}

			mDepthStencilRenderTarget.mDepthStencilTarget = nullptr;
			mDepthStencilRenderTarget.mActions = EDepthStencilTargetActions::DontLoad_DontStore;
			mDepthStencilRenderTarget.mExculusiveDepthStencil = FExclusiveDepthStencil::DepthNop_StencilNop;
			mDepthStencilRenderTarget.mResolveTarget = nullptr;
			if (numColorRTs < MaxSimultaneousRenderTargets)
			{
				Memory::memzero(&mColorRenderTargets[numColorRTs], sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - numColorRTs));
			}
		}

		explicit RHIRenderPassInfo(int32 numColorRTs, RHITexture* colorRTs[], ERenderTargetActions colorAction, RHITexture* depthRT, EDepthStencilTargetActions depthActions, FExclusiveDepthStencil inEDS = FExclusiveDepthStencil::DepthWrite_StencilWrite)
		{
			BOOST_ASSERT(numColorRTs > 0);
			for (int32 index = 0; index < numColorRTs; index++)
			{
				BOOST_ASSERT(colorRTs[index]);
				mColorRenderTargets[index].mRenderTarget = colorRTs[index];
				mColorRenderTargets[index].mResolveTarget = nullptr;
				mColorRenderTargets[index].mArraySlice = -1;
				mColorRenderTargets[index].mMipIndex = 0;
				mColorRenderTargets[index].mAction = colorAction;
			}

			BOOST_ASSERT(depthRT);
			mDepthStencilRenderTarget.mDepthStencilTarget = depthRT;
			mDepthStencilRenderTarget.mResolveTarget = nullptr;
			mDepthStencilRenderTarget.mActions = depthActions;
			mDepthStencilRenderTarget.mExculusiveDepthStencil = inEDS;
			bIsMSAA = depthRT->getNumSamples() > 1;
			if (numColorRTs < MaxSimultaneousRenderTargets)
			{
				Memory::memzero(&mColorRenderTargets[numColorRTs], sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - numColorRTs));
			}
		}

		explicit RHIRenderPassInfo(int32 numColorRTs, RHITexture* colorRTs[], ERenderTargetActions colorAction, RHITexture* resolveRTs[], RHITexture* depthRT, EDepthStencilTargetActions depthActions, RHITexture* resolveDepthRT, FExclusiveDepthStencil inEDS = FExclusiveDepthStencil::DepthWrite_StencilWrite)
		{
			BOOST_ASSERT(numColorRTs > 0);
			for (int32 index = 0; index < numColorRTs; index++)
			{
				BOOST_ASSERT(colorRTs[index]);
				mColorRenderTargets[index].mRenderTarget = colorRTs[index];
				mColorRenderTargets[index].mResolveTarget = resolveRTs[index];
				mColorRenderTargets[index].mArraySlice = -1;
				mColorRenderTargets[index].mMipIndex = 0;
				mColorRenderTargets[index].mAction = colorAction;
			}

			BOOST_ASSERT(depthRT);
			mDepthStencilRenderTarget.mDepthStencilTarget = depthRT;
			mDepthStencilRenderTarget.mResolveTarget = resolveDepthRT;
			mDepthStencilRenderTarget.mActions = depthActions;
			mDepthStencilRenderTarget.mExculusiveDepthStencil = inEDS;
			bIsMSAA = depthRT->getNumSamples() > 1;
			if (numColorRTs < MaxSimultaneousRenderTargets)
			{
				Memory::memzero(&mColorRenderTargets[numColorRTs], sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - numColorRTs));
			}
		}


		explicit RHIRenderPassInfo(RHITexture* depthRT, EDepthStencilTargetActions depthActions, RHITexture* resolveDepthRT = nullptr, FExclusiveDepthStencil inEDS = FExclusiveDepthStencil::DepthWrite_StencilWrite)
		{
			
			BOOST_ASSERT(depthRT);
			mDepthStencilRenderTarget.mDepthStencilTarget = depthRT;
			mDepthStencilRenderTarget.mResolveTarget = resolveDepthRT;
			mDepthStencilRenderTarget.mActions = depthActions;
			mDepthStencilRenderTarget.mExculusiveDepthStencil = inEDS;
			bIsMSAA = depthRT->getNumSamples() > 1;
		}

		explicit RHIRenderPassInfo(RHITexture* depthRT, uint32 inNumOcclusionQueries, EDepthStencilTargetActions depthActions, RHITexture* resolveDepthRT, FExclusiveDepthStencil inEDS = FExclusiveDepthStencil::DepthWrite_StencilWrite)
			:mNumOcclusionQueries(inNumOcclusionQueries)
			, bOcclusionQueries(true)
		{

			BOOST_ASSERT(depthRT);
			mDepthStencilRenderTarget.mDepthStencilTarget = depthRT;
			mDepthStencilRenderTarget.mResolveTarget = resolveDepthRT;
			mDepthStencilRenderTarget.mActions = depthActions;
			mDepthStencilRenderTarget.mExculusiveDepthStencil = inEDS;
			bIsMSAA = depthRT->getNumSamples() > 1;
		}

		explicit RHIRenderPassInfo(RHITexture* colorRT, ERenderTargetActions colorAction, RHITexture* depthRT, EDepthStencilTargetActions depthActions, FExclusiveDepthStencil inEDS = FExclusiveDepthStencil::DepthWrite_StencilWrite)
		{
			BOOST_ASSERT(colorRT);
			mColorRenderTargets[0].mRenderTarget = colorRT;
			mColorRenderTargets[0].mResolveTarget = nullptr;
			mColorRenderTargets[0].mArraySlice = -1;
			mColorRenderTargets[0].mMipIndex = 0;
			mColorRenderTargets[0].mAction = colorAction;
			bIsMSAA = colorRT->getNumSamples() > 1;
			BOOST_ASSERT(depthRT);

			mDepthStencilRenderTarget.mDepthStencilTarget = depthRT;
			mDepthStencilRenderTarget.mResolveTarget = nullptr;
			mDepthStencilRenderTarget.mActions = depthActions;
			mDepthStencilRenderTarget.mExculusiveDepthStencil = inEDS;
			Memory::memzero(&mColorRenderTargets[1], sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - 1));
		}

		explicit RHIRenderPassInfo(RHITexture* colorRT, ERenderTargetActions colorAction, RHITexture* resolveColorRT,  RHITexture* depthRT, EDepthStencilTargetActions depthActions, RHITexture* resolveDepthRT, FExclusiveDepthStencil inEDS = FExclusiveDepthStencil::DepthWrite_StencilWrite)
		{
			BOOST_ASSERT(colorRT);
			mColorRenderTargets[0].mRenderTarget = colorRT;
			mColorRenderTargets[0].mResolveTarget = resolveColorRT;
			mColorRenderTargets[0].mArraySlice = -1;
			mColorRenderTargets[0].mMipIndex = 0;
			mColorRenderTargets[0].mAction = colorAction;
			bIsMSAA = colorRT->getNumSamples() > 1;
			BOOST_ASSERT(depthRT);

			mDepthStencilRenderTarget.mDepthStencilTarget = depthRT;
			mDepthStencilRenderTarget.mResolveTarget = resolveDepthRT;
			mDepthStencilRenderTarget.mActions = depthActions;
			mDepthStencilRenderTarget.mExculusiveDepthStencil = inEDS;
			Memory::memzero(&mColorRenderTargets[1], sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - 1));
		}

		explicit RHIRenderPassInfo(int32 inNumUAVs, RHIUnorderedAccessView** inUAVs)
		{
			if (inNumUAVs > MaxSimultaneousUAVs)
			{
				onVerifyNumUAVsFailed(inNumUAVs);
				inNumUAVs = MaxSimultaneousUAVs;
			}

			Memory::memzero(*this);
			mNumUAVs = inNumUAVs;
			for (int32 index = 0; index < inNumUAVs; index++)
			{
				mUAVs[index] = inUAVs[index];
			}
		}

		inline int32 getNumColorRenderTargets() const
		{
			int32 colorIndex = 0;
			for (; colorIndex < MaxSimultaneousRenderTargets; ++colorIndex)
			{
				const ColorEntry& entry = mColorRenderTargets[colorIndex];
				if (!entry.mRenderTarget)
				{
					break;
				}
			}
			return colorIndex;
		}

		explicit RHIRenderPassInfo()
		{
			Memory::memzero(*this);
		}

		inline bool isMSAA() const
		{
			return bIsMSAA;
		}

		RHI_API void validate() const{}

		RHI_API void convertToRenderTargetsInfo(RHISetRenderTargetsInfo& outRTInfo) const;
	private:
		RHI_API void onVerifyNumUAVsFailed(int32 inNumUAVs);
	};

	struct ImmutableSamplerState
	{
		using TImmutableSamplers = TStaticArray<RHISamplerState*, MaxImmutableSamplers>;

		ImmutableSamplerState()
			:mImmutableSamplers(nullptr)
		{}

		void reset()
		{
			for (int32 index = 0; index < MaxImmutableSamplers; ++index)
			{
				mImmutableSamplers[index] = nullptr;
			}
		}

		bool operator == (const ImmutableSamplerState& rhs) const
		{
			return mImmutableSamplers == rhs.mImmutableSamplers;
		}

		bool operator != (const ImmutableSamplerState& rhs) const
		{
			return mImmutableSamplers != rhs.mImmutableSamplers;
		}



		TImmutableSamplers mImmutableSamplers;
	};

	class GraphicsMinimalPipelineStateInitializer
	{
	public:
		using TRenderTargetFormats = TStaticArray<uint8, MaxSimultaneousRenderTargets>;
		using TRenderTargetFlags = TStaticArray<uint32, MaxSimultaneousRenderTargets>;

		GraphicsMinimalPipelineStateInitializer()
			:mBlendState(nullptr)
			, mRasterizerState(nullptr)
			, mDepthStencilState(nullptr)
			, mPrimitiveType(PT_Num)
		{
			static_assert(sizeof(EPixelFormat) != sizeof(uint8), "Change TRenderTargetFormats's uint8 to EPixelFormat");
			static_assert(PF_MAX < std::numeric_limits<uint8>::max(), "TRenderTargetFormats assumes EPixelFormat can fit in a uint8!");
		}

		GraphicsMinimalPipelineStateInitializer(
			BoundShaderStateInput inBoundShaderState,
			RHIBlendState* inBlendState,
			RHIRasterizerState* inRasterizerState,
			RHIDepthStencilState* inDepthStencilState,
			ImmutableSamplerState inImmutableSamplerState,
			EPrimitiveType			inPrimitiveType
		)
			:mBoundShaderState(inBoundShaderState)
			,mBlendState(inBlendState)
			,mRasterizerState(inRasterizerState)
			,mDepthStencilState(inDepthStencilState)
			,mImmutableSamplerState(inImmutableSamplerState)
			,mPrimitiveType(inPrimitiveType)
		{}

		GraphicsMinimalPipelineStateInitializer(const GraphicsMinimalPipelineStateInitializer& inMinimalState)
			:mBoundShaderState(inMinimalState.mBoundShaderState)
			,mBlendState(inMinimalState.mBlendState)
			,mRasterizerState(inMinimalState.mRasterizerState)
			,mDepthStencilState(inMinimalState.mDepthStencilState)
			,mImmutableSamplerState(inMinimalState.mImmutableSamplerState)
			,mPrimitiveType(inMinimalState.mPrimitiveType)
		{}

		inline bool operator == (const GraphicsMinimalPipelineStateInitializer& rhs) const
		{
			if (mBoundShaderState.mVertexDeclarationRHI != rhs.mBoundShaderState.mVertexDeclarationRHI ||
				mBoundShaderState.mVertexShaderRHI != rhs.mBoundShaderState.mVertexShaderRHI ||
				mBoundShaderState.mPixelShaderRHI != rhs.mBoundShaderState.mPixelShaderRHI ||
#if PLATFORM_SUPPORTS_GEOMETRY_SHADERS
				mBoundShaderState.mGeometryShaderRHI != rhs.mBoundShaderState.mGeometryShaderRHI ||
#endif
#if PLATFORM_SUPPORTS_TESSELLATION_SHADERS
				mBoundShaderState.mHullShaderRHI != rhs.mBoundShaderState.mHullShaderRHI ||
				mBoundShaderState.mDomainShaderRHI != rhs.mBoundShaderState.mDomainShaderRHI ||
#endif
				mBlendState != rhs.mBlendState ||
				mRasterizerState != rhs.mRasterizerState ||
				mDepthStencilState != rhs.mDepthStencilState ||
				mImmutableSamplerState != rhs.mImmutableSamplerState ||
				bDepthBounds != rhs.bDepthBounds ||
				bMultiView != rhs.bMultiView ||
				mPrimitiveType != rhs.mPrimitiveType
				)
			{
				return false;
			}
			return true;
		}

		inline bool operator != (const GraphicsMinimalPipelineStateInitializer& rhs) const
		{
			return !(*this == rhs);
		}

		inline friend uint32 getTypeHash(const GraphicsMinimalPipelineStateInitializer& initializer)
		{
			return pointerHash(initializer.mBoundShaderState.mVertexDeclarationRHI, pointerHash(initializer.mBoundShaderState.mVertexShaderRHI, pointerHash(initializer.mBoundShaderState.mPixelShaderRHI, pointerHash(initializer.mRasterizerState))));
		}

#define COMPARE_FIELD_BEGIN(field) \
		if(field != rhs.field) \
		{return field COMPARE_OP rhs.field;}

#define COMPARE_FIELD(field) \
		else if(field != rhs.field)\
		{return field COMPARE_OP rhs.field;}

#define COMPARE_FIELD_END \
		else{return false;}

		bool operator < (const GraphicsMinimalPipelineStateInitializer& rhs) const
		{
#define COMPARE_OP <
			COMPARE_FIELD_BEGIN(mBoundShaderState.mVertexDeclarationRHI)
				COMPARE_FIELD(mBoundShaderState.mVertexShaderRHI)
				COMPARE_FIELD(mBoundShaderState.mPixelShaderRHI)
#if PLATFORM_SUPPORTS_GEOMETRY_SHADERS
				COMPARE_FIELD(mBoundShaderState.mGeometryShaderRHI)
#endif
#if PLATFORM_SUPPORTS_TESSELLATION_SHADERS
				COMPARE_FIELD(mBoundShaderState.mHullShaderRHI)
				COMPARE_FIELD(mBoundShaderState.mDomainShaderRHI)
#endif
				COMPARE_FIELD(mBlendState)
				COMPARE_FIELD(mRasterizerState)
				COMPARE_FIELD(mDepthStencilState)
				COMPARE_FIELD(bDepthBounds)
				COMPARE_FIELD(bMultiView)
				COMPARE_FIELD(mPrimitiveType)
				COMPARE_FIELD_END;
#undef COMPARE_OP
		}


		bool operator > (const GraphicsMinimalPipelineStateInitializer& rhs) const
		{
#define COMPARE_OP >
			COMPARE_FIELD_BEGIN(mBoundShaderState.mVertexDeclarationRHI)
				COMPARE_FIELD(mBoundShaderState.mVertexShaderRHI)
				COMPARE_FIELD(mBoundShaderState.mPixelShaderRHI)
#if PLATFORM_SUPPORTS_GEOMETRY_SHADERS
				COMPARE_FIELD(mBoundShaderState.mGeometryShaderRHI)
#endif
#if PLATFORM_SUPPORTS_TESSELLATION_SHADERS
				COMPARE_FIELD(mBoundShaderState.mHullShaderRHI)
				COMPARE_FIELD(mBoundShaderState.mDomainShaderRHI)
#endif
				COMPARE_FIELD(mBlendState)
				COMPARE_FIELD(mRasterizerState)
				COMPARE_FIELD(mDepthStencilState)
				COMPARE_FIELD(bDepthBounds)
				COMPARE_FIELD(bMultiView)
				COMPARE_FIELD(mPrimitiveType)
				COMPARE_FIELD_END;
#undef COMPARE_OP
		}

#undef COMPARE_FIELD_BEGIN
#undef COMPARE_FIELD
#undef COMPARE_FIELD_END

		BoundShaderStateInput	mBoundShaderState;
		RHIBlendState* mBlendState;
		RHIRasterizerState* mRasterizerState;
		RHIDepthStencilState* mDepthStencilState;
		ImmutableSamplerState mImmutableSamplerState;

		bool bDepthBounds = false;
		bool bMultiView = false;
		uint8 mPadding[2] = {};

		EPrimitiveType mPrimitiveType;
	};


	class GraphicsPipelineStateInitializer final : public GraphicsMinimalPipelineStateInitializer
	{
	public:
		using TRenderTargetFormats		= TStaticArray<uint8, MaxSimultaneousRenderTargets>;
		using TRenderTargetFlags		= TStaticArray<uint32, MaxSimultaneousRenderTargets>;

		GraphicsPipelineStateInitializer()
			:mRenderTargetsEnabled(0)
			, mRenderTargetFormats(PF_Unknown)
			, mRenderTargetFlags(0)
			, mDepthStencilTargetFormat(PF_Unknown)
			, mDepthStencilTargetFlags(0)
			, mDepthTargetLoadAction(ERenderTargetLoadAction::ENoAction)
			, mDepthTargetStoreAction(ERenderTargetStoreAction::ENoAction)
			, mStencilTargetLoadAction(ERenderTargetLoadAction::ENoAction)
			, mStencilTargetStoreAction(ERenderTargetStoreAction::ENoAction)
			, mNumSamples(0)
			, mSubpassHint(ESubpassHint::None)
			, mSubpassIndex(0)
			, mFlags(0)
		{
			static_assert(sizeof(EPixelFormat) != sizeof(uint8), "Change TRenderTargetFormats's uint8 to EPixelFormat");
			static_assert(PF_MAX < std::numeric_limits<uint8>::max(), "TRenderTargetFormats assumes EPixelFormat can fit in a uint8!");
		}

		GraphicsPipelineStateInitializer(const GraphicsMinimalPipelineStateInitializer& inMinimalState)
			:GraphicsMinimalPipelineStateInitializer(inMinimalState)
			,mRenderTargetsEnabled(0)
			,mRenderTargetFormats(PF_Unknown)
			,mRenderTargetFlags(0)
			,mDepthStencilTargetFormat(PF_Unknown)
			,mDepthStencilTargetFlags(0)
			,mDepthTargetLoadAction(ERenderTargetLoadAction::ENoAction)
			,mDepthTargetStoreAction(ERenderTargetStoreAction::ENoAction)
			,mStencilTargetLoadAction(ERenderTargetLoadAction::ENoAction)
			,mStencilTargetStoreAction(ERenderTargetStoreAction::ENoAction)
			,mNumSamples(0)
			,mSubpassHint(ESubpassHint::None)
			,mSubpassIndex(0)
			,mFlags(0)
		{}

		GraphicsPipelineStateInitializer(
			BoundShaderStateInput	inBoundShaderState,
			RHIBlendState* inBlendState,
			RHIRasterizerState* inRasterizerState,
			RHIDepthStencilState* inDepthStencilState,
			ImmutableSamplerState	inImmutableSamplerState,
			EPrimitiveType			inPrimtiveType,
			uint32					inRenderTargetEnabled,
			const TRenderTargetFormats& inRenderTargetFormats,
			const TRenderTargetFlags& inRenderTargetFlags,
			EPixelFormat			inDepthStencilTargetFormat,
			uint32					inDepthStencilTargetFlags,
			ERenderTargetLoadAction	inDepthTargetLoadAction,
			ERenderTargetStoreAction inDepthTargetStoreAction,
			ERenderTargetLoadAction inStencilTargetLoadAction,
			ERenderTargetStoreAction inStencilTargetStoreAction,
			FExclusiveDepthStencil	inDepthStencilAccess,
			uint32					inNumSamples,
			ESubpassHint			inSubpassHint,
			uint8					inSubpassIndex,
			uint16					inFlags
		)
			:GraphicsMinimalPipelineStateInitializer(
				inBoundShaderState, inBlendState, inRasterizerState, inDepthStencilState, inImmutableSamplerState, inPrimtiveType
			)
			,mRenderTargetsEnabled(inRenderTargetEnabled)
			,mRenderTargetFormats(inRenderTargetFormats)
			,mRenderTargetFlags(inRenderTargetFlags)
			,mDepthStencilTargetFormat(inDepthStencilTargetFormat)
			,mDepthStencilTargetFlags(inDepthStencilTargetFlags)
			,mDepthTargetLoadAction(inDepthTargetLoadAction)
			,mDepthTargetStoreAction(inDepthTargetStoreAction)
			,mStencilTargetLoadAction(inStencilTargetLoadAction)
			,mStencilTargetStoreAction(inStencilTargetStoreAction)
			,mDepthStencilAccess(inDepthStencilAccess)
			,mNumSamples(inNumSamples)
			,mSubpassHint(inSubpassHint)
			,mSubpassIndex(inSubpassIndex)
			,mFlags(inFlags)
		{}

		bool operator == (const GraphicsPipelineStateInitializer& rhs) const
		{
			if (!GraphicsMinimalPipelineStateInitializer::operator==(rhs) ||
				mVertexShaderHash != rhs.mVertexShaderHash ||
				mPixelShaderHash != rhs.mPixelShaderHash ||
#if PLATFORM_SUPPORTS_GEOMETRY_SHADERS
				mGeometryShaderHash != rhs.mGeometryShaderHash ||
#endif
#if PLATFORM_SUPPORTS_TESSELLATION_SHADERS
				mHullShaderHash != rhs.mHullShaderHash ||
				mDomainShaderHash != rhs.mDomainShaderHash ||
#endif
				mRenderTargetsEnabled != rhs.mRenderTargetsEnabled ||
				mRenderTargetFormats != rhs.mRenderTargetFormats ||
				mRenderTargetFlags != rhs.mRenderTargetFlags ||
				mDepthStencilTargetFormat != rhs.mDepthStencilTargetFormat ||
				mDepthStencilTargetFlags != rhs.mDepthStencilTargetFlags ||
				mDepthTargetLoadAction != rhs.mDepthTargetLoadAction ||
				mDepthTargetStoreAction != rhs.mDepthTargetStoreAction ||
				mStencilTargetLoadAction != rhs.mStencilTargetLoadAction ||
				mStencilTargetStoreAction != rhs.mStencilTargetStoreAction ||
				mDepthStencilAccess != rhs.mDepthStencilAccess ||
				mNumSamples != rhs.mNumSamples ||
				mSubpassHint != rhs.mSubpassHint ||
				mSubpassIndex != rhs.mSubpassIndex)
			{
				return false;
			}
			return true;
		}

		uint32 computeNumValidRenderTargets() const
		{
			if (mRenderTargetsEnabled > 0)
			{
				int32 lastValidTarget = -1;
				for (int32 i = (int32)mRenderTargetsEnabled - 1; i >= 0; i--)
				{
					if (mRenderTargetFormats[i] != PF_Unknown)
					{
						lastValidTarget = i;
						break;
					}
				}
				return uint32(lastValidTarget + 1);
			}
			return mRenderTargetsEnabled;
		}

		SHAHash							mVertexShaderHash;
		SHAHash							mPixelShaderHash;
#if PLATFORM_SUPPORTS_GEOMETRY_SHADERS
		SHAHash							mGeometryShaderHash;
#endif

#if PLATFORM_SUPPORTS_TESSELLATION_SHADERS
		SHAHash							mHullShaderHash;
		SHAHash							mDomainShaderHash;
#endif
		uint32							mRenderTargetsEnabled;
		TRenderTargetFormats			mRenderTargetFormats;
		TRenderTargetFlags				mRenderTargetFlags;
		EPixelFormat					mDepthStencilTargetFormat;
		uint32							mDepthStencilTargetFlags;
		ERenderTargetLoadAction			mDepthTargetLoadAction;
		ERenderTargetStoreAction		mDepthTargetStoreAction;
		ERenderTargetLoadAction			mStencilTargetLoadAction;
		ERenderTargetStoreAction		mStencilTargetStoreAction;
		FExclusiveDepthStencil			mDepthStencilAccess;
		uint16							mNumSamples;
		ESubpassHint					mSubpassHint;
		uint8							mSubpassIndex;

		union
		{
			struct
			{
				uint16				mReserved : 15;
				uint16				bFromPSOFileCache : 1;
			};
			uint16				mFlags;
		};

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

}