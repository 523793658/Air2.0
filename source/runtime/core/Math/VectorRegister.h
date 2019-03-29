#pragma once

#include "CoreType.h"
#include "Math/MathUtility.h"
#if PLATFORM_ENABLE_VECTORINTRINSICS
#define SIMD_ALIGNMENT (16)
#include "Math/MathSSE.h"
#endif

#include "Math/MathVectorCommon.h"

extern CORE_API const VectorRegister VECTOR_INV_255;

#define ZERO_ANIMWEIGHT_THRESH	(0.00001f)

namespace GlobalVectorConstants
{
	static const VectorRegister AnimWeightThreshold = MakeVectorRegister(ZERO_ANIMWEIGHT_THRESH, ZERO_ANIMWEIGHT_THRESH, ZERO_ANIMWEIGHT_THRESH, ZERO_ANIMWEIGHT_THRESH);

	static const VectorRegister RotationSignificantThreshold = MakeVectorRegister(1.0f - DELTA * DELTA, 1.0f - DELTA * DELTA, 1.0f - DELTA * DELTA, 1.0f - DELTA * DELTA);
}