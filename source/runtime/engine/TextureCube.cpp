#include "TextureCube.h"
#include "SimpleReflection.h"
#include "RHICommandList.h"
#include "RenderUtils.h"
#include "ResImporter/TextureImporter.h"
namespace Air
{
	class TextureCubeResource : public TextureResource
	{
	public:
		TextureCubeResource(RTextureCube* inOwner)
			:mOwner(inOwner)
			, mTextureSize(0)
		{
		}

		virtual void initRHI() override
		{
			uint32 texCreateFlags = (mOwner->bSRGB ? TexCreate_SRGB : 0) | TexCreate_OfflineProcessed;
			RHIResourceCreateInfo createInfo;
			mTextureCubeRHI = RHICreateTextureCube(mOwner->getWidth(), mOwner->getPixelFormat(), mOwner->getNumMips(), texCreateFlags, createInfo);
			mTextureRHI = mTextureCubeRHI;
			mTextureRHI->setName(mOwner->getName());
			RHIBindDebugLabelName(mTextureRHI, mOwner->getName().c_str());
			RHIUpdateTextureReference(mOwner->mTextureReference.mTextureReferenceRHI, mTextureRHI);
			int32 numMips = mOwner->getNumMips();
			for (int32 faceIndex = 0; faceIndex < 6; faceIndex++)
			{
				uint32 width = mOwner->getWidth();
				uint32 height = mOwner->getHeight();
				for (int32 mipIndex = 0 ; mipIndex < numMips; mipIndex++)
				{
					size_t const index = (faceIndex - RTexture::CF_Positive_X) * mOwner->getNumMips() + mipIndex;
					if (index < mOwner->mTextureData->mInitData.size())
					{
						uint32 destStride;
						void* theMipData = RHILockTextureCubeFace(mTextureCubeRHI, faceIndex, 0, mipIndex, RLM_WriteOnly, destStride, false);
						getData(faceIndex, mipIndex, theMipData, destStride);
						RHIUnlockTextureCubeFace(mTextureCubeRHI, faceIndex, 0, mipIndex, false);
					}
				}
			}

			SamplerStateInitializerRHI samplerStateInitializer(SF_Bilinear,
				AM_Clamp,
				AM_Clamp,
				AM_Clamp);
			mSamplerStateRHI = RHICreateSamplerState(samplerStateInitializer);
			EPixelFormat pixelFormat = mOwner->getPixelFormat();
			bGreyScaleFormat = (pixelFormat == PF_G8) || (pixelFormat == PF_BC4);

		}

		virtual void releaseRHI() override
		{
			RHIUpdateTextureReference(mOwner->mTextureReference.mTextureReferenceRHI, TextureCubeRHIParamRef());
			mTextureCubeRHI.safeRelease();
			TextureResource::releaseRHI();
		}

		virtual uint32 getWidth() const override
		{
			return mOwner->getWidth();
		}

		virtual uint32 getHeight() const override
		{
			return mOwner->getHeight();
		}

	private:
		void getData(int32 faceIndex, int32 mipIndex, void* dest, uint32 destPitch)
		{
			size_t const index = (faceIndex - RTexture::CF_Positive_X) * mOwner->getNumMips() + mipIndex;

			BOOST_ASSERT(index < mOwner->mTextureData->mInitData.size());
			if (destPitch == 0)
			{
				Memory::memcpy(dest, mOwner->mTextureData->mInitData[index].mData, mOwner->mTextureData->mInitData[index].mSlicePitch);
			}
			else
			{
				EPixelFormat pixelFormat = mOwner->getPixelFormat();
				uint32 numRows = 0;
				uint32 srcPitch = 0;
				uint32 blockSizeX = GPixelFormats[pixelFormat].BlockSizeX;
				uint32 blockSizeY = GPixelFormats[pixelFormat].BlockSizeY;
				uint32 blockBytes = GPixelFormats[pixelFormat].BlockBytes;

				int2 mipExtent = calcMipMapExtent(mOwner->getWidth(), mOwner->getHeight(), pixelFormat, mipIndex);
				uint32 numColumns = (mipExtent.x + blockSizeX - 1) / blockSizeX;
				numRows = (mipExtent.y + blockSizeY - 1) / blockSizeY;
				srcPitch = numColumns * blockBytes;
				size_t mipSizeInBytes = CalcTextureMipMapSize(mipExtent.x, mipExtent.y, pixelFormat, 0);
				if (srcPitch == destPitch)
				{
					Memory::memcpy(dest, mOwner->mTextureData->mInitData[index].mData, mipSizeInBytes);
				}
				else
				{
					uint8* src = (uint8*)mOwner->mTextureData->mInitData[index].mData;
					uint8* dst = (uint8*)dest;
					for (uint32 row = 0; row < numRows; row++)
					{
						Memory::memcpy(dst, src, srcPitch);
						src += srcPitch;
						dst += destPitch;
					}
					BOOST_ASSERT((PTRINT(src) - PTRINT(mOwner->mTextureData->mInitData[index].mData)) == PTRINT(mipSizeInBytes));
				}
			}
		}

	private:
		TextureCubeRHIRef mTextureCubeRHI;
		const RTextureCube* mOwner;
		int32 mTextureSize;
	};


	RTextureCube::RTextureCube(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
	
	}


	TextureResource* RTextureCube::createResource()
	{
		TextureResource* newResource = nullptr;
		if (getNumMips() > 0)
		{
			newResource = new TextureCubeResource(this);
		}
		return newResource;
	}

	DECLARE_SIMPLER_REFLECTION(RTextureCube);
}