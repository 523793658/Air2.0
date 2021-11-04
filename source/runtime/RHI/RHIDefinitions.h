#pragma once
#include "RHIConfig.h"
#include "Misc/EnumClassFlags.h"

#ifndef USE_STATIC_SHADER_PLATFORM_ENUMS
#define USE_STATIC_SHADER_PLATFORM_ENUMS 0
#endif

namespace Air
{

#ifndef RHI_RAYTRACING
#if (PLATFORM_WINDOWS && PLATFORM_64BITS)
#define	RHI_RAYTRACING	0
#else
#define RHI_RAYTRACING	0
#endif
#endif

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

	enum EConstantBufferBaseType : uint8
	{
		CBMT_INVALID,
		CBMT_BOOL,
		CBMT_INT32,
		CBMT_UINT32,
		CBMT_FLOAT32,

		CBMT_TEXTURE,
		CBMT_SRV,
		CBMT_UAV,
		CBMT_SAMPLER,

		CBMT_RDG_TEXTURE,
		CBMT_RDG_TEXTURE_SRV,
		CBMT_RDG_TEXTURE_UAV,
		CBMT_RDG_BUFFER,
		CBMT_RDG_BUFFER_SRV,
		CBMT_RDG_BUFFER_UAV,

		CBMT_NESTED_STRUCT,
		CBMT_INCLUDED_STRUCT,
		CBMT_REFERENCED_STRUCT,
		CBMT_RENDER_TARGET_BINDING_SLOTS,

		EConstantBufferBaseType_Num,
		EConstantBufferBaseType_NumBits = 5,

	};

	static_assert(EConstantBufferBaseType_Num <= (1 << EConstantBufferBaseType_NumBits), "EConstantBufferBaseType_Num will not fit on EConstantBufferBaseType_NumBits");

	enum EShaderPlatform
	{
		SP_PCD3D_SM5 = 0,
		SP_PS4 = 1,
		SP_XBOXONE_D3D12 = 2,
		SP_OPENGL_ES2_ANDROID_REMOVED = 3,
		SP_OPENGL_ES2_WEBGL_REMOVED = 4,
		SP_METAL = 5,
		SP_METAL_MRT = 6,
		SP_PCD3D_ES3_1 = 7,
		SP_OPENGL_PCES3_1 = 8,
		SP_METAL_SM5 = 9,
		SP_VULKAN_PCES3_1 = 10,
		SP_METAL_SM5_NOTESS = 11,
		SP_VULKAN_SM5 = 12,
		SP_VULKAN_ES3_1_ANDROID = 13,
		SP_METAL_MACES3_1 = 14,
		SP_OPENGL_ES3_1_ANDROID = 15,
		SP_SWITCH = 16,
		SP_SWITCH_FORWARD = 17,
		SP_METAL_MRT_MAC = 18,
		SP_VULKAN_SM5_LUMIN = 19,
		SP_VULKAN_ES3_1_LUMIN = 20,
		SP_METAL_TVOS = 21,
		SP_METAL_MRT_TVOS = 22,
#define DDPI_NUM_STATIC_SHADER_PLATFORMS 16
		SP_StaticPlatform_First = 17,

		SP_StaticPlatform_Last = (SP_StaticPlatform_First + DDPI_NUM_STATIC_SHADER_PLATFORMS - 1),
		SP_VULKAN_SM5_ANDROID = SP_StaticPlatform_Last + 1,

		SP_NumPlatforms,
		SP_NumBits = 7,
	};

	

	inline ERHIFeatureLevel::Type getMaxSupportedFeatureLevel(EShaderPlatform inShaderPlatform)
	{
		switch (inShaderPlatform)
		{
		case SP_PCD3D_SM5:
		case SP_PS4:
		case SP_XBOXONE_D3D12:
		case SP_METAL_SM5:
		case SP_METAL_MRT:
		case SP_METAL_MRT_TVOS:
		case SP_METAL_MRT_MAC:
		case SP_METAL_SM5_NOTESS:
		case SP_VULKAN_SM5:
		case SP_VULKAN_SM5_LUMIN:
		case SP_SWITCH:
		case SP_VULKAN_SM5_ANDROID:
			return ERHIFeatureLevel::SM5;
		case SP_METAL:
		case SP_METAL_TVOS:
		case SP_METAL_MACES3_1:
		case SP_PCD3D_ES3_1:
		case SP_OPENGL_PCES3_1:
		case SP_VULKAN_PCES3_1:
		case SP_VULKAN_ES3_1_ANDROID:
		case SP_VULKAN_ES3_1_LUMIN:
		case SP_OPENGL_ES3_1_ANDROID:
		case SP_SWITCH_FORWARD:
			return ERHIFeatureLevel::ES3_1;
		}
	}

