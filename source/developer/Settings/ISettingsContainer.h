#pragma once
#include "CoreMinimal.h"
#include "ISettingsSection.h"
namespace Air
{
	class ISettingsContainer
	{
	public:
		virtual void describe(const wstring& inDisplayName, const wstring& inDescription, const wstring& inIconName) = 0;


		virtual void describeCategory(const wstring& categoryName, const wstring& displayName, const wstring& description) = 0;

		virtual ISettingsSectionPtr addSection(const wstring& categoryName, const wstring& sectionName, const wstring& inDisplayName, const wstring& inDescription, const std::weak_ptr<void>& settingsObj) = 0;

		virtual void removeSection(const wstring& categoryName, const wstring& sectionName) = 0;
	};
}