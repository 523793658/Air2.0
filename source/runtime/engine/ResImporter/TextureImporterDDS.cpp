#include "ResImporter/TextureImporterDDS.h"
#include "HAL/FileManager.h"
#include "RenderUtils.h"
namespace Air
{
	struct BC1Block
	{
		uint16 Color0, Color1;
		uint16 Bitmap[2];
	};

	struct BC2Block
	{
		uint16 alpha[4];
		BC1Block BC1;
	};

	struct BC4Block
	{
		uint8 Alpha0, Alpha1;
		uint8 bitmap[6];
	};

	struct BC3Block
	{
		BC4Block alpha;
		BC1Block bc1;
	};

	struct BC5Block
	{
		BC4Block red;
		BC4Block green;
	};

	enum D3D_RESOURCE_MISC_FLAG
	{
		D3D_RESOURCE_MISC_GENERATE_MIPS = 0x1L,
		D3D_RESOURCE_MISC_SHADER = 0x2L,
		D3D_RESOURCE_MISC_TEXTURECUBE = 0x4L,
		D3D_RESOURCE_MISC_SHADER_KEYEDMUTEX = 0x10L,
		D3D_RESOURCE_MISC_GDI_COMPATIBLE = 0x20L
	};
	enum
	{
		DDSCAPS2_CUBEMAP = 0x00000200,
		DDSCAPS2_CUBEMAP_POSITIVEX = 0x00000400,
		DDSCAPS2_CUBEMAP_NEGATIVEX = 0x00000800,
		DDSCAPS2_CUBEMAP_POSITIVEY = 0x00001000,
		DDSCAPS2_CUBEMAP_NEGATIVEY = 0x00002000,
		DDSCAPS2_CUBEMAP_POSITIVEZ = 0x00004000,
		DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x00008000,
		DDSCAPS2_VOLUME			= 0x00200000,
	};

