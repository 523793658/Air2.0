#include "dds.h"
#include <texture.h>

/* DDS文件头的字节大小 */
#define DDS_HEAD_SIZE 128

/* 产生FourCC常量 */
#define MAKE_FOUR_CC(ch0, ch1, ch2, ch3) ( ((ch0) << 0) + ((ch1) << 8) + ((ch2) << 16) + ((ch3) << 24) )

enum
{
	// The surface has alpha channel information in the pixel format.
	DDSPF_ALPHAPIXELS = 0x00000001,

	// The pixel format contains alpha only information
	DDSPF_ALPHA = 0x00000002,

	// The FourCC code is valid.
	DDSPF_FOURCC = 0x00000004,

	// The RGB data in the pixel format structure is valid.
	DDSPF_RGB = 0x00000040,

	// Luminance data in the pixel format is valid.
	// Use this flag for luminance-only or luminance+alpha surfaces,
	// the bit depth is then ddpf.dwLuminanceBitCount.
	DDSPF_LUMINANCE = 0x00020000,

	// Bump map dUdV data in the pixel format is valid.
	DDSPF_BUMPDUDV = 0x00080000
};

typedef struct
{
	uint	size;				// size of structure
	uint	flags;				// pixel format flags
	uint	four_cc;			// (FOURCC code)
	uint	rgb_bit_count;		// how many bits per pixel
	uint	r_bit_mask;			// mask for red bit
	uint	g_bit_mask;			// mask for green bits
	uint	b_bit_mask;			// mask for blue bits
	uint	rgb_alpha_bit_mask;	// mask for alpha channels
}DDSPIXELFORMAT;

enum
{
	// Indicates a complex surface structure is being described.  A
	// complex surface structure results in the creation of more than
	// one surface.  The additional surfaces are attached to the root
	// surface.  The complex structure can only be destroyed by
	// destroying the root.
	DDSCAPS_COMPLEX		= 0x00000008,

	// Indicates that this surface can be used as a 3D texture.  It does not
	// indicate whether or not the surface is being used for that purpose.
	DDSCAPS_TEXTURE		= 0x00001000,

	// Indicates surface is one level of a mip-map. This surface will
	// be attached to other DDSCAPS_MIPMAP surfaces to form the mip-map.
	// This can be done explicitly, by creating a number of surfaces and
	// attaching them with AddAttachedSurface or by implicitly by CreateSurface.
	// If this bit is set then DDSCAPS_TEXTURE must also be set.
	DDSCAPS_MIPMAP		= 0x00400000,
};

enum
{
	// This flag is used at CreateSurface time to indicate that this set of
	// surfaces is a cubic environment map
	DDSCAPS2_CUBEMAP	= 0x00000200,

	// These flags preform two functions:
	// - At CreateSurface time, they define which of the six cube faces are
	//   required by the application.
	// - After creation, each face in the cubemap will have exactly one of these
	//   bits set.
	DDSCAPS2_CUBEMAP_POSITIVEX	= 0x00000400,
	DDSCAPS2_CUBEMAP_NEGATIVEX	= 0x00000800,
	DDSCAPS2_CUBEMAP_POSITIVEY	= 0x00001000,
	DDSCAPS2_CUBEMAP_NEGATIVEY	= 0x00002000,
	DDSCAPS2_CUBEMAP_POSITIVEZ	= 0x00004000,
	DDSCAPS2_CUBEMAP_NEGATIVEZ	= 0x00008000,

	// Indicates that the surface is a volume.
	// Can be combined with DDSCAPS_MIPMAP to indicate a multi-level volume
	DDSCAPS2_VOLUME		= 0x00200000,
};

typedef struct 
{
	uint	caps1;			// capabilities of surface wanted
	uint	caps2;
	uint	reserved[2];
}DDSCAPS2;

enum
{
	DDSD_CAPS			= 0x00000001,	// default, dds_caps field is valid.
	DDSD_HEIGHT			= 0x00000002,	// height field is valid.
	DDSD_WIDTH			= 0x00000004,	// width field is valid.
	DDSD_PITCH			= 0x00000008,	// pitch is valid.
	DDSD_PIXELFORMAT	= 0x00001000,	// pixel_format is valid.
	DDSD_MIPMAPCOUNT	= 0x00020000,	// mip_map_count is valid.
	DDSD_LINEARSIZE		= 0x00080000,	// linear_size is valid
	DDSD_DEPTH			= 0x00800000,	// depth is valid
};

typedef struct 
{
	uint	size;					// size of the DDSURFACEDESC structure
	uint	flags;					// determines what fields are valid
	uint	height;					// height of surface to be created
	uint	width;					// width of input surface
	union
	{
		sint	pitch;				// distance to start of next line (return value only)
		uint	linear_size;		// Formless late-allocated optimized surface size
	};
	uint		depth;				// the depth if this is a volume texture
	uint		mip_map_count;		// number of mip-map levels requestde
	uint		reserved1[11];		// reserved
	DDSPIXELFORMAT	pixel_format;		// pixel format description of the surface
	DDSCAPS2		dds_caps;			// direct draw surface capabilities
	uint		reserved2;
}DDSSURFACEDESC2;

typedef enum 
{
	D3D10_RESOURCE_DIMENSION_UNKNOWN = 0,
	D3D10_RESOURCE_DIMENSION_BUFFER = 1,
	D3D10_RESOURCE_DIMENSION_TEXTURE1D = 2,
	D3D10_RESOURCE_DIMENSION_TEXTURE2D = 3,
	D3D10_RESOURCE_DIMENSION_TEXTURE3D = 4,
}D3D10_RESOURCE_DIMENSION;

