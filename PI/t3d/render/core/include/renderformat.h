#ifndef INCLUDE_RENDERFORMAT_H
#define INCLUDE_RENDERFORMAT_H

#include <pi_lib.h>

typedef enum 
{
	EC_R = 0UL,
	EC_G = 1UL,
	EC_B = 2UL,
	EC_A = 3UL,
	EC_D = 4UL,
	EC_S = 5UL,
	EC_BC = 6UL,
	EC_E = 7UL
}ElementChannel;

typedef enum 
{
	ECT_UNORM = 0UL,
	ECT_SNORM = 1UL,
	ECT_UINT = 2UL,
	ECT_SINT = 3UL,
	ECT_FLOAT = 4UL,
	ECT_UNORM_SRGB = 5UL,
	ECT_TYPELESS = 6UL,
	ECT_SHAREDEXP = 7UL
}ElementChannelType;

/**
 * ElementFormat就是64位无符号整形：uint64，格式如下：
 * 00000000 T3[4] T2[4] T1[4] T0[4] S3[6] S2[6] S1[6] S0[6] C3[4] C2[4] C1[4] C0[4]
 */

#define MAKE_RENDERFORMAT_4(ch_0, ch_1, ch_2, ch_3, size_0, size_1, size_2, size_3, type_0, type_1, type_2, type_3) \
	(((uint64)(ch_0) << 0) | ((uint64)(ch_1) << 4) | ((uint64)(ch_2) << 8) | ((uint64)(ch_3) << 12) \
	| ((uint64)(size_0) << 16) | ((uint64)(size_1) << 22) | ((uint64)(size_2) << 28) | ((uint64)(size_3) << 34) \
	| ((uint64)(type_0) << 40) | ((uint64)(type_1) << 44) | ((uint64)(type_2) << 48) | ((uint64)(type_3) << 52))

#define MAKE_RENDERFORMAT_3(ch_0, ch_1, ch_2, size_0, size_1, size_2, type_0, type_1, type_2) \
	(((uint64)(ch_0) << 0) | ((uint64)(ch_1) << 4) | ((ch_2) << 8) | (0ULL << 12) \
	| ((uint64)(size_0) << 16) | ((uint64)(size_1) << 22) | ((uint64)(size_2) << 28) | (0ULL << 34) \
	| ((uint64)(type_0) << 40) | ((uint64)(type_1) << 44) | ((uint64)(type_2) << 48) | (0ULL << 52))

#define MAKE_RENDERFORMAT_2(ch_0, ch_1, size_0, size_1, type_0, type_1) \
	(((uint64)(ch_0) << 0) | ((uint64)(ch_1) << 4) | (0ULL << 8) | (0ULL << 12) \
	| ((uint64)(size_0) << 16) | ((uint64)(size_1) << 22) | (0ULL << 28) | (0ULL << 34) \
	| ((uint64)(type_0) << 40) | ((uint64)(type_1) << 44) | (0ULL << 48) | (0ULL << 52))

#define MAKE_RENDERFORMAT_1(ch_0, size_0, type_0) \
	(((uint64)(ch_0) << 0) | (0ULL << 4) | (0ULL << 8) | (0ULL << 12) \
	| ((uint64)(size_0) << 16) | (0ULL << 22) | (0ULL << 28) | (0ULL << 34) \
	| ((uint64)(type_0) << 40) | (0ULL << 44) | (0ULL << 48) | (0ULL << 52))

// Unknown element format.
#define RFIMPL_UNKNOWN 0

// 8-bit element format, all bits alpha.
#define RFIMPL_A8 MAKE_RENDERFORMAT_1(EC_A, 8, ECT_UNORM)

// 16-bit element format, 4 bits for alpha, red, green and blue.
#define RFIMPL_ARGB4 MAKE_RENDERFORMAT_4(EC_A, EC_R, EC_G, EC_B, 4, 4, 4, 4, ECT_UNORM, ECT_UNORM, ECT_UNORM, ECT_UNORM)