	inline bool isFeatureLevelSupported(EShaderPlatform inShaderPlatform, ERHIFeatureLevel::Type inFeatureLevel)
	{
		return inFeatureLevel <= getMaxSupportedFeatureLevel(inShaderPlatform);
	}

	inline bool isD3DPlatform(const EShaderPlatform platform, bool bIncludeXboxone)
	{
		switch (platform)
		{
		case SP_PCD3D_SM5:
		case SP_PCD3D_ES3_1:
			return true;
		case SP_XBOXONE_D3D12:
			return bIncludeXboxone;
		default:
			return false;
		}
		return false;
	}

	enum EShaderFrequency
	{
		SF_Vertex = 0,
		SF_Hull = 1,
		SF_Domain = 2,
		SF_Pixel = 3,
		SF_Geometry = 4,
		SF_Compute = 5,
		SF_RayGen = 6,
		SF_RayMiss = 7,
		SF_RayHitGroup = 8,
		SF_RayCallable = 9,

		SF_NumFrequencies = 10,

		// Number of standard SM5-style shader frequencies for graphics pipeline (excluding compute)
		SF_NumGraphicsFrequencies = 5,

		// Number of standard SM5-style shader frequencies (including compute)
		SF_NumStandardFrequencies = 6,

		SF_NumBits = 4,
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
	ENUM_CLASS_FLAGS(ETextureCreateFlags);


	class StaticShaderPlatformNames
	{
	private:
		static const uint32 NumPlatforms = DDPI_NUM_STATIC_SHADER_PLATFORMS;

		struct FPlatform
		{
			wstring Name;
			wstring ShaderPlatform;
			wstring ShaderFormat;
		} Platforms[NumPlatforms];

		StaticShaderPlatformNames()
		{

		}

	public:
		static inline StaticShaderPlatformNames const& Get()
		{
			static StaticShaderPlatformNames Names;
			return Names;
		}

		static inline bool IsStaticPlatform(EShaderPlatform Platform)
		{
			return Platform >= SP_StaticPlatform_First && Platform <= SP_StaticPlatform_Last;
		}

		inline const wstring& GetShaderPlatform(EShaderPlatform Platform) const
		{
			return Platforms[GetStaticPlatformIndex(Platform)].ShaderPlatform;
		}

		inline const wstring& GetShaderFormat(EShaderPlatform Platform) const
		{
			return Platforms[GetStaticPlatformIndex(Platform)].ShaderFormat;
		}

		inline const wstring& GetPlatformName(EShaderPlatform Platform) const
		{
			return Platforms[GetStaticPlatformIndex(Platform)].Name;
		}

	private:
		static inline uint32 GetStaticPlatformIndex(EShaderPlatform Platform)
		{
			BOOST_ASSERT(IsStaticPlatform(Platform));
			return uint32(Platform) - SP_StaticPlatform_First;
		}
	};

	enum class ERHIZBuffer
	{
		FarPlane = 1,
		NearPlane = 0,
		IsInverted = (int32)((int32)ERHIZBuffer::FarPlane < (int32)ERHIZBuffer::NearPlane),
	};

	inline bool isOpenGLPlatform(const EShaderPlatform platform)
	{
		return platform == SP_OPENGL_PCES3_1 || platform == SP_OPENGL_ES3_1_ANDROID;
	}



	inline bool isPCPlatform(const EShaderPlatform Platform)
	{
		return Platform == SP_PCD3D_SM5 || Platform == SP_PCD3D_ES3_1 ||
			Platform == SP_OPENGL_PCES3_1 || Platform == SP_METAL_SM5 ||
			Platform == SP_VULKAN_PCES3_1 || Platform == SP_VULKAN_SM5 || Platform == SP_METAL_MACES3_1;
	}

	inline bool isVulkanMobilePlatform(const EShaderPlatform platform)
	{
		return platform == SP_VULKAN_ES3_1_ANDROID || platform == SP_VULKAN_PCES3_1 || platform == SP_VULKAN_SM5;
	}

	inline bool RHIHasTiledGPU(const EShaderPlatform platform)
	{
		return platform == SP_METAL || platform == SP_METAL_TVOS
			|| platform == SP_OPENGL_ES3_1_ANDROID
			|| platform == SP_VULKAN_ES3_1_ANDROID;
	}

	

