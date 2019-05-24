#pragma once
#include "CoreMinimal.h"
#include "Settings/ISettingsSection.h"
namespace Air
{
	class ISettingsCategory
	{
	public:
		virtual const wstring& getDescription() const = 0; 
		virtual const wstring& getDisplayName() const = 0; 

		virtual ISettingsSectionPtr getSection(const wstring& sectionName) const = 0; 

		virtual int32 getSections(TArray<ISettingsSectionPtr>& outSections) const = 0; 

		virtual ISettingsSectionPtr addSection(const wstring& sectionName, const wstring& inDisplayName, const wstring& inDescription, const std::weak_ptr<void>& settingsObject) = 0;

		virtual const wstring& getName()const = 0;

		virtual void describe(const wstring& inDisplayName, const wstring& inDescription) = 0;

	
		virtual ~ISettingsCategory() {}
	};

	typedef std::shared_ptr<ISettingsCategory> ISettingCategoryPtr;
}