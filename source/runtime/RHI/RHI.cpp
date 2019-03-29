#include "RHI.h"
#include "GenericPlatform/genericPlatformDriver.h"
#include "RHIResource.h"
#include "RHICommandList.h"
namespace Air
{
	bool GRHISupportsRHIThread = false;
	bool GUsingNullRHI = false;

	wstring FeatureLevelNames[] =
	{
		TEXT("ES2"),
		TEXT("ES3_1"),
		TEXT("SM4"),
		TEXT("SM5")
	};


	RHI_API int32 volatile GCurrentTextureMemorySize = 0;
	RHI_API int32 volatile GCurrentRendertargetMemorySize = 0;
	RHI_API int64 GTexturePoolSize = 0 * 1024 * 1024;
	RHI_API int32 GPoolSizeVRAMPercentage = 0;
	RHI_API EShaderPlatform GMaxRHIShaderPlatform = SP_PCD3D_SM5;
	wstring GRHIAdapterName;
	wstring GRHIAdapterInternalDriverVersion;
	wstring GRHIAdapterUserDriverVersion;
	wstring GRHIAdapterDriverDate;
	uint32 GRHIVendorId = 0;
	uint32 GRHIDeviceId = 0;
	uint32 GRHIDeviceRevision = 0;

	bool GSupportsDepthBoundsTest = false;


	bool GRHISupportsAsyncTextureCreation = false;

	bool GRHIDeviceIsAMDPreGCNArchitecture = false;

	bool GRHINeedsExtraDeletionLatency = false;

	bool GRHISupportsTextureStreaming = false;

	bool GRHISupportsParallelRHIExecute = false;

	bool GRHISupportsFirstInstance = false;

	bool GSupportsRenderTargetFormat_PF_FloatRGBA = true;

	bool GSupportsSeparateRenderTargetBlendState = false;

	bool GSupportsDepthFetchDuringDepthTest = true;

	float GMinClipZ = 0.0f;

	float GProjectionSignY = 1.0f;

	// Set the RHI initialized flag.
	bool GIsRHIInitialized = false;

	EPixelFormat GRHIHDRDisplayOutputFormat = PF_FloatRGBA;
	bool GRHISupportsHDROutputs = false;

	bool GSupportsRenderTargetWriteMask = false;

	bool GRHISupportsMSAADepthSampleAccess = false;

	bool GSupportsTimestampRenderQueries = false;

	bool GSupportsResolveCubemapFaces = false;

	int32 GMaxTextureMipCount = MAX_TEXTURE_MIP_COUNT;

	int32 GMaxShadowDepthBufferSizeX = 2048;

	int32 GMaxShadowDepthBufferSizeY = 2048;

	int32 GMaxTexture2DDemensions = 2048;

	int32 GMaxTextureDepth = 2048;

	int32 GMaxTextureCubeDemensions = 2048;

	int32 GMaxTextureArrayLayers = 256;

	RHI_API ERHIFeatureLevel::Type GMaxRHIFeatureLevel = ERHIFeatureLevel::SM5;

#if WITH_SLI
	int32 GNumActiveGPUsForRendering = 1;
#endif

	TLockFreePointerListUnordered<RHIResource, PLATFORM_CACHE_LINE_SIZE> RHIResource::mPendingDeletes;

	TArray<RHIResource::ResourceToDelete> RHIResource::mdeferredDeletionQueue;

	RHIResource* RHIResource::mCurrentlyDeleting = nullptr;

	uint32 RHIResource::mCurrentFrame = 0;


	RHI_API EShaderPlatform GShaderPlatformForFeatureLevel[ERHIFeatureLevel::Num] = { SP_NumPlatforms, SP_NumPlatforms , SP_NumPlatforms , SP_NumPlatforms };

	RHI_API bool isRHIDeviceIntel()
	{
		return GRHIVendorId == RHIV_Intel;
	}

	RHI_API bool isRHIDeviceAMD()
	{
		return GRHIVendorId == RHIV_AMD;
	}

	RHI_API void getFeatureLevelName(ERHIFeatureLevel::Type inFeatureLevel, wstring& outName)
	{
		BOOST_ASSERT(inFeatureLevel < ARRAY_COUNT(FeatureLevelNames));
		outName = FeatureLevelNames[(int32)inFeatureLevel];
	}

	RHI_API bool isRHIDeviceNVIDIA()
	{
		return RHIV_NVIDIA == GRHIVendorId;
	}

	RHI_API bool RHISupportsTessellation(const EShaderPlatform platform)
	{
		if (isFeatureLevelSupported(platform, ERHIFeatureLevel::SM5) && !isMetalPlatform(platform))
		{
			return (platform == SP_PCD3D_SM5) || (platform == SP_XBOXONE) || (platform == SP_OPENGL_SM5) || (platform == SP_OPENGL_ES31_EXT) || (platform == SP_VULKAN_SM5);
		}
	}