typedef enum 
{
	D3D10_RESOURCE_MISC_GENERATE_MIPS = 0x1L,
	D3D10_RESOURCE_MISC_SHARED = 0x2L,
	D3D10_RESOURCE_MISC_TEXTURECUBE	= 0x4L,
	D3D10_RESOURCE_MISC_SHARED_KEYEDMUTEX = 0x10L,
	D3D10_RESOURCE_MISC_GDI_COMPATIBLE = 0x20L
}D3D10_RESOURCE_MISC_FLAG;

typedef struct 
{
	uint dxgi_format;
	D3D10_RESOURCE_DIMENSION resource_dim;
	uint misc_flag;
	uint array_size;
	uint reserved;
}DDS_HEADER_DXT10;

#ifndef DXGI_FORMAT_DEFINED
typedef enum 
{
	DXGI_FORMAT_UNKNOWN	                    = 0,
	DXGI_FORMAT_R32G32B32A32_TYPELESS       = 1,
	DXGI_FORMAT_R32G32B32A32_FLOAT          = 2,
	DXGI_FORMAT_R32G32B32A32_UINT           = 3,
	DXGI_FORMAT_R32G32B32A32_SINT           = 4,
	DXGI_FORMAT_R32G32B32_TYPELESS          = 5,
	DXGI_FORMAT_R32G32B32_FLOAT             = 6,
	DXGI_FORMAT_R32G32B32_UINT              = 7,
	DXGI_FORMAT_R32G32B32_SINT              = 8,
	DXGI_FORMAT_R16G16B16A16_TYPELESS       = 9,
	DXGI_FORMAT_R16G16B16A16_FLOAT          = 10,
	DXGI_FORMAT_R16G16B16A16_UNORM          = 11,
	DXGI_FORMAT_R16G16B16A16_UINT           = 12,
	DXGI_FORMAT_R16G16B16A16_SNORM          = 13,
	DXGI_FORMAT_R16G16B16A16_SINT           = 14,
	DXGI_FORMAT_R32G32_TYPELESS             = 15,
	DXGI_FORMAT_R32G32_FLOAT                = 16,
	DXGI_FORMAT_R32G32_UINT                 = 17,
	DXGI_FORMAT_R32G32_SINT                 = 18,
	DXGI_FORMAT_R32G8X24_TYPELESS           = 19,
	DXGI_FORMAT_D32_FLOAT_S8X24_UINT        = 20,
	DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS    = 21,
	DXGI_FORMAT_X32_TYPELESS_G8X24_UINT     = 22,
	DXGI_FORMAT_R10G10B10A2_TYPELESS        = 23,
	DXGI_FORMAT_R10G10B10A2_UNORM           = 24,
	DXGI_FORMAT_R10G10B10A2_UINT            = 25,
	DXGI_FORMAT_R11G11B10_FLOAT             = 26,
	DXGI_FORMAT_R8G8B8A8_TYPELESS           = 27,
	DXGI_FORMAT_R8G8B8A8_UNORM              = 28,
	DXGI_FORMAT_R8G8B8A8_UNORM_SRGB         = 29,
	DXGI_FORMAT_R8G8B8A8_UINT               = 30,
	DXGI_FORMAT_R8G8B8A8_SNORM              = 31,
	DXGI_FORMAT_R8G8B8A8_SINT               = 32,
	DXGI_FORMAT_R16G16_TYPELESS             = 33,
	DXGI_FORMAT_R16G16_FLOAT                = 34,
	DXGI_FORMAT_R16G16_UNORM                = 35,
	DXGI_FORMAT_R16G16_UINT                 = 36,
	DXGI_FORMAT_R16G16_SNORM                = 37,
	DXGI_FORMAT_R16G16_SINT                 = 38,
	DXGI_FORMAT_R32_TYPELESS                = 39,
	DXGI_FORMAT_D32_FLOAT                   = 40,
	DXGI_FORMAT_R32_FLOAT                   = 41,
	DXGI_FORMAT_R32_UINT                    = 42,
	DXGI_FORMAT_R32_SINT                    = 43,
	DXGI_FORMAT_R24G8_TYPELESS              = 44,
	DXGI_FORMAT_D24_UNORM_S8_UINT           = 45,
	DXGI_FORMAT_R24_UNORM_X8_TYPELESS       = 46,
	DXGI_FORMAT_X24_TYPELESS_G8_UINT        = 47,
	DXGI_FORMAT_R8G8_TYPELESS               = 48,
	DXGI_FORMAT_R8G8_UNORM                  = 49,
	DXGI_FORMAT_R8G8_UINT                   = 50,
	DXGI_FORMAT_R8G8_SNORM                  = 51,
	DXGI_FORMAT_R8G8_SINT                   = 52,
	DXGI_FORMAT_R16_TYPELESS                = 53,
	DXGI_FORMAT_R16_FLOAT                   = 54,
	DXGI_FORMAT_D16_UNORM                   = 55,
	DXGI_FORMAT_R16_UNORM                   = 56,
	DXGI_FORMAT_R16_UINT                    = 57,
	DXGI_FORMAT_R16_SNORM                   = 58,
	DXGI_FORMAT_R16_SINT                    = 59,
	DXGI_FORMAT_R8_TYPELESS                 = 60,
	DXGI_FORMAT_R8_UNORM                    = 61,
	DXGI_FORMAT_R8_UINT                     = 62,
	DXGI_FORMAT_R8_SNORM                    = 63,
	DXGI_FORMAT_R8_SINT                     = 64,
	DXGI_FORMAT_A8_UNORM                    = 65,
	DXGI_FORMAT_R1_UNORM                    = 66,
	DXGI_FORMAT_R9G9B9E5_SHAREDEXP          = 67,
	DXGI_FORMAT_R8G8_B8G8_UNORM             = 68,
	DXGI_FORMAT_G8R8_G8B8_UNORM             = 69,
	DXGI_FORMAT_BC1_TYPELESS                = 70,
	DXGI_FORMAT_BC1_UNORM                   = 71,
	DXGI_FORMAT_BC1_UNORM_SRGB              = 72,
	DXGI_FORMAT_BC2_TYPELESS                = 73,
	DXGI_FORMAT_BC2_UNORM                   = 74,
	DXGI_FORMAT_BC2_UNORM_SRGB              = 75,
	DXGI_FORMAT_BC3_TYPELESS                = 76,
	DXGI_FORMAT_BC3_UNORM                   = 77,
	DXGI_FORMAT_BC3_UNORM_SRGB              = 78,
	DXGI_FORMAT_BC4_TYPELESS                = 79,
	DXGI_FORMAT_BC4_UNORM                   = 80,
	DXGI_FORMAT_BC4_SNORM                   = 81,
	DXGI_FORMAT_BC5_TYPELESS                = 82,
	DXGI_FORMAT_BC5_UNORM                   = 83,
	DXGI_FORMAT_BC5_SNORM                   = 84,
	DXGI_FORMAT_B5G6R5_UNORM                = 85,
	DXGI_FORMAT_B5G5R5A1_UNORM              = 86,
	DXGI_FORMAT_B8G8R8A8_UNORM              = 87,
	DXGI_FORMAT_B8G8R8X8_UNORM              = 88,
	DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM  = 89,
	DXGI_FORMAT_B8G8R8A8_TYPELESS           = 90,
	DXGI_FORMAT_B8G8R8A8_UNORM_SRGB         = 91,
	DXGI_FORMAT_B8G8R8X8_TYPELESS           = 92,
	DXGI_FORMAT_B8G8R8X8_UNORM_SRGB         = 93,
	DXGI_FORMAT_BC6H_TYPELESS               = 94,
	DXGI_FORMAT_BC6H_UF16                   = 95,
	DXGI_FORMAT_BC6H_SF16                   = 96,
	DXGI_FORMAT_BC7_TYPELESS                = 97,
	DXGI_FORMAT_BC7_UNORM                   = 98,
	DXGI_FORMAT_BC7_UNORM_SRGB              = 99,
	DXGI_FORMAT_AYUV                        = 100,
	DXGI_FORMAT_Y410                        = 101,
	DXGI_FORMAT_Y416                        = 102,
	DXGI_FORMAT_NV12                        = 103,
	DXGI_FORMAT_P010                        = 104,
	DXGI_FORMAT_P016                        = 105,
	DXGI_FORMAT_420_OPAQUE                  = 106,
	DXGI_FORMAT_YUY2                        = 107,
	DXGI_FORMAT_Y210                        = 108,
	DXGI_FORMAT_Y216                        = 109,
	DXGI_FORMAT_NV11                        = 110,
	DXGI_FORMAT_AI44                        = 111,
	DXGI_FORMAT_IA44                        = 112,
	DXGI_FORMAT_P8                          = 113,
	DXGI_FORMAT_A8P8                        = 114,
	DXGI_FORMAT_B4G4R4A4_UNORM              = 115,
	DXGI_FORMAT_FORCE_UINT                  = 0xffffffff
}DXGI_FORMAT;
#endif /* DXGI_FORMAT_DEFINED */

