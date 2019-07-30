#include "Texture2D.h"
#include "RenderUtils.h"
#include "RenderingThread.h"
#include "SimpleReflection.h"
#include "ResImporter/TextureImporter.h"
namespace Air
{


	RTexture2D::RTexture2D(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{

	}

	float RTexture2D::getGlobalMipMapLODBias()
	{
		return 0.0f;
	}

	TextureResource* RTexture2D::createResource()
	{
		int32 numMips = getNumMips();
		bIsStreamable = false;
		const EPixelFormat pixelFormat = getPixelFormat();
		const bool bIncompactibleTexture = (numMips == 0);
		const bool bTextureTooLarge = Math::max(getWidth(), getHeight()) > (int32)getMax2DTextureDemension();
		const bool bNotSupportedByRHI = numMips == 1 && bTextureTooLarge;
		const bool bFormatNotSupported = !(GPixelFormats[pixelFormat].Supported);
		if (bIncompactibleTexture || bNotSupportedByRHI || bFormatNotSupported)
		{
			mResidentMips = 0;
			mRequestedMips = 0;

			if (bFormatNotSupported)
			{
				AIR_LOG(LogTexture, Error, TEXT("is not supported."));
			}
			BOOST_ASSERT(false);
		}
		else
		{
			int32 numNonStreamingMips = numMips;
			if (bIsStreamable)
			{

			}
			else
			{
				mRequestedMips = GMaxTextureMipCount;
			}
			int32 minAllowedMips = numNonStreamingMips;
			int32 maxAllowedMips = numMips;

			calcAllowedMips(numMips, numNonStreamingMips, 0, minAllowedMips, maxAllowedMips);
			mRequestedMips = Math::min(maxAllowedMips, mRequestedMips);
			mRequestedMips = Math::max(minAllowedMips, mRequestedMips);

			if (mResourceMem)
			{
				mRequestedMips = Math::max(mRequestedMips, mResourceMem->getNumMips());
			}

			mRequestedMips = Math::max(mRequestedMips, 1);
			mResidentMips = mRequestedMips;
		}
		Texture2DResource* texture2DResource = nullptr;
		if (mRequestedMips > 0)
		{
			texture2DResource = new Texture2DResource(this, mRequestedMips);

			mResourceMem = nullptr;
		}
		else
		{
			bIsStreamable = false;
		}

		return texture2DResource;
	}

	void RTexture2D::getMipData(int32 firstMipToLoad, void** outMipData)
	{
		if (mTextureData->tryLoadMips(firstMipToLoad, outMipData) == false)
		{
			AIR_LOG(LogTexture, Warning, TEXT("GetMipData failed"));
		}
	}

	void RTexture2D::calcAllowedMips(int32 mipCount, int32 numNonStreamingMips, int32 LODBias, int32& outMinAllowedMips, int32& outMaxAllowedMips)
	{
		int32 minAllowedMips = 10;
		outMinAllowedMips = Math::max(1, outMinAllowedMips);

		outMaxAllowedMips = Math::min(12, outMaxAllowedMips);
	}

	Texture2DResource::Texture2DResource(class RTexture2D* inOwner, int32 initialMipCount)
		:mOwner(inOwner)
		, mResourceMem(inOwner->mResourceMem)
		,mAsyncCreateTextureTask(nullptr)
		
	{
		bIgnoreGammaConversions = !mOwner->bSRGB;
		bSRGB = inOwner->bSRGB;
		if (mOwner->mPendingMipChangeRequestStatus.getValue() == TexState_ReadyFor_Requests)
		{
			mOwner->mPendingMipChangeRequestStatus.decrement();
		}
		else
		{
			BOOST_ASSERT(mOwner->mPendingMipChangeRequestStatus.getValue() == TexState_InProgress_Initialization);
		}


		BOOST_ASSERT(initialMipCount > 0);
		BOOST_ASSERT(ARRAY_COUNT(mMipData) >= GMaxTextureMipCount);
		BOOST_ASSERT(initialMipCount == mOwner->mResidentMips);
		BOOST_ASSERT(initialMipCount == mOwner->mRequestedMips);

		mPendingFirstMip = mCurrentFirstMap = inOwner->getNumMips() - initialMipCount;
		BOOST_ASSERT(mCurrentFirstMap >= 0);
		BOOST_ASSERT(mCurrentFirstMap <= mOwner->getMipTailBaseIndex());
		BOOST_ASSERT(inOwner->getNumMips() == mCurrentFirstMap + mOwner->mRequestedMips);

		Memory::memzero(mMipData, sizeof(mMipData));
		inOwner->getMipData(mCurrentFirstMap, &mMipData[mCurrentFirstMap]);

	}

	Texture2DResource::~Texture2DResource()
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			DeleteResourceMem,
			Texture2DResourceMem*, resourceMem, mResourceMem,
			{
				delete resourceMem;
			}
		);