// 8-bit element format, 8 bits for red.
#define RFIMPL_R8 MAKE_RENDERFORMAT_1(EC_R, 8, ECT_UNORM)
// 8-bit element format, 8 bits for signed red.
#define RFIMPL_SIGNED_R8 MAKE_RENDERFORMAT_1(EC_R, 8, ECT_SNORM)
// 16-bit element format, 8 bits for red, green.
#define RFIMPL_GR8 MAKE_RENDERFORMAT_2(EC_G, EC_R, 8, 8, ECT_UNORM, ECT_UNORM)
// 16-bit element format, 8 bits for signed red, green.
#define RFIMPL_SIGNED_GR8 MAKE_RENDERFORMAT_2(EC_G, EC_R, 8, 8, ECT_SNORM, ECT_SNORM)
// 24-bit element format, 8 bits for red, green and blue.
#define RFIMPL_BGR8 MAKE_RENDERFORMAT_3(EC_B, EC_G, EC_R, 8, 8, 8, ECT_UNORM, ECT_UNORM, ECT_UNORM)
// 24-bit element format, 8 bits for signed red, green and blue.
#define RFIMPL_SIGNED_BGR8 MAKE_RENDERFORMAT_3(EC_B, EC_G, EC_R, 8, 8, 8, ECT_SNORM, ECT_SNORM, ECT_SNORM)
// 32-bit element format, 8 bits for alpha, red, green and blue.
#define RFIMPL_ARGB8 MAKE_RENDERFORMAT_4(EC_A, EC_R, EC_G, EC_B, 8, 8, 8, 8, ECT_UNORM, ECT_UNORM, ECT_UNORM, ECT_UNORM)
// 32-bit element format, 8 bits for alpha, red, green and blue.
#define RFIMPL_ABGR8 MAKE_RENDERFORMAT_4(EC_A, EC_B, EC_G, EC_R, 8, 8, 8, 8, ECT_UNORM, ECT_UNORM, ECT_UNORM, ECT_UNORM)
// 32-bit element format, 8 bits for signed alpha, red, green and blue.
#define RFIMPL_SIGNED_ABGR8 MAKE_RENDERFORMAT_4(EC_A, EC_B, EC_G, EC_R, 8, 8, 8, 8, ECT_SNORM, ECT_SNORM, ECT_SNORM, ECT_SNORM)
// 32-bit element format, 2 bits for alpha, 10 bits for red, green and blue.
#define RFIMPL_A2BGR10 MAKE_RENDERFORMAT_4(EC_A, EC_B, EC_G, EC_R, 2, 10, 10, 10, ECT_UNORM, ECT_UNORM, ECT_UNORM, ECT_UNORM)
// 32-bit element format, 2 bits for alpha, 10 bits for signed red, green and blue.
#define RFIMPL_SIGNED_A2BGR10 MAKE_RENDERFORMAT_4(EC_A, EC_B, EC_G, EC_R, 2, 10, 10, 10, ECT_SNORM, ECT_SNORM, ECT_SNORM, ECT_SNORM)

