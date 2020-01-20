#include "HAL/PlatformProperties.h"
#include "Modules/ModuleManager.h"
#include "Interface/ITargetPlatformManagerModule.h"
#include "DerivedDataCacheInterface.h"
#include "Misc/CoreMisc.h"
#include "CoreGlobals.h"
namespace Air
{
	void UrlConfig::init()
	{
		//mDefaultProtocol
	}

	void UrlConfig::reset()
	{

	}

	class ITargetPlatformManagerModule* getTargetPlatformManager()
	{
		static class ITargetPlatformManagerModule* singletonInterface = nullptr;
		if (!PlatformProperties::requiresCookedData())
		{
			static bool bInitialized = false;
			if (!bInitialized)
			{
				BOOST_ASSERT(isInGameThread());
				bInitialized = true;
				singletonInterface = ModuleManager::loadModulePtr<ITargetPlatformManagerModule>(TEXT("TargetPlatform"));
			}
		}
		return singletonInterface;
	}


	class ITargetPlatformManagerModule& getTargetPlatformManagerRef()
	{
		class ITargetPlatformManagerModule* singletonInterface = getTargetPlatformManager();
		if (!singletonInterface)
		{
			BOOST_ASSERT(false);
		}
		return *singletonInterface;
	}


	class DerivedDataCacheInterface* getDerivedDataCache()
	{
		static class DerivedDataCacheInterface* singletonInterface = nullptr;
		if (!PlatformProperties::requiresCookedData())
		{
			static bool bInitialized = false;
			if (!bInitialized)
			{
				BOOST_ASSERT(isInGameThread());
				bInitialized = true;
				class IDerivedDataCacheModule* module = ModuleManager::loadModulePtr<IDerivedDataCacheModule>(TEXT("DerivedDataCache"));
				if (module)
				{
					singletonInterface = &module->getDDC();
				}
			}
		}
		return singletonInterface;
	}

	class DerivedDataCacheInterface& getDerivedDataCacheRef()
	{
		class DerivedDataCacheInterface* singletonInterface = getDerivedDataCache();
		if (!singletonInterface)
		{

		}
		return *singletonInterface;
	}

	bool CORE_API stringHasBadDashes(const TCHAR* str)
	{
		while (TCHAR ch = *str++)
		{
			if ((UCS2CHAR)ch == 0x2013)
				return true;
		}
		return false;
	}
}