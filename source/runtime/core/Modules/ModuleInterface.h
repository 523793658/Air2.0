#pragma once
#include "CoreType.h"
namespace Air
{

	class IModuleInterface
	{
	public:
		virtual ~IModuleInterface()
		{

		}

		virtual void startupModule()
		{

		}

		virtual void preUnloadCallback()
		{
		}
		virtual void postLoadCallback()
		{

		}

		virtual void shutdownModule()
		{

		}

		virtual bool supportsDynamicReloading()
		{
			return true;
		}

		virtual bool supportsAutomaticShutdown()
		{
			return true;
		}

		virtual bool isGameModule() const
		{
			return false;
		}
	};

	class DefaultModuleImpl
		: public IModuleInterface
	{};
}