// 32-bit element format, 8 bits for alpha, red, green and blue.
#define RFIMPL_R8UI MAKE_RENDERFORMAT_1(EC_R, 8, ECT_UINT)
// 32-bit element format, 8 bits for alpha, red, green and blue.
#define RFIMPL_R8I MAKE_RENDERFORMAT_1(EC_R, 8, ECT_SINT)
// 16-bit element format, 8 bits for red, green.
#define RFIMPL_GR8UI MAKE_RENDERFORMAT_2(EC_G, EC_R, 8, 8, ECT_UINT, ECT_UINT)
// 16-bit element format, 8 bits for red, green.
#define RFIMPL_GR8I MAKE_RENDERFORMAT_2(EC_G, EC_R, 8, 8, ECT_SINT, ECT_SINT)
// 24-bit element format, 8 bits for red, green and blue.
#define RFIMPL_BGR8UI MAKE_RENDERFORMAT_3(EC_B, EC_G, EC_R, 8, 8, 8, ECT_UINT, ECT_UINT, ECT_UINT)
// 24-bit element format, 8 bits for red, green and blue.
#define RFIMPL_BGR8I MAKE_RENDERFORMAT_3(EC_B, EC_G, EC_R, 8, 8, 8, ECT_SINT, ECT_SINT, ECT_SINT)
// 32-bit element format, 8 bits for alpha, red, green and blue.
#define RFIMPL_ABGR8UI MAKE_RENDERFORMAT_4(EC_A, EC_B, EC_G, EC_R, 8, 8, 8, 8, ECT_UINT, ECT_UINT, ECT_UINT, ECT_UINT)
// 32-bit element format, 8 bits for signed alpha, red, green and blue.
#define RFIMPL_ABGR8I MAKE_RENDERFORMAT_4(EC_A, EC_B, EC_G, EC_R, 8, 8, 8, 8, ECT_SINT, ECT_SINT, ECT_SINT, ECT_SINT)
// 32-bit element format, 2 bits for alpha, 10 bits for red, green and blue.
#define RFIMPL_A2BGR10UI MAKE_RENDERFORMAT_4(EC_A, EC_B, EC_G, EC_R, 2, 10, 10, 10, ECT_UINT, ECT_UINT, ECT_UINT, ECT_UINT)
// 32-bit element format, 2 bits for alpha, 10 bits for red, green and blue.
#define RFIMPL_A2BGR10I MAKE_RENDERFORMAT_4(EC_A, EC_B, EC_G, EC_R, 2, 10, 10, 10, ECT_SINT, ECT_SINT, ECT_SINT, ECT_SINT)

// 16-bit element format, 16 bits for red.
#define RFIMPL_R16 MAKE_RENDERFORMAT_1(EC_R, 16, ECT_UNORM)
// 16-bit element format, 16 bits for signed red.
#define RFIMPL_SIGNED_R16 MAKE_RENDERFORMAT_1(EC_R, 16, ECT_SNORM)
// 32-bit element format, 16 bits for red and green.
#define RFIMPL_GR16 MAKE_RENDERFORMAT_2(EC_G, EC_R, 16, 16, ECT_UNORM, ECT_UNORM)
// 32-bit element format, 16 bits for signed red and green.
#define RFIMPL_SIGNED_GR16 MAKE_RENDERFORMAT_2(EC_G, EC_R, 16, 16, ECT_SNORM, ECT_SNORM)
// 48-bit element format, 16 bits for alpha, blue, green and red.
#define RFIMPL_BGR16 MAKE_RENDERFORMAT_3(EC_B, EC_G, EC_R, 16, 16, 16, ECT_UNORM, ECT_UNORM, ECT_UNORM)
// 48-bit element format, 16 bits for signed alpha, blue, green and red.
#define RFIMPL_SIGNED_BGR16 MAKE_RENDERFORMAT_3(EC_B, EC_G, EC_R, 16, 16, 16, ECT_SNORM, ECT_SNORM, ECT_SNORM)
// 64-bit element format, 16 bits for alpha, blue, green and red.
#define RFIMPL_ABGR16 MAKE_RENDERFORMAT_4(EC_A, EC_B, EC_G, EC_R, 16, 16, 16, 16, ECT_UNORM, ECT_UNORM, ECT_UNORM, ECT_UNORM)
// 64-bit element format, 16 bits for signed alpha, blue, green and red.
#define RFIMPL_SIGNED_ABGR16 MAKE_RENDERFORMAT_4(EC_A, EC_B, EC_G, EC_R, 16, 16, 16, 16, ECT_SNORM, ECT_SNORM, ECT_SNORM, ECT_SNORM)
// 32-bit element format, 32 bits for red.
#define RFIMPL_R32 MAKE_RENDERFORMAT_1(EC_R, 32, ECT_UNORM)
// 32-bit element format, 32 bits for signed red.
#define RFIMPL_SIGNED_R32 MAKE_RENDERFORMAT_1(EC_R, 32, ECT_SNORM)
// 64-bit element format, 16 bits for red and green.
#define RFIMPL_GR32 MAKE_RENDERFORMAT_2(EC_G, EC_R, 32, 32, ECT_UNORM, ECT_UNORM)
// 64-bit element format, 16 bits for signed red and green.
#define RFIMPL_SIGNED_GR32 MAKE_RENDERFORMAT_2(EC_G, EC_R, 32, 32, ECT_SNORM, ECT_SNORM)
// 96-bit element format, 16 bits for alpha, blue, green and red.
#define RFIMPL_BGR32 MAKE_RENDERFORMAT_3(EC_B, EC_G, EC_R, 32, 32, 32, ECT_UNORM, ECT_UNORM, ECT_UNORM)
// 96-bit element format, 16 bits for signed_alpha, blue, green and red.
#define RFIMPL_SIGNED_BGR32 MAKE_RENDERFORMAT_3(EC_B, EC_G, EC_R, 32, 32, 32, ECT_SNORM, ECT_SNORM, ECT_SNORM)
// 128-bit element format, 16 bits for alpha, blue, green and red.
#define RFIMPL_ABGR32 MAKE_RENDERFORMAT_4(EC_A, EC_B, EC_G, EC_R, 32, 32, 32, 32, ECT_UNORM, ECT_UNORM, ECT_UNORM, ECT_UNORM)
// 128-bit element format, 16 bits for signed alpha, blue, green and red.
#define RFIMPL_SIGNED_ABGR32 MAKE_RENDERFORMAT_4(EC_A, EC_B, EC_G, EC_R, 32, 32, 32, 32, ECT_SNORM, ECT_SNORM, ECT_SNORM, ECT_SNORM)

