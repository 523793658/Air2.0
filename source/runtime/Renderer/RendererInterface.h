#pragma once
#include "RendererMininal.h"
#include "Modules/ModuleInterface.h"
#include "RHI.h"
#include "Template/RefCounting.h"
#include "SceneView.h"
#include "PixelFormat.h"
#include "HAL/PlatformProperties.h"
#include "Containers/Set.h"
namespace Air
{
	class Canvas;
	class SceneViewFamily;
	class SceneViewStateInterface;
	class World;
	class SceneInterface;

	class SceneRenderingAllocator : public TMemStackAllocator<>
	{};

	class IRendererModule : public IModuleInterface
	{
	public:
		virtual void beginRenderingViewFamily(Canvas* canvas, SceneViewFamily* viewFamily) = 0;

		virtual SceneViewStateInterface* allocateViewState() = 0;
	
		virtual SceneInterface* allocateScene(World* world, bool bInRequiresHitProxies, bool bCreateFXSystem, ERHIFeatureLevel::Type inFeatureLevel) = 0;

		virtual void tickRenderTargetPool() = 0;

		virtual const TSet<SceneInterface*>& getAllocatedScenes() = 0;
	};

	class ICustomVisibilityQuery : public IRefCountedObject
	{
	public:
		virtual bool prepare() = 0;

		virtual bool isVisible(int32 visibilityId, const BoxSphereBounds& bounds) = 0;

		virtual bool isThreadSafe()
		{
			return false;
		}
	};

	class ICustomCulling
	{
	public:
		virtual ICustomVisibilityQuery * createQuery(const SceneView& view) = 0;
	};

	struct PooledRenderTargetDesc
	{
	public:
		static PooledRenderTargetDesc create2DDesc(int2 inExtent, EPixelFormat inFormat, const ClearValueBinding& inClearValue, uint32 inFlags, uint32 inTargetableFlags, bool bInForceSeparateTargetAndShaderResource, uint16 inNumMips = 1, bool inAutoWritable = true, bool inCreateRTWriteMask = false)
		{
			PooledRenderTargetDesc newDesc;
			newDesc.mClearValue = inClearValue;
			newDesc.mExtent = inExtent;
			newDesc.mDepth = 0;
			newDesc.mArraySize = 1;
			newDesc.bIsArray = false;
			newDesc.bIsCubemap = false;
			newDesc.mNumMips = inNumMips;
			newDesc.mNumSamples = 1;
			newDesc.mFormat = inFormat;
			newDesc.mFlags = inFlags;
			newDesc.mTargetableFlags = inTargetableFlags;
			newDesc.bForceSeparateTargetAndShaderResource = bInForceSeparateTargetAndShaderResource;
			newDesc.mDebugName = TEXT("UnKnown");
			newDesc.bAutoWritable = inAutoWritable;
			newDesc.bCreateRenderTargetWriteMask = inCreateRTWriteMask;
			BOOST_ASSERT(newDesc.is2DTexture());
			return newDesc;
		}

		bool is2DTexture() const
		{
			return mExtent.x != 0 && mExtent.y != 0 && mDepth == 0 && !bIsCubemap;
		}
		bool is3DTexture() const
		{
			return mExtent.x != 0 && mExtent.y != 0 && mDepth != 0 && !bIsCubemap;
		}

		bool isArray() const
		{
			return bIsArray;
		}
		bool isValid() const
		{
			if (mNumSamples != 1)
			{
				if (mNumSamples < 1 || mNumSamples >8)
				{
					return false;
				}
				if (!is2DTexture())
				{
					return false;
				}
			}
			return mExtent.x != 0 && mNumMips != 0 && mNumSamples >= 1 && mNumSamples <= 16 && mFormat != PF_Unknown && ((mTargetableFlags & TexCreate_UAV) == 0 || GMaxRHIFeatureLevel == ERHIFeatureLevel::SM5);
		}