	typedef enum DXGI_FORMAT
	{
		DXGI_FORMAT_UNKNOWN = 0,
		DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
		DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
		DXGI_FORMAT_R32G32B32A32_UINT = 3,
		DXGI_FORMAT_R32G32B32A32_SINT = 4,
		DXGI_FORMAT_R32G32B32_TYPELESS = 5,
		DXGI_FORMAT_R32G32B32_FLOAT = 6,
		DXGI_FORMAT_R32G32B32_UINT = 7,
		DXGI_FORMAT_R32G32B32_SINT = 8,
		DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
		DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
		DXGI_FORMAT_R16G16B16A16_UNORM = 11,
		DXGI_FORMAT_R16G16B16A16_UINT = 12,
		DXGI_FORMAT_R16G16B16A16_SNORM = 13,
		DXGI_FORMAT_R16G16B16A16_SINT = 14,
		DXGI_FORMAT_R32G32_TYPELESS = 15,
		DXGI_FORMAT_R32G32_FLOAT = 16,
		DXGI_FORMAT_R32G32_UINT = 17,
		DXGI_FORMAT_R32G32_SINT = 18,
		DXGI_FORMAT_R32G8X24_TYPELESS = 19,
		DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
		DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
		DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
		DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
		DXGI_FORMAT_R10G10B10A2_UNORM = 24,
		DXGI_FORMAT_R10G10B10A2_UINT = 25,
		DXGI_FORMAT_R11G11B10_FLOAT = 26,
		DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
		DXGI_FORMAT_R8G8B8A8_UNORM = 28,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
		DXGI_FORMAT_R8G8B8A8_UINT = 30,
		DXGI_FORMAT_R8G8B8A8_SNORM = 31,
		DXGI_FORMAT_R8G8B8A8_SINT = 32,
		DXGI_FORMAT_R16G16_TYPELESS = 33,
		DXGI_FORMAT_R16G16_FLOAT = 34,
		DXGI_FORMAT_R16G16_UNORM = 35,
		DXGI_FORMAT_R16G16_UINT = 36,
		DXGI_FORMAT_R16G16_SNORM = 37,
		DXGI_FORMAT_R16G16_SINT = 38,
		DXGI_FORMAT_R32_TYPELESS = 39,
		DXGI_FORMAT_D32_FLOAT = 40,
		DXGI_FORMAT_R32_FLOAT = 41,
		DXGI_FORMAT_R32_UINT = 42,
		DXGI_FORMAT_R32_SINT = 43,
		DXGI_FORMAT_R24G8_TYPELESS = 44,
		DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
		DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
		DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,
		DXGI_FORMAT_R8G8_TYPELESS = 48,
		DXGI_FORMAT_R8G8_UNORM = 49,
		DXGI_FORMAT_R8G8_UINT = 50,
		DXGI_FORMAT_R8G8_SNORM = 51,
		DXGI_FORMAT_R8G8_SINT = 52,
		DXGI_FORMAT_R16_TYPELESS = 53,
		DXGI_FORMAT_R16_FLOAT = 54,
		DXGI_FORMAT_D16_UNORM = 55,
		DXGI_FORMAT_R16_UNORM = 56,
		DXGI_FORMAT_R16_UINT = 57,
		DXGI_FORMAT_R16_SNORM = 58,
		DXGI_FORMAT_R16_SINT = 59,
		DXGI_FORMAT_R8_TYPELESS = 60,
		DXGI_FORMAT_R8_UNORM = 61,
		DXGI_FORMAT_R8_UINT = 62,
		DXGI_FORMAT_R8_SNORM = 63,
		DXGI_FORMAT_R8_SINT = 64,
		DXGI_FORMAT_A8_UNORM = 65,
		DXGI_FORMAT_R1_UNORM = 66,
		DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,
		DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
		DXGI_FORMAT_G8R8_G8B8_UNORM = 69,
		DXGI_FORMAT_BC1_TYPELESS = 70,
		DXGI_FORMAT_BC1_UNORM = 71,
		DXGI_FORMAT_BC1_UNORM_SRGB = 72,
		DXGI_FORMAT_BC2_TYPELESS = 73,
		DXGI_FORMAT_BC2_UNORM = 74,
		DXGI_FORMAT_BC2_UNORM_SRGB = 75,
		DXGI_FORMAT_BC3_TYPELESS = 76,
		DXGI_FORMAT_BC3_UNORM = 77,
		DXGI_FORMAT_BC3_UNORM_SRGB = 78,
		DXGI_FORMAT_BC4_TYPELESS = 79,
		DXGI_FORMAT_BC4_UNORM = 80,
		DXGI_FORMAT_BC4_SNORM = 81,
		DXGI_FORMAT_BC5_TYPELESS = 82,
		DXGI_FORMAT_BC5_UNORM = 83,
		DXGI_FORMAT_BC5_SNORM = 84,
		DXGI_FORMAT_B5G6R5_UNORM = 85,
		DXGI_FORMAT_B5G5R5A1_UNORM = 86,
		DXGI_FORMAT_B8G8R8A8_UNORM = 87,
		DXGI_FORMAT_B8G8R8X8_UNORM = 88,
		DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
		DXGI_FORMAT_B8G8R8A8_TYPELESS = 90,
		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
		DXGI_FORMAT_B8G8R8X8_TYPELESS = 92,
		DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
		DXGI_FORMAT_BC6H_TYPELESS = 94,
		DXGI_FORMAT_BC6H_UF16 = 95,
		DXGI_FORMAT_BC6H_SF16 = 96,
		DXGI_FORMAT_BC7_TYPELESS = 97,
		DXGI_FORMAT_BC7_UNORM = 98,
		DXGI_FORMAT_BC7_UNORM_SRGB = 99,
		DXGI_FORMAT_AYUV = 100,
		DXGI_FORMAT_Y410 = 101,
		DXGI_FORMAT_Y416 = 102,
		DXGI_FORMAT_NV12 = 103,
		DXGI_FORMAT_P010 = 104,
		DXGI_FORMAT_P016 = 105,
		DXGI_FORMAT_420_OPAQUE = 106,
		DXGI_FORMAT_YUY2 = 107,
		DXGI_FORMAT_Y210 = 108,
		DXGI_FORMAT_Y216 = 109,
		DXGI_FORMAT_NV11 = 110,
		DXGI_FORMAT_AI44 = 111,
		DXGI_FORMAT_IA44 = 112,
		DXGI_FORMAT_P8 = 113,
		DXGI_FORMAT_A8P8 = 114,
		DXGI_FORMAT_B4G4R4A4_UNORM = 115,

		DXGI_FORMAT_P208 = 130,
		DXGI_FORMAT_V208 = 131,
		DXGI_FORMAT_V408 = 132,


		DXGI_FORMAT_FORCE_UINT = 0xffffffff
	} DXGI_FORMAT;

	EPixelFormat fromDXGIFormat(uint32 format)
	{
		switch (format)
		{
		case DXGI_FORMAT_A8_UNORM:
			return PF_A8;
		case DXGI_FORMAT_B5G6R5_UNORM:
			return PF_R5G6B5_UNORM;
		case DXGI_FORMAT_B5G5R5A1_UNORM:
			return PF_R5G5B5A1_UNORM;
		case DXGI_FORMAT_R8_UNORM:
			return PF_R8;
		case DXGI_FORMAT_R8_SNORM:
			return PF_R8_SNORM;
		case DXGI_FORMAT_R8G8_UNORM:
			return PF_R8G8;
		case DXGI_FORMAT_R8G8_SNORM:
			return PF_R8G8_SNORM;
		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return PF_B8G8R8A8;
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			return PF_B8G8R8A8_SRGB;
		case DXGI_FORMAT_R8G8B8A8_UNORM:
			return PF_R8G8B8A8;
		case DXGI_FORMAT_R8G8B8A8_SNORM:
			return PF_R8G8B8A8_SNORM;
		case DXGI_FORMAT_R8G8B8A8_SINT:
			return PF_R8G8B8A8_SINT;
		case DXGI_FORMAT_R10G10B10A2_UINT:
			return PF_A2B10G10R10_UINT;
		default:
			BOOST_ASSERT(false);
			break;
		}
	}