	inline bool isMobilePlatform(const EShaderPlatform platform)
	{
		return
			platform == SP_METAL || platform == SP_METAL_MACES3_1 || platform == SP_METAL_TVOS
			|| platform == SP_PCD3D_ES3_1
			|| platform == SP_OPENGL_PCES3_1 || platform == SP_OPENGL_ES3_1_ANDROID
			|| platform == SP_VULKAN_ES3_1_ANDROID || platform == SP_VULKAN_PCES3_1 || platform == SP_VULKAN_ES3_1_LUMIN
			|| platform == SP_SWITCH_FORWARD;
	}

	inline bool isMetalPlatform(const EShaderPlatform platform)
	{
		return platform == SP_METAL || platform == SP_METAL_MRT || platform == SP_METAL_TVOS || platform == SP_METAL_MRT_TVOS
			|| platform == SP_METAL_SM5_NOTESS || platform == SP_METAL_SM5
			|| platform == SP_METAL_MACES3_1 || platform == SP_METAL_MRT_MAC;
	}

	inline bool isVulkanPlatform(const EShaderPlatform platform)
	{
		return platform == SP_VULKAN_SM5 || platform == SP_VULKAN_SM5_LUMIN || platform == SP_VULKAN_PCES3_1
			|| platform == SP_VULKAN_ES3_1_ANDROID || platform == SP_VULKAN_ES3_1_LUMIN || platform == SP_VULKAN_SM5_ANDROID;
	}

	inline bool isAndroidOpenGLESPlatform(EShaderPlatform platform)
	{
		return platform == SP_OPENGL_ES3_1_ANDROID;
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

		ERasterizerFillMode_Num,
		ERasterizerFillMode_NumBits = 2,
	};

	static_assert(ERasterizerFillMode_Num <= (1 << ERasterizerFillMode_NumBits), "ERasterizerFillMode_Num will not fit on ERasterizerFillMode_NumBits");

	enum ERasterizerCullMode
	{
		CM_None,
		CM_CW,
		CM_CCW,	//	ÄæÊ±Õë
		ERasterizerCullMode_Num,
		ERasterizerCullMode_NumBits,
	};

	static_assert(ERasterizerCullMode_Num <= (1 << ERasterizerCullMode_NumBits), "ERasterizerCullMode_Num will not fit on ERasterizerCullMode_NumBits");

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
		BUF_Transient	= 0x2000,
		BUF_Shared		= 0x4000,
		UBF_AccelerationStructure = 0x8000,

		BUF_VertexBuffer = 0x10000,
		BUF_IndexBuffer = 0x20000,
		BUF_StructuredBuffer	= 0x40000,

		BUF_AnyDynamic = (BUF_Dynamic | BUF_Volatile),


	};
	ENUM_CLASS_FLAGS(EBufferUsageFlags);
	enum { MAX_TEXTURE_MIP_COUNT = 14 };

	enum class EClearDepthStencil
	{
		Depth,
		Stencil,
		DepthStencil
	};

	enum class EConstantBufferValidation
	{
		None,
		ValidateResources
	};

	inline bool isRDGResourceReferenceShaderParameterType(EConstantBufferBaseType baseType)
	{
		return
			baseType == CBMT_RDG_TEXTURE ||
			baseType == CBMT_RDG_TEXTURE_SRV ||
			baseType == CBMT_RDG_TEXTURE_UAV ||
			baseType == CBMT_RDG_BUFFER ||
			baseType == CBMT_RDG_BUFFER_SRV ||
			baseType == CBMT_RDG_BUFFER_UAV;
	}

	inline bool isShaderParameterTypeForConstantBufferLayout(EConstantBufferBaseType baseType)
	{
		return baseType == CBMT_TEXTURE ||
			baseType == CBMT_SRV ||
			baseType == CBMT_SAMPLER ||
			isRDGResourceReferenceShaderParameterType(baseType) ||
			baseType == CBMT_REFERENCED_STRUCT ||
			baseType == CBMT_RENDER_TARGET_BINDING_SLOTS;
	}


	inline bool RHISupportsShaderPipelines(EShaderPlatform platform)
	{
		return !isMobilePlatform(platform);
	}

	inline bool isSwitchPlatform(CONST EShaderPlatform platform)
	{
		return platform == SP_SWITCH || platform == SP_SWITCH_FORWARD;
	}

	inline bool RHISupportsComputeShaders(const EShaderPlatform platform)
	{
		return isFeatureLevelSupported(platform, ERHIFeatureLevel::SM5) || (getMaxSupportedFeatureLevel(platform) == ERHIFeatureLevel::ES3_1 && !isSwitchPlatform(platform));
	}

