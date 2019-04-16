#pragma once
#include "EngineMininal.h"
namespace Air
{
	struct MeshReductionSettings
	{
		float mPercentTriangles{ 1.0f };
		float mMaxDeviation{ 0.0f };
		float mPixelError{ 8.0f };
	};
}