	void bc4Tobc1G(BC1Block& bc1, BC4Block const & bc4)
	{
		bc1.Color0 = (bc4.Alpha0 >> 2) << 5;
		bc1.Color1 = (bc4.Alpha1 >> 2) << 5;
		bool swapClr = false;
		if (bc4.Alpha0 < bc4.Alpha1)
		{
			swapClr = true;
		}
		for (int i = 0; i < 2; i++)
		{
			uint32 alpha32 = (bc4.bitmap[i * 3 + 2] << 16) | (bc4.bitmap[i * 3 + 1] << 8) | (bc4.bitmap[i * 3 + 0
			] << 0);
			uint16 mask = 0;
			for (int j = 0; j < 8; j++)
			{
				uint16 bit = (alpha32 >> (j * 3)) & 0x7;
				if (swapClr)
				{
					switch (bit)
					{
					case 0:
					case 6:
						bit = 0;
						break;
					case 1:
					case 7:
						bit = 1;
						break;
					case 2:
					case 3:
						bit = 2;
						break;
					case 4:
					case 5:
						bit = 3;
						break;
					}
				}
				else
				{
					switch (bit)
					{
					case 0:
					case 2:
						bit = 0;
						break;
					case 1:
					case 5:
					case 7:
						bit = 1;
						break;
					case 3:
					case 4:
					case 6:
						bit = 2;
						break;
					}
				}
				mask |= bit << (j * 2);
			}
			bc1.Bitmap[i] = mask;
		}
	}


	TextureImporterDDS::TextureImporterDDS()
		:TextureImporter(TEXT("dds"))
	{

	}

	

