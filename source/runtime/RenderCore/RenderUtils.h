#pragma once
#include "CoreMinimal.h"
#include "RHIResource.h"
#include "PackedNormal.h"
#include "RenderCore.h"
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

	extern RENDER_CORE_API class Texture* GBlackTextureCube;

	extern RENDER_CORE_API class Texture* GWhiteTextureCube;

	RENDER_CORE_API SIZE_T calcTextureMipWidthInBlocks(uint32 width, EPixelFormat format, uint32 mipIndex);

	RENDER_CORE_API SIZE_T calcTextureMipHeightInBlocks(uint32 height, EPixelFormat format, uint32 MipIndex);

	RENDER_CORE_API SIZE_T calcTextureMipMapSize(uint32 width, uint32 height, EPixelFormat format, uint32 mipIndex);

	RENDER_CORE_API SIZE_T calcTextureSize(uint32 sizeX, uint32 sizeY, EPixelFormat format, uint32 MipCount);

	RENDER_CORE_API SIZE_T calcTextureSize3D(uint32 width, uint32 height, uint32 depth, EPixelFormat format, uint32 mipCount);

	RENDER_CORE_API bool GPUSceneUseTexture2D(const StaticShaderPlatform platform);

	inline bool isForwardShadingEnabled(EShaderPlatform platform)
	{
		extern RENDER_CORE_API uint64 GForwardShadingPlatformMask;
		return !!(GForwardShadingPlatformMask & (1ull << platform)) && getMaxSupportedFeatureLevel(platform) >= ERHIFeatureLevel::SM5;
	}

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
		return isForwardShadingEnabled(platform) || isSimpleForwardShadingEnable(platform);
	}

	inline bool isUsingSelectiveBasePassOutputs(EShaderPlatform platform)
	{
		extern RENDER_CORE_API uint64 GSelectiveBasePassOutputsPlatformMask;
		return !!(GSelectiveBasePassOutputsPlatformMask & (1ull << platform));
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


	RENDER_CORE_API void quantizeSceneBufferSize(const int2& inOutBufferSizeX, int2& inOutBufferSizeY);

	RENDER_CORE_API VertexDeclarationRHIRef& getVertexDeclarationVector4();

	RENDER_CORE_API int2 calcMipMapExtent(uint32 textureWidth, uint32 textureHeight, EPixelFormat format, uint32 mipIndex);

	RENDER_CORE_API void copyTextureData2D(const void* source, void* dest, uint32 height, EPixelFormat format, uint32 sourceStirde, uint32 destStride);

	RENDER_CORE_API bool mobileSupportsGPUScene(EShaderPlatform platform);

	inline bool useGPUScene(EShaderPlatform platform, ERHIFeatureLevel::Type featureLevel)
	{
		if (featureLevel == ERHIFeatureLevel::ES3_1)
		{
			return mobileSupportsGPUScene(platform);
		}

		return featureLevel >= ERHIFeatureLevel::SM5 && !isOpenGLPlatform(platform) && !isSwitchPlatform(platform);
	}

	RENDER_CORE_API bool useVirtualTexturing(ERHIFeatureLevel::Type inFeatureLevel, const class ITargetPlatform* targetPlatform = nullptr);

#define NUM_DEBUG_UTIL_COLORS (32)
}