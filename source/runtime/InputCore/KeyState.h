#pragma once
#include "InputCoreMinimal.h"
#include "Classes/Engine/EngineBaseTypes.h"
namespace Air
{
	struct KeyState
	{
		float3 mRawValue;
		float3 mValue;
		float mLastUpDownTransitionTime;
		uint32 bDown : 1;
		uint32 bDownPrevious : 1;
		uint32 bConsumed : 1;
		TArray<uint32> mEventCount[IE_MAX];
		TArray<uint32> mEventAccumulator[IE_MAX];

		float3 mRawValueAccumulator;

		uint8 mSampleCountAccumulator;

		KeyState()
			:mRawValue(0.f, 0.f, 0.f)
			,mValue(0.f, 0.f, 0.f)
			,mLastUpDownTransitionTime(0.f)
			,bDown(false)
			,bDownPrevious(false)
			,bConsumed(false)
			,mRawValueAccumulator(0.f, 0.f, 0.f)
			,mSampleCountAccumulator(0)
		{}
	};
}