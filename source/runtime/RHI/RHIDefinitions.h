#pragma once
#include "RHIConfig.h"
namespace Air
{
	namespace ERHIFeatureLevel
	{
		enum Type
		{
			ES2,
			ES3_1,
			SM4,
			SM5,
			Num
		};
	}

	enum EShaderPlatform
	{
		SP_PCD3D_SM5		= 0,
		SP_OPENGL_SM4		= 1,
		SP_PS4				= 2,
		/** Used when running in Feature Level ES2 in OpenGL. */
		SP_OPENGL_PCES2 = 3,
		SP_XBOXONE = 4,
		SP_PCD3D_SM4 = 5,
		SP_OPENGL_SM5 = 6,
		/** Used when running in Feature Level ES2 in D3D11. */
		SP_PCD3D_ES2 = 7,
		SP_OPENGL_ES2_ANDROID = 8,
		SP_OPENGL_ES2_WEBGL = 9,
		SP_OPENGL_ES2_IOS = 10,
		SP_METAL = 11,
		SP_OPENGL_SM4_MAC = 12,
		SP_METAL_MRT = 13,
		SP_OPENGL_ES31_EXT = 14,
		/** Used when running in Feature Level ES3_1 in D3D11. */
		SP_PCD3D_ES3_1 = 15,
		/** Used when running in Feature Level ES3_1 in OpenGL. */
		SP_OPENGL_PCES3_1 = 16,
		SP_METAL_SM5 = 17,
		SP_VULKAN_PCES3_1 = 18,
		SP_METAL_SM4 = 19,
		SP_VULKAN_SM4 = 20,
		SP_VULKAN_SM5 = 21,
		SP_VULKAN_ES3_1_ANDROID = 22,
		SP_METAL_MACES3_1 = 23,
		SP_METAL_MACES2 = 24,
		SP_OPENGL_ES3_1_ANDROID = 25,
		SP_SWITCH = 26,
		SP_SWITCH_FORWARD = 27,

		SP_NumPlatforms = 28,
		SP_NumBits = 5,
	};


	inline ERHIFeatureLevel::Type getMaxSupportedFeatureLevel(EShaderPlatform inShaderPlatform)
	{
		switch (inShaderPlatform)
		{
		case SP_PCD3D_SM5:
			return ERHIFeatureLevel::SM5;

		case SP_OPENGL_SM4:
			return ERHIFeatureLevel::SM4;
		default:
			break;
		}
		return ERHIFeatureLevel::Num;
	}

	inline bool isFeatureLevelSupported(EShaderPlatform inShaderPlatform, ERHIFeatureLevel::Type inFeatureLevel)
	{
		return inFeatureLevel <= getMaxSupportedFeatureLevel(inShaderPlatform);
	}

	enum EShaderFrequency
	{
		SF_Vertex				= 0,
		SF_Hull					= 1,
		SF_Domain				= 2,
		SF_Pixel				= 3,
		SF_Geometry				= 4,
		SF_Compute				= 5,
		SF_NumFrequencies		= 6,
		SF_NumBits				= 3,
	};

	enum ECubeFace
	{
		CubeFace_PosX = 0,
		CubeFace_NegX,
		CubeFace_PosY,
		CubeFace_NegY,
		CubeFace_PosZ,
		CubeFace_NegZ,
		CubeFace_MAX
	};

	enum class ERenderTargetLoadAction
	{
		ENoAction,
		ELoad,
		EClear,
	};

	enum class ERenderTargetStoreAction
	{
		ENoAction,
		EStore,
		EMultisampleResolve
	};

	enum { MaxSimultaneousRenderTargets = 8 };

	enum { MaxSimultaneousUAVs = 8 };
	enum ETextureCreateFlags
	{
		TexCreate_None = 0,

		// Texture can be used as a render target
		TexCreate_RenderTargetable = 1 << 0,
		// Texture can be used as a resolve target
		TexCreate_ResolveTargetable = 1 << 1,
		// Texture can be used as a depth-stencil target.
		TexCreate_DepthStencilTargetable = 1 << 2,
		// Texture can be used as a shader resource.
		TexCreate_ShaderResource = 1 << 3,