// 16-bit element format, 16 bits for red.
#define RFIMPL_R16UI MAKE_RENDERFORMAT_1(EC_R, 16, ECT_UINT)
// 16-bit element format, 16 bits for signed red.
#define RFIMPL_R16I MAKE_RENDERFORMAT_1(EC_R, 16, ECT_SINT)
// 32-bit element format, 16 bits for red and green.
#define RFIMPL_GR16UI MAKE_RENDERFORMAT_2(EC_G, EC_R, 16, 16, ECT_UINT, ECT_UINT)
// 32-bit element format, 16 bits for signed red and green.
#define RFIMPL_GR16I MAKE_RENDERFORMAT_2(EC_G, EC_R, 16, 16, ECT_SINT, ECT_SINT)
// 48-bit element format, 16 bits for alpha, blue, green and red.
#define RFIMPL_BGR16UI MAKE_RENDERFORMAT_3(EC_B, EC_G, EC_R, 16, 16, 16, ECT_UINT, ECT_UINT, ECT_UINT)
// 48-bit element format, 16 bits for signed alpha, blue, green and red.
#define RFIMPL_BGR16I MAKE_RENDERFORMAT_3(EC_B, EC_G, EC_R, 16, 16, 16, ECT_SINT, ECT_SINT, ECT_SINT)
// 64-bit element format, 16 bits for alpha, blue, green and red.
#define RFIMPL_ABGR16UI MAKE_RENDERFORMAT_4(EC_A, EC_B, EC_G, EC_R, 16, 16, 16, 16, ECT_UINT, ECT_UINT, ECT_UINT, ECT_UINT)
// 64-bit element format, 16 bits for signed alpha, blue, green and red.
#define RFIMPL_ABGR16I MAKE_RENDERFORMAT_4(EC_A, EC_B, EC_G, EC_R, 16, 16, 16, 16, ECT_SINT, ECT_SINT, ECT_SINT, ECT_SINT)
// 32-bit element format, 32 bits for red.
#define RFIMPL_R32UI MAKE_RENDERFORMAT_1(EC_R, 32, ECT_UINT)
// 32-bit element format, 32 bits for signed red.
#define RFIMPL_R32I MAKE_RENDERFORMAT_1(EC_R, 32, ECT_SINT)
// 64-bit element format, 16 bits for red and green.
#define RFIMPL_GR32UI MAKE_RENDERFORMAT_2(EC_G, EC_R, 32, 32, ECT_UINT, ECT_UINT)
// 64-bit element format, 16 bits for signed red and green.
#define RFIMPL_GR32I MAKE_RENDERFORMAT_2(EC_G, EC_R, 32, 32, ECT_SINT, ECT_SINT)
// 96-bit element format, 16 bits for alpha, blue, green and red.
#define RFIMPL_BGR32UI MAKE_RENDERFORMAT_3(EC_B, EC_G, EC_R, 32, 32, 32, ECT_UINT, ECT_UINT, ECT_UINT)
// 96-bit element format, 16 bits for signed_alpha, blue, green and red.
#define RFIMPL_BGR32I MAKE_RENDERFORMAT_3(EC_B, EC_G, EC_R, 32, 32, 32, ECT_SINT, ECT_SINT, ECT_SINT)
// 128-bit element format, 16 bits for alpha, blue, green and red.
#define RFIMPL_ABGR32UI MAKE_RENDERFORMAT_4(EC_A, EC_B, EC_G, EC_R, 32, 32, 32, 32, ECT_UINT, ECT_UINT, ECT_UINT, ECT_UINT)
// 128-bit element format, 16 bits for signed alpha, blue, green and red.
#define RFIMPL_ABGR32I MAKE_RENDERFORMAT_4(EC_A, EC_B, EC_G, EC_R, 32, 32, 32, 32, ECT_SINT, ECT_SINT, ECT_SINT, ECT_SINT)

