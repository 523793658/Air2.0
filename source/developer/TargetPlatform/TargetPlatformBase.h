#pragma once
#include "CoreMinimal.h"
#include "Interface/ITargetPlatform.h"
namespace Air
{

	class TargetPlatformBase
		: public ITargetPlatform
	{
	public:
		virtual const Name_PlatformInfo::PlatformInfo& getPlatformInfo() const override
		{
			return *mPlatformInfo;
		}

	protected:
		const Name_PlatformInfo::PlatformInfo *mPlatformInfo;
	};


	template<typename PlatformProperties>
	class TTargetPlatformBase
		: public TargetPlatformBase
	{
	public:
		virtual wstring platformName() const override
		{
			return PlatformProperties::platformName();
		}
	};
}