		// Texture is encoded in sRGB gamma space
		TexCreate_SRGB = 1 << 4,
		// Texture will be created without a packed miptail
		TexCreate_NoMipTail = 1 << 5,
		// Texture will be created with an un-tiled format
		TexCreate_NoTiling = 1 << 6,
		// Texture that may be updated every frame
		TexCreate_Dynamic = 1 << 8,
		// Allow silent texture creation failure
		TexCreate_AllowFailure = 1 << 9,
		// Disable automatic defragmentation if the initial texture memory allocation fails.
		TexCreate_DisableAutoDefrag = 1 << 10,
		// Create the texture with automatic -1..1 biasing
		TexCreate_BiasNormalMap = 1 << 11,
		// Create the texture with the flag that allows mip generation later, only applicable to D3D11
		TexCreate_GenerateMipCapable = 1 << 12,
		// UnorderedAccessView (DX11 only)
		// Warning: Causes additional synchronization between draw calls when using a render target allocated with this flag, use sparingly
		// See: GCNPerformanceTweets.pdf Tip 37
		TexCreate_UAV = 1 << 16,
		// Render target texture that will be displayed on screen (back buffer)
		TexCreate_Presentable = 1 << 17,
		// Texture data is accessible by the CPU
		TexCreate_CPUReadback = 1 << 18,
		// Texture was processed offline (via a texture conversion process for the current platform)
		TexCreate_OfflineProcessed = 1 << 19,
		// Texture needs to go in fast VRAM if available (HINT only)
		TexCreate_FastVRAM = 1 << 20,
		// by default the texture is not showing up in the list - this is to reduce clutter, using the FULL option this can be ignored
		TexCreate_HideInVisualizeTexture = 1 << 21,
		// Texture should be created in virtual memory, with no physical memory allocation made
		// You must make further calls to RHIVirtualTextureSetFirstMipInMemory to allocate physical memory
		// and RHIVirtualTextureSetFirstMipVisible to map the first mip visible to the GPU
		TexCreate_Virtual = 1 << 22,
		// Creates a RenderTargetView for each array slice of the texture
		// Warning: if this was specified when the resource was created, you can't use SV_RenderTargetArrayIndex to route to other slices!
		TexCreate_TargetArraySlicesIndependently = 1 << 23,
		// Texture that may be shared with DX9 or other devices
		TexCreate_Shared = 1 << 24,
		// RenderTarget will not use full-texture fast clear functionality.
		TexCreate_NoFastClear = 1 << 25,
		// Texture is a depth stencil resolve target
		TexCreate_DepthStencilResolveTarget = 1 << 26,
		// Render target will not FinalizeFastClear; Caches and meta data will be flushed, but clearing will be skipped (avoids potentially trashing metadata)
		TexCreate_NoFastClearFinalize = 1 << 28,
		// Hint to the driver that this resource is managed properly by the engine for Alternate-Frame-Rendering in mGPU usage.
		TexCreate_AFRManual = 1 << 29,
	};

	enum class ERHIZBuffer
	{
		FarPlane = 1,
		NearPlane = 0,
		IsInverted = (int32)((int32)ERHIZBuffer::FarPlane < (int32)ERHIZBuffer::NearPlane),
	};

	inline bool isOpenGLPlatform(const EShaderPlatform platform)
	{
		return platform == SP_OPENGL_SM4 || platform == SP_OPENGL_SM4_MAC || platform == SP_OPENGL_SM5 || platform == SP_OPENGL_PCES2 || platform == SP_OPENGL_PCES3_1 || platform == SP_OPENGL_ES2_ANDROID || platform == SP_OPENGL_ES2_WEBGL || platform == SP_OPENGL_ES2_IOS || platform == SP_OPENGL_ES31_EXT || platform == SP_OPENGL_ES3_1_ANDROID || platform == SP_SWITCH || platform == SP_SWITCH_FORWARD;
	}



	inline bool isPCPlatform(const EShaderPlatform Platform)
	{
		return Platform == SP_PCD3D_SM5 || Platform == SP_PCD3D_SM4 || Platform == SP_PCD3D_ES2 || Platform == SP_PCD3D_ES3_1 ||
			Platform == SP_OPENGL_SM4 || Platform == SP_OPENGL_SM4_MAC || Platform == SP_OPENGL_SM5 || Platform == SP_OPENGL_PCES2 || Platform == SP_OPENGL_PCES3_1 ||
			Platform == SP_METAL_SM4 || Platform == SP_METAL_SM5 ||
			Platform == SP_VULKAN_PCES3_1 || Platform == SP_VULKAN_SM4 || Platform == SP_VULKAN_SM5 || Platform == SP_METAL_MACES3_1 || Platform == SP_METAL_MACES2;
	}

