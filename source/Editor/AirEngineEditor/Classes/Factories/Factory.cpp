#include "Classes/Factories/Factory.h"
#include "Modules/ModuleManager.h"
namespace Air
{
	TMap<wstring, Factory*> Factory::mExtensionMaps;

	void Factory::initAllFactory()
	{
		IFactoryModule* factoryModule = &ModuleManager::loadModuleChecked<IFactoryModule>(TEXT("FbxFactory"));
		if (factoryModule)
		{
			Factory* factory = factoryModule->createFactory();
			TCHAR** extensions = nullptr;
			uint32 num = 0;
			factory->getExtensions(extensions, num);
			for (uint32 i = 0; i < num; i++)
			{
				mExtensionMaps.emplace(extensions[i], factory);
			}
		}
	}
}