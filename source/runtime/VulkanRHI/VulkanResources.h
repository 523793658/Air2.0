#pragma once
#include "RHIResource.h"
#include "VulkanConfiguration.h"
#include "VulkanState.h"
#include "VulkanUtil.h"
#include "BoundShaderStateCache.h"
#include "VulkanContext.h"
#include "VulkanMemory.h"
namespace Air
{
	struct SamplerYcbcrConversionInitializer
	{
		VkFormat Format;
		uint64 ExtenalFormat;
		VkComponentMapping Components;
		VkSamplerYcbcrModelConversion Model;
		VkSamplerYcbcrRange Range;
		VkChromaLocation XOffset;
		VkChromaLocation YOffset;
	};

	template<typename T>
	struct TVulkanResourceTraits
	{

	};

	class VulkanVertexDeclaration : public RHIVertexDeclaration
	{
	public:
		VertexDeclarationElementList mElements;

		VulkanVertexDeclaration(const VertexDeclarationElementList& inElements);

		virtual bool getInitializer(VertexDeclarationElementList& out) final override
		{
			out = mElements;
			return true;
		}

		static void emptyCache();
	};

	struct VulkanTextureView
	{
		VulkanTextureView()
			:mView(VK_NULL_HANDLE)
			,mImage(VK_NULL_HANDLE)
			,mViewId(0)
		{}

		void create(VulkanDevice& device, VkImage inImage, VkImageViewType viewType, VkImageAspectFlags aspectFlags, EPixelFormat AirFormat, VkFormat format, uint32 firstMip, uint32 numMips, uint32 arraySliceIndex, uint32 numArraySlices, bool bUseIdentitySwizzle = false);

		void create(VulkanDevice& device, VkImage inImage, VkImageViewType viewType, VkImageAspectFlags aspectFlags, EPixelFormat AirFormat, VkFormat format, uint32 firstMip, uint32 numMips, uint32 arraySliceIndex, uint32 numArraySlices, SamplerYcbcrConversionInitializer& conversionInitializer, bool bUseIdentitySwizzle = false);

		void destroy(VulkanDevice& device);

		VkImageView mView;
		VkImage mImage;
		uint32 mViewId;
	};

	class VulkanBaseShaderResource : public IRefCountedObject
	{};

	class VulkanSurface
	{
	public:

		VkImage mImage;
	};

	struct VulkanTextureBase : public VulkanBaseShaderResource
	{
		inline static VulkanTextureBase* cast(RHITexture* texture)
		{
			BOOST_ASSERT(texture);
			VulkanTextureBase* outTexture = (VulkanTextureBase*)texture->getTextureBaseRHI();
			BOOST_ASSERT(outTexture);
			return outTexture;
		}

		VulkanTextureBase(VulkanDevice& device, VkImageViewType resourceType, EPixelFormat format, uint32 sizeX, uint32 sizeY, uint32 sizeZ, uint32 arraySize, uint32 numMips, uint32 numSampler, uint32 AirFlags, const RHIResourceCreateInfo& createInfo);

		VulkanTextureBase(VulkanDevice& device, VkImageViewType resourceType, EPixelFormat format, uint32 sizeX, uint32 sizeY, uint32 sizeZ, uint32 arraySize, uint32 numMips, uint32 numSampler, VkImage inImage, VkDeviceMemory inMem, uint32 AirFlags, const RHIResourceCreateInfo& createInfo = RHIResourceCreateInfo());
		VulkanTextureBase(VulkanDevice& device, VkImageViewType resourceType, EPixelFormat format, uint32 sizeX, uint32 sizeY, uint32 sizeZ, uint32 arraySize, uint32 numMips, uint32 numSampler, VkImage inImage, VkDeviceMemory inMem, SamplerYcbcrConversionInitializer& conversionInitializer,  uint32 AirFlags, const RHIResourceCreateInfo& createInfo = RHIResourceCreateInfo());
		
		virtual ~VulkanTextureBase();

		void AliasTextureResource(const VulkanTextureBase* srcTexture);

		VulkanSurface mSurface;

		VulkanTextureView mDefaultView;

		VulkanTextureView* partialView;

		bool bIsAliased;

		virtual void OnTransitionResource(VulkanCommandListContext& context, EResourceTransitionAccess transitionType){}
	
	private:
		void destroyViews();
	};