static uint _get_compress_block_size(RenderFormat format)
{
	uint size = 0;
	switch(format)
	{
	case RF_BC1:
	case RF_SIGNED_BC1:
	case RF_BC1_SRGB:
	case RF_BC4:
	case RF_SIGNED_BC4:
	case RF_BC4_SRGB:
		size = 8;
		break;
	default:
		size = 16;
		break;
	}
	return size;
}

static RenderFormat _from_dxgi_format(uint format)
{
	RenderFormat r = RF_UNKNOWN;
	switch (format)
	{
	case DXGI_FORMAT_A8_UNORM:
		r = RF_A8;
		break;
	case DXGI_FORMAT_B5G6R5_UNORM:
		// r = RF_R5G6B5;
		break;
	case DXGI_FORMAT_B5G5R5A1_UNORM:
		// r = RF_A1RGB5;
		break;
	case DXGI_FORMAT_B4G4R4A4_UNORM:
		r = RF_ARGB4;
		break;
	case DXGI_FORMAT_R8_UNORM:
		r = RF_R8;
		break;
	case DXGI_FORMAT_R8_SNORM:
		r = RF_SIGNED_R8;
		break;
	case DXGI_FORMAT_R8G8_UNORM:
		r = RF_GR8;
		break;
	case DXGI_FORMAT_R8G8_SNORM:
		r = RF_SIGNED_GR8;
		break;
	case DXGI_FORMAT_R8G8B8A8_UNORM:
		r = RF_ABGR8;
		break;
	case DXGI_FORMAT_R8G8B8A8_SNORM:
		r = RF_SIGNED_ABGR8;
		break;
	case DXGI_FORMAT_R10G10B10A2_UNORM:
		r = RF_A2BGR10;
		break;
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
		r = RF_SIGNED_A2BGR10;
		break;
	case DXGI_FORMAT_R8_UINT:
		r = RF_R8UI;
		break;
	case DXGI_FORMAT_R8_SINT:
		r = RF_R8I;
		break;
	case DXGI_FORMAT_R8G8_UINT:
		r = RF_GR8UI;
		break;
	case DXGI_FORMAT_R8G8_SINT:
		r = RF_GR8I;
		break;
	case DXGI_FORMAT_R8G8B8A8_UINT:
		r = RF_ABGR8UI;
		break;
	case DXGI_FORMAT_R8G8B8A8_SINT:
		r = RF_ABGR8I;
		break;
	case DXGI_FORMAT_R10G10B10A2_UINT:
		r = RF_A2BGR10UI;
		break;
	case DXGI_FORMAT_R16_UNORM:
		r = RF_R16;
		break;
	case DXGI_FORMAT_R16_SNORM:
		r = RF_SIGNED_R16;
		break;
	case DXGI_FORMAT_R16G16_UNORM:
		r = RF_GR16;
		break;
	case DXGI_FORMAT_R16G16_SNORM:
		r = RF_SIGNED_GR16;
		break;
	case DXGI_FORMAT_R16G16B16A16_UNORM:
		r = RF_ABGR16;
		break;
	case DXGI_FORMAT_R16G16B16A16_SNORM:
		r = RF_SIGNED_ABGR16;
		break;
	case DXGI_FORMAT_R16_UINT:
		r = RF_R16UI;
		break;
	case DXGI_FORMAT_R16_SINT:
		r = RF_R16I;
		break;
	case DXGI_FORMAT_R16G16_UINT:
		r = RF_GR16UI;
		break;
	case DXGI_FORMAT_R16G16_SINT:
		r = RF_GR16I;
		break;
	case DXGI_FORMAT_R16G16B16A16_UINT:
		r = RF_ABGR16UI;
		break;
	case DXGI_FORMAT_R16G16B16A16_SINT:
		r = RF_ABGR16I;
		break;
	case DXGI_FORMAT_R32_UINT:
		r = RF_R32UI;
		break;
	case DXGI_FORMAT_R32_SINT:
		r = RF_R32I;
		break;
	case DXGI_FORMAT_R32G32_UINT:
		r = RF_GR32UI;
		break;
	case DXGI_FORMAT_R32G32_SINT:
		r = RF_GR32I;
		break;
	case DXGI_FORMAT_R32G32B32_UINT:
		r = RF_BGR32UI;
		break;
	case DXGI_FORMAT_R32G32B32_SINT:
		r = RF_BGR32I;
		break;
	case DXGI_FORMAT_R32G32B32A32_UINT:
		r = RF_ABGR32UI;
		break;
	case DXGI_FORMAT_R32G32B32A32_SINT:
		r = RF_ABGR32I;
		break;
	case DXGI_FORMAT_R16_FLOAT:
		r = RF_R16F;
		break;
	case DXGI_FORMAT_R16G16_FLOAT:
		r = RF_GR16F;
		break;
	case DXGI_FORMAT_R11G11B10_FLOAT:
		r = RF_B10G11R11F;
		break;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		r = RF_ABGR16F;
		break;
	case DXGI_FORMAT_R32_FLOAT:
		r = RF_R32F;
		break;
	case DXGI_FORMAT_R32G32_FLOAT:
		r = RF_GR32F;
		break;
	case DXGI_FORMAT_R32G32B32_FLOAT:
		r = RF_BGR32F;
		break;
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		r = RF_ABGR32F;
		break;
	case DXGI_FORMAT_BC1_UNORM:
		r = RF_BC1;
		break;
	case DXGI_FORMAT_BC2_UNORM:
		r = RF_BC2;
		break;
	case DXGI_FORMAT_BC3_UNORM:
		r = RF_BC3;
		break;
	case DXGI_FORMAT_BC4_UNORM:
		r = RF_BC4;
		break;
	case DXGI_FORMAT_BC4_SNORM:
		r = RF_SIGNED_BC4;
		break;
	case DXGI_FORMAT_BC5_UNORM:
		r = RF_BC5;
		break;
	case DXGI_FORMAT_BC5_SNORM:
		r = RF_SIGNED_BC5;
		break;
	case DXGI_FORMAT_BC6H_UF16:
		r = RF_BC6;
		break;
	case DXGI_FORMAT_BC6H_SF16:
		r = RF_SIGNED_BC6;
		break;
	case DXGI_FORMAT_BC7_UNORM:
		r = RF_BC7;
		break;
	case DXGI_FORMAT_D16_UNORM:
		r = RF_D16;
		break;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		r = RF_D24S8;
		break;
	case DXGI_FORMAT_D32_FLOAT:
		r = RF_D32F;
		break;
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		r = RF_ABGR8_SRGB;
		break;
	case DXGI_FORMAT_BC1_UNORM_SRGB:
		r = RF_BC1_SRGB;
		break;
	case DXGI_FORMAT_BC2_UNORM_SRGB:
		r = RF_BC2_SRGB;
		break;
	case DXGI_FORMAT_BC3_UNORM_SRGB:
		r = RF_BC3_SRGB;
		break;
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		r = RF_BC7_SRGB;
		break;
	case DXGI_FORMAT_B8G8R8A8_UNORM:
		break;
	default:
		break;
	}
	return r;
}