// 16-bit element format, 16 bits floating-point for red.
#define RFIMPL_R16F MAKE_RENDERFORMAT_1(EC_R, 16, ECT_FLOAT)
// 32-bit element format, 16 bits floating-point for green and red.
#define RFIMPL_GR16F MAKE_RENDERFORMAT_2(EC_G, EC_R, 16, 16, ECT_FLOAT, ECT_FLOAT)
// 32-bit element format, 11 bits floating-point for green and red, 10 bits floating-point for blue.
#define RFIMPL_B10G11R11F MAKE_RENDERFORMAT_3(EC_B, EC_G, EC_R, 10, 11, 11, ECT_FLOAT, ECT_FLOAT, ECT_FLOAT)
// 48-bit element format, 16 bits floating-point for blue, green and red.
#define RFIMPL_BGR16F MAKE_RENDERFORMAT_3(EC_B, EC_G, EC_R, 16, 16, 16, ECT_FLOAT, ECT_FLOAT, ECT_FLOAT)
// 64-bit element format, 16 bits floating-point for alpha, blue, green and red.
#define RFIMPL_ABGR16F MAKE_RENDERFORMAT_4(EC_A, EC_B, EC_G, EC_R, 16, 16, 16, 16, ECT_FLOAT, ECT_FLOAT, ECT_FLOAT, ECT_FLOAT)
// 32-bit element format, 32 bits floating-point for red.
#define RFIMPL_R32F MAKE_RENDERFORMAT_1(EC_R, 32, ECT_FLOAT)
// 64-bit element format, 32 bits floating-point for green and red.
#define RFIMPL_GR32F MAKE_RENDERFORMAT_2(EC_G, EC_R, 32, 32, ECT_FLOAT, ECT_FLOAT)
// 96-bit element format, 32 bits floating-point for blue, green and red.
#define RFIMPL_BGR32F MAKE_RENDERFORMAT_3(EC_B, EC_G, EC_R, 32, 32, 32, ECT_FLOAT, ECT_FLOAT, ECT_FLOAT)
// 128-bit element format, 32 bits floating-point for alpha, blue, green and red.
#define RFIMPL_ABGR32F MAKE_RENDERFORMAT_4(EC_A, EC_B, EC_G, EC_R, 32, 32, 32, 32, ECT_FLOAT, ECT_FLOAT, ECT_FLOAT, ECT_FLOAT)

