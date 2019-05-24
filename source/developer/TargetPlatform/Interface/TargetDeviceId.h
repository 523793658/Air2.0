#pragma once
#include "CoreMinimal.h"

namespace Air
{
	class TargetDeviceId
	{
	public:
		TargetDeviceId() {}

		TargetDeviceId(const wstring& inPlatformName, const wstring& inDeviceName)
			:mPlatformName(inPlatformName),
			mDeviceName(inDeviceName)
		{}

	public:
		bool operator == (const TargetDeviceId& other) const
		{
			return (mPlatformName == other.mPlatformName) && (mDeviceName == other.mDeviceName);
		}

		bool operator != (const TargetDeviceId & other) const
		{
			return (mPlatformName != other.mPlatformName) || (mDeviceName != other.mDeviceName);
		}

		friend Archive& operator << (Archive& ar, TargetDeviceId& id)
		{
			return ar << id.mPlatformName << id.mDeviceName;
		}

		const wstring& getDeviceName() const
		{
			return mDeviceName;
		}

		const wstring& getPlatformName() const
		{
			return mPlatformName;
		}

		bool isValid() const
		{
			return (!mPlatformName.empty() && !mDeviceName.empty());
		}

		wstring toString() const
		{
			return mPlatformName + TEXT("@") + mDeviceName;
		}

		friend size_t getTypeHash(const TargetDeviceId& id)
		{
			return getTypeHash(id.toString());
		}

		static bool parse(const wstring& idString, TargetDeviceId& outId)
		{

			size_t index = idString.find_first_of(TEXT('@'));
			if (index == -1)
			{
				return false;
			}
			outId.mPlatformName = idString.substr(0, index);
			outId.mDeviceName = idString.substr(index + 1);
			return true;
		}
	private:
		wstring mPlatformName;
		wstring mDeviceName;
	};
}