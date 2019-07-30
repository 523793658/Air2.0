#pragma once
#include "CoreMinimal.h"
namespace Air
{
	class DistanceFieldAOParameters
	{
	public:
		float GlobalMaxOcclusionDistance;
		float ObjectMaxOcclusionDistance;
		float Contrast;

		DistanceFieldAOParameters(float inOcclusionMaxDistance, float inContrast = 0);
	};
}