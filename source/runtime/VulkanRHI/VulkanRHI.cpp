

#include "VulkanRHIPrivate.h"
#include "VulkanDynamicRHI.h"
#include "Misc/Parse.h"
#include "Misc/CommandLine.h"
#include "VulkanDevice.h"
#include "RHICommandList.h"
#include "RenderingThread.h"
#include "Containers/LinkList.h"
#include "RenderResource.h"
#include "BoundShaderStateCache.h"
#include "VulkanResources.h"
#include "VulkanConfiguration.h"
#include "VulkanState.h"
namespace Air
{
	extern RHI_API bool GUseTexture3DBulkDataRHI;

	bool VulkanDynamicRHIModule::isSupported()
	{
		return VulkanPlatform::isSupported();
	}

	DynamicRHI* VulkanDynamicRHIModule::createRHI(ERHIFeatureLevel::Type inRequestedFeatureLevel)
	{
		if (!GIsEditor && (VulkanPlatform::requiresMobileRenderer() || inRequestedFeatureLevel == ERHIFeatureLevel::ES3_1 || inRequestedFeatureLevel == ERHIFeatureLevel::ES2 || Parse::param(CommandLine::get(), TEXT("featureleveles31")) || Parse::param(CommandLine::get(), TEXT("featureleveles2"))))
		{
			GMaxRHIFeatureLevel = ERHIFeatureLevel::ES3_1;
			GMaxRHIShaderPlatform = SP_VULKAN_PCES3_1;
		}
		else if (inRequestedFeatureLevel == ERHIFeatureLevel::SM4)
		{
			GMaxRHIFeatureLevel = ERHIFeatureLevel::SM4;
			GMaxRHIShaderPlatform = SP_VULKAN_SM4;
		}
		else
		{
			GMaxRHIFeatureLevel = ERHIFeatureLevel::SM5;
			GMaxRHIShaderPlatform = SP_VULKAN_SM5;
		}

		GVulkanRHI = new VulkanDynamicRHI();

		return GVulkanRHI;
	}

	void VulkanDynamicRHI::init()
	{
		if (!VulkanPlatform::loadVulkanLibrary())
		{
			AIR_LOG(LogVulkanRHI, Fatal, TEXT("Failed to find all required Vulkan entry points;"));
		}

		{

		}

		initInstance();

		if (GPoolSizeVRAMPercentage > 0)
		{
			const uint64 totalGPUMemory = mDevice->getMemoryMananger().getTotalMemory(true);
			float poolSize = float(GPoolSizeVRAMPercentage) * 0.01f * float(totalGPUMemory);
			GTexturePoolSize = int64(GenericPlatformMath::truncToFloat(poolSize / 1024.0f / 1024.0f)) * 1024 * 1024;

			AIR_LOG(LogRHI, Log, TEXT("Texture pool is %llu MB (%d%% of %llu MB)"), GTexturePoolSize / 1024 / 1024,
				GPoolSizeVRAMPercentage,
				totalGPUMemory / 1024 / 1024);
		}
		else
		{
		}
	}

	void VulkanDynamicRHI::shutdown()
	{
		if (Parse::param(CommandLine::get(), TEXT("saveulkansocacheonexit")))
		{

		}

		BOOST_ASSERT(isInGameThread() && isInRenderingThread());
		BOOST_ASSERT(mDevice);

		mDevice->prepareForDestroy();

		emptyCachedBoundShaderStates();

		VulkanVertexDeclaration::emptyCache();

		if (GIsRHIInitialized)
		{
			GIsRHIInitialized = false;
			VulkanPlatform::overridePlatformHandlers(false);
			GRHINeedsExtraDeletionLatency = false;
			BOOST_ASSERT(!GIsCriticalError);
			for (TLinkedList<RenderResource*>::TIterator resourceIt(RenderResource::getResourceList()); resourceIt; resourceIt.next())
			{
				RenderResource* resource = *resourceIt;
				BOOST_ASSERT(resource->isInitialized());
				resource->releaseRHI();
			}

			for (TLinkedList<RenderResource*>::TIterator resourceIt(RenderResource::getResourceList()); resourceIt; resourceIt.next())
			{
				resourceIt->releaseDynamicRHI();
			}

			{
				for (auto& pair : mDevice->mSamplerMap)
				{
					VulkanSamplerState* samplerState = (VulkanSamplerState*)pair.second.getReference();
					vkDestroySampler(mDevice->getInstanceHandle(), samplerState->mSampler, VULKAN_CPU_ALLOCATOR);
				}
				mDevice->mSamplerMap.empty();
			}
			RHIResource::flushPendingDeletes();

			RHIResource::flushPendingDeletes();
		}

		mDevice->destroy();

		delete mDevice;

		mDevice = nullptr;

#if VULKAN_HAS_DEBUGGING_ENABLED

#endif
		vkDestroyInstance(mInstance, VULKAN_CPU_ALLOCATOR);

		VulkanPlatform::freeVulkanLibrary();
	}