static DXGI_FORMAT _to_dxgi_format(RenderFormat format)
{
	DXGI_FORMAT r = DXGI_FORMAT_UNKNOWN;
	switch (format)
	{
	case RF_A8:
		r = DXGI_FORMAT_A8_UNORM;
		break;
	// case RF_R5G6B5:
		// r = DXGI_FORMAT_B5G6R5_UNORM;
	// 	break;
	// case RF_A1RGB5:
	// 	r = DXGI_FORMAT_B5G5R5A1_UNORM;
	// 	break;
	case RF_ARGB4:
		r = DXGI_FORMAT_B4G4R4A4_UNORM;
		break;
	case RF_R8:
		r = DXGI_FORMAT_R8_UNORM;
		break;
	case RF_SIGNED_R8:
		r = DXGI_FORMAT_R8_SNORM;
		break;
	case RF_GR8:
		r = DXGI_FORMAT_R8G8_UNORM;
		break;
	case RF_SIGNED_GR8:
		r = DXGI_FORMAT_R8G8_SNORM;
		break;
	case RF_ABGR8:
		r = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case RF_SIGNED_ABGR8:
		r = DXGI_FORMAT_R8G8B8A8_SNORM;
		break;
	case RF_A2BGR10:
		r = DXGI_FORMAT_R10G10B10A2_UNORM;
		break;
	case RF_SIGNED_A2BGR10:
		r = DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
		break;
	case RF_R8UI:
		r = DXGI_FORMAT_R8_UINT;
		break;
	case RF_R8I:
		r = DXGI_FORMAT_R8_SINT;
		break;
	case RF_GR8UI:
		r = DXGI_FORMAT_R8G8_UINT;
		break;
	case RF_GR8I:
		r = DXGI_FORMAT_R8G8_SINT;
		break;
	case RF_ABGR8UI:
		r = DXGI_FORMAT_R8G8B8A8_UINT;
		break;
	case RF_ABGR8I:
		r = DXGI_FORMAT_R8G8B8A8_SINT;
		break;
	case RF_A2BGR10UI:
		r = DXGI_FORMAT_R10G10B10A2_UINT;
		break;
	case RF_R16:
		r = DXGI_FORMAT_R16_UNORM;
		break;
	case RF_SIGNED_R16:
		r = DXGI_FORMAT_R16_SNORM;
		break;
	case RF_GR16:
		r = DXGI_FORMAT_R16G16_UNORM;
		break;
	case RF_SIGNED_GR16:
		r = DXGI_FORMAT_R16G16_SNORM;
		break;
	case RF_ABGR16:
		r = DXGI_FORMAT_R16G16B16A16_UNORM;
		break;
	case RF_SIGNED_ABGR16:
		r = DXGI_FORMAT_R16G16B16A16_SNORM;
		break;
	case RF_R16UI:
		r = DXGI_FORMAT_R16_UINT;
		break;
	case RF_R16I:
		r = DXGI_FORMAT_R16_SINT;
		break;
	case RF_GR16UI:
		r = DXGI_FORMAT_R16G16_UINT;
		break;
	case RF_GR16I:
		r = DXGI_FORMAT_R16G16_SINT;
		break;
	case RF_ABGR16UI:
		r = DXGI_FORMAT_R16G16B16A16_UINT;
		break;
	case RF_ABGR16I:
		r = DXGI_FORMAT_R16G16B16A16_SINT;
		break;
	case RF_R32UI:
		r = DXGI_FORMAT_R32_UINT;
		break;
	case RF_R32I:
		r = DXGI_FORMAT_R32_SINT;
		break;
	case RF_GR32UI:
		r = DXGI_FORMAT_R32G32_UINT;
		break;
	case RF_GR32I:
		r = DXGI_FORMAT_R32G32_SINT;
		break;
	case RF_BGR32UI:
		r = DXGI_FORMAT_R32G32B32_UINT;
		break;
	case RF_BGR32I:
		r = DXGI_FORMAT_R32G32B32_SINT;
		break;
	case RF_ABGR32UI:
		r = DXGI_FORMAT_R32G32B32A32_UINT;
		break;
	case RF_ABGR32I:
		r = DXGI_FORMAT_R32G32B32A32_SINT;
		break;
	case RF_R16F:
		r = DXGI_FORMAT_R16_FLOAT;
		break;
	case RF_GR16F:
		r = DXGI_FORMAT_R16G16_FLOAT;
		break;
	case RF_B10G11R11F:
		r = DXGI_FORMAT_R11G11B10_FLOAT;
		break;
	case RF_ABGR16F:
		r = DXGI_FORMAT_R16G16B16A16_FLOAT;
		break;
	case RF_R32F:
		r = DXGI_FORMAT_R32_FLOAT;
		break;
	case RF_GR32F:
		r = DXGI_FORMAT_R32G32_FLOAT;
		break;
	case RF_BGR32F:
		r = DXGI_FORMAT_R32G32B32_FLOAT;
		break;
	case RF_ABGR32F:
		r = DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	case RF_BC1:
		r = DXGI_FORMAT_BC1_UNORM;
		break;
	case RF_BC2:
		r = DXGI_FORMAT_BC2_UNORM;
		break;
	case RF_BC3:
		r = DXGI_FORMAT_BC3_UNORM;
		break;
	case RF_BC4:
		r = DXGI_FORMAT_BC4_UNORM;
		break;
	case RF_SIGNED_BC4:
		r = DXGI_FORMAT_BC4_SNORM;
		break;
	case RF_BC5:
		r = DXGI_FORMAT_BC5_UNORM;
		break;
	case RF_SIGNED_BC5:
		r = DXGI_FORMAT_BC5_SNORM;
		break;
	case RF_D16:
		r = DXGI_FORMAT_D16_UNORM;
		break;
	case RF_D24S8:
		r = DXGI_FORMAT_D24_UNORM_S8_UINT;
		break;
	case RF_D32F:
		r = DXGI_FORMAT_D32_FLOAT;
		break;
	case RF_ABGR8_SRGB:
		r = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		break;
	case RF_BC1_SRGB:
		r = DXGI_FORMAT_BC1_UNORM_SRGB;
		break;
	case RF_BC2_SRGB:
		r = DXGI_FORMAT_BC2_UNORM_SRGB;
		break;
	case RF_BC3_SRGB:
		r = DXGI_FORMAT_BC3_UNORM_SRGB;
		break;
	default:
		break;
	}
	return r;
}