	inline bool isVulkanMobilePlatform(const EShaderPlatform platform)
	{
		return platform == SP_VULKAN_ES3_1_ANDROID || platform == SP_VULKAN_PCES3_1 || platform == SP_VULKAN_SM4 || platform == SP_VULKAN_SM5;
	}

	inline bool RHIHasTiledGPU(const EShaderPlatform platform)
	{
		return (platform == SP_METAL_MRT) || platform == SP_METAL || platform == SP_OPENGL_ES2_IOS || platform == SP_OPENGL_ES2_ANDROID || platform == SP_OPENGL_ES3_1_ANDROID;
	}

	inline bool isES2Platform(const EShaderPlatform plaform)
	{
		return plaform == SP_PCD3D_ES2 || plaform == SP_OPENGL_PCES2 || plaform == SP_OPENGL_ES2_ANDROID || plaform == SP_OPENGL_ES2_WEBGL || plaform == SP_OPENGL_ES2_IOS || plaform == SP_METAL_MACES2;
	}

	inline bool isMobilePlatform(const EShaderPlatform platform)
	{
		return isES2Platform(platform) || platform == SP_METAL || platform == SP_PCD3D_ES3_1 || platform == SP_OPENGL_PCES3_1 || platform == SP_VULKAN_ES3_1_ANDROID || platform == SP_VULKAN_PCES3_1 || platform == SP_METAL_MACES3_1 || platform == SP_OPENGL_ES3_1_ANDROID || platform == SP_SWITCH_FORWARD;
	}

	inline bool isMetalPlatform(const EShaderPlatform platform)
	{
		return platform == SP_METAL || platform == SP_METAL_MACES2 || platform == SP_METAL_MACES3_1 || platform == SP_METAL_MRT || platform == SP_METAL_SM4 || platform == SP_METAL_SM5;
	}

	inline bool isVulkanPlatform(const EShaderPlatform platform)
	{
		return platform == SP_VULKAN_SM5 || platform == SP_VULKAN_SM4 || platform == SP_VULKAN_PCES3_1 || platform == SP_VULKAN_ES3_1_ANDROID;
	}

	inline bool isAndroidOpenGLESPlatform(EShaderPlatform platform)
	{
		return platform == SP_OPENGL_ES2_ANDROID || SP_OPENGL_ES3_1_ANDROID;
	}

	inline bool RHISupportsSeparateMSAAAndResolveTextures(const EShaderPlatform platform)
	{
		return !isMetalPlatform(platform) && !isVulkanPlatform(platform) && !isAndroidOpenGLESPlatform(platform);
	}

	inline bool RHISupportsShaderCompression(const EShaderPlatform platform)
	{
		//return platform != SP_XBOXONE;
		return false;
	}

	enum EConstantBufferBaseType
	{
		CBMT_INVALID,
		CBMT_BOOL,
		CBMT_INT32,
		CBMT_UINT32,
		CBMT_FLOAT32,
		CBMT_STRUCT,
		CBMT_SRV,
		CBMT_UAV,
		CBMT_SAMPLER,
		CBMT_TEXTURE
	};

	inline bool isConstantBufferResourceType(EConstantBufferBaseType baseType)
	{
		return baseType == CBMT_SRV || baseType == CBMT_UAV || baseType == CBMT_SAMPLER || baseType == CBMT_TEXTURE;
	}

	enum EConstantBufferUsage
	{
		ConstantBuffer_SingleDraw = 0,
		ConstantBuffer_SingleFrame,
		ConstantBuffer_MultiFrame,
	};

	enum EResourceLockMode
	{
		RLM_ReadOnly,
		RLM_WriteOnly,
		RLM_Num
	};

	enum class ESimpleRenderTargetMode
	{
		EExistingColorAndDepth,
		EUninitializedColorAndDepth,					// Color = ????, Depth = ????
		EUninitializedColorExistingDepth,				// Color = ????, Depth = Existing
		EUninitializedColorClearDepth,					// Color = ????, Depth = Default
		EClearColorExistingDepth,						// Clear Color = whatever was bound to the rendertarget at creation time. Depth = Existing
		EClearColorAndDepth,							// Clear color and depth to bound clear values.
		EExistingContents_NoDepthStore,					// Load existing contents, but don't store depth out.  depth can be written.
		EExistingColorAndClearDepth,					// Color = Existing, Depth = clear value
		EExistingColorAndDepthAndClearStencil,
	};

