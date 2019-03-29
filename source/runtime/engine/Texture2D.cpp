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

	DECLARE_SIMPLER_REFLECTION(RTexture2D);
}