		for (int32 mipIndex = 0; mipIndex < ARRAY_COUNT(mMipData); mipIndex++)
		{
			if (mMipData[mipIndex])
			{
				Memory::free(mMipData[mipIndex]);
			}
			mMipData[mipIndex] = nullptr;
		}
	}

	void Texture2DResource::createSamplerState(float mipMapBias)
	{
		SamplerStateInitializerRHI samplerStateInitializer(
			mOwner->mFilter,
			mOwner->mAddressX,
			mOwner->mAddressY,
			AM_Wrap,
			mipMapBias
		);
		mSamplerStateRHI = RHICreateSamplerState(samplerStateInitializer);

		SamplerStateInitializerRHI deferredPassSamplerStateInitializer(
			mOwner->mFilter,
			mOwner->mAddressX,
			mOwner->mAddressY,
			AM_Wrap,
			mipMapBias,
			1, 0, 2
		);
		mDeferredPassSamplerStateRHI = RHICreateSamplerState(deferredPassSamplerStateInitializer);
	}

	void Texture2DResource::initRHI()
	{
		EPixelFormat effectiveFormat = mOwner->getPixelFormat();
		auto& texData = mOwner->mTextureData;
		uint32 texCreateFlags = (mOwner->bSRGB ? TexCreate_SRGB : 0) | TexCreate_OfflineProcessed;
		if (mOwner->getMipTailBaseIndex() == -1)
		{
			texCreateFlags |= TexCreate_NoMipTail;
		}
		if (mOwner->bNoTiling)
		{
			texCreateFlags |= TexCreate_NoTiling;
		}

		createSamplerState(RTexture2D::getGlobalMipMapLODBias() + getDefaultMipMapBias());
		bGreyScaleFormat = (effectiveFormat == PF_G8) || (effectiveFormat == PF_BC4);
		if (mOwner->mPendingMipChangeRequestStatus.getValue() == TexState_InProgress_Initialization)
		{
			bool bSkipRHITextureCreation = false;
			if (GIsEditor || (!bSkipRHITextureCreation))
			{
				RHIResourceCreateInfo createInfo(mResourceMem);
				mTexture2DRHI = RHICreateTexture2D(texData->mInfo.mWidth, texData->mInfo.mHeight, effectiveFormat, mOwner->mRequestedMips, 1, texCreateFlags, createInfo);
				mTextureRHI = mTexture2DRHI;
				mTextureRHI->setName(mOwner->getName());
				RHIBindDebugLabelName(mTextureRHI, mOwner->getName().c_str());
				RHIUpdateTextureReference(mOwner->mTextureReference.mTextureReferenceRHI, mTextureRHI);
				BOOST_ASSERT(mOwner->mResidentMips == mTexture2DRHI->getNumMips());
				if (mResourceMem)
				{
					BOOST_ASSERT(mOwner->mRequestedMips == mResourceMem->getNumMips());
					BOOST_ASSERT(mOwner->mTextureData->mInfo.mWidth == mResourceMem->getWidth() && mOwner->mTextureData->mInfo.mHeight == mResourceMem->getHeight());
					for (int32 mipIndex = 0; mipIndex < mOwner->mTextureData->mInitData.size(); mipIndex++)
					{
						mMipData[mipIndex] = nullptr;
					}
				}
				else
				{
					for (int32 mipIndex = mCurrentFirstMap; mipIndex < mOwner->mTextureData->mInfo.mNumMipmaps; mipIndex++)
					{
						if (mMipData[mipIndex] != nullptr)
						{
							uint32 destPitch;
							void* theMipData = RHILockTexture2D(mTexture2DRHI, mipIndex - mCurrentFirstMap, RLM_WriteOnly, destPitch, false);
							getData(mipIndex, theMipData, destPitch);
							RHIUnlockTexture2D(mTexture2DRHI, mipIndex - mCurrentFirstMap, false);
						}
					}
				}
			}
			EMipFadeSettings mipFadeSetting = MipFade_Normal;
			mMipBiasFade.setNewMipCount(mOwner->mRequestedMips, mOwner->mRequestedMips, mLastRenderTime, mipFadeSetting);
			mOwner->mPendingMipChangeRequestStatus.increment();
		}
		else
		{
			bool bSkipRHITextureCreation = false;
			if (GIsEditor || (!bSkipRHITextureCreation))
			{
				RHIResourceCreateInfo createInfo;
				mTexture2DRHI = RHICreateTexture2D(mOwner->mTextureData->mInfo.mWidth, mOwner->mTextureData->mInfo.mHeight, effectiveFormat, mOwner->mRequestedMips, 1, texCreateFlags, createInfo);
				mTextureRHI = mTexture2DRHI;
				mTextureRHI->setName(mOwner->getName());
				RHIBindDebugLabelName(mTextureRHI, mOwner->getName().c_str());
				RHIUpdateTextureReference(mOwner->mTextureReference.mTextureReferenceRHI, mTextureRHI);
				for (int32 mipIndex = mCurrentFirstMap; mipIndex < mOwner->mTextureData->mInitData.size(); mipIndex++)
				{
					if (mMipData[mipIndex] != nullptr)
					{
						uint32 destPitch;
						void* theMipData = RHILockTexture2D(mTexture2DRHI, mipIndex - mCurrentFirstMap, RLM_WriteOnly, destPitch, false);
						getData(mipIndex, theMipData, destPitch);
						RHIUnlockTexture2D(mTexture2DRHI, mipIndex, false);

					}
				}
			}
		}
	}

	void Texture2DResource::getData(uint32 mipIndex, void* dest, uint32 destPitch)
	{
		BOOST_ASSERT(mipIndex < mOwner->mTextureData->mInitData.size());
		int32 width = Math::max<uint32>(1, mOwner->mTextureData->mInfo.mWidth >> mipIndex);
		int32 height = Math::max<uint32>(1, mOwner->mTextureData->mInfo.mHeight >> mipIndex);
		auto& initData = mOwner->mTextureData->mInitData[mipIndex];
		if (destPitch == 0)
		{
			Memory::memcpy(dest, mMipData[mipIndex], initData.mRowPitch * initData.mSlicePitch);

		}
		else
		{
			EPixelFormat pixelFormat = mOwner->getPixelFormat();
			const uint32 blockSizeX = GPixelFormats[pixelFormat].BlockSizeX;
			const uint32 blockSizeY = GPixelFormats[pixelFormat].BlockSizeY;
			const uint32 blockBytes = GPixelFormats[pixelFormat].BlockBytes;
			uint32 numColumns = (width + blockSizeX - 1) / blockSizeX;
			uint32 numRows = (height + blockSizeY - 1) / blockSizeY;
			if (pixelFormat == PF_PVRTC2 || pixelFormat == PF_PVRTC4)
			{
				numColumns = Math::max<uint32>(numColumns, 2);
				numRows = Math::max<uint32>(numRows, 2);
			}

			const uint32 srcPitch = numColumns * blockBytes;
			const uint32 effectiveSize = blockBytes * numColumns * numRows;

			copyTextureData2D(mMipData[mipIndex], dest, height, pixelFormat, srcPitch, destPitch);
		}
		Memory::free(mMipData[mipIndex]);
		mMipData[mipIndex] = nullptr;
	}

	void Texture2DResource::releaseRHI()
	{
		TextureResource::releaseRHI();
		mTextureRHI.safeRelease();
	}

	int32 Texture2DResource::getDefaultMipMapBias() const
	{
		return 0;
	}

	DECLARE_SIMPLER_REFLECTION(RTexture2D);
}