	RHI_API int32 GNumDrawCallsRHI = 0;

	RHI_API int32 GNumPrimitivesDrawnRHI = 0;

	void RHIPrivateBeginFrame()
	{
		GNumDrawCallsRHI = 0;
		GNumPrimitivesDrawnRHI = 0;
	}

	static wstring NAME_PCD3D_SM5(TEXT("PCD3D_SM5"));
	static wstring NAME_PCD3D_SM4(TEXT("PCD3D_SM4"));
	static wstring NAME_PCD3D_ES3_1(TEXT("PCD3D_ES31"));
	static wstring NAME_PCD3D_ES2(TEXT("PCD3D_ES2"));
	static wstring NAME_GLSL_150(TEXT("GLSL_150"));
	static wstring NAME_GLSL_150_MAC(TEXT("GLSL_150_MAC"));
	static wstring NAME_SF_PS4(TEXT("SF_PS4"));
	static wstring NAME_SF_XBOXONE(TEXT("SF_XBOXONE"));
	static wstring NAME_GLSL_430(TEXT("GLSL_430"));
	static wstring NAME_GLSL_150_ES2(TEXT("GLSL_150_ES2"));
	static wstring NAME_GLSL_150_ES2_NOUB(TEXT("GLSL_150_ES2_NOUB"));
	static wstring NAME_GLSL_150_ES31(TEXT("GLSL_150_ES31"));
	static wstring NAME_GLSL_ES2(TEXT("GLSL_ES2"));
	static wstring NAME_GLSL_ES2_WEBGL(TEXT("GLSL_ES2_WEBGL"));
	static wstring NAME_GLSL_ES2_IOS(TEXT("GLSL_ES2_IOS"));
	static wstring NAME_SF_METAL(TEXT("SF_MATAL"));
	static wstring NAME_SF_MATAL_MRT(TEXT("SF_MATAL_MRT"));
	static wstring NAME_GLSL_310_ES_EXT(TEXT("GLSL_310_ES_EXT"));
	static wstring NAME_GLSL_ES3_1_ANDROID(TEXT("GLSL_310_ES3_1_ANDROID"));
	static wstring NAME_SF_MATAL_SM5(TEXT("SF_MATAL_SM5"));
	static wstring NAME_VULKAN_ES3_1_ANDROID(TEXT("SF_VULKAN_ES31_ANDROID"));
	static wstring NAME_VULKAN_ES3_1(TEXT("SF_VULKAN_ES31"));
	static wstring NAME_VULKAN_SM4_UB(TEXT("SF_VULKAN_SM4_UB"));
	static wstring NAME_VULKAN_SM4(TEXT("SF_VULKAN_SM4"));
	static wstring NAME_VULKAN_SM5(TEXT("SF_VULKAN_SM5"));
	static wstring NAME_SF_METAL_SM4(TEXT("SF_METAL_SM4"));
	static wstring NAME_SF_METAL_MACES3_1(TEXT("SF_METAL_MACES3_1"));
	static wstring NAME_SF_METAL_MACES2(TEXT("SF_METAL_MACES2"));
	static wstring NAME_GLSL_SWITCH(TEXT("GLSL_SWITCH"));
	static wstring NAME_GLSL_SWITCH_FORWARD(TEXT("GLSL_SWITCH_FORWARD"));