PiBool PI_API dds_is_valid(byte *data, uint size)
{
	return size > DDS_HEAD_SIZE
		&& data[0] == 'D'
		&& data[1] == 'D' 
		&& data[2] == 'S'
		&& data[3] == ' ';
}

static PiBool _dds_get_info(PiImage *image, PiBytes *bb)
{
	uint magic;
	DDSSURFACEDESC2 desc;
	DDS_HEADER_DXT10 desc10;

	pi_bytes_read_int(bb, (sint *)&magic);
	if(magic != MAKE_FOUR_CC('D', 'D', 'S', ' '))
	{
		return FALSE;
	}
	{
		void *p;
		pi_bytes_read_data(bb, &p, sizeof(desc));
		pi_memcpy_inline(&desc, p, sizeof(desc));
	}
		
	if (MAKE_FOUR_CC('D', 'X', '1', '0') == desc.pixel_format.four_cc)
	{
		void *p;
		pi_bytes_read_data(bb, &p, sizeof(desc10));
		pi_memcpy_inline(&desc10, p, sizeof(desc10));
		image->array_size = desc10.array_size;
	}
	else
	{
		pi_memset_inline(&desc10, 0, sizeof(desc10));
		image->array_size = 1;

		PI_ASSERT((desc.flags & DDSD_CAPS) != 0, "");
		PI_ASSERT((desc.flags & DDSD_PIXELFORMAT) != 0, "");
	}

	PI_ASSERT((desc.flags & DDSD_WIDTH) != 0, "");
	PI_ASSERT((desc.flags & DDSD_HEIGHT) != 0, "");

	if (0 == (desc.flags & DDSD_MIPMAPCOUNT))
	{
		desc.mip_map_count = 1;
	}

	image->format = RF_ABGR8;
	if ((desc.pixel_format.flags & DDSPF_FOURCC) != 0)
	{/* From "Programming Guide for DDS", http://msdn.microsoft.com/en-us/library/bb943991.aspx */
		switch (desc.pixel_format.four_cc)
		{
		case 36:
			image->format = RF_ABGR16;
			break;
		case 110:
			image->format = RF_SIGNED_ABGR16;
			break;
		case 111:
			image->format = RF_R16F;
			break;
		case 112:
			image->format = RF_GR16F;
			break;
		case 113:
			image->format = RF_ABGR16F;
			break;
		case 114:
			image->format = RF_R32F;
			break;
		case 115:
			image->format = RF_GR32F;
			break;
		case 116:
			image->format = RF_ABGR32F;
			break;
		case MAKE_FOUR_CC('D', 'X', 'T', '1'):
			image->format = RF_BC1;
			break;
		case MAKE_FOUR_CC('D', 'X', 'T', '3'):
			image->format = RF_BC2;
			break;
		case MAKE_FOUR_CC('D', 'X', 'T', '5'):
			image->format = RF_BC3;
			break;
		case MAKE_FOUR_CC('B', 'C', '4', 'U'):
		case MAKE_FOUR_CC('A', 'T', 'I', '1'):
			image->format = RF_BC4;
			break;
		case MAKE_FOUR_CC('B', 'C', '4', 'S'):
			image->format = RF_SIGNED_BC4;
			break;
		case MAKE_FOUR_CC('A', 'T', 'I', '2'):
			image->format = RF_BC5;
			break;
		case MAKE_FOUR_CC('B', 'C', '5', 'S'):
			image->format = RF_SIGNED_BC5;
			break;
		case MAKE_FOUR_CC('D', 'X', '1', '0'):
			image->format = _from_dxgi_format(desc10.dxgi_format);
			break;
		default:
			break;
		}
	}
	else
	{
		if ((desc.pixel_format.flags & DDSPF_RGB) != 0)
		{
			switch (desc.pixel_format.rgb_bit_count)
			{
			case 16:
				if ((0xF000 == desc.pixel_format.rgb_alpha_bit_mask)
					&& (0x0F00 == desc.pixel_format.r_bit_mask)
					&& (0x00F0 == desc.pixel_format.g_bit_mask)
					&& (0x000F == desc.pixel_format.b_bit_mask))
				{
					image->format = RF_ARGB4;
				}
				else
				{
					PI_ASSERT(FALSE, "");
				}
				break;

			case 32:
				if ((0x00FF0000 == desc.pixel_format.r_bit_mask)
					&& (0x0000FF00 == desc.pixel_format.g_bit_mask)
					&& (0x000000FF == desc.pixel_format.b_bit_mask))
				{
					PI_ASSERT(FALSE, "");
				}
				else
				{
					if ((0xC0000000 == desc.pixel_format.rgb_alpha_bit_mask)
						&& (0x000003FF == desc.pixel_format.r_bit_mask)
						&& (0x000FFC00 == desc.pixel_format.g_bit_mask)
						&& (0x3FF00000 == desc.pixel_format.b_bit_mask))
					{
						image->format = RF_A2BGR10;
					}
					else
					{
						if ((0xFF000000 == desc.pixel_format.rgb_alpha_bit_mask)
							&& (0x000000FF == desc.pixel_format.r_bit_mask)
							&& (0x0000FF00 == desc.pixel_format.g_bit_mask)
							&& (0x00FF0000 == desc.pixel_format.b_bit_mask))
						{
							image->format = RF_ABGR8;
						}
						else
						{
							if ((0x00000000 == desc.pixel_format.rgb_alpha_bit_mask)
								&& (0x0000FFFF == desc.pixel_format.r_bit_mask)
								&& (0xFFFF0000 == desc.pixel_format.g_bit_mask)
								&& (0x00000000 == desc.pixel_format.b_bit_mask))
							{
								image->format = RF_GR16;
							}
							else
							{
								PI_ASSERT(FALSE, "");
							}
						}
					}
				}
				break;
			default:
				PI_ASSERT(FALSE, "");
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
						image->format = RF_R8;
					}
					else
					{
						PI_ASSERT(FALSE, "");
					}
					break;

				case 16:
					if (0 == (desc.pixel_format.flags & DDSPF_ALPHAPIXELS))
					{
						image->format = RF_R16;
					}
					else
					{
						PI_ASSERT(FALSE, "");
					}
					break;

				default:
					PI_ASSERT(FALSE, "");
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
						if ((0x000000FF == desc.pixel_format.r_bit_mask)
							&& (0x0000FF00 == desc.pixel_format.g_bit_mask))
						{
							image->format = RF_SIGNED_GR8;
						}
						else
						{
							if (0x0000FFFF == desc.pixel_format.r_bit_mask)
							{
								image->format = RF_SIGNED_R16;
							}
							else
							{
								PI_ASSERT(FALSE, "");
							}
						}
						break;

					case 32:
						if ((0x000000FF == desc.pixel_format.r_bit_mask)
							&& (0x0000FF00 == desc.pixel_format.g_bit_mask)
							&& (0x00FF0000 == desc.pixel_format.b_bit_mask))
						{
							image->format = RF_SIGNED_ABGR8;
						}
						else
						{
							if ((0xC0000000 == desc.pixel_format.rgb_alpha_bit_mask)
								&& (0x000003FF == desc.pixel_format.r_bit_mask)
								&& (0x000FFC00 == desc.pixel_format.g_bit_mask)
								&& (0x3FF00000 == desc.pixel_format.b_bit_mask))
							{
								image->format = RF_SIGNED_A2BGR10;
							}
							else
							{
								if ((0x00000000 == desc.pixel_format.rgb_alpha_bit_mask)
									&& (0x0000FFFF == desc.pixel_format.r_bit_mask)
									&& (0xFFFF0000 == desc.pixel_format.g_bit_mask)
									&& (0x00000000 == desc.pixel_format.b_bit_mask))
								{
									image->format = RF_SIGNED_GR16;
								}
								else
								{
									PI_ASSERT(FALSE, "");
								}
							}
						}
						break;

					default:
						PI_ASSERT(FALSE, "");
						break;
					}
				}
				else
				{
					if ((desc.pixel_format.flags & DDSPF_ALPHA) != 0)
					{
						image->format = RF_A8;
					}
					else
					{
						PI_ASSERT(FALSE, "");
					}
				}
			}
		}
	}

	if ((desc.flags & DDSD_PITCH) != 0)
	{
		image->row_pitch = desc.pitch;
	}
	else
	{
		if ((desc.flags & desc.pixel_format.flags & 0x00000040) != 0)
		{
			image->row_pitch = desc.width * desc.pixel_format.rgb_bit_count / 8;
		}
		else
		{
			image->row_pitch = desc.width * pi_renderformat_get_numbytes(image->format);
		}
	}
	image->slice_pitch = image->row_pitch * desc.height;

	if (desc.reserved1[0] != 0)
	{
		image->format = pi_renderformat_make_srgb(image->format);
	}

	image->width = desc.width;
	image->num_mipmap = desc.mip_map_count;

	if (MAKE_FOUR_CC('D', 'X', '1', '0') == desc.pixel_format.four_cc)
	{
		if (D3D10_RESOURCE_MISC_TEXTURECUBE == desc10.misc_flag)
		{
			image->type = TT_CUBE;
			image->array_size /= 6;
			image->height = desc.width;
			image->depth = 1;
		}
		else
		{
			switch (desc10.resource_dim)
			{
			case D3D10_RESOURCE_DIMENSION_TEXTURE2D:
				image->type = TT_2D;
				image->height = desc.height;
				image->depth = 1;
				break;

			case D3D10_RESOURCE_DIMENSION_TEXTURE3D:
				image->type = TT_3D;
				image->height = desc.height;
				image->depth = desc.depth;
				break;

			default:
				PI_ASSERT(FALSE, "");
				break;
			}
		}	
	}
	else
	{
		if ((desc.dds_caps.caps2 & DDSCAPS2_CUBEMAP) != 0)
		{
			image->type = TT_CUBE;
			image->height = desc.width;
			image->depth = 1;
		}
		else
		{
			if ((desc.dds_caps.caps2 & DDSCAPS2_VOLUME) != 0)
			{
				image->type = TT_3D;
				image->height = desc.height;
				image->depth = desc.depth;
			}
			else
			{
				image->type = TT_2D;
				image->height = desc.height;
				image->depth = 1;
			}
		}
	}
	return TRUE;
}