	enum ESamplerFilter
	{
		SF_Point,
		SF_Bilinear,
		SF_Trilinear,
		SF_AnisotropicPoint,
		SF_AnisotropicLinear,
	};

	enum ESamplerAddressMode
	{
		AM_Wrap,
		AM_Clamp,
		AM_Mirror,
		AM_Border
	};

	enum ESamplerCompareFunction
	{
		SCF_Never,
		SCF_Less
	};

	enum ERasterizerFillMode
	{
		FM_Point,
		FM_Wireframe,
		FM_Solid,
	};

	enum ERasterizerCullMode
	{
		CM_None,
		CM_CW,
		CM_CCW	//	ÄæÊ±Õë
	};

	enum ECompareFunction
	{
		CF_Less,
		CF_LessEqual,
		CF_Greater,
		CF_GreateEqual,
		CF_Equal,
		CF_NotEqual,
		CF_Never,
		CF_Always,

		CF_DepthNearOrEqual = (((int32)ERHIZBuffer::IsInverted != 0) ? CF_GreateEqual : CF_LessEqual),
		CF_DepthNear		= (((int32)ERHIZBuffer::IsInverted != 0) ?CF_Greater : CF_LessEqual),
		CF_DepthFartherOrEqual = (((int32)ERHIZBuffer::IsInverted != 0)? CF_LessEqual : CF_GreateEqual),
		CF_DepthFarther		= (((int32)ERHIZBuffer::IsInverted != 0) ? CF_Less : CF_Greater),
	};

	enum EStencilOp
	{
		SO_Keep,
		SO_Zero,
		SO_Replace,
		SO_SaturatedIncrement,
		SO_SaturatedDecrement,
		SO_Invert,
		SO_Increment,
		SO_Decrement
	};

	enum EColorWriteMask
	{
		CW_RED = 0x01,
		CW_GREEN = 0x02,
		CW_BLUE = 0x04,
		CW_ALPHA = 0x08,

		CW_NONE = 0,
		CW_RGB = CW_RED | CW_GREEN | CW_BLUE,
		CW_RGBA = CW_RED | CW_GREEN | CW_BLUE |CW_ALPHA,
		CW_RG = CW_RED | CW_GREEN,
		CW_BA	= CW_BLUE | CW_ALPHA
	};

	enum EBlendOperation
	{
		BO_Add,
		BO_Subtract,
		BO_Min,
		BO_Max,
		BO_ReverseSubtract,
	};

	enum EBlendFactor
	{
		BF_Zero,
		BF_One,
		BF_SourceColor,
		BF_InverseSourceColor,
		BF_SourceAlpha,
		BF_InverseSourceAlpha,
		BF_DestAlpha,
		BF_InverseDestAlpha,
		BF_DestColor,
		BF_InverseDestColor,
		BF_constantBlendFactor,
		BF_InverseConstantBlendFactor
	};

	enum EPrimitiveType
	{
		PT_TriangleList,
		PT_TriangleStrip,
		PT_LineList,
		PT_QuadList,
		PT_PointList,
		PT_1_ControlPointPatchList,
		PT_2_ControlPointPatchList,
		PT_3_ControlPointPatchList,
		PT_4_ControlPointPatchList,
		PT_5_ControlPointPatchList,
		PT_6_ControlPointPatchList,
		PT_7_ControlPointPatchList,
		PT_8_ControlPointPatchList,
		PT_9_ControlPointPatchList,
		PT_10_ControlPointPatchList,
		PT_11_ControlPointPatchList,
		PT_12_ControlPointPatchList,
		PT_13_ControlPointPatchList,
		PT_14_ControlPointPatchList,
		PT_15_ControlPointPatchList,
		PT_16_ControlPointPatchList,
		PT_17_ControlPointPatchList,
		PT_18_ControlPointPatchList,
		PT_19_ControlPointPatchList,
		PT_20_ControlPointPatchList,
		PT_21_ControlPointPatchList,
		PT_22_ControlPointPatchList,
		PT_23_ControlPointPatchList,
		PT_24_ControlPointPatchList,
		PT_25_ControlPointPatchList,
		PT_26_ControlPointPatchList,
		PT_27_ControlPointPatchList,
		PT_28_ControlPointPatchList,
		PT_29_ControlPointPatchList,
		PT_30_ControlPointPatchList,
		PT_31_ControlPointPatchList,
		PT_32_ControlPointPatchList,
		PT_Num,
		PT_NumBits = 6
	};

