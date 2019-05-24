#include "CoreMinimal.h"
#include "RHIResource.h"
#include "Interface/ITargetPlatformManagerModule.h"
#include "Interface/IShaderFormat.h"
#include "Modules/ModuleManager.h"
#include "Interface/IShaderFormatModule.h"
#include "Interface/ITargetPlatform.h"
#include "Misc/ScopedSlowTask.h"
#include "Interface/ITargetPlatformModule.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "boost/algorithm/string.hpp"

namespace Air
{
#if !IS_MONOLITHIC && (PLATFORM_WINDOWS)
#define AUTOSDK_ENABLED		0
#else
#define AUTOSDK_ENABLED		0
#endif

	class TargetPlatformManagerModule : public ITargetPlatformManagerModule
	{
	private:
		bool bForceCacheUpdate{ false };
	public:
		virtual uint16 shaderFormatVersion(wstring name)
		{
			static TMap<wstring, uint16> alreadyFound;
			auto result = alreadyFound.find(name);
			if (result == alreadyFound.end())
			{
				const IShaderFormat* sf = findShaderFormat(name);
				if (sf)
				{
					result = alreadyFound.emplace(name, sf->getVersion(name)).first;
				}
			}
			BOOST_ASSERT(result != alreadyFound.end());
			return result->second;
		}

		virtual const TArray<const class IShaderFormat*>& getShaderFormats() override
		{
			static bool bInitialized = false;
			static TArray<const IShaderFormat*> result;
			if (!bInitialized || bForceCacheUpdate)
			{
				bInitialized = true;
				result.empty(result.size());
				TArray<wstring> modules;
				ModuleManager::get().findModules(SHADERFORMAT_MODULE_WILDCARD, modules);
				if (!modules.size())
				{
					AIR_LOG(LogTargetPlatformManager, ERROR, TEXT("No target shader formats found"));
				}
				for (int32 index = 0; index < modules.size(); index++)
				{
					IShaderFormatModule* module = ModuleManager::loadModulePtr<IShaderFormatModule>(modules[index]);
					if (module)
					{
						IShaderFormat* format = module->getShaderFormat();
						if (format != nullptr)
						{
							result.add(format);
						}
					}
				}
			}
			return result;
		}

		virtual const class IShaderFormat* findShaderFormat(wstring name)
		{
			const TArray<const IShaderFormat*>& shaderFormats = getShaderFormats();
			for (int32 index = 0; index < shaderFormats.size(); index++)
			{
				TArray<wstring> formats;
				shaderFormats[index]->getSupportedFormats(formats);
				for (int32 formatIndex = 0; formatIndex < formats.size(); formatIndex++)
				{
					if (formats[formatIndex] == name)
					{
						return shaderFormats[formatIndex];
					}
				}
			}
			return nullptr;
		}

		virtual ITargetPlatform* getRunningTargetPlatform() override
		{
			static bool bInitialized = false;
			static ITargetPlatform* result = nullptr;
			if (!bInitialized || bForceCacheUpdate)
			{
				bInitialized = true;
				result = nullptr;

				const TArray<ITargetPlatform*>& targetPlatforms = getTargetPlatforms();

				for (int32 index = 0; index < targetPlatforms.size(); index++)
				{
					if (targetPlatforms[index]->isRunningPlatform())
					{
						BOOST_ASSERT(result == nullptr);
						result = targetPlatforms[index];
					}
				}
			}
			return result;
		}

		bool isAutoSDKsEnabled()
		{
			static const wstring sdkRootEnvFar(TEXT("AIR_SDKS_ROOT"));
			const int32 maxPathSize = 16384;
			TCHAR sdkPath[maxPathSize] = { 0 };
			PlatformMisc::getEnvironmentVariable(sdkRootEnvFar.c_str(), sdkPath, maxPathSize);

			if (sdkPath[0] != 0)
			{
				return true;
			}
			return false;
		}

		bool setupEnvironmentFromAutoSDK(const wstring& autoSDKPath)
		{
			return true;
		}

		bool setupAndValidateAutoSDK(const wstring& autoSDKPath)
		{
#if AUTOSDK_ENABLED
#else
			return true;
#endif
		}


		void discoverAvailablePlatforms()
		{
			mPlatforms.empty(mPlatforms.size());
			TArray<wstring> modules;

			wstring moduleWildCard = TEXT("*TargetPlatform");

#if WITH_EDITOR
#if PLATFORM_WINDOWS
#endif
#endif
			ModuleManager::get().findModules(moduleWildCard.c_str(), modules);
			modules.remove(TEXT("TargetPlatform"));
			if (!modules.size())
			{
				AIR_LOG(LogTargetPlatformManager, ERROR, TEXT("No target platforms found!"));
			}

			ScopedSlowTask slowTask(modules.size());
			for (int32 index = 0; index < modules.size(); index++)
			{
				slowTask.enterProgressFrame(1);
				ITargetPlatformModule* mod = ModuleManager::loadModulePtr<ITargetPlatformModule>(modules[index]);
				if (mod)
				{
					ITargetPlatform* platform = mod->getTargetPlatform();
					if (platform != nullptr)
					{
					RETRY_SETUPANDVALIDATE:
						if (setupAndValidateAutoSDK(platform->getPlatformInfo().mAutoSDKPath))
						{
							mPlatforms.add(platform);
						}
						else
						{
							//static bool bIsChildCooker
						}
					}
					else
					{

					}
				}
			}
		}

		virtual const TArray<ITargetPlatform*>& getTargetPlatforms() override
		{
			if (mPlatforms.size() == 0 || bForceCacheUpdate)
			{
				discoverAvailablePlatforms();
			}
			return mPlatforms;
		}

		virtual const TArray<ITargetPlatform*>& getActiveTargetPlatforms() override
		{
			static bool bInitialized = false;
			static TArray<ITargetPlatform*> results;
			if (!bInitialized || bForceCacheUpdate)
			{
				bInitialized = true;
				results.empty(results.size());
				const TArray<ITargetPlatform*>& targetPlatforms = getTargetPlatforms();

				for (int32 index = 0; index < targetPlatforms.size(); index++)
				{
					if (targetPlatforms[index]->isRunningPlatform())
					{
						results.add(targetPlatforms[index]);
					}
				}
				if (!results.size())
				{
					AIR_LOG(LOGTargetPlatformManager, Display, TEXT("Not building assets for any platform."));
				}
				else
				{
					for (int32 index = 0; index < results.size(); index++)
					{
						AIR_LOG(LogTargetPlatformManager, Display, TEXT("Building Assets for %s"), results[index]->platformName().c_str());
					}
				}
			}
			return results;
		}

		TArray<ITargetPlatform*> mPlatforms;
	};
	IMPLEMENT_MODULE(TargetPlatformManagerModule, TargetPlatform);
}