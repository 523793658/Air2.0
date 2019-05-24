#include "GenericPlatform/GenericPlatformProcess.h"
#include "HAL/PlatformProcess.h"
#include "Misc/EventPool.h"
#include "Misc/Paths.h"
namespace Air
{
	bool GenericPlatformProcess::supportsMultithreading()
	{
		return true;
	}
	class Event* GenericPlatformProcess::createSynchEvent(bool bIsManualReset)
	{
		return NULL;
	}

	class Event* GenericPlatformProcess::getSynchEventFromPool(bool bIsManualReset)
	{
		return bIsManualReset ?
			EventPool<EEventPoolTypes::ManualReset>::get().getEventFromPool()
			: EventPool<EEventPoolTypes::AutoReset>::get().getEventFromPool();
	}

	void GenericPlatformProcess::returnSynchEventToPool(Event* event)
	{
		if (!event)
		{
			return;
		}
		if (event->isManualReset())
		{
			EventPool<EEventPoolTypes::ManualReset>::get().returnToPool(event);
		}
		else
		{
			EventPool < EEventPoolTypes::AutoReset>::get().returnToPool(event);
		}
	}

	const wstring GenericPlatformProcess::getModulesDirectory()
	{
		return Paths::engineDir() + TEXT("/bin");
	}

	const TCHAR* GenericPlatformProcess::getModulePrefix()
	{
		return L"";
	}


	static wstring Generic_ShaderDir;

	const TCHAR* GenericPlatformProcess::shaderDir()
	{
		if (Generic_ShaderDir.length() == 0)
		{
			Generic_ShaderDir = Paths::combine(Paths::engineDir().c_str(), TEXT("Shaders"));
		}
		return Generic_ShaderDir.c_str();
	}

}