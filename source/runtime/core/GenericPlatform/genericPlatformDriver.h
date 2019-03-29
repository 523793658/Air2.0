#pragma once
#include "CoreType.h"
#include "boost/algorithm/string.hpp"
namespace Air
{
	enum RHIVendor
	{
		RHIV_AMD = 0x1002,
		RHIV_Intel = 0x8086,
		RHIV_NVIDIA = 0x10DE,
		RHIV_Macrosoft = 0x1414,
	};

	struct GPUDriverInfo
	{
		GPUDriverInfo()
			:mVendorId(0)
		{}
		uint32 mVendorId;
		wstring mDeviceDescription;
		wstring mProviderName;
		wstring mInternalDriverVersion;
		wstring mUserDriverVersion;
		wstring mDriverDate;
		bool isValid() const
		{
			return !mDeviceDescription.empty()
				&& mVendorId
				&& (mInternalDriverVersion != TEXT("UnKnown"))
				&& (mInternalDriverVersion != TEXT(""));
		}

		void setAMD() 
		{
			mVendorId = RHIV_AMD;
		}

		void setIntel()
		{
			mVendorId = RHIV_Intel;
		}

		void setNVIDIA()
		{
			mVendorId = RHIV_NVIDIA;
		}
		bool isAMD() const { return mVendorId == RHIV_AMD; }
		bool isIntel() const { return mVendorId == RHIV_Intel; }
		bool isNVIDIA() const { return mVendorId == RHIV_NVIDIA; }
		wstring getUnifiedDriverVersion() const
		{
			const wstring & fullVersion = mInternalDriverVersion;
			if (isNVIDIA())
			{
				wstring rightPart = fullVersion.substr(fullVersion.length() - 6);
				boost::algorithm::replace_all(rightPart, TEXT("."), TEXT(""));
				rightPart.insert(3, TEXT("."));
				return rightPart;
			}
			else if (isAMD())
			{

			}
			else if (isIntel())
			{

			}
			return fullVersion;
		}
	};
}