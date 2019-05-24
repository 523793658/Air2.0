#pragma once
#include "CoreMinimal.h"

namespace Air
{
	class ISettingsSection
	{
		virtual std::weak_ptr<void> getSettingsObject() const = 0;
	};


	typedef std::shared_ptr<ISettingsSection> ISettingsSectionPtr;

}