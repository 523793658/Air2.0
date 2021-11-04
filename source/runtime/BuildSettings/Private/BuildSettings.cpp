// Copyright Epic Games, Inc. All Rights Reserved.

#include "BuildSettings.h"

namespace BuildSettings
{
	bool IsLicenseeVersion()
	{
		return ENGINE_IS_LICENSEE_VERSION;
	}

	int GetCurrentChangelist()
	{
		return 0;
	}

	int GetCompatibleChangelist()
	{
		return 0;
	}

	const TCHAR* GetBranchName()
	{
		return TEXT("1");
	}
	
	const TCHAR* GetBuildDate()
	{
		return TEXT(__DATE__);
	}

	const TCHAR* GetBuildVersion()
	{
		return TEXT("1");
	}

	bool IsPromotedBuild()
	{
		return false;
	}
}


