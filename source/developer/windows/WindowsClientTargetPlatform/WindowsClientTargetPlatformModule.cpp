#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "windows/GenericWindowsTargetPlatform.h"
#include "Interface/ITargetPlatformModule.h"
namespace Air
{
	static ITargetPlatform* mSingleton = nullptr;
	static ITargetPlatformModule* mCachedTargetPlatformModule = nullptr;
	class WindowsClientTargetPlatformModule : public ITargetPlatformModule
	{
	public:
		virtual ~WindowsClientTargetPlatformModule()
		{
			mSingleton = nullptr;
		}

		virtual ITargetPlatform* getTargetPlatform()
		{
			if (mSingleton == nullptr)
			{
				mSingleton = new TGenericWindowsTargetPlatform<false, false, true>();
			}
			return mSingleton;
		}
	};

	extern WINDOWS_TARGET_PLATFORM_API ITargetPlatformModule& getTargetPlatformModule()
	{
		if (!mCachedTargetPlatformModule)
		{
			mCachedTargetPlatformModule = &ModuleManager::loadModuleChecked<ITargetPlatformModule>(TEXT("TargetPlatform"));
		}
		return *mCachedTargetPlatformModule;
	}
}