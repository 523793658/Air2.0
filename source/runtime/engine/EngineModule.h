#pragma once
#include "EngineMininal.h"

#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"
namespace Air
{

	class IRendererModule;

	class EngineModule : public DefaultModuleImpl
	{
	public:
		virtual void startupModule() override;
	};

	extern ENGINE_API IRendererModule& getRendererModule();

	extern ENGINE_API void resetCachedRendererModule();
}