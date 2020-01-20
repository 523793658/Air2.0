#pragma once

#include "CoreMinimal.h"
#include "VulkanConfiguration.h"

namespace Air
{
	class VulkanCmdBuffer
	{
	public:
		inline volatile uint64 getFenceSignaledCounter() const
		{
			return mFenceSignaledCounter;
		}

		inline volatile uint64 getFenceSignaledCounterC() const
		{
			return mFenceSignaledCounter;
		}
	private:
		volatile uint64 mFenceSignaledCounter;
	};
}