// BC1 compression element format, DXT1
#define RFIMPL_BC1 MAKE_RENDERFORMAT_1(EC_BC, 1, ECT_UNORM)
// BC1 compression element format, signed DXT1
#define RFIMPL_SIGNED_BC1 MAKE_RENDERFORMAT_1(EC_BC, 1, ECT_SNORM)
// BC2 compression element format, DXT3
#define RFIMPL_BC2 MAKE_RENDERFORMAT_1(EC_BC, 2, ECT_UNORM)
// BC2 compression element format, signed DXT3
#define RFIMPL_SIGNED_BC2 MAKE_RENDERFORMAT_1(EC_BC, 2, ECT_SNORM)
// BC3 compression element format, DXT5
#define RFIMPL_BC3 MAKE_RENDERFORMAT_1(EC_BC, 3, ECT_UNORM)
// BC3 compression element format, signed DXT5
#define RFIMPL_SIGNED_BC3 MAKE_RENDERFORMAT_1(EC_BC, 3, ECT_SNORM)
// BC4 compression element format, 1 channel
#define RFIMPL_BC4 MAKE_RENDERFORMAT_1(EC_BC, 4, ECT_UNORM)
// BC4 compression element format, 1 channel signed
#define RFIMPL_SIGNED_BC4 MAKE_RENDERFORMAT_1(EC_BC, 4, ECT_SNORM)
// BC5 compression element format, 2 channels
#define RFIMPL_BC5 MAKE_RENDERFORMAT_1(EC_BC, 5, ECT_UNORM)
// BC5 compression element format, 2 channels signed
#define RFIMPL_SIGNED_BC5 MAKE_RENDERFORMAT_1(EC_BC, 5, ECT_SNORM)
// BC6 compression element format, 3 channels
#define RFIMPL_BC6 MAKE_RENDERFORMAT_1(EC_BC, 6, ECT_UNORM)
// BC6 compression element format, 3 channels
#define RFIMPL_SIGNED_BC6 MAKE_RENDERFORMAT_1(EC_BC, 6, ECT_SNORM)
// BC7 compression element format, 3 channels
#define RFIMPL_BC7 MAKE_RENDERFORMAT_1(EC_BC, 7, ECT_UNORM)

// 16-bit element format, 16 bits depth
#define RFIMPL_D16 MAKE_RENDERFORMAT_1(EC_D, 16, ECT_UNORM)
// 24-bit element format, 24 bits depth
#define RFIMPL_D24 MAKE_RENDERFORMAT_1(EC_D, 24, ECT_UNORM)
// 24-bit element format, 24 bits depth
#define RFIMPL_INTZ MAKE_RENDERFORMAT_1(EC_D, 24, ECT_SINT)

#define RFIMPL_NULL MAKE_RENDERFORMAT_1(EC_A, 1, ECT_UNORM)

// 32-bit element format, 24 bits depth and 8 bits stencil
#define RFIMPL_D24S8 MAKE_RENDERFORMAT_2(EC_D, EC_S, 24, 8, ECT_UNORM, ECT_UINT)
// 32-bit element format, 32 bits depth
#define RFIMPL_D32F MAKE_RENDERFORMAT_1(EC_D, 32, ECT_FLOAT)