	enum { MaxVertexElementCount = 16 };
	enum { ShaderArrayElementAlignBytes = 16 };

	struct RHIResourceTableEntry
	{
	public:
		static CONSTEXPR uint32 getEndOffStreamToken()
		{
			return 0xffffffff;
		}

		static uint32 create(uint16 constantBufferIndex, uint16 resourceIndex, uint32 bindIndex)
		{
			return ((constantBufferIndex & RTD_Mask_ConstantBufferIndex) << RTD_Shift_ConstantBufferIndex) | ((resourceIndex & RTD_Mask_ResourceIndex) << RTD_Shift_ResourceIndex) | ((bindIndex & RTD_Mask_BindIndex) << RTD_Shift_BindIndex);
		}


		static inline uint16 getConstantBufferIndex(uint32 data)
		{
			return (data >> RTD_Shift_ConstantBufferIndex) & RTD_Mask_ConstantBufferIndex;
		}

		static inline uint16 getResourceIndex(uint32 data)
		{
			return (data >> RTD_Shift_ResourceIndex) & RTD_Mask_ResourceIndex;
		}

		static inline uint16 getBindIndex(uint32 data)
		{
			return (data >> RTD_Shift_BindIndex) & RTD_Mask_BindIndex;
		}


	private:
		enum EResourceTableDefinitions
		{
			RTD_NumBits_ConstantBufferIndex = 8,
			RTD_NumBits_ResourceIndex = 16,
			RTD_NumBits_BindIndex = 8,
			RTD_Mask_ConstantBufferIndex	= (1 << RTD_NumBits_ConstantBufferIndex) -1,
			RTD_Mask_ResourceIndex			= (1 << RTD_NumBits_ResourceIndex) -1 ,
			RTD_Mask_BindIndex				= (1 << RTD_NumBits_BindIndex) -1,
			RTD_Shift_BindIndex				= 0,
			RTD_Shift_ResourceIndex			= RTD_Shift_BindIndex + RTD_NumBits_BindIndex,
			RTD_Shift_ConstantBufferIndex	= RTD_Shift_ResourceIndex + RTD_NumBits_ResourceIndex,
		};

		static_assert(RTD_NumBits_ConstantBufferIndex + RTD_NumBits_ResourceIndex + RTD_NumBits_BindIndex <= sizeof(uint32) * 8, "RTD_* values must fit in 32 bits");
	};

	enum EVertexElementType
	{
		VET_None,
		VET_Float1,
		VET_Float2,
		VET_Float3,
		VET_Float4,
		VET_PackedNormal,
		VET_UByte4,
		VET_UByte4N,
		VET_Color,
		VET_Short2,
		VET_Short4,
		VET_Short2N,
		VET_Half2,
		VET_Half4,
		VET_Short4N,
		VET_UShort2,
		VET_UShort4,
		VET_UShort2N,
		VET_UShort4N,
		VET_URGB10A2N,
		VET_MAX
	};

	enum EBufferUsageFlags
	{
		BUF_Static = 0x0001,
		BUF_Dynamic = 0x0002,
		BUF_Volatile	=0x0004,
		BUF_UnorderedAccess = 0x0008,
		BUF_ByteAddressBuffer = 0x0020,
		BUF_UAVCounter = 0x0040,
		BUF_StreamOutput = 0x0080,
		BUF_DrawIndirect = 0x0100,
		BUF_ShaderResource = 0x0200,
		BUF_KeepCPUAccessible = 0x0400,
		BUF_ZeroStride = 0x0800,
		BUF_FastVRAM	= 0x1000,
		BUF_AnyDynamic = (BUF_Dynamic | BUF_Volatile),
	};

	enum { MAX_TEXTURE_MIP_COUNT = 14 };

	enum class EClearDepthStencil
	{
		Depth,
		Stencil,
		DepthStencil
	};
}