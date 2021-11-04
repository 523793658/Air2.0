// Copyright Epic Games, Inc. All Rights Reserved.
#if PLATFORM_LINUX
#include "Linux/LinuxPlatformFile.h"

IPlatformFile& IPlatformFile::GetPlatformPhysical()
{
	static FUnixPlatformFile Singleton;
	return Singleton;
}
#endif