	wstring legacyShaderPlatformToShaderFormat(EShaderPlatform platform)
	{
		switch (platform)
		{
		case Air::SP_PCD3D_SM5:
			return NAME_PCD3D_SM5;
		case Air::SP_OPENGL_SM4:
			return NAME_GLSL_150;
		case Air::SP_PS4:
			return NAME_SF_PS4;
		case Air::SP_OPENGL_PCES2:
		{
			return NAME_GLSL_150_ES2;
		}
		case Air::SP_XBOXONE:
			return NAME_SF_XBOXONE;
		case Air::SP_PCD3D_SM4:
			return NAME_PCD3D_SM4;
		case Air::SP_OPENGL_SM5:
			return NAME_GLSL_430;
		case Air::SP_PCD3D_ES2:
			return NAME_PCD3D_ES2;
		case Air::SP_OPENGL_ES2_ANDROID:
			return NAME_GLSL_ES2;
		case Air::SP_OPENGL_ES2_WEBGL:
			return NAME_GLSL_ES2_WEBGL;
		case Air::SP_OPENGL_ES2_IOS:
			return NAME_GLSL_ES2_IOS;
		case Air::SP_METAL:
			return NAME_SF_METAL;
		case Air::SP_OPENGL_SM4_MAC:
			return NAME_GLSL_150_MAC;
		case Air::SP_METAL_MRT:
			return NAME_SF_MATAL_MRT;
		case Air::SP_OPENGL_ES31_EXT:
			return NAME_GLSL_310_ES_EXT;
		case Air::SP_PCD3D_ES3_1:
			return NAME_PCD3D_ES3_1;
		case Air::SP_OPENGL_PCES3_1:
			return NAME_GLSL_150_ES31;
		case Air::SP_METAL_SM5:
			return NAME_SF_MATAL_SM5;
		case Air::SP_VULKAN_PCES3_1:
			return NAME_VULKAN_ES3_1;
		case Air::SP_METAL_SM4:
			return NAME_SF_METAL_SM4;
		case Air::SP_VULKAN_SM4:
			return NAME_VULKAN_SM4;
		case Air::SP_VULKAN_SM5:
			return NAME_VULKAN_SM5;
		case Air::SP_VULKAN_ES3_1_ANDROID:
			return NAME_VULKAN_ES3_1_ANDROID;
		case Air::SP_METAL_MACES3_1:
			return NAME_SF_METAL_MACES3_1;
		case Air::SP_METAL_MACES2:
			return NAME_SF_METAL_MACES2;
		case Air::SP_OPENGL_ES3_1_ANDROID:
			return NAME_GLSL_ES3_1_ANDROID;
		case Air::SP_SWITCH:
			return NAME_GLSL_SWITCH;
		case Air::SP_SWITCH_FORWARD:
			return NAME_GLSL_SWITCH_FORWARD;
		default:
			BOOST_ASSERT(false);
			return NAME_PCD3D_SM5;
		}
	}

	void RHIResource::flushPendingDeletes()
	{
		RHICommandListExecutor::checkNoOutstandingCmdLists();
		RHICommandListExecutor::getImmediateCommandList().immediateFlush(EImmediateFlushType::FlushRHIThread);

		auto Delete = [](TArray<RHIResource*>& toDelete)
		{
			for (int32 index = 0; index < toDelete.size(); index++)
			{
				RHIResource* ref = toDelete[index];
				BOOST_ASSERT(ref->mMarkedForDelete == 1);
				if (ref->GetRefCount() == 0)
				{
					mCurrentlyDeleting = ref;
					delete ref;
					mCurrentlyDeleting = nullptr;
				}
				else
				{
					ref->mMarkedForDelete = 0;
				}
			}
		};
		while (1)
		{
			if (mPendingDeletes.isEmpty())
			{
				break;
			}
			if (platformNeedsExtradeletionLatency())
			{
				const int32 index = mdeferredDeletionQueue.addDefaulted();
				ResourceToDelete& resourceBatch = mdeferredDeletionQueue[index];
				resourceBatch.mFrameDeleted = mCurrentFrame;
				mPendingDeletes.popAll(resourceBatch.mResource);
			}
			else
			{
				TArray<RHIResource*> toDelete;
				mPendingDeletes.popAll(toDelete);
				Delete(toDelete);
			}
		}

		const uint32 numFrameToExpire = 3;
		if (mdeferredDeletionQueue.size())
		{
			int32 deletebatchCount = 0;
			while (deletebatchCount < mdeferredDeletionQueue.size())
			{
				ResourceToDelete& resourceBatch = mdeferredDeletionQueue[deletebatchCount];
				if (((resourceBatch.mFrameDeleted + numFrameToExpire) < mCurrentFrame) || !GIsRHIInitialized)
				{
					Delete(resourceBatch.mResource);
					++deletebatchCount;
				}
				else
				{
					break;
				}
			}
			if (deletebatchCount)
			{
				mdeferredDeletionQueue.removeAt(0, deletebatchCount);
			}
			++mCurrentFrame;
		}
	}

	bool RHIResource::bypass()
	{
		return GRHICommandList.bypass();
	}

	const ClearValueBinding ClearValueBinding::None(EClearBinding::ENoneBound);
	const ClearValueBinding ClearValueBinding::Black(LinearColor(0.0f, 0.0f, 0.0f, 1.0f));
	const ClearValueBinding ClearValueBinding::White(LinearColor(1.0f, 1.0f, 1.0f, 1.0f));
	const ClearValueBinding ClearValueBinding::Transparent(LinearColor(0.0f, 0.0f, 0.0f, 0.0f));
	const ClearValueBinding ClearValueBinding::DepthOne(1.0f, 0);
	const ClearValueBinding ClearValueBinding::DepthZero(0.0f, 0);
	const ClearValueBinding ClearValueBinding::DepthNear((float)ERHIZBuffer::NearPlane, 0);
	const ClearValueBinding ClearValueBinding::DepthFar((float)ERHIZBuffer::FarPlane, 0);
	const ClearValueBinding ClearValueBinding::Green(LinearColor(0.0f, 1.0f, 0.0f, 1.0f));
	const ClearValueBinding ClearValueBinding::MidGray(LinearColor(0.5f, 0.5f, 0.5f, 1.0f));

}