	void VulkanDynamicRHI::RHIAcquireThreadOwnership()
	{
	}

	void VulkanDynamicRHI::RHIReleaseThreadOwnership()
	{

	}



	void VulkanDynamicRHI::initInstance()
	{
		BOOST_ASSERT(isInGameThread());

		SCOPED_SUSPEND_RENDERING_THREAD(false);

		if (!mDevice)
		{
			BOOST_ASSERT(!GIsRHIInitialized);
			VulkanPlatform::overridePlatformHandlers(true);

			GRHISupportsAsyncTextureCreation = false;

			GEnableAsyncCompute = false;

			createInstance();

			selectAndInitDevice();

			const VkPhysicalDeviceProperties& props = mDevice->getDevicePropertise();

			GRHISupportsFirstInstance = true;

			GRHISupportsDynamicResolution = VulkanPlatform::supportsDynamicResolution();

			GSupportsDepthBoundsTest = mDevice->getPhysicalFeatures().depthBounds != 0;

			GSupportsRenderTargetFormat_PF_G8 = false;

			GRHISupportsTextureStreaming = true;

			GSupportsTimestampRenderQueries = VulkanPlatform::supportsTimestampRenderQueries();

			GRHISupportsRHIThread = false;

			GRHISupportsParallelRHIExecute = false;

			GSupportsParallelRenderingTasksWithSeparateRHIThread = GRHISupportsRHIThread ? VulkanPlatform::supportParallelRenderingTask() : false;

			GSupportsEfficientAsyncCompute = isRHIDeviceAMD() && (mDevice->mComputeContext != mDevice->mImmediateContext);

			GSupportsVolumeTextureRendering = true;

			GRHINeedsExtraDeletionLatency = true;

			GRHISupportsCopyToTextureMultipleMips = true;

			GMaxShadowDepthBufferSizeX = PlatformMath::min<int32>(props.limits.maxImageDimension2D, GMaxShadowDepthBufferSizeX);
			GMaxShadowDepthBufferSizeY = PlatformMath::min<int32>(props.limits.maxImageDimension2D, GMaxShadowDepthBufferSizeY);
			GMaxTextureDemensions = props.limits.maxImageDimension2D;
			GMaxTextureMipCount = PlatformMath::ceilLogTwo(GMaxTextureDemensions) + 1;
			GMaxTextureMipCount = PlatformMath::min<int32>(MAX_TEXTURE_MIP_COUNT, GMaxTextureMipCount);
			GMaxTextureCubeDemensions = props.limits.maxImageDimensionCube;
			GMaxTextureArrayLayers = props.limits.maxImageArrayLayers;
			GRHISupportsBaseVertexIndex = true;

			GSupportsSeparateRenderTargetBlendState = true;

			GSupportsDepthFetchDuringDepthTest = VulkanPlatform::supportsDepthFetchDuringDepthTest();

			VulkanPlatform::setupFeatureLevels();

			GRHIRequiresRenderTargetForPixelShaderUAVs = true;

			GUseTexture3DBulkDataRHI = true;

			for (TLinkedList<RenderResource*>::TIterator resourceIt(RenderResource::getResourceList()); resourceIt; resourceIt.next())
			{
				resourceIt->initRHI();
			}

			for (TLinkedList<RenderResource*>::TIterator resourceIt(RenderResource::getResourceList()); resourceIt; resourceIt.next())
			{
				resourceIt->initDynamicRHI();
			}

			GProjectionSignY = 1.0f;

			GIsRHIInitialized = true;


		}
	}

