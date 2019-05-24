#include "Settings/ISettingsSection.h"
namespace Air
{
	class ISettingsCategory;

	class SettingsSection : public ISettingsSection
	{
	public:
		SettingsSection(const std::weak_ptr<ISettingsCategory> inCategory, const wstring& inName, const wstring& inDisplayName, const wstring& inDescription, const std::weak_ptr<void>& inSettingsObject);

		virtual std::weak_ptr<void> getSettingsObject() const override;

	private:

		std::weak_ptr<ISettingsCategory> mCategory;

		wstring mDescription;

		wstring mDisplayName;

		wstring mName;

		std::weak_ptr<void> mSettingsObject;
	};
}