// 32-bit element format, 8 bits for alpha, red, green and blue. Standard RGB (gamma 2.2).
#define RFIMPL_ARGB8_SRGB MAKE_RENDERFORMAT_4(EC_A, EC_R, EC_G, EC_B, 8, 8, 8, 8, ECT_UNORM_SRGB, ECT_UNORM_SRGB, ECT_UNORM_SRGB, ECT_UNORM_SRGB)
// 32-bit element format, 8 bits for alpha, red, green and blue. Standard RGB (gamma 2.2).
#define RFIMPL_ABGR8_SRGB MAKE_RENDERFORMAT_4(EC_A, EC_B, EC_G, EC_R, 8, 8, 8, 8, ECT_UNORM_SRGB, ECT_UNORM_SRGB, ECT_UNORM_SRGB, ECT_UNORM_SRGB)
// BC1 compression element format. Standard RGB (gamma 2.2).
#define RFIMPL_BC1_SRGB MAKE_RENDERFORMAT_1(EC_BC, 1, ECT_UNORM_SRGB)
// BC2 compression element format. Standard RGB (gamma 2.2).
#define RFIMPL_BC2_SRGB MAKE_RENDERFORMAT_1(EC_BC, 2, ECT_UNORM_SRGB)
// BC3 compression element format. Standard RGB (gamma 2.2).
#define RFIMPL_BC3_SRGB MAKE_RENDERFORMAT_1(EC_BC, 3, ECT_UNORM_SRGB)
// BC4 compression element format. Standard RGB (gamma 2.2).
#define RFIMPL_BC4_SRGB MAKE_RENDERFORMAT_1(EC_BC, 4, ECT_UNORM_SRGB)
// BC5 compression element format. Standard RGB (gamma 2.2).
#define RFIMPL_BC5_SRGB MAKE_RENDERFORMAT_1(EC_BC, 5, ECT_UNORM_SRGB)
// BC7 compression element format. Standard RGB (gamma 2.2).
#define RFIMPL_BC7_SRGB MAKE_RENDERFORMAT_1(EC_BC, 7, ECT_UNORM_SRGB)

/**
 * 格式统一按小端理解，这是微软D3D的理解方式
 * 比如RF_ABGR8，地址从小到大，分别存放：R, G, B, A
 */
typedef enum 
{
	RF_UNKNOWN = 0,
	RF_A8 = 1,
	RF_ARGB4 = 2,
	RF_R8 = 3,
	RF_SIGNED_R8 = 4,
	RF_GR8 = 5,
	RF_SIGNED_GR8 = 6,
	RF_BGR8 = 7,
	RF_SIGNED_BGR8 = 8,
	//RF_ARGB8 = 9,
	RF_ABGR8 = 10,
	RF_SIGNED_ABGR8 = 11,
	RF_A2BGR10 = 12,
	RF_SIGNED_A2BGR10 = 13,
	RF_R8UI = 14,
	RF_R8I = 15,
	RF_GR8UI = 16,
	RF_GR8I = 17,
	RF_BGR8UI = 18,
	RF_BGR8I = 19,
	RF_ABGR8UI = 20,
	RF_ABGR8I = 21,
	RF_A2BGR10UI = 22,
	RF_A2BGR10I = 23,
	RF_R16 = 24,
	RF_SIGNED_R16 = 25,
	RF_GR16 = 26,
	RF_SIGNED_GR16 = 27,
	RF_BGR16 = 28,
	RF_SIGNED_BGR16 = 29,
	RF_ABGR16 = 30,
	RF_SIGNED_ABGR16 = 31,
	RF_R32 = 32,
	RF_SIGNED_R32 = 33,
	RF_GR32 = 34,
	RF_SIGNED_GR32 = 35,
	RF_BGR32 = 36,
	RF_SIGNED_BGR32 = 37,
	RF_ABGR32 = 38,
	RF_SIGNED_ABGR32 = 39,
	RF_R16UI = 40,
	RF_R16I = 41,
	RF_GR16UI = 42,
	RF_GR16I = 43,
	RF_BGR16UI = 44,
	RF_BGR16I = 45,
	RF_ABGR16UI = 46,
	RF_ABGR16I = 47,
	RF_R32UI = 48,
	RF_R32I = 49,
	RF_GR32UI = 50,
	RF_GR32I = 51,
	RF_BGR32UI = 52,
	RF_BGR32I = 53,
	RF_ABGR32UI = 54,
	RF_ABGR32I = 55,
	RF_R16F = 56,
	RF_GR16F = 57,
	RF_B10G11R11F = 58,
	RF_BGR16F = 59,
	RF_ABGR16F = 60,
	RF_R32F = 61,
	RF_GR32F = 62,
	RF_BGR32F = 63,
	RF_ABGR32F = 64,
	RF_BC1 = 65,
	RF_SIGNED_BC1 = 66,
	RF_BC2 = 67,
	RF_SIGNED_BC2 = 68,
	RF_BC3 = 69,
	RF_SIGNED_BC3 = 70,
	RF_BC4 = 71,
	RF_SIGNED_BC4 = 72,
	RF_BC5 = 73,
	RF_SIGNED_BC5 = 74,
	RF_BC6 = 75,
	RF_SIGNED_BC6 = 76,
	RF_BC7 = 77,
	RF_D16 = 78,
	RF_D24 = 79,
	RF_D24S8 = 80,
	RF_D32F = 81,
	RF_ARGB8_SRGB = 82,
	RF_ABGR8_SRGB = 83,
	RF_BC1_SRGB = 84,
	RF_BC2_SRGB = 85,
	RF_BC3_SRGB = 86,
	RF_BC4_SRGB = 87,
	RF_BC5_SRGB = 88,
	RF_BC7_SRGB = 89,
	RF_INTZ = 90,
	RF_NULL = 91,
	RF_NUM
}RenderFormat;

