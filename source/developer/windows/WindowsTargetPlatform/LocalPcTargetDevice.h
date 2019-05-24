#pragma once
#include "Misc/Guid.h"
#include "HAL/PlatformProcess.h"
#include "TargetPlatform/Interface/ITargetDevice.h"
namespace Air
{
	class LocalPcTargetDevice : public ITargetDevice
	{
	public:
		LocalPcTargetDevice(const ITargetPlatform& inTargetPlatform);

	private:
		TMap<Guid, void*> mProcesses;

		const ITargetPlatform& mTargetPlatform;
	};
}