static PiBool _load_1d_image(PiImage *image, PiBytes *bb, PiBool is_padding)
{
	uint array_index;
	for (array_index = 0; array_index < image->array_size; ++array_index)
	{
		uint level, the_width = image->width;
		for (level = 0; level < image->num_mipmap; ++ level)
		{
			uint image_size;
			if (pi_renderformat_is_compressed_format(image->format))
			{
				uint block_size = _get_compress_block_size(image->format);
				image_size = ((the_width + 3) / 4) * block_size;
			}
			else
			{
				uint fmt_size = pi_renderformat_get_numbytes(image->format);
				image_size = (is_padding ? ((the_width + 3) & ~3) : the_width) * fmt_size;
			}

			{
				void *p = NULL;
				ImageInitData idata;
				idata.row_pitch = image_size;
				idata.slice_pitch = image_size;
				idata.size = image_size;
				idata.data = pi_malloc0(idata.size);
				if(!pi_bytes_read_data(bb, &p, image_size))
				{
					PI_ASSERT(FALSE, "read 1d image data failed");
					return FALSE;
				}
				pi_memcpy_inline(idata.data, p, image_size);
				pi_dvector_push(&image->data_vector, &idata);
			}
			the_width = MAX(1, the_width / 2);
		}
	}
	return TRUE;
}

