#pragma once
#include "EngineMininal.h"
#include "ResImporter/ImporterMamanger.h"
#include "Texture.h"
#include "PixelFormat.h"
#include "Misc/Paths.h"
#include "Texture.h"
namespace Air
{
	struct TextureInfo
	{
		RTexture::TextureType mType;
		uint32 mWidth;
		uint32 mHeight;
		uint32 mDepth;
		uint32 mArraySize;
		uint32 mNumMipmaps;
		EPixelFormat mFormat;
		uint32 mRowPitch;
		uint32 mSlicePitch;
		uint32 mHeaderSize;
	};

	struct TextureData : EngineResourceData
	{
		std::vector<ElementInitData> mInitData;
		std::vector<uint8>	mDataBlock;
		TextureInfo mInfo;

		bool tryLoadMips(int32 firstMipToLoad, void** outMipData);
	private:
		bool tryLoadMips_TT_2D(int32 firstMipToLoad, void** outMipData);
	};

	class TextureImporter : public Importer
	{
	public:

		virtual ~TextureImporter() {};

		virtual TextureInfo getTextureInfo(Archive& file) = 0;

	protected:
		TextureImporter(TCHAR* extension);
	};
}