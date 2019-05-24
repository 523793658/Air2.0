#include "windows/WindowsTargetPlatform/GenericWindowsTargetPlatform.h"
#include "Interface/ITargetPlatformModule.h"
#include "Settings/ISettingsModule.h"
#include "Modules/ModuleManager.h"
#include "WindowsTargetSettings.h"
namespace Air
{
	static ITargetPlatform* mSingleton = nullptr;

	class WindowsTargetPlatformModule
		: public ITargetPlatformModule
	{
	public:
		~WindowsTargetPlatformModule()
		{
			mSingleton = nullptr;
		}

		virtual ITargetPlatform* getTargetPlatform() override
		{
			if (mSingleton == nullptr)
			{
				mSingleton = new TGenericWindowsTargetPlatform<true, false, false>();
			}
			return mSingleton;
		}

	public:
		virtual void startupModule() override
		{
			mTargetSettings = MakeSharedPtr< WindowsTargetSettings>();
			GConfig->getArray(TEXT("/Script/WindowsTargetPlatform.WindowsTargetSettings"), TEXT("TargetedRHIs"), mTargetSettings->mTargetedRHIs, GEngineIni);
			mTargetSettings->mMinimumOSVersion = EMinmumSupportedOS::MSOS_Win7;

			ISettingsModule* settingsModule = ModuleManager::getModulePtr<ISettingsModule>(TEXT("Settings"));
			if (settingsModule != nullptr)
			{
				settingsModule->registerSettings(TEXT("Project"), TEXT("Platforms"), TEXT("Windows"), TEXT("Windows"), TEXT("Settings for windows target platform"), std::dynamic_pointer_cast<Object>(mTargetSettings));
			}

		}


		virtual void shutdownModule() override
		{
			ISettingsModule* settingsModule = ModuleManager::getModulePtr<ISettingsModule>(TEXT("Settings"));
			if (settingsModule != nullptr)
			{
				settingsModule->unregisterSettings(TEXT("Project"), TEXT("Platforms"), TEXT("Windows"));
			}
			mTargetSettings.reset();
		}

	private:
		std::shared_ptr<WindowsTargetSettings> mTargetSettings;
	};


	IMPLEMENT_MODULE(WindowsTargetPlatformModule, WindowsTargetPlatform);
}