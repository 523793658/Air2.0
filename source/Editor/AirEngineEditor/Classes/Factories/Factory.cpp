#include "Classes/Factories/Factory.h"
#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"
#include "boost/algorithm/string.hpp"
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

	std::shared_ptr<Object> Factory::createFromFile(Object* inObject, wstring filename, wstring name, EObjectFlags flags)
	{
		wstring extension = Paths::getExtension(filename);
		boost::to_lower(extension);
		Factory* factory = mExtensionMaps.findRef(extension);
		if (factory == nullptr)
		{
			return nullptr;
		}
		return factory->createFromFileInner(inObject, filename, name, flags);
	}

}