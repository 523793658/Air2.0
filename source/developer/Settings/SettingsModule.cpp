#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"
#include "ISettingsSection.h"
#include "ISettingsContainer.h"
#include "Settings/SettingsContainer.h"
namespace Air
{
	class SettingsModule
		: public ISettingsModule
	{
	public:
		virtual ISettingsSectionPtr registerSettings(const wstring& containerName, const wstring& categoryName, const wstring& sectionName, const wstring& displayName, const wstring& description, const std::weak_ptr<void>& settingsObject) override
		{
			return findOrAddContainer(containerName)->addSection(categoryName, sectionName, displayName, description, settingsObject);
		}

		virtual void unregisterSettings(const wstring& containerName, const wstring& categoryName, const wstring& sectionName) override
		{
			auto& it = mContainerNamesToContainers.find(containerName);
			if (it != mContainerNamesToContainers.end() && it->second)
			{
				it->second->removeSection(categoryName, sectionName);
			}
		}

	protected:
		std::shared_ptr<SettingsContainer> findOrAddContainer(const wstring& containerName)
		{
			std::shared_ptr<SettingsContainer>& container = mContainerNamesToContainers.findOrAdd(containerName);
			if (!container)
			{
				container = MakeSharedPtr<SettingsContainer>(containerName);
			}
			return container;
		}

	private:
		std::shared_ptr<ISettingsContainer> mDefaultSettingsContainer;
		TMap<wstring, std::shared_ptr<SettingsContainer>> mContainerNamesToContainers;
	};

	IMPLEMENT_MODULE(SettingsModule, Settings);
}