	inline bool RHISupportsGeometryShaders(const EShaderPlatform platform)
	{
		return isFeatureLevelSupported(platform, ERHIFeatureLevel::SM4) && !isMetalPlatform(platform) && !isVulkanMobilePlatform(platform);
	}

	enum
	{
		MaxImmutableSamplers = 2
	};

	inline bool isMetalMobilePlatform(const EShaderPlatform platform)
	{
		return platform == SP_METAL;
	}

	inline bool RHINeedsToSwitchVerticalAxis(EShaderPlatform platform)
	{
#if 0
#endif
		return isOpenGLPlatform(platform) && isMobilePlatform(platform) && !isPCPlatform(platform) && !isMetalMobilePlatform(platform) && !isVulkanPlatform(platform) && platform != SP_SWITCH && platform != SP_SWITCH_FORWARD;
	}

	inline bool RHIVolumeTextureRenderingSupportGuaranteed(const EShaderPlatform platform)
	{
		return isFeatureLevelSupported(platform, ERHIFeatureLevel::SM4)
			&& !isMetalPlatform(platform)
			&& !isOpenGLPlatform(platform);
	}

	struct GenericStaticShaderPlatform final
	{
		inline GenericStaticShaderPlatform(const EShaderPlatform inPlatform)
			:mPlatform(inPlatform)
		{
		}

		inline operator EShaderPlatform() const
		{
			return mPlatform;
		}


		inline bool operator != (const EShaderPlatform& other)
		{
			return other != mPlatform;
		}

		inline bool operator == (const EShaderPlatform& other)
		{
			return other == mPlatform;
		}
	private:
		const EShaderPlatform mPlatform;
	};

	extern RHI_API const wstring LANGUAGE_D3D;
	extern RHI_API const wstring LANGUAGE_Metal;
	extern RHI_API const wstring LANGUAGE_OpenGL;
	extern RHI_API const wstring LANGUAGE_Vulkan;
	extern RHI_API const wstring LANGUAGE_Sony;
	extern RHI_API const wstring LANGUAGE_Nintendo;

#if USE_STATIC_SHADER_PLATFORM_ENUMS
#else
	using StaticShaderPlatform = GenericStaticShaderPlatform;
#endif // USE_STATIC_SHADER_PLATFORM_ENUMS
	inline bool isSimulatedPlatform(const StaticShaderPlatform platform)
	{
		switch (platform)
		{
		case SP_PCD3D_ES3_1:
		case SP_OPENGL_PCES3_1:
		case SP_METAL_MACES3_1:
		case SP_VULKAN_PCES3_1:
			return true;
		default:
			return false;
		}
		return false;
	}


	class RHI_API FGenericDataDrivenShaderPlatformInfo
	{
		wstring Language;
		ERHIFeatureLevel::Type MaxFeatureLevel;
		uint32 bIsMobile : 1;
		uint32 bIsMetalMRT : 1;
		uint32 bIsPC : 1;
		uint32 bIsConsole : 1;
		uint32 bIsAndroidOpenGLES : 1;

		uint32 bSupportsMobileMultiView : 1;
		uint32 bSupportsVolumeTextureCompression : 1;
		uint32 bSupportsDistanceFields : 1; // used for DFShadows and DFAO - since they had the same checks
		uint32 bSupportsDiaphragmDOF : 1;
		uint32 bSupportsRGBColorBuffer : 1;
		uint32 bSupportsCapsuleShadows : 1;
		uint32 bSupportsVolumetricFog : 1; // also used for FVVoxelization
		uint32 bSupportsIndexBufferUAVs : 1;
		uint32 bSupportsInstancedStereo : 1;
		uint32 bSupportsMultiView : 1;
		uint32 bSupportsMSAA : 1;
		uint32 bSupports4ComponentUAVReadWrite : 1;
		uint32 bSupportsRenderTargetWriteMask : 1;
		uint32 bSupportsRayTracing : 1;
		uint32 bSupportsRayTracingIndirectInstanceData : 1; // Whether instance transforms can be copied from the GPU to the TLAS instances buffer
		uint32 bSupportsGPUSkinCache : 1;
		uint32 bSupportsGPUScene : 1;
		uint32 bSupportsByteBufferComputeShaders : 1;
		uint32 bSupportsPrimitiveShaders : 1;
		uint32 bSupportsUInt64ImageAtomics : 1;
		uint32 bSupportsTemporalHistoryUpscale : 1;
		uint32 bSupportsRTIndexFromVS : 1;
		uint32 bSupportsWaveOperations : 1; // Whether HLSL SM6 shader wave intrinsics are supported
		uint32 bRequiresExplicit128bitRT : 1;
		uint32 bSupportsGen5TemporalAA : 1;
		uint32 bTargetsTiledGPU : 1;
		uint32 bNeedsOfflineCompiler : 1;

