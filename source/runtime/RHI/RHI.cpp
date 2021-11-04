#include "RHI.h"
#include "GenericPlatform/genericPlatformDriver.h"
#include "RHIResource.h"
#include "RHICommandList.h"
#include "DynamicRHI.h"
#include "RHIShaderFormatDefinitions.inl"
namespace Air
{
	bool GRHISupportsRHIThread = false;
	bool GUsingNullRHI = false;
	bool GRHISupportsRayTracing = false;

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

	bool GSupportsEfficientAsyncCompute = false;

	bool GRHIDeviceIsAMDPreGCNArchitecture = false;

	bool GRHINeedsExtraDeletionLatency = false;

	bool GRHISupportsTextureStreaming = false;

	bool GRHISupportsParallelRHIExecute = false;

	bool GRHISupportsFirstInstance = false;

	bool GSupportsRenderTargetFormat_PF_FloatRGBA = true;

	bool GSupportsSeparateRenderTargetBlendState = false;

	bool GSupportsDepthFetchDuringDepthTest = true;

	bool GSupportsTransientResourceAliasing = false;

	bool GSupportsResourceView = true;

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

	int32 GMaxTextureDemensions = 2048;

	int32 GMaxTextureDepth = 2048;

	int32 GMaxTextureCubeDemensions = 2048;

	int32 GMaxTextureArrayLayers = 256;

	int64 GMaxBufferDimensions = 2 << 27;

	RHI_API ERHIFeatureLevel::Type GMaxRHIFeatureLevel = ERHIFeatureLevel::SM5;

#if WITH_SLI
	int32 GNumActiveGPUsForRendering = 1;
#endif

	TLockFreePointerListUnordered<RHIResource, PLATFORM_CACHE_LINE_SIZE> RHIResource::mPendingDeletes;

	TArray<RHIResource::ResourceToDelete> RHIResource::mdeferredDeletionQueue;

	RHIResource* RHIResource::mCurrentlyDeleting = nullptr;

	uint32 RHIResource::mCurrentFrame = 0;

	VertexElementTypeSupportInfo GVertexElementTypeSupport;


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
	inline bool IsVulkanSM5Platform(const StaticShaderPlatform Platform)
	{
		return Platform == SP_VULKAN_SM5 || Platform == SP_VULKAN_SM5_LUMIN || Platform == SP_VULKAN_SM5_ANDROID;
	}
	RHI_API bool RHISupportsTessellation(const EShaderPlatform platform)
	{
		if (isFeatureLevelSupported(platform, ERHIFeatureLevel::SM5))
		{
			return (platform == SP_PCD3D_SM5) || (platform == SP_XBOXONE_D3D12) || (platform == SP_METAL_SM5) || (IsVulkanSM5Platform(platform));
		}
		return false;
	}

	RHI_API int32 GNumDrawCallsRHI = 0;

	RHI_API int32 GNumPrimitivesDrawnRHI = 0;

	void RHIPrivateBeginFrame()
	{
		GNumDrawCallsRHI = 0;
		GNumPrimitivesDrawnRHI = 0;
	}

	

	EShaderPlatform shaderFormatToLegacyShaderPlatform(wstring shaderFormat)
	{
		return shaderFormatNameToShaderPlatform(shaderFormat);
	}

	wstring legacyShaderPlatformToShaderFormat(EShaderPlatform platform)
	{
		return shaderPlatformToShaderFormatName(platform);
	}

	void RHIResource::flushPendingDeletes()
	{

		BOOST_ASSERT(isInRenderingThread());
		RHICommandListImmediate& RHICmdList = RHICommandListExecutor::getImmediateCommandList();
		if (GDynamicRHI)
		{
			RHICmdList.submitCommandHint();
		}
		RHICmdList.immediateFlush(EImmediateFlushType::FlushRHIThread);
		RHICommandListExecutor::checkNoOutstandingCmdLists();
		if (GDynamicRHI)
		{
			GDynamicRHI->RHIPerFrameRHIFlushComplete();
		}

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
					PlatformMisc::memoryBarrier();
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
	const wstring LANGUAGE_D3D(TEXT("D3D"));
	const wstring LANGUAGE_Metal(TEXT("Metal"));
	const wstring LANGUAGE_OpenGL(TEXT("OpenGL"));
	const wstring LANGUAGE_Vulkan(TEXT("Vulkan"));
	const wstring LANGUAGE_Sony(TEXT("Sony"));
	const wstring LANGUAGE_Nintendo(TEXT("Nintendo"));
	RHI_API uint64 GRHITransitionPrivateData_SizeInBytes = 0;
	RHI_API uint64 GRHITransitionPrivateData_AlignInBytes = 0;


	RHI_API FGenericDataDrivenShaderPlatformInfo FGenericDataDrivenShaderPlatformInfo::Infos[SP_NumPlatforms];

	void RHITransition::cleanup() const
	{
		RHITransition* transition = const_cast<RHITransition*>(this);
		RHIReleaseTransition(transition);
		transition->~RHITransition();
		Memory::free(transition);
	}
	
}