#pragma once

#include "CoreMinimal.h"
#include "TargetPlatform/Interface/TargetDeviceId.h"

namespace Air
{
	class ITargetDevice;
	class ITargetDeviceOutput;

	enum class ETargetDeviceFeatures
	{
		MultiLaunch,
		PowerOff,
		PowerOn,
		ProcessSnapshot,
		Reboot
	};


	enum class ETargetDeviceTypes
	{
		Indeterminate,
		Desktop,
		Phone,
	};

	namespace TargetDeviceTypes
	{
		inline wstring toString(ETargetDeviceTypes deviceType)
		{
			switch (deviceType)
			{
			case Air::ETargetDeviceTypes::Desktop:
				return TEXT("Desktop");
			case Air::ETargetDeviceTypes::Phone:
				return TEXT("Phone");
			default:
				return TEXT("Indeterminate");
			}
		}
	}

	enum class ETargetDeviceThreadStates
	{
		Unknown,
		CanRun,
		Inactive,
		Inhibited,
		RunQueue,
		Running,
	};

	enum class ETargetDeviceThreadWaitStates
	{
		Unknown,
		Locked,
		Sleeping,
		Suspended,
		Swapped,
		Waiting,
	};

	struct TargetDeviceThreadInfo
	{
		uint64 mExitCode;
		uint32 mId;
		wstring mName;
		uint64 mStatckSize;
		ETargetDeviceThreadStates mState;
		ETargetDeviceThreadWaitStates mWaitState;
	};

	struct TargetDeviceProcessInfo
	{
		int64 mId;
		wstring mName;
		uint64 mParentId;

		TArray<TargetDeviceThreadInfo> mThreads;
		wstring mUserName;
	};


	typedef std::shared_ptr<class ITargetDevice> ITargetDevicePtr;

	typedef std::weak_ptr<ITargetDevice> ITargetDeviceWeakPtr;

	class ITargetDevice
	{

	public:

	};
}