PI_BEGIN_DECLS 

uint64 PI_API pi_renderformat_get_impl(RenderFormat format);

ElementChannel PI_API pi_renderformat_get_channel(RenderFormat format, uint64 index);

RenderFormat PI_API pi_renderformat_set_channel(RenderFormat format, uint64 index, ElementChannel ch);

uint8 PI_API pi_renderformat_get_size(RenderFormat format, uint64 index);

RenderFormat PI_API pi_renderformat_set_size(RenderFormat format, uint64 index, uint8 size);

ElementChannelType PI_API pi_renderformat_get_type(RenderFormat format, uint64 index);

RenderFormat PI_API pi_renderformat_set_type(RenderFormat format, uint64 index, ElementChannelType type);

PiBool PI_API pi_renderformat_is_float_format(RenderFormat format);

PiBool PI_API pi_renderformat_is_compressed_format(RenderFormat format);

PiBool PI_API pi_renderformat_is_depth_format(RenderFormat format);

PiBool PI_API pi_renderformat_is_stencil_format(RenderFormat format);

PiBool PI_API pi_renderformat_is_srgb(RenderFormat format);

PiBool PI_API pi_renderformat_is_signed(RenderFormat format);

uint8 PI_API pi_renderformat_get_numbits(RenderFormat format);

uint8 PI_API pi_renderformat_get_numbytes(RenderFormat format);

RenderFormat PI_API pi_renderformat_make_srgb(RenderFormat format);

RenderFormat PI_API pi_renderformat_make_non_srgb(RenderFormat format);

RenderFormat PI_API pi_renderformat_make_signed(RenderFormat format);

RenderFormat PI_API pi_renderformat_make_unsigned(RenderFormat format);

uint8 PI_API pi_renderformat_get_numdepthbits(RenderFormat format);

uint8 PI_API pi_renderformat_get_numstencilbits(RenderFormat format);

uint32 PI_API pi_renderformat_get_numcomponents(RenderFormat format);

uint32 PI_API pi_renderformat_get_component_bpps(RenderFormat format);

PI_END_DECLS

#endif	/* INCLUDE_RENDERFORMAT_H */