	bool TextureImporterDDS::decode(filesystem::path path, std::shared_ptr<EngineResourceData>& outResourceData)
	{
		TextureData& texData = dynamic_cast<TextureData&>(*outResourceData);

		Archive* reader = IFileManager::get().createFileReader(path.c_str());

		TextureInfo& info = texData.mInfo;

		reader->seek(info.mHeaderSize);

		uint32 const fmtSize = GPixelFormats[info.mFormat].BlockBytes;
		bool padding = false;

		if (!isCompressedFormat(info.mFormat))
		{
			if (info.mRowPitch != info.mWidth * fmtSize)
			{
				BOOST_ASSERT(info.mRowPitch == ((info.mWidth + 3) & ~3) * fmtSize);
				padding = true;
			}
		}
		
		std::vector<size_t> base;
		switch (info.mType)
		{
		case RTexture::TT_1D:
		{
			texData.mInitData.resize(info.mArraySize * info.mNumMipmaps);
			base.resize(info.mArraySize * info.mNumMipmaps);
			for (uint32 array_index = 0; array_index < info.mArraySize; ++array_index)
			{
				uint32 width = info.mWidth;
				for (uint32 level = 0; level < info.mNumMipmaps; ++level)
				{
					size_t const index = array_index * info.mNumMipmaps + level;
					uint32 image_size;
					if (isCompressedFormat(info.mFormat))
					{
						uint32 const block_size = GPixelFormats[info.mFormat].BlockBytes * 4;
						image_size = ((width + 3) / 4) * block_size;
					}
					else
					{
						image_size = (padding ? ((width + 3) & ~3) : width) * fmtSize;
					}
					base[index] = texData.mDataBlock.size();
					texData.mDataBlock.resize(base[index] + image_size);
					texData.mInitData[index].mRowPitch = image_size;
					texData.mInitData[index].mSlicePitch = image_size;
					reader->serialize(&texData.mDataBlock[base[index]], image_size);
					BOOST_ASSERT(reader->totalSize() == image_size);
					width = std::max<uint32>(width / 2, 1);
				}
			}
		}
			break;
		case RTexture::TT_2D:
		{
			texData.mInitData.resize(info.mArraySize * info.mNumMipmaps);
			base.resize(info.mArraySize * info.mNumMipmaps);
			for (uint32 array_index = 0; array_index < info.mArraySize; ++array_index)
			{
				uint32 width = info.mWidth;
				uint32 height = info.mHeight;
				for (uint32 level = 0; level < info.mNumMipmaps; ++level)
				{
					size_t const index = array_index * info.mNumMipmaps + level;
					if (isCompressedFormat(info.mFormat))
					{
						uint32 const blockSize = GPixelFormats[info.mFormat].BlockBytes;
						uint32 image_size = ((width + 3) / 4) * ((height + 3) / 4)*blockSize;
						base[index] = texData.mDataBlock.size();
						texData.mDataBlock.resize(base[index] + image_size);
						texData.mInitData[index].mRowPitch = (width + 3) / 4 * blockSize;
						texData.mInitData[index].mSlicePitch = image_size;

						reader->serialize(&texData.mDataBlock[base[index]], image_size);
					}
					else
					{
						texData.mInitData[index].mRowPitch = (padding ? ((width + 3) & ~3) : width)*fmtSize;
						texData.mInitData[index].mSlicePitch = texData.mInitData[index].mRowPitch * height;
						base[index] = texData.mDataBlock.size();
						texData.mDataBlock.resize(base[index] + texData.mInitData[index].mSlicePitch);
						reader->serialize(&texData.mDataBlock[base[index]], texData.mInitData[index].mSlicePitch);
						//BOOST_ASSERT(reader->totalSize() == texData.mInitData[index].mSlicePitch);
					}
					width = Math::max<uint32>(width / 2, 1);
					height = Math::max<uint32>(height / 2, 1);
				}
			}
		}
			break;
		case RTexture::TT_3D:
		{
			texData.mInitData.resize(info.mArraySize * info.mNumMipmaps);
			base.resize(info.mArraySize * info.mNumMipmaps);
			for (uint32 arrayIndex = 0; arrayIndex < info.mArraySize; ++arrayIndex)
			{
				uint32 width = info.mWidth;
				uint32 height = info.mHeight;
				uint32 depth = info.mDepth;
				for (uint32 level = 0; level < info.mNumMipmaps; ++level)
				{
					size_t const index = arrayIndex * info.mNumMipmaps + level;
					if (isCompressedFormat(info.mFormat))
					{
						uint32 const blockSize = GPixelFormats[info.mFormat].BlockBytes * 4;
						uint32 imageSize = ((width + 3) / 4) * ((height + 3) / 4) * depth * blockSize;
						base[index] = texData.mDataBlock.size();
						texData.mDataBlock.resize(base[index] + imageSize);
						texData.mInitData[index].mRowPitch = (width + 3) / 4 * blockSize;
						texData.mInitData[index].mSlicePitch = ((width + 3) / 4) * ((height + 3) / 4) * blockSize;

						reader->serialize(&texData.mDataBlock[base[index]], imageSize);
						BOOST_ASSERT(reader->totalSize() == imageSize);
					}
					else
					{
						texData.mInitData[index].mRowPitch = (padding ? ((width + 3) & ~3) : width) * fmtSize;
						texData.mInitData[index].mSlicePitch = texData.mInitData[index].mRowPitch * height;
						base[index] = texData.mDataBlock.size();
						texData.mDataBlock.resize(base[index] + texData.mInitData[index].mSlicePitch * depth);

						reader->serialize(&texData.mDataBlock[base[index]], texData.mInitData[index].mSlicePitch * depth);
						BOOST_ASSERT(reader->totalSize() == texData.mInitData[index].mSlicePitch * depth);
					}
					width = Math::max<uint32>(width / 2, 1);
					height = Math::max<uint32>(height / 2, 1);
					depth = Math::max<uint32>(depth / 2, 1);


				}
			}
		}
		break;
		case RTexture::TT_Cube:
		{
			texData.mInitData.resize(info.mArraySize * 6 * info.mNumMipmaps);
			base.resize(info.mArraySize * 6 * info.mNumMipmaps);
			for (uint32 arrayIndex = 0; arrayIndex < info.mArraySize; ++arrayIndex)
			{
				for (uint32 face = RTexture::CF_Positive_X; face <= RTexture::CF_Negative_Z; ++face)
				{
					uint32 width = info.mWidth;
					uint32 height = info.mHeight;
					for (uint32 level = 0; level < info.mNumMipmaps; ++level)
					{
						size_t const index = (arrayIndex * 6 + face - RTexture::CF_Positive_X) * info.mNumMipmaps + level;
						if (isCompressedFormat(info.mFormat))
						{
							uint32 const blockSize = GPixelFormats[info.mFormat].BlockBytes * 4;
							uint32 imageSize = ((width + 3) / 4) * ((height + 3) / 4) * blockSize;
							base[index] = texData.mDataBlock.size();
							texData.mDataBlock.resize(base[index] + imageSize);
							texData.mInitData[index].mRowPitch = (width + 3) / 4 * blockSize;
							texData.mInitData[index].mSlicePitch = imageSize;
							reader->serialize(&texData.mDataBlock[base[index]], imageSize);
							//BOOST_ASSERT(reader->totalSize() == imageSize);
						}
						else
						{
							texData.mInitData[index].mRowPitch = (padding ? ((width + 3) & ~3) : width)*fmtSize;
							texData.mInitData[index].mSlicePitch = texData.mInitData[index].mRowPitch * width;
							base[index] = texData.mDataBlock.size();
							texData.mDataBlock.resize(base[index] + texData.mInitData[index].mSlicePitch);

							reader->serialize(&texData.mDataBlock[base[index]], texData.mInitData[index].mSlicePitch);
							//BOOST_ASSERT(reader->totalSize() == texData.mInitData[index].mSlicePitch);
						}
						width = Math::max<uint32>(width / 2, 1);
						height = Math::max<uint32>(height / 2, 1);
					}
				}
			}
		}
		break;
		default:
			break;
		}
		for (size_t i = 0; i < base.size(); ++i)
		{
			texData.mInitData[i].mData = &texData.mDataBlock[base[i]];
		}


		if ((RTexture::TT_3D == info.mType) && (GMaxTextureDepth < info.mDepth))
		{
			info.mType = RTexture::TT_2D;
			info.mHeight *= info.mDepth;
			info.mDepth = 1;
			info.mNumMipmaps = 1;
			texData.mInitData.resize(1);
		}
		uint32 arraySize = info.mArraySize;
		if (RTexture::TT_Cube == info.mType)
		{
			arraySize *= 6;
		}

		if (((PF_BC5 == info.mFormat) && !GPixelFormats[PF_BC5].Supported) || ((PF_BC5_SRGB == info.mFormat) && !GPixelFormats[PF_BC5_SRGB].Supported))
		{
			BC1Block tmp;
			for (size_t i = 0; i < texData.mInitData.size(); ++i)
			{
				for (size_t j = 0; j < texData.mInitData[i].mSlicePitch; j += sizeof(BC4Block) * 2)
				{
					char* p = static_cast<char*>(const_cast<void*>(texData.mInitData[i].mData)) + j;
					bc4Tobc1G(tmp, *reinterpret_cast<BC4Block const *>(p + sizeof(BC4Block)));
					Memory::memcpy(p + sizeof(BC4Block), &tmp, sizeof(BC1Block));
				}
			}
			if (isSRGB(info.mFormat))
			{
				info.mFormat = PF_BC3_SRGB;
			}
			else
			{
				info.mFormat = PF_BC3;
			}
		}

		if (((PF_BC4 == info.mFormat) && !GPixelFormats[PF_BC4].Supported) || ((PF_BC4_SRGB == info.mFormat) && !GPixelFormats[PF_BC4_SRGB].Supported))
		{
			BC1Block tmp;
			for (size_t i = 0; i < texData.mInitData.size(); ++i)
			{
				for (size_t j = 0; j < texData.mInitData[i].mSlicePitch; j += sizeof(BC4Block))
				{
					char* p = static_cast<char*>(const_cast<void*>(texData.mInitData[i].mData)) + j;
					bc4Tobc1G(tmp, *reinterpret_cast<BC4Block const *>(p));
					Memory::memcpy(p, &tmp, sizeof(BC1Block));
				}
			}
			if (isSRGB(info.mFormat))
			{
				info.mFormat = PF_BC1_SRGB;
			}
			else
			{
				info.mFormat = PF_BC1;
			}
		}
		static EPixelFormat const convertFmts[][2] = 
		{
			{ PF_BC1, PF_B8G8R8A8},
			{ PF_BC1_SRGB, PF_B8G8R8A8_SRGB },
			{ PF_BC2, PF_B8G8R8A8 },
			{ PF_BC2_SRGB, PF_B8G8R8A8_SRGB },
			{ PF_BC3, PF_B8G8R8A8 },
			{ PF_BC3_SRGB, PF_B8G8R8A8_SRGB },
			{ PF_BC4,	PF_R8},
			{ PF_BC4_SRGB, PF_R8},
			{ PF_BC5, PF_R8G8},
			{ PF_BC5_SRGB, PF_R8G8 },
			{ PF_BC6H, PF_A16B16G16R16F},
			{ PF_BC7, PF_B8G8R8A8},
			{ PF_BC7_SRGB, PF_B8G8R8A8_SRGB}
		};
		BOOST_ASSERT(GPixelFormats[info.mFormat].Supported);
		return true;
	}

