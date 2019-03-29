#include "EngineModule.h"
#include "Modules/ModuleManager.h"
#include "RendererModule.h"
namespace Air
{
	IRendererModule* mCachedRendererModule = nullptr;


	ENGINE_API IRendererModule& getRendererModule()
	{
		if (!mCachedRendererModule)
		{
			mCachedRendererModule = &ModuleManager::loadModuleChecked<IRendererModule>(TEXT("Renderer"));
		}
		return *mCachedRendererModule;
	}
}