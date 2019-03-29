#pragma once
#include "CoreType.h"
#include "RenderCore.h"
#include "PixelFormat.h"
#include "RHIResource.h"
#include "RHIDefinitions.h"
namespace Air
{
	enum PixelFormatFlags
	{
		PFF_None = 0,
		PFF_Compressed = 1 << 0,
		PFF_SRGB = 1 << 1,
	};

	struct PixelFormatInfo
	{
		const TCHAR* mName;
		int32	BlockSizeX, 
			BlockSizeY, 
			BlockSizeZ, 
			BlockBytes,
			NumComponents;

		uint32 PlatformFormat;
		bool Supported;
		EPixelFormat AirFormat;
		uint32 Flags;
	};

	extern RENDER_CORE_API PixelFormatInfo GPixelFormats[PF_MAX];

	extern RENDER_CORE_API class Texture* GBlackTexture;

	extern RENDER_CORE_API class Texture* GWhiteTexture;

	RENDER_CORE_API SIZE_T calcTextureMipWidthInBlocks(uint32 width, EPixelFormat format, uint32 mipIndex);

	RENDER_CORE_API SIZE_T calcTextureMipHeightInBlocks(uint32 height, EPixelFormat format, uint32 MipIndex);

	RENDER_CORE_API SIZE_T CalcTextureMipMapSize(uint32 width, uint32 height, EPixelFormat format, uint32 mipIndex);

	RENDER_CORE_API SIZE_T calcTextureSize(uint32 sizeX, uint32 sizeY, EPixelFormat format, uint32 MipCount);

	RENDER_CORE_API bool isForwardShadingEnabled(ERHIFeatureLevel::Type  featureLevel);

	RENDER_CORE_API bool isSimpleForwardShadingEnable(EShaderPlatform platform);

	FORCEINLINE float getBasisDeterminantSign(const float3& XAxis, const float3& YAxis, const float3& ZAxis)
	{
		Matrix basis(Plane(XAxis, 0),
			Plane(YAxis, 0),
			Plane(ZAxis, 0),
			Plane(0, 0, 0, 1));
		return (basis.determinant() < 0) ? -1.0f : +1.0f;
	}

	inline bool isAnyForwardShadingEnabled(EShaderPlatform platform)
	{
		return isForwardShadingEnabled(getMaxSupportedFeatureLevel(platform)) || isSimpleForwardShadingEnable(platform);
	}

	inline bool isUsingGBuffers(EShaderPlatform platform)
	{
		return !isAnyForwardShadingEnabled(platform);
	}

	inline bool isCompressedFormat(EPixelFormat format)
	{
		return (GPixelFormats[format].Flags & PFF_Compressed) != 0;
	}

	inline bool isSRGB(EPixelFormat format)
	{
		return (GPixelFormats[format].Flags & PFF_SRGB) != 0;
	}


	RENDER_CORE_API void quantizeSceneBufferSize(int32& inOutBufferSizeX, int32& inOutBufferSizeY);

	RENDER_CORE_API VertexDeclarationRHIRef& getVertexDeclarationVector4();

	RENDER_CORE_API int2 calcMipMapExtent(uint32 textureWidth, uint32 textureHeight, EPixelFormat format, uint32 mipIndex);

#define NUM_DEBUG_UTIL_COLORS (32)
}