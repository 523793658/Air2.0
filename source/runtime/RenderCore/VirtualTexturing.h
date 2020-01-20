#pragma once
#include "CoreMinimal.h"
#include "RHICommandList.h"
namespace Air
{

	union VirtualTextureProducerHandle
	{
		VirtualTextureProducerHandle(): mPackedValue(0u){}

		explicit VirtualTextureProducerHandle(uint32 inPackedValue) : mPackedValue(inPackedValue){}

		VirtualTextureProducerHandle(uint32 inIndex, uint32 inMagic)
			:mIndex(inIndex)
			,mMagic(inMagic)
		{}


		uint32 mPackedValue;
		struct  
		{
			uint32 mIndex : 22;
			uint32 mMagic : 10;
		};
	};

	enum class EVTRequestPageStatus
	{
		Invalid,
		Saturated,
		Pending,
		Available
	};

	FORCEINLINE bool VTRequestPageStatus_HasData(EVTRequestPageStatus inStatus) { return inStatus == EVTRequestPageStatus::Pending || inStatus == EVTRequestPageStatus::Available; }

	enum class EVTRequestPagePriority
	{
		Normal,
		High,
	};

	enum class EVTProducePageFlags : uint8
	{
		None = 0u,
		SkipPageBorders = (1u << 0),
	};

	struct VTRequestPageResult
	{
		VTRequestPageResult(EVTRequestPageStatus inStatus = EVTRequestPageStatus::Invalid, uint64 inHandle = 0u)
			:mHandle(inHandle)
			,mStatus(inStatus)
		{}
		uint64 mHandle;

		EVTRequestPageStatus mStatus;
	};

	class IVirtualTextureFinalizer
	{
	public:
		virtual void finalize(RHICommandListImmediate& RHICmdList) = 0;
	};

	struct VTProduceTargetLayer
	{
		RHITexture* mTextureRHI = nullptr;

		TRefCountPtr<struct IPooledRenderTarget> mPooledRenderTarget;

		int3 pPageLocation;
	};


	class IVertualTexture
	{
	public:
		inline IVertualTexture() {}

		virtual ~IVertualTexture() {}

		virtual uint32 getLocalMipBias(uint8 vLevel, uint32 vAddress)const
		{
			return 0u;
		}

		virtual VTRequestPageResult requestPageData(const VirtualTextureProducerHandle& producerHandle, uint8 layerMask, uint8 vLevel, uint32 vAddress, EVTRequestPagePriority priority) = 0;

		virtual IVirtualTextureFinalizer* producePageData(RHICommandListImmediate& RHICmdList, ERHIFeatureLevel::Type featureLevel, EVTProducePageFlags flags, const VirtualTextureProducerHandle& producerHandle, uint8 layerMask, uint8 vLevel, uint32 vAddress, uint64 requestHandle, const VTProduceTargetLayer* targetLayers) = 0;

		virtual void dumpToConsole(bool verbose) {	};

	};

	enum class EVTPageTableFormat : uint8
	{
		UInt16,
		UInt32,
	};

	class IAllocatedVirtualTexture
	{
	protected:
		//friend class VirtualTexture
	};
}