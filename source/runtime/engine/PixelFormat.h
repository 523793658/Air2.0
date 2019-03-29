#pragma once
namespace Air
{
#if defined(PF_MAX)
#undef PF_MAX
#endif

	enum EPixelFormat
	{
		PF_Unknown = 0,
		PF_A32B32G32R32F,
		PF_B8G8R8A8,
		PF_B8G8R8A8_SRGB,
		PF_G8,
		PF_G16,
		PF_BC1,
		PF_BC2,
		PF_BC3,
		PF_UYVY,
		PF_FloatRGB,
		PF_FloatRGBA,
		PF_DepthStencil,
		PF_ShadowDepth,
		PF_R32_FLOAT,
		PF_G16R16,
		PF_G16R16F,
		PF_G16R16F_FILTER,
		PF_G32R32F,
		PF_A2B10G10R10,
		PF_A2B10G10R10_UINT,
		PF_A16B16G16R16,
		PF_A16B16G16R16_SINT,
		PF_A16B16G16R16F,
		PF_D24,
		PF_R16_UNORM,
		PF_R16F,
		PF_R16F_FILTER,
		PF_BC5,
		PF_BC5_SNORM,
		PF_V8U8,
		PF_A1,
		PF_R8,
		PF_FloatR11G11B10,
		PF_A8,
		PF_R32_UINT,
		PF_R32_SINT,
		PF_PVRTC2,
		PF_PVRTC4,
		PF_R16_UINT,
		PF_R16_SINT,
		PF_R16_SNORM,
		PF_R16G16B16A16_UINT,
		PF_R16G16B16A16_SINT,
		PF_R5G6B5_UNORM,
		PF_R5G5B5A1_UNORM,
		PF_R8G8B8A8,
		PF_R8G8B8A8_SNORM,
		PF_R8G8B8A8_SINT,
		PF_BC4,
		PF_BC4_SNORM,
		PF_R8G8,
		PF_R8G8_SNORM,
		PF_ATC_RGB,
		PF_ATC_RGBA_E,
		PF_ATC_RGBA_I ,
		PF_X24_G8,	// Used for creating SRVs to alias a DepthStencil buffer to read Stencil. Don't use for creating textures.
		PF_ETC1,
		PF_ETC2_RGB,
		PF_ETC2_RGBA,
		PF_R32G32B32A32_UINT,
		PF_R16G16_UINT,
		PF_R16G16_FLOAT,
		PF_ASTC_4x4,	// 8.00 bpp
		PF_ASTC_6x6,	// 3.56 bpp
		PF_ASTC_8x8,	// 2.00 bpp
		PF_ASTC_10x10,	// 1.28 bpp
		PF_ASTC_12x12,	// 0.89 bpp
		PF_BC6H,
		PF_BC7,
		PF_R8_UINT,
		PF_R8_SNORM,
		PF_BC1_SRGB,
		PF_BC2_SRGB ,
		PF_BC3_SRGB ,
		PF_BC4_SRGB ,
		PF_BC5_SRGB,
		PF_BC7_SRGB ,
		PF_R4G4B4A4,
		PF_MAX,
	};

	struct ElementInitData
	{
		void const * mData;
		uint32_t mRowPitch;
		uint32_t mSlicePitch;
	};
}