	void VulkanDynamicRHI::createInstance()
	{
		VkApplicationInfo appInfo;
		zeroVulkanStruct(appInfo, VK_STRUCTURE_TYPE_APPLICATION_INFO);
		appInfo.pApplicationName = nullptr;
		appInfo.applicationVersion = 0;
		appInfo.pEngineName = nullptr;
		appInfo.engineVersion = 0;
		appInfo.apiVersion = AIR_VK_API_VERSION;

		VkInstanceCreateInfo instInfo;
		zeroVulkanStruct(instInfo, VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
		instInfo.pApplicationInfo = &appInfo;
		getInstanceLayersAndExtensions(mInstanceExtension, mInstanceLayers, bSupportsDebugUtilsExt);

		instInfo.enabledExtensionCount = mInstanceExtension.size();
		instInfo.ppEnabledExtensionNames = instInfo.enabledExtensionCount > 0 ? (const ANSICHAR * const*)mInstanceExtension.getData() : nullptr;

		instInfo.enabledLayerCount = mInstanceLayers.size();
		instInfo.ppEnabledLayerNames = instInfo.enabledLayerCount > 0 ? (const ANSICHAR * const*)mInstanceLayers.getData() : nullptr;

#if VULKAN_HAS_DEBUGGING_ENABLED
		bSupportsDebugCallbackExt = bSupportsDebugUtilsExt && mInstanceExtension.containsByPredicate([](const ANSICHAR* key) {
			return key && !CStringAnsi::strcmp(key, VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			});
#endif
		VkResult result = vkCreateInstance(&instInfo, VULKAN_CPU_ALLOCATOR, &mInstance);
		if (result == VK_ERROR_INCOMPATIBLE_DRIVER)
		{
			PlatformMisc::requestExit(true);
			return;
		}
		else if (result == VK_ERROR_EXTENSION_NOT_PRESENT)
		{
			wstring missingExtensions;
			uint32 propertyCount;
			vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr);

			TArray<VkExtensionProperties> properties;
			properties.setNum(propertyCount);
			vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, properties.getData());

			for (const ANSICHAR* extension : mInstanceExtension)
			{
				bool bExtensionFound = false;
				for (uint32 propertyIndex = 0; propertyIndex < propertyCount; propertyIndex++)
				{
					const char* propertyExtensionName = properties[propertyIndex].extensionName;
					if (!CStringAnsi::strcmp(propertyExtensionName, extension))
					{
						bExtensionFound = true;
						break;
					}
				}

				if (!bExtensionFound)
				{
					wstring extensionStr = ANSI_TO_TCHAR(extension);
					AIR_LOG(logVulkanRHI, Error, TEXT("Missing required Vulkan extension: %s"), extensionStr.c_str());
					missingExtensions += extensionStr + TEXT("\n");
				}
			}
			PlatformMisc::messageBoxExt(EAppMsgType::Ok, String::printf(TEXT("Vulkan driver doesn't contain specified extensions:\n%s;\n\
			make sure your layers path is set appropriately."), missingExtensions.c_str()).c_str(), TEXT("Incomplete Vulkan drier found!"));
		}
		else if (result != VK_SUCCESS)
		{
			PlatformMisc::messageBoxExt(EAppMsgType::Ok, TEXT("´´½¨ÊµÀýÊ§°Ü"), TEXT("No Vulkan driver found!"));
			PlatformMisc::requestExit(true);
			return;
		}

		VERIFYVULKANRESULT(result);
		if (!VulkanPlatform::loadVulkanInstanceFunctions(mInstance))
		{
			PlatformMisc::messageBoxExt(EAppMsgType::Ok, TEXT("Failed to find all required vulkan entry poinsts"), TEXT("No Vulkan entry points found!"));
		}

#if VULKAN_HAS_DEBUGGING_ENABLED
		
#endif
	}

}