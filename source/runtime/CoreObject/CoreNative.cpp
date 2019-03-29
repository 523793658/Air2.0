#include "CoreNative.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
namespace Air
{

	class CoreObjectModule : public DefaultModuleImpl
	{
	public:
		virtual void startupModule() override
		{
			
		}
	};

	IMPLEMENT_MODULE(CoreObjectModule, CoreObject);
}