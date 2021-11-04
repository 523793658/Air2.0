#include "RHI.h"
#include "Modules/ModuleManager.h"
#include "Misc/Parse.h"
#include "Misc/CommandLine.h"
#include "RHI.h"
#include "Misc/App.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProperties.h"
#include "DynamicRHI.h"
#if PLATFORM_WINDOWS
namespace Air
{
	static const TCHAR* GLoadedRHIModuleName;
	class IDynamicRHIModule;
	static bool shouldPreferD3D12()
	{
#if 0
#else
		return false;
#endif
	}

	static IDynamicRHIModule* loadDynamicRHIMOdule(ERHIFeatureLevel::Type& desiredFeature, const TCHAR*& loadedRHIModuleName)
	{
		bool bPreferD3D12 = shouldPreferD3D12();

		bool bForceSM5 = Parse::param(CommandLine::get(), TEXT("sm5"));

		bool bForceSM4 = Parse::param(CommandLine::get(), TEXT("sm4"));

		bool bForceVulkan = Parse::param(CommandLine::get(), TEXT("vulkan"));

		bool bForceOpenGL = WindowsPlatformMisc::verifyWindowsVersion(6, 0) == false || Parse::param(CommandLine::get(), TEXT("opengl")) || Parse::param(CommandLine::get(), TEXT("opengl3")) || Parse::param(CommandLine::get(), TEXT("opengl4"));
		bool bForceD3D11 = Parse::param(CommandLine::get(), TEXT("d3d11")) || Parse::param(CommandLine::get(), TEXT("dx11")) || (bForceSM5 && !bForceVulkan && !bForceOpenGL);
		bool bForceD3D12 = Parse::param(CommandLine::get(), TEXT("d3d12")) || Parse::param(CommandLine::get(), TEXT("dx12"));
		desiredFeature = ERHIFeatureLevel::Num;

		if (!(bForceVulkan || bForceOpenGL || bForceD3D11 || bForceD3D12))
		{
			ConfigFile engineSettings;
			wstring platformNameString(PlatformProperties::platformName());
			const TCHAR* platformName = platformNameString.c_str();
			ConfigCacheIni::loadLocalIniFile(engineSettings, TEXT("Engine"), true, platformName);
			wstring defaultGraphicsRHI;
			if (engineSettings.getString(TEXT("/Script/WindowsTargetPlatform.WindowsTargetSettings"), TEXT("DefaultGraphicsRHI"), defaultGraphicsRHI))
			{
				static wstring NAME_DX11(TEXT("DefaultGraphicsRHI_DX11"));
				static wstring NAME_DX12(TEXT("DefaultGraphicsRHI_DX12"));
				static wstring NAME_VULKAN(TEXT("DefaultGraphicsRHI_Vulkan"));
				if (defaultGraphicsRHI == NAME_DX11)
				{
					bForceD3D11 = true;
				}
				else if (defaultGraphicsRHI == NAME_DX12)
				{
					bForceD3D12 = true;
				}
				else if (defaultGraphicsRHI == NAME_VULKAN)
				{
					bForceVulkan = true;
				}

			}
		}
		int32 sum = ((bForceD3D12 ? 1 : 0) + (bForceD3D11 ? 1 : 0) + (bForceOpenGL ? 1 : 0) + (bForceVulkan ? 1 : 0));
		if (bForceSM5 && bForceSM4)
		{
			AIR_LOG(LogRHI, Fatal, TEXT("-sm4 and -sm5 are mutually exclusive options, but more than one was specified on the command-line"));
		}
		if (sum > 1)
		{

		}
		else if (sum == 0)
		{
			TArray<wstring> targetedShaderFormats;
			GConfig->getArray(TEXT("/Script/WindowsTargetPlatform.WindowsTargetSettings"), TEXT("TargetedRHIs"), targetedShaderFormats, GEngineIni);
			if (targetedShaderFormats.size() > 0)
			{
				wstring shaderFormatName(targetedShaderFormats[0]);
				EShaderPlatform targetedPlatform = shaderFormatToLegacyShaderPlatform(shaderFormatName);
				bForceVulkan = isVulkanPlatform(targetedPlatform);
				bForceD3D11 = !bPreferD3D12 && isD3DPlatform(targetedPlatform, false);
				bForceOpenGL = isOpenGLPlatform(targetedPlatform);
				desiredFeature = getMaxSupportedFeatureLevel(targetedPlatform);
			}
		}
		else
		{
			if (bForceSM5)
			{
				desiredFeature = ERHIFeatureLevel::SM5;
			}
			if (bForceSM4)
			{
				desiredFeature = ERHIFeatureLevel::SM4;
				bPreferD3D12 = false;
			}
		}

		IDynamicRHIModule* dynamicRHIModule = nullptr;

		if (bForceOpenGL)
		{
			
		}
		else if (bForceVulkan)
		{
			App::setGraphicsRHI(TEXT("Vulkan"));
			const TCHAR* vulkanRHIModuleName = TEXT("VulkanRHI");
			dynamicRHIModule = &ModuleManager::loadModuleChecked<IDynamicRHIModule>(vulkanRHIModuleName);
			if (!dynamicRHIModule->isSupported())
			{
				PlatformMisc::requestExit(1);
				dynamicRHIModule = nullptr;
			}
			loadedRHIModuleName = vulkanRHIModuleName;
		}
		else if (bForceD3D12 || bPreferD3D12)
		{

		}
		if (!dynamicRHIModule)
		{
			App::setGraphicsRHI(TEXT("DirectX 11"));
			const TCHAR* d3d11RHIModuleName = TEXT("D3D11RHI");
			dynamicRHIModule = &ModuleManager::loadModuleChecked<IDynamicRHIModule>(d3d11RHIModuleName);
			if (!dynamicRHIModule->isSupported())
			{
				PlatformMisc::requestExit(1);
				dynamicRHIModule = nullptr;
			}
			loadedRHIModuleName = d3d11RHIModuleName;
		}
		return dynamicRHIModule;
	}

	DynamicRHI* platformCreateDynamicRHI()
	{
		DynamicRHI* dynamicRHI = nullptr;

		ERHIFeatureLevel::Type requestedFeatureLevel;
		const TCHAR* loadedRHIModuleName;
		IDynamicRHIModule* dynamicRHIModule = loadDynamicRHIMOdule(requestedFeatureLevel, loadedRHIModuleName);
		if (dynamicRHIModule)
		{
			dynamicRHI = dynamicRHIModule->createRHI(requestedFeatureLevel);
			GLoadedRHIModuleName = loadedRHIModuleName;
		}
		return dynamicRHI;
	}
}
#endif
