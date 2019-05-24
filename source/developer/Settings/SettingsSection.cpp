#include "SettingsSection.h"
namespace Air
{
	SettingsSection::SettingsSection(const std::weak_ptr<ISettingsCategory> inCategory, const wstring& inName, const wstring& inDisplayName, const wstring& inDescription, const std::weak_ptr<void>& inSettingsObject)
		:mCategory(inCategory)
		,mDescription(inDescription)
		,mDisplayName(inDisplayName)
		,mName(inName)
		,mSettingsObject(inSettingsObject)
	{
		
	}

	std::weak_ptr<void> SettingsSection::getSettingsObject() const
	{
		return mSettingsObject;
	}
}