	struct DDSPIXELFORMAT
	{
		uint32 size;
		uint32 flags;
		uint32 four_cc;
		uint32 rgb_bit_count;
		uint32 r_bit_mask;
		uint32 g_bit_mask;
		uint32 b_bit_mask;
		uint32 rgb_alpha_bit_mask;
	};

	struct  DDSCAPS2
	{
		uint32 caps1;
		uint32 caps2;
		uint32 reserved[2];
	};

	struct DDSSURFACEDESC2
	{
		uint32 size;
		uint32 flags;
		uint32 height;
		uint32 width;
		union
		{
			int32 pitch;
			uint32 linear_size;
		};
		uint32 depth;
		uint32 mip_map_count;
		uint32 reserved1[11];
		DDSPIXELFORMAT pixel_format;
		DDSCAPS2 dds_caps;
		uint32 reserved2;
	};

	enum
	{
		DDSD_CAPS			= 0x00000001,
		DDSD_HEIGHT			= 0x00000002,
		DDSD_WIDTH			= 0x00000004,
		DDSD_PITCH			= 0x00000008,
		DDSD_PIXELFORMAT	= 0x00001000,
		DDSD_MIPMAPCOUNT	= 0x00020000,
		DDSD_LINEEARSIZE	= 0x00080000,
		DDSD_DEPTH			= 0x00800000,


	};