static PiBool _load_2d_image(PiImage *image, PiBytes *bb, PiBool is_padding)
{
	uint array_index;
	for (array_index = 0; array_index < image->array_size; ++ array_index)
	{
		uint level;
		uint the_width = image->width;
		uint the_height = image->height;
		for (level = 0; level < image->num_mipmap; ++level)
		{
			void *p = NULL;
			ImageInitData idata;
			if (pi_renderformat_is_compressed_format(image->format))
			{
				uint block_size = _get_compress_block_size(image->format);
				idata.row_pitch = (the_width + 3) / 4 * block_size;
				idata.size = idata.slice_pitch = ((the_width + 3) / 4) * ((the_height + 3) / 4) * block_size;
			}
			else
			{
				uint fmt_size = pi_renderformat_get_numbytes(image->format);
				idata.row_pitch = (is_padding ? ((the_width + 3) & ~3) : the_width) * fmt_size;
				idata.size = idata.slice_pitch = idata.row_pitch * the_height;
			}

			idata.data = pi_malloc0(idata.size);
			if(!pi_bytes_read_data(bb, &p, idata.size))
			{
				PI_ASSERT(FALSE, "read 2d image data failed");
				return FALSE;
			}
			pi_memcpy_inline(idata.data, p, idata.size);
			pi_dvector_push(&image->data_vector, &idata);
			
			the_width = MAX(1, the_width / 2);
			the_height = MAX(1, the_height / 2);
		}
	}
	return TRUE;
}

