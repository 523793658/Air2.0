#pragma once
#include "CoreMinimal.h"

#include "Modules/ModuleInterface.h"
#include "ISettingsModule.h"
namespace Air
{
	class ISettingsSection;

	class ISettingsModule
		: public IModuleInterface
	{
	public:
		virtual void unregisterSettings(const wstring& containerName, const wstring& categoryName, const wstring& sectionName) = 0;

		virtual std::shared_ptr<ISettingsSection> registerSettings(const wstring& containerName, const wstring& categoryName, const wstring& sectionName, const wstring& displayName, const wstring& description, const std::weak_ptr<void>& settingsObject) = 0;


		virtual ~ISettingsModule() {}
	};
}