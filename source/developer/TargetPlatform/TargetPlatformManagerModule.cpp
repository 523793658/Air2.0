#include "CoreMinimal.h"
#include "Interface/ITargetPlatformManagerModule.h"
#include "Interface/IShaderFormat.h"
#include "Modules/ModuleManager.h"
#include "Interface/IShaderFormatModule.h"
#include "Interface/ITargetPlatform.h"
#include "Misc/ScopedSlowTask.h"
#include "Interface/ITargetPlatformModule.h"
namespace Air
{
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

		void discoverAvailablePlatforms()
		{
			mPlatforms.empty(mPlatforms.size());
			TArray<wstring> modules;

			wstring moduleWildCard = TEXT("TargetPlatform");

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
					}
				}
			}

		}

		virtual const TArray<ITargetPlatform*>& getTargetPlatforms() override
		{
			if (mPlatforms.size() == 0 || bForceCacheUpdate)
			{
				diso
			}
		}

		TArray<ITargetPlatform*> mPlatforms;
	};
	IMPLEMENT_MODULE(TargetPlatformManagerModule, TargetPlatform);
}