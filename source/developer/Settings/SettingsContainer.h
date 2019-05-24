#pragma once
#include "Settings/ISettingsContainer.h"
namespace Air
{
	class SettingsCategory;

	class SettingsContainer : public ISettingsContainer
	{
	public:
		SettingsContainer(const wstring & inName);

		ISettingsSectionPtr addSection(const wstring& categoryName, const wstring& sectionName, const wstring& inDisplayName, const wstring& inDescription, const std::weak_ptr<void>& settingsObj) override;

		void removeSection(const wstring& categoryName, const wstring& sectionName) override;

	public:
		virtual void describe(const wstring& inDisplayName, const wstring& inDescription, const wstring& inIconName) override;

		virtual void describeCategory(const wstring& categoryName, const wstring& displayName, const wstring& description) override;
	private:
		wstring mName;

		TMap<wstring, std::shared_ptr<SettingsCategory>> mCategories;
		
		wstring mDisplayName;

		wstring mIconName;

		wstring mDescription;
	};
}