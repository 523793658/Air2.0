// Copyright Epic Games, Inc. All Rights Reserved.
#if PLATFORM_MAC
#include "Mac/MacPlatformOutputDevices.h"
#include "Mac/MacErrorOutputDevice.h"

class FOutputDevice* FMacPlatformOutputDevices::GetEventLog()
{
	return NULL;
}

FOutputDeviceError* FMacPlatformOutputDevices::GetError()
{
	static FMacErrorOutputDevice Singleton;
	return &Singleton;
}

#endif
