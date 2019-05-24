#include "SettingsContainer.h"
#include "SettingsCategory.h"
namespace Air
{
	SettingsContainer::SettingsContainer(const wstring & inName)
		:mName(inName)
	{}


	void SettingsContainer::describe(const wstring& inDisplayName, const wstring& inDescription, const wstring& inIconName)
	{
		mDescription = inDescription;
		mDisplayName = inDisplayName;
		mIconName = inIconName;
	}

	void SettingsContainer::describeCategory(const wstring& categoryName, const wstring& displayName, const wstring& description)
	{
		std::shared_ptr<SettingsCategory>& category = mCategories.findOrAdd(categoryName);
		if (!category)
		{
			category = MakeSharedPtr<SettingsCategory>(categoryName);
		}
		category->describe(displayName, description);
	}


	ISettingsSectionPtr SettingsContainer::addSection(const wstring& categoryName, const wstring& sectionName, const wstring& inDisplayName, const wstring& inDescription, const std::weak_ptr<void>& settingsObj)
	{
		std::shared_ptr<SettingsCategory> category = mCategories.findRef(categoryName);
		if (!category)
		{
			describeCategory(categoryName, categoryName, Name_None);
			category = mCategories.findRef(categoryName);
		}
		ISettingsSectionPtr section = category->addSection(sectionName, inDisplayName, inDescription, settingsObj);
		return section;
	}

	void SettingsContainer::removeSection(const wstring& categoryName, const wstring& sectionName)
	{
		auto& it = mCategories.find(categoryName);
		if (it != mCategories.end())
		{
			mCategories.erase(it);
		}

	}
}