	enum D3D_RESOURCE_DIMENSION
	{
		D3D_RESOURCE_DIMENSION_UNKNOWN = 0,
		D3D_RESOURCE_DIMENSION_BUFFER = 1,
		D3D_RESOURCE_DIMENSION_TEXTURE1D = 2,
		D3D_RESOURCE_DIMENSION_TEXTURE2D = 3,
		D3D_RESOURCE_DIMENSION_TEXTURE3D = 4,

	};

	struct DDS_HEADER_DXT10
	{
		uint32 dxgi_format;
		D3D_RESOURCE_DIMENSION resource_dim;
		uint32 misc_flag;
		uint32 array_size;
		uint32 reserved;
	};

	enum
	{
		DDSPF_ALPHAPIXELS = 0x00000001,

		DDSPF_ALPHA = 0x00000002,
		DDSPF_FOURCC = 0x00000004,
		DDSPF_RGB = 0x00000040,
		DDSPF_LUMINANCE = 0x00020000,
		DDSPF_BUMPDUDV = 0x00080000,
	};

	TextureInfo TextureImporterDDS::getTextureInfo(Archive& file)
	{
		TextureInfo info;
		uint32 magic;
		file << magic;
		magic = LE2Native(magic);
		BOOST_ASSERT((MakeFourCC<'D', 'D', 'S', ' '>::value) == magic);
		DDSSURFACEDESC2 desc;
		file.serialize(&desc, sizeof(desc));
		desc.flags = LE2Native(desc.flags);
		desc.height = LE2Native(desc.height);
		desc.width = LE2Native(desc.width);
		desc.pitch = LE2Native(desc.pitch);
		desc.depth = LE2Native(desc.depth);
		desc.mip_map_count = LE2Native(desc.mip_map_count);
		for (uint32 i = 0; i < std::size(desc.reserved1); ++i)
		{
			desc.reserved1[i] = LE2Native(desc.reserved1[i]);
		}

		desc.pixel_format.size = LE2Native(desc.pixel_format.size);
		desc.pixel_format.flags = LE2Native(desc.pixel_format.flags);
		desc.pixel_format.four_cc = LE2Native(desc.pixel_format.four_cc);
		desc.pixel_format.rgb_bit_count = LE2Native(desc.pixel_format.rgb_bit_count);
		desc.pixel_format.r_bit_mask = LE2Native(desc.pixel_format.r_bit_mask);
		desc.pixel_format.g_bit_mask = LE2Native(desc.pixel_format.g_bit_mask);
		desc.pixel_format.b_bit_mask = LE2Native(desc.pixel_format.b_bit_mask);
		desc.pixel_format.rgb_alpha_bit_mask = LE2Native(desc.pixel_format.rgb_alpha_bit_mask);
		desc.dds_caps.caps1 = LE2Native(desc.dds_caps.caps1);
		desc.dds_caps.caps2 = LE2Native(desc.dds_caps.caps2);
		for (uint32 i = 0; i < std::size(desc.dds_caps.reserved); ++i)
		{
			desc.dds_caps.reserved[i] = LE2Native(desc.dds_caps.reserved[i]);
		}
		desc.reserved2 = LE2Native(desc.reserved2);

		DDS_HEADER_DXT10 desc10;
		if (MakeFourCC<'D', 'X', '1', '0'>::value == desc.pixel_format.four_cc)
		{
			file.serialize(&desc10, sizeof(desc10));
			desc10.dxgi_format = LE2Native(desc10.dxgi_format);
			desc10.resource_dim = LE2Native(desc10.resource_dim);
			desc10.misc_flag = LE2Native(desc10.misc_flag);
			desc10.array_size = LE2Native(desc10.array_size);
			desc10.reserved = LE2Native(desc10.reserved);
			info.mArraySize = desc10.array_size;
		}
		else
		{
			Memory::Memset(&desc10, 0, sizeof(desc10));
			info.mArraySize = 1;
			BOOST_ASSERT((desc.flags & DDSD_CAPS) != 0);
			BOOST_ASSERT((desc.flags & DDSD_PIXELFORMAT != 0));
		}

		BOOST_ASSERT((desc.flags & DDSD_WIDTH) != 0);
		BOOST_ASSERT((desc.flags & DDSD_HEIGHT) != 0);

		if (0 == (desc.flags & DDSD_MIPMAPCOUNT))
		{
			desc.mip_map_count = 1;
		}
		info.mFormat = PF_B8G8R8A8;
		if ((desc.pixel_format.flags & DDSPF_FOURCC) != 0)
		{
			switch (desc.pixel_format.four_cc)
			{
			case 36:
				info.mFormat = PF_A16B16G16R16;
				break;
			case  110:
				info.mFormat = PF_A16B16G16R16_SINT;
				break;
			case 111:
				info.mFormat = PF_R16F;
				break;
			case 112:
				info.mFormat = PF_R16G16_FLOAT;
				break;
			case 113:
				info.mFormat = PF_A16B16G16R16F;
				break;
			case 114:
				info.mFormat = PF_R32_FLOAT;
				break;
			case 115:
				info.mFormat = PF_G32R32F;
				break;
			case 116:
				info.mFormat = PF_A32B32G32R32F;
				break;
			case MakeFourCC<'D', 'X', 'T', '1'>::value:
				info.mFormat = PF_BC1;
				break;
			case MakeFourCC<'D', 'X', 'T', '3'>::value:
				info.mFormat = PF_BC2;
				break;
			case MakeFourCC<'D', 'X', 'T', '5'>::value:
				info.mFormat = PF_BC3;
				break;
			case MakeFourCC<'B', 'C', '4', 'U'>::value:
			case MakeFourCC<'A', 'T', 'I', '1'>::value:
				info.mFormat = PF_BC4;
				break;
			case MakeFourCC<'B', 'C', '4', 'S'>::value:
				info.mFormat = PF_BC4_SNORM;
				break;
			case MakeFourCC<'B', 'C', '5', 'U'>::value:
			case MakeFourCC<'A', 'T', 'I', '2'>::value:
				info.mFormat = PF_BC5;
				break;
			case MakeFourCC<'B', 'C', '5', 'S'>::value:
				info.mFormat = PF_BC5_SNORM;
				break;
			case MakeFourCC<'D', 'x', '1', '0'>::value:
				info.mFormat = fromDXGIFormat(desc10.dxgi_format);
				break;
			default:
				BOOST_ASSERT(false);
			}
		}
		else
		{
			if ((desc.pixel_format.flags & DDSPF_RGB) != 0)
			{
				switch (desc.pixel_format.rgb_bit_count)
				{
				case 16:
					if ((0xF000 == desc.pixel_format.rgb_alpha_bit_mask) && (0x0F00 == desc.pixel_format.r_bit_mask) && (0x00F0 == desc.pixel_format.g_bit_mask) && (0x000F == desc.pixel_format.b_bit_mask))
					{
						info.mFormat = PF_R4G4B4A4;
					}
					else
					{
						BOOST_ASSERT(false);
					}
					break;
				case 32:
					if ((0x00ff0000 == desc.pixel_format.r_bit_mask) && (0x0000ff00 == desc.pixel_format.g_bit_mask) && (0x000000ff == desc.pixel_format.b_bit_mask))
					{
						info.mFormat = PF_R8G8B8A8;
					}
					else
					{
						if((0xC0000000== desc.pixel_format.rgb_alpha_bit_mask)
							&& (0x000003FF == desc.pixel_format.r_bit_mask)
							&& (0x000FFC00 == desc.pixel_format.g_bit_mask)
							&& (0x3FF00000 == desc.pixel_format.b_bit_mask))
						{
							info.mFormat = PF_A2B10G10R10;
						}
						else
						{
							if ((0x000000FF == desc.pixel_format.r_bit_mask) && (0x0000FF00 == desc.pixel_format.g_bit_mask) && (0x00FF0000 == desc.pixel_format.b_bit_mask))
							{
								info.mFormat = PF_B8G8R8A8;
							}
							else
							{
								if ((0x00000000 == desc.pixel_format.rgb_alpha_bit_mask) && (0xffffFFFF == desc.pixel_format.r_bit_mask) && (0xFFFF0000 == desc.pixel_format.g_bit_mask) && (0x00000000 == desc.pixel_format.b_bit_mask))
								{
									info.mFormat = PF_G16R16;
								}
								else
								{
									BOOST_ASSERT(false);
								}
							}
						}
					}
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			else
			{
				if ((desc.pixel_format.flags & DDSPF_LUMINANCE) != 0)
				{
					switch (desc.pixel_format.rgb_bit_count)
					{
					case 8:
						if (0 == (desc.pixel_format.flags & DDSPF_ALPHAPIXELS))
						{
							info.mFormat = PF_R8;
						}
						else
						{
							BOOST_ASSERT(false);
						}
						break;
					case 16:
						if (0 == (desc.pixel_format.flags & DDSPF_ALPHAPIXELS))
						{
							info.mFormat = PF_R16_UNORM;
						}
						else
						{
							BOOST_ASSERT(false);
						}
						break;

					default:
						BOOST_ASSERT(false);
						break;
					}
				}
				else
				{
					if ((desc.pixel_format.flags & DDSPF_BUMPDUDV) != 0)
					{
						switch (desc.pixel_format.rgb_bit_count)
						{
						case 16:
							if ((0x000000FF == desc.pixel_format.r_bit_mask) && (0x0000FF00 == desc.pixel_format.g_bit_mask))
							{
								info.mFormat = PF_R8G8_SNORM;
							}
							else
							{
								if (0x0000ffff == desc.pixel_format.r_bit_mask)
								{
									info.mFormat = PF_R16_SNORM;
								}
								else
								{
									BOOST_ASSERT(false);
								}
							}
							break;
						case 32:
							if ((0x000000FF == desc.pixel_format.r_bit_mask) && (0x0000FF00 == desc.pixel_format.g_bit_mask) && (0x00FF0000 == desc.pixel_format.b_bit_mask))
							{
								info.mFormat = PF_R8G8B8A8_SNORM;
							}
							else
							{
								if ((0xC0000000 == desc.pixel_format.rgb_alpha_bit_mask) && (0x000003FF == desc.pixel_format.r_bit_mask) && (0x000FFC00 == desc.pixel_format.g_bit_mask) && (0x3FF00000 == desc.pixel_format.b_bit_mask))
								{
									info.mFormat = PF_A2B10G10R10;
								}
								else
								{
									if ((0x00000000 == desc.pixel_format.rgb_alpha_bit_mask) && (0x0000FFFF == desc.pixel_format.r_bit_mask) && (0xFFFF0000) && (0x00000000 == desc.pixel_format.b_bit_mask))
									{
										info.mFormat = PF_G16R16;
									}
									else
									{
										BOOST_ASSERT(false);
									}
								}
							}
							break;
						default:
							BOOST_ASSERT(false);
							break;
						}
					}
					else
					{
						if ((desc.pixel_format.flags & DDSPF_ALPHA) != 0)
						{
							info.mFormat = PF_A8;
						}
						else
						{
							BOOST_ASSERT(false);
						}
					}
				}
			}
		}
		if ((desc.flags & DDSD_PITCH) != 0)
		{
			info.mRowPitch = desc.pitch;

		}
		else
		{
			if ((desc.flags & desc.pixel_format.flags & 0x00000040) != 0)
			{
				info.mRowPitch = desc.width * desc.pixel_format.rgb_bit_count / 8;
			}
			else
			{
				info.mRowPitch = desc.width * GPixelFormats[info.mFormat].BlockBytes;
			}
		}
		info.mSlicePitch = info.mRowPitch * desc.height;
		if (desc.reserved1[0] != 0)
		{
			BOOST_ASSERT(false);//srgb;
		}

		info.mWidth = desc.width;
		info.mNumMipmaps = desc.mip_map_count;

		if ((MakeFourCC<'D', 'X', '1', '0'>::value == desc.pixel_format.four_cc))
		{
			if (D3D_RESOURCE_MISC_TEXTURECUBE == desc10.misc_flag)
			{
				info.mType = RTexture::TT_Cube;
				info.mArraySize /= 6;
				info.mHeight = desc.width;
				info.mDepth = 1;
			}
			else
			{
				switch (desc10.resource_dim)
				{
				case D3D_RESOURCE_DIMENSION_TEXTURE1D:
					info.mType = RTexture::TT_1D;
					info.mHeight = 1;
					info.mDepth = 1;
					break;
				case D3D_RESOURCE_DIMENSION_TEXTURE2D:
					info.mType = RTexture::TT_2D;
					info.mHeight = desc.height;
					info.mDepth = 1;
					break;
				case D3D_RESOURCE_DIMENSION_TEXTURE3D:
					info.mType = RTexture::TT_3D;
					info.mHeight = desc.height;
					info.mDepth = desc.depth;
					break;
				default:
					BOOST_ASSERT(false);
					break;
				}
			}
		}
		else
		{
			if ((desc.dds_caps.caps2 & DDSCAPS2_CUBEMAP) != 0)
			{
				info.mType = RTexture::TT_Cube;
				info.mHeight = desc.width;
				info.mDepth = 1;
			}
			else
			{
				if ((desc.dds_caps.caps2&DDSCAPS2_VOLUME) != 0)
				{
					info.mType = RTexture::TT_3D;
					info.mHeight = desc.height;
					info.mDepth = desc.depth;
				}
				else
				{
					info.mType = RTexture::TT_2D;
					info.mHeight = desc.height;
					info.mDepth = 1;
				}
			}
		}
		info.mHeaderSize = file.tell();
		return info;
	}

	TextureImporterDDS* TextureImporterDDS::mInstance = new TextureImporterDDS();
}