	class VulkanTexture2D : public RHITexture2D, public VulkanTextureBase
	{
	public:
		VulkanTexture2D(VulkanDevice& device, EPixelFormat format, uint32 sizeX, uint32 sizeY, uint32 numMips, uint32 numSamples, uint32 AirFlags, const RHIResourceCreateInfo& createInfo);
		VulkanTexture2D(VulkanDevice& device, EPixelFormat format, uint32 sizeX, uint32 sizeY, uint32 numMips, uint32 numSamples, VkImage image, uint32 AirFlags, const RHIResourceCreateInfo& createInfo);
		VulkanTexture2D(VulkanDevice& device, EPixelFormat format, uint32 sizeX, uint32 sizeY, uint32 numMips, uint32 numSamples, VkImage image, struct SamplerYchcrConversionInitializer& conversionIntializer, uint32 AirFlags, const RHIResourceCreateInfo& createInfo);

		virtual ~VulkanTexture2D();

		virtual uint32 AddRef() const override final
		{
			return RHIResource::AddRef();
		}

		virtual uint32 Release() const override final
		{
			return RHIResource::Release();
		}

		virtual uint32 GetRefCount() const override final
		{
			return RHIResource::GetRefCount();
		}

		virtual void* getTextureBaseRHI() override final
		{
			VulkanTextureBase* base = static_cast<VulkanTextureBase*>(this);
			return base;
		}

		virtual void* getNativeResource() const
		{
			return (void*)mSurface.mImage;
		}
	};

	class VulkanTextureCube : public RHITextureCube, public VulkanTextureBase
	{
	public:	 
		VulkanTextureCube(VulkanDevice& device, EPixelFormat format, uint32 size, bool bArray, uint32 arraySize, uint32 numMips, uint32 flags, ResourceBulkDataInterface* bulkData, const ClearValueBinding& inClearValue);
		VulkanTextureCube(VulkanDevice& device, EPixelFormat format, uint32 size, bool bArray, uint32 arraySize, uint32 numMips, VkImage image, uint32 flags, ResourceBulkDataInterface* bulkData, const ClearValueBinding& inClearValue);
		virtual ~VulkanTextureCube();

		virtual uint32 AddRef() const override final
		{
			return RHIResource::AddRef();
		}

		virtual uint32 Release() const override final
		{
			return RHIResource::Release();
		}

		virtual uint32 GetRefCount() const override final
		{
			return RHIResource::GetRefCount();
		}

		virtual void* getTextureBaseRHI() override final
		{
			return (VulkanTextureBase*)this;
		}

		virtual void* getNativeResource() const
		{
			return (void*)mSurface.mImage;
		}

	};

	class VulkanQueryPool : public DeviceChild
	{
	public:
		VulkanQueryPool(VulkanDevice* inDevice, uint32 inMaxQueries, VkQueryType inQueryType);

		virtual ~VulkanQueryPool();

		inline uint32 getMaxQueries() const
		{
			return mMaxQueries;
		}

		inline VkQueryPool getHandle() const
		{
			return mQueryPool;
		}

		inline uint64 getResultValue(uint32 index) const
		{
			return mQueryOutput[index];
		}

	protected:
		VkQueryPool mQueryPool;
		VkEvent mResetEvent;
		const uint32 mMaxQueries;
		const VkQueryType mQueryType;
		TArray<uint64> mQueryOutput;
	};

	class VulkanTimingQueryPool : public VulkanQueryPool
	{
	public:
		VulkanTimingQueryPool(VulkanDevice* inDevice, uint32 inBufferSize)
			:VulkanQueryPool(inDevice, inBufferSize * 2, VK_QUERY_TYPE_TIMESTAMP)
			,mBufferSie(inBufferSize)
		{
			mTimestampListHandles.addZeroed(inBufferSize * 2);
		}

		uint32 mCurrentTimestamp = 0;

		uint32 mNumIssumedTimestamps = 0;

		const uint32 mBufferSie;

		struct CmdBufferFence
		{
			VulkanCmdBuffer* mCmdBuffer;
			uint64 mFenceCounter;
		};

		TArray<CmdBufferFence> mTimestampListHandles;

		StagingBuffer* mResultsBuffer = nullptr;
	};

	template<typename TRHIType> 
	static FORCEINLINE typename TVulkanResourceTraits<TRHIType>::TConcreteType* resourceCast(TRHIType* resource)
	{
		return static_cast<typename TVulkanResourceTraits<TRHIType>::TConcreteType*>(resource);
	}

	template<typename TRHIType>
	static FORCEINLINE typename TVulkanResourceTraits<TRHIType>::TConcreteType* resourceCast(const TRHIType* resource)
	{
		return static_cast<const typename TVulkanResourceTraits<TRHIType>::TConcreteType*>(resource);
	}
}