static PiBool _load_3d_image(PiImage *image, PiBytes *bb, PiBool is_padding)
{
	uint level;
	uint the_width = image->width;
	uint the_height = image->height;
	uint the_depth = image->depth;
	for (level = 0; level < image->num_mipmap; ++ level)
	{
		void *p;
		ImageInitData idata;
		if (pi_renderformat_is_compressed_format(image->format))
		{
			uint block_size = _get_compress_block_size(image->format);
			idata.size = ((the_width + 3) / 4) * ((the_height + 3) / 4) * the_depth * block_size;
			
			idata.row_pitch = (the_width + 3) / 4 * block_size;
			idata.slice_pitch = ((the_width + 3) / 4) * ((the_height + 3) / 4) * block_size;
		}
		else
		{
			uint fmt_size = pi_renderformat_get_numbytes(image->format);
			idata.row_pitch = (is_padding ? ((the_width + 3) & ~3) : the_width) * fmt_size;
			idata.slice_pitch = idata.row_pitch * the_height;
			idata.size = idata.slice_pitch * the_depth;
		}
		
		idata.data = pi_malloc0(idata.size);
		if(!pi_bytes_read_data(bb, &p, idata.size))
		{
			PI_ASSERT(FALSE, "read 3d image data failed");
			return FALSE;
		}
		pi_memcpy_inline(idata.data, p, idata.size);
		pi_dvector_push(&image->data_vector, &idata);

		the_width = MAX(1, the_width / 2);
		the_height = MAX(1, the_height / 2);
		the_depth = MAX(1, the_depth / 2);
	}
	return TRUE;
}

static PiBool _load_cube_image(PiImage *image, PiBytes *bb, PiBool is_padding)
{
	uint face;
	for (face = CF_POSITIVE_X; face <= CF_NEGATIVE_Z; ++face)
	{
		uint level;
		uint the_width = image->width;
		uint the_height = image->height;
		for (level = 0; level < image->num_mipmap; ++level)
		{
			void *p;
			ImageInitData idata;
			if (pi_renderformat_is_compressed_format(image->format))
			{
				uint block_size = _get_compress_block_size(image->format);
				
				idata.size = ((the_width + 3) / 4) * ((the_height + 3) / 4) * block_size;
				idata.row_pitch = (the_width + 3) / 4 * block_size;
				idata.slice_pitch = idata.size;
			}
			else
			{
				uint fmt_size = pi_renderformat_get_numbytes(image->format);
				idata.row_pitch = (is_padding ? ((the_width + 3) & ~3) : the_width) * fmt_size;
				idata.size = idata.slice_pitch = idata.row_pitch * the_width;
			}
			
			idata.data = pi_malloc0(idata.size);
			if(!pi_bytes_read_data(bb, &p, idata.size))
			{
				PI_ASSERT(FALSE, "read 3d image data failed");
				return FALSE;
			}
			pi_memcpy_inline(idata.data, p, idata.size);
			pi_dvector_push(&image->data_vector, &idata);
			
			the_width = MAX(1, the_width / 2);
			the_height = MAX(1, the_height / 2);
		}
	}
	return TRUE;
}

PiBool PI_API dds_load(PiImage *image, byte *data, uint size)
{
	PiBytes bb;
	uint fmt_size;
	PiBool r = TRUE, is_padding = FALSE;
	
	if(size < DDS_HEAD_SIZE)
	{
		return FALSE;
	}
	
	pi_bytes_load(&bb, data, size, FALSE);
	if(!_dds_get_info(image, &bb))
	{
		return FALSE;
	}
	
	fmt_size = pi_renderformat_get_numbytes(image->format);
	if (!pi_renderformat_is_compressed_format(image->format))
	{
		if (image->row_pitch != image->width * fmt_size)
		{
			is_padding = TRUE;
			PI_ASSERT(image->row_pitch == ((image->width + 3) & ~3) * fmt_size, "");
		}
	}

	switch (image->type)
	{
	case TT_2D:
		r = _load_2d_image(image, &bb, is_padding);
		break;
	case TT_3D:
		PI_ASSERT(image->array_size == 1, "array size is invalid");
		r = _load_3d_image(image, &bb, is_padding);
		break;
	case TT_CUBE:
		PI_ASSERT(image->array_size == 1, "array size is invalid");
		r = _load_cube_image(image, &bb, is_padding);
		break;
	default:
		r = FALSE;
		break;
	}
	return r;
}