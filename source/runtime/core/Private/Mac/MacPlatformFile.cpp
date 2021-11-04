// Copyright Epic Games, Inc. All Rights Reserved.
#if PLATFORM_MAC
#include "Mac/MacPlatformFile.h"

IPlatformFile& IPlatformFile::GetPlatformPhysical()
{
	static FApplePlatformFile MacPlatformSingleton;
	return MacPlatformSingleton;
}
#endif
