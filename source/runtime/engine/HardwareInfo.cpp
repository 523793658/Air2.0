#include "HardwareInfo.h"
#include <map>
namespace Air
{
	static std::map<wstring, wstring> mHardwareDetailsMap;

	void HardwareInfo::registerHardwareInfo(const wstring specIdentifier, const wstring & hardwareInfo)
	{
		mHardwareDetailsMap.emplace(specIdentifier, hardwareInfo);
	}

	const wstring HardwareInfo::getHardwareDetailsString()
	{
		wstring detailsString;
		int32 detailsAdded = 0;
		for (auto it = mHardwareDetailsMap.begin(); it != mHardwareDetailsMap.end(); it++)
		{
			if (detailsAdded++ > 0)
			{
				detailsString += TEXT(", ");
			}
			wstring specId = it->first;
			wstring specValue = it->second;
			detailsString += ((specId + TEXT("=")) + specValue);
		}
		return detailsString;
	}

	wstring HardwareInfo::getHardwareInfo(const wstring specIdentifier)
	{
		return mHardwareDetailsMap[specIdentifier];
	}
}