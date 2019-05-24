#include "SettingsCategory.h"
#include "SettingsSection.h"
namespace Air
{
	SettingsCategory::SettingsCategory(const wstring& inName)
		:mName (inName)
	{}

	void SettingsCategory::describe(const wstring& inDisplayName, const wstring& inDescription)
	{
		mDisplayName = inDisplayName;
		mDescription = inDescription;
	}

	ISettingsSectionPtr SettingsCategory::getSection(const wstring& sectionName) const
	{
		return mSections.findRef(sectionName);
	}


	int32 SettingsCategory::getSections(TArray<ISettingsSectionPtr>& outSections) const
	{
		outSections.empty(mSections.size());
		for (auto it : mSections)
		{
			outSections.add(it.second);
		}
		return outSections.size();
	}

	const wstring& SettingsCategory::getName() const
	{
		return mName;
	}

	ISettingsSectionPtr SettingsCategory::addSection(const wstring& sectionName, const wstring& inDisplayName, const wstring& inDescription, const std::weak_ptr<void>& settingsObject)
	{
		std::shared_ptr<SettingsSection>& section = mSections.findOrAdd(sectionName);
		if (!section || (!section->getSettingsObject().lock()))
		{
			section = MakeSharedPtr<SettingsSection>(this->shared_from_this(), sectionName, inDisplayName, inDescription, settingsObject);
		}
		return section;
	}

	const wstring& SettingsCategory::getDescription() const
	{
		return mDescription;
	}

	const wstring& SettingsCategory::getDisplayName() const
	{
		return mDisplayName;
	}
}