		bool compare(const PooledRenderTargetDesc& rhs, bool bExact) const
		{
			auto lhsFlags = mFlags;
			auto rhsFlags = rhs.mFlags;
			if (!bExact || !PlatformProperties::supportsFastVRAMMemory())
			{
				lhsFlags &= (~TexCreate_FastVRAM);
				rhsFlags &= (~TexCreate_FastVRAM);
			}
			return mExtent == rhs.mExtent
				&& mDepth == rhs.mDepth
				&& bIsArray == rhs.bIsArray
				&& bIsCubemap == rhs.bIsCubemap
				&& mArraySize == rhs.mArraySize
				&& mNumMips == rhs.mNumMips
				&& mNumSamples == rhs.mNumSamples
				&& mFormat == rhs.mFormat
				&& lhsFlags == rhsFlags
				&& mTargetableFlags == rhs.mTargetableFlags
				&& bForceSeparateTargetAndShaderResource == rhs.bForceSeparateTargetAndShaderResource
				&& mClearValue == rhs.mClearValue
				&& bAutoWritable == rhs.bAutoWritable;
		}

		void reset()
		{
			mNumSamples = 1;
			bForceSeparateTargetAndShaderResource = false;
			bAutoWritable = true;
			mTargetableFlags &= (~TexCreate_UAV);
		}

		ClearValueBinding	mClearValue;
		int2				mExtent;
		uint32				mDepth;
		uint32				mArraySize;
		bool				bIsArray;
		bool				bIsCubemap;
		uint16				mNumMips;
		uint16				mNumSamples;
		EPixelFormat		mFormat;
		uint32				mFlags;
		uint32				mTargetableFlags;
		bool				bForceSeparateTargetAndShaderResource;
		const TCHAR*		mDebugName;
		bool				bAutoWritable;
		bool				bCreateRenderTargetWriteMask;
	};

	struct SceneRenderTargetItem
	{
		void safeRelease()
		{
			mTargetableTexture.safeRelease();
			mShaderResourceTexture.safeRelease();
			for (int32 i = 0; i < MipSRVs.size(); ++i)
			{
				MipSRVs[i].safeRelease();
			}
		}
		bool isValid() const
		{
			return mTargetableTexture != 0 || mShaderResourceTexture != 0;
		}

		TextureRHIRef mTargetableTexture;

		TextureRHIRef mShaderResourceTexture;

		TArray<ShaderResourceViewRHIRef> MipSRVs;
	};

	struct IPooledRenderTarget : public IRefCountedObject 
	{
		virtual bool isFree() const = 0;
		virtual const PooledRenderTargetDesc& getDesc() const = 0;

		virtual void setDebugName(const TCHAR* inName) = 0;

		virtual uint32 computeMemorySize() const = 0;

		inline SceneRenderTargetItem& getRenderTargetItem() 
		{
			return mRenderTargetItem;
		}

		inline const SceneRenderTargetItem& getRenderTargetItem() const
		{
			return mRenderTargetItem;
		}

	protected:
		SceneRenderTargetItem mRenderTargetItem;
	};

	struct FilterVertex
	{
		float4 mPosition;
		float2 mUV;
	};

	class FilterVertexDeclaration : public RenderResource
	{
	public:
		VertexDeclarationRHIRef mVertexDeclarationRHI;
		virtual ~FilterVertexDeclaration() {}
		virtual void initRHI()
		{
			VertexDeclarationElementList elements;
			uint32 stride = sizeof(FilterVertex);
			elements.add(VertexElement(0, STRUCT_OFFSET(FilterVertex, mPosition), VET_Float4, 0, stride));
			elements.add(VertexElement(0, STRUCT_OFFSET(FilterVertex, mUV), VET_Float2, 1, stride));
			mVertexDeclarationRHI = RHICreateVertexDeclaration(elements);
		}

		virtual void releaseRHI()
		{
			mVertexDeclarationRHI.safeRelease();
		}
	};
}