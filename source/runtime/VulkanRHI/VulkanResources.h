#pragma once
#include "RHIResource.h"
#include "VulkanConfiguration.h"
#include "VulkanState.h"
#include "VulkanUtil.h"
#include "BoundShaderStateCache.h"
#include "VulkanMemory.h"
#include "VulkanRHIPrivate.h"
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

	class VulkanResourceMultiBuffer : public DeviceChild
	{
	public:	 
		VulkanResourceMultiBuffer(VulkanDevice* inDevice, VkBufferUsageFlags inBufferUsageFlags, uint32 inSize, uint32 inUsage, RHIResourceCreateInfo& createInfo, class RHICommandListImmediate* inRHICmdList = nullptr);

		void* lock(bool bFromRenderingThread, EResourceLockMode lockMode, uint32 size, uint32 offset);

		void unlock(bool bFromRenderingThread);
	protected:
		uint32 mUsage;
		VkBufferUsageFlags mBufferUsageFlags;
		uint32 mNumBuffers;
		uint32 mDynamicBufferIndex;
		enum
		{
			NUM_BUFFERS = 3,
		};

		TRefCountPtr<BufferSuballocation> mBuffers[NUM_BUFFERS];
		struct  
		{
			BufferSuballocation* mSubAlloc = nullptr;
			BufferAllocation* mBufferAllocation = nullptr;
			VkBuffer mHandle = VK_NULL_HANDLE;
			uint64 mOffset = 0;
		}mCurrent;
		TempFrameAllocationBuffer::TempAllocInfo mVolatileLockInfo;

		static void internalUnlock(VulkanCommandListContext& context, PendingBufferLock& pendingLock, VulkanResourceMultiBuffer* multiBuffer, int32 inDynamicBufferIndex);

		friend class VulkanCommandListContext;
		friend struct RHICommandMultiBufferUnlock;
	};

	class VulkanBuffer : RHIResource
	{
		VulkanBuffer(VulkanDevice& device, uint32 inSize, VkFlags usage, VkMemoryPropertyFlags inMemPropertyFlags, bool bAllowMultiLock, const char* file, int32 line);

		virtual ~VulkanBuffer();

		inline VkBuffer getBufferHandle() const { return mBuffer; }

		inline uint32 getSize() const { return mSize; }

		void* lock(uint32 inSize, uint32 inOffset = 0);

		void unlock();

		inline VkFlags getFlags() const { return mUsage; }
	private:  
		VulkanDevice& mDevice;
		VkBuffer mBuffer;
		DeviceMemoryAllocation* mAllocation;
		uint32 mSize;
		VkFlags mUsage;

		void* mBufferPtr;
		VkMappedMemoryRange mMappedRange;

		bool bAllowMultiLock;

		int32 mLockStack;
	};

	class VulkanStructuredBuffer : public RHIStructuredBuffer, public VulkanResourceMultiBuffer
	{
	public:
		VulkanStructuredBuffer(VulkanDevice* inDevice, uint32 stride, uint32 size, RHIResourceCreateInfo& createInfo, uint32 inUsage);
		~VulkanStructuredBuffer();
	};

	class VulkanVertexBuffer : public RHIVertexBuffer, public VulkanResourceMultiBuffer
	{
	public:
		VulkanVertexBuffer(VulkanDevice* inDevice, uint32 inSize, uint32 inUsage, RHIResourceCreateInfo& createInfo, class RHICommandListImmediate* inRHICmdList);

		void swap(VulkanVertexBuffer& other);
	};

	class VulkanIndexBuffer : public RHIIndexBuffer, public VulkanResourceMultiBuffer
	{
	public:
		VulkanIndexBuffer(VulkanDevice* inDevice, uint32 inStride, uint32 inSize, uint32 inUsage, RHIResourceCreateInfo& createInfo, class RHICommandListImmediate* inRHICommandList);

		inline VkIndexType getIndexType()const
		{
			return mIndexType;
		}

		void swap(VulkanIndexBuffer& other);

	private:
		VkIndexType mIndexType;
	};

	class VulkanConstantBuffer : public RHIConstantBuffer 
	{
	public:
		VulkanConstantBuffer(const RHIConstantBufferLayout& inLayout, const void* contents, EConstantBufferUsage inUsage, EConstantBufferValidation validation);

		const TArray<TRefCountPtr<RHIResource>>& getResourceTable() const { return mResourceTable; }

		void updateResourceTable(const RHIConstantBufferLayout& inLayout, const void* contents, int32 resourceNum);

		void updateResourceTable(RHIResource** resources, int32 resourceNum);
	protected:
		TArray<TRefCountPtr<RHIResource>> mResourceTable;
	};

	class VulkanRealConstantBuffer : public VulkanConstantBuffer {

	public:
		VulkanRealConstantBuffer(VulkanDevice& device, const RHIConstantBufferLayout& inLayout, const void* contents, EConstantBufferUsage inUsage, EConstantBufferValidation validation);

		virtual ~VulkanRealConstantBuffer();

		BufferAllocation* getBufferAllocation() const
		{
			BOOST_ASSERT(mCBAllocation);
			return mCBAllocation->getBufferAllocation();
		}

		inline uint32 getOffset() const
		{
			return mCBAllocation->getOffset();
		}

		inline BufferSuballocation* updateCBAllocation(BufferSuballocation* newAlloc)
		{
			BOOST_ASSERT(mCBAllocation);
			BufferSuballocation* prevBufferSuballoc = mCBAllocation;
			mCBAllocation = newAlloc;
			return prevBufferSuballoc;
		}


		VulkanDevice* mDevice;
		BufferSuballocation* mCBAllocation = nullptr;
	};

	class VulkanEmulatedConstantBuffer : public VulkanConstantBuffer
	{
	public:
		VulkanEmulatedConstantBuffer(const RHIConstantBufferLayout& inLayout, const void* contents, EConstantBufferUsage inUsage, EConstantBufferValidation validation);

		void udpateConstantData(const void* contents, int32 contentsSize);

		TArray<uint8> mConstantData;


	};

	struct VulkanBufferView : public RHIResource, public DeviceChild
	{
		VulkanBufferView(VulkanDevice* inDevice)
			:DeviceChild(inDevice)
			,mView(VK_NULL_HANDLE)
			,mViewId(0)
			,mFlags(0)
			,mOffset(0)
			,mSize(0)
		{
			
		}

		~VulkanBufferView()
		{
			destroy();
		}

		void create(VulkanBuffer& buffer, EPixelFormat format, uint32 inOffset, uint32 inSize);

		void create(VulkanResourceMultiBuffer* buffer, EPixelFormat format, uint32 inOffset, uint32 inSize);

		void create(VkFormat format, VulkanResourceMultiBuffer* buffer, uint32 inOffset, uint32 inSize);

		void destroy();

		VkBufferView mView;
		uint32 mViewId;
		VkFlags mFlags;
		uint32 mOffset;
		uint32 mSize;
	};


	class VulkanShaderResourceView : public RHIShaderResourceView, public DeviceChild
	{
	public:
		VulkanShaderResourceView(VulkanDevice* device, RHIResource* inRHIBuffer, VulkanResourceMultiBuffer* inSourceBuffer, uint32 inSize, EPixelFormat inFormat);

		VulkanShaderResourceView(VulkanDevice* device, RHITexture* inSourceTexture, const RHITextureSRVCreateInfo& inCreateInfo)
			:DeviceChild(device)
			,mBufferViewFormat((EPixelFormat)inCreateInfo.mFormat)
			,mSRGBOverride(inCreateInfo.mSRGBOverride)
			,mSourceTexture(inSourceTexture)
			, mSourceStructuredBuffer(nullptr)
			, mMipLevel(inCreateInfo.mMipLevel)
			,mNumMips(inCreateInfo.mNumMipLevels)
			,mFirstArraySlice(inCreateInfo.mFirstArraySize)
			,mNumArraySlices(inCreateInfo.mNumArraySlices)
			,mSize(0)
			,mSourceBuffer(nullptr)
		{

		}

		VulkanShaderResourceView(VulkanDevice* device, VulkanStructuredBuffer* inStructuredBuffer)
			:DeviceChild(device)
			,mBufferViewFormat(PF_Unknown)
			,mSourceTexture(nullptr)
			,mSourceStructuredBuffer(inStructuredBuffer)
			,mNumMips(0)
			,mSize(inStructuredBuffer->getSize())
			,mSourceBuffer(nullptr)
		{

		}


		EPixelFormat mBufferViewFormat;
		ERHITextureSRVOverrideSRGBType mSRGBOverride = SRGBO_Default;

		TRefCountPtr<RHITexture> mSourceTexture;
		VulkanTextureView mTextureView;
		VulkanStructuredBuffer* mSourceStructuredBuffer;
		uint32 mMipLevel = 0;
		uint32 mNumMips = std::numeric_limits<uint32>::max();
		uint32 mFirstArraySlice = 0;
		uint32 mNumArraySlices = 0;

		~VulkanShaderResourceView();

		TArray<TRefCountPtr<VulkanBufferView>> mBufferViews;

		uint32 mBufferIndex = 0; 
		uint32 mSize;

		VulkanResourceMultiBuffer* mSourceBuffer;

		TRefCountPtr<RHIResource> mSourceRHIBuffer;

	protected:
		VkBuffer mVolatileBufferHandle = VK_NULL_HANDLE;

		uint32 mVolatileLockCounter = std::numeric_limits<uint32>::max();
	};

	class VulkanUnorderedAccessView : public RHIUnorderedAccessView, public DeviceChild
	{
	public:

		VulkanUnorderedAccessView(VulkanDevice* device)
			:DeviceChild(device)
			,mMipLevel(0)
			,mBufferViewFormat(PF_Unknown)
			,mVolatileLockCounter(std::numeric_limits<uint32>::max())
		{}
		~VulkanUnorderedAccessView();

		void updateView();

		TRefCountPtr<VulkanStructuredBuffer> mSourceStructuredBuffer;

		TRefCountPtr<RHITexture> mSourceTexture;

		VulkanTextureView mTextureView;

		uint32 mMipLevel;

		TRefCountPtr<VulkanVertexBuffer> mSourceVertexBuffer;
		TRefCountPtr<VulkanIndexBuffer> mSourceIndexBuffer;
		TRefCountPtr<VulkanBufferView> mBufferView;

		EPixelFormat mBufferViewFormat;

	protected:

		uint32 mVolatileLockCounter;
	};

	class VulkanShader : public IRefCountedObject
	{
	protected:
#if VULKAN_ENABLE_SHADER_DEBUG_NAMES
		wstring mDebugEntryPoint;
#endif
		uint64 mShaderKey;

		VulkanShaderHeader	mCodeHeader;

		TMap<uint32, VkShaderModule> mShaderModules;

		const VkShaderStageFlagBits		mStageFlag;

		EShaderFrequency				mFrequency;

		TArray<uint32>					mSpirv;

		VulkanDevice* mDevice;

		VkShaderModule createHandle(const VulkanLayout)
	};


	template<typename BaseResourceType, EShaderFrequency ShaderType, VkShaderStageFlagBits StageFlagBits>
	class TVulkanBaseShader : public BaseResourceType, public VulkanShader


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

	template<>
	struct TVulkanResourceTraits<RHIStructuredBuffer>
	{
		typedef VulkanStructuredBuffer TConcreteType;
	};

	template<>
	struct TVulkanResourceTraits<RHIUnorderedAccessView>
	{
		typedef VulkanUnorderedAccessView TConcreteType;
	};

	template<>
	struct TVulkanResourceTraits<RHIVertexBuffer>
	{
		typedef VulkanVertexBuffer TConcreteType;
	};

	template<>
	struct TVulkanResourceTraits<RHIIndexBuffer>
	{
		typedef VulkanIndexBuffer TConcreteType;
	};

	template<>
	struct TVulkanResourceTraits<RHIConstantBuffer>
	{
		typedef VulkanConstantBuffer TConcreteType;
	};
}