		// NOTE: When adding fields, you must also add to ParseDataDrivenShaderInfo!
		uint32 bContainsValidPlatformInfo : 1;

		FGenericDataDrivenShaderPlatformInfo()
		{
			Memory::memzero(this, sizeof(*this));
			MaxFeatureLevel = ERHIFeatureLevel::Num;
		}

	public:
		static void initialize();
		static void parseDataDrivenShaderInfo(const ConfigSection& Section, FGenericDataDrivenShaderPlatformInfo& Info);

		static FORCEINLINE_DEBUGGABLE const bool getIsLanguageD3D(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].Language == LANGUAGE_D3D;
		}

		static FORCEINLINE_DEBUGGABLE const bool getIsLanguageMetal(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].Language == LANGUAGE_Metal;
		}

		static FORCEINLINE_DEBUGGABLE const bool getIsLanguageOpenGL(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].Language == LANGUAGE_OpenGL;
		}

		static FORCEINLINE_DEBUGGABLE const bool getIsLanguageVulkan(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].Language == LANGUAGE_Vulkan;
		}

		static FORCEINLINE_DEBUGGABLE const bool getIsLanguageSony(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].Language == LANGUAGE_Sony;
		}

		static FORCEINLINE_DEBUGGABLE const bool getIsLanguageNintendo(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].Language == LANGUAGE_Nintendo;
		}

		static FORCEINLINE_DEBUGGABLE const ERHIFeatureLevel::Type getMaxFeatureLevel(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].MaxFeatureLevel;
		}

		static FORCEINLINE_DEBUGGABLE const bool getIsMobile(const EShaderPlatform Platform)
		{
			return Infos[Platform].bIsMobile;
		}

		static FORCEINLINE_DEBUGGABLE const bool getIsMetalMRT(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bIsMetalMRT;
		}

		static FORCEINLINE_DEBUGGABLE const bool getIsPC(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bIsPC;
		}

		static FORCEINLINE_DEBUGGABLE const bool getIsConsole(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bIsConsole;
		}

		static FORCEINLINE_DEBUGGABLE const bool getIsAndroidOpenGLES(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bIsAndroidOpenGLES;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupportsMobileMultiView(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupportsMobileMultiView;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupportsVolumeTextureCompression(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupportsVolumeTextureCompression;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupportsDistanceFields(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupportsDistanceFields;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupportsDiaphragmDOF(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupportsDiaphragmDOF;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupportsRGBColorBuffer(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupportsRGBColorBuffer;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupportsCapsuleShadows(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupportsCapsuleShadows;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupportsVolumetricFog(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupportsVolumetricFog;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupportsIndexBufferUAVs(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupportsIndexBufferUAVs;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupportsInstancedStereo(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupportsInstancedStereo;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupportsMultiView(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupportsMultiView;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupportsMSAA(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupportsMSAA;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupports4ComponentUAVReadWrite(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupports4ComponentUAVReadWrite;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupportsRenderTargetWriteMask(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupportsRenderTargetWriteMask;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupportsRayTracing(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupportsRayTracing;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupportsRayTracingIndirectInstanceData(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupportsRayTracingIndirectInstanceData;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupportsGPUSkinCache(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupportsGPUSkinCache;
		}

		static FORCEINLINE_DEBUGGABLE const bool getTargetsTiledGPU(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bTargetsTiledGPU;
		}

		static FORCEINLINE_DEBUGGABLE const bool getNeedsOfflineCompiler(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bNeedsOfflineCompiler;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupportsByteBufferComputeShaders(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupportsByteBufferComputeShaders;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupportsWaveOperations(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupportsWaveOperations;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupportsTemporalHistoryUpscale(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupportsTemporalHistoryUpscale;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupportsGPUScene(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupportsGPUScene;
		}

		static FORCEINLINE_DEBUGGABLE const bool getRequiresExplicit128bitRT(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bRequiresExplicit128bitRT;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupportsGen5TemporalAA(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupportsGen5TemporalAA;
		}

		static FORCEINLINE_DEBUGGABLE const bool getSupportsUInt64ImageAtomics(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bSupportsUInt64ImageAtomics;
		}

	private:
		static FGenericDataDrivenShaderPlatformInfo Infos[SP_NumPlatforms];

	public:
		static bool isValid(const StaticShaderPlatform Platform)
		{
			return Infos[Platform].bContainsValidPlatformInfo;
		}
	};


}