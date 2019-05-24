#pragma once
#include "Settings/ISettingsCategory.h"
namespace Air
{
	class SettingsSection;
	class SettingsCategory : public ISettingsCategory, public std::enable_shared_from_this<SettingsCategory>
	{
	public:
		SettingsCategory(const wstring& inName);

		virtual void describe(const wstring& inDisplayName, const wstring& inDescription) override;

		virtual ISettingsSectionPtr addSection(const wstring& sectionName, const wstring& inDisplayName, const wstring& inDescription, const std::weak_ptr<void>& settingsObject) override;

		virtual const wstring& getDescription() const override;

		virtual const wstring& getDisplayName() const override;

		virtual ISettingsSectionPtr getSection(const wstring& sectionName) const override;

		virtual int32 getSections(TArray<ISettingsSectionPtr>& outSections) const override;

		virtual const wstring& getName() const override;


	private:
		wstring mDescription;
		wstring mDisplayName;
		TMap<wstring, std::shared_ptr<SettingsSection>> mSections;

		wstring mName;
	};
}