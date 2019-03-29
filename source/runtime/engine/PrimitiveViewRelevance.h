#pragma once
#include "CoreMinimal.h"
namespace Air
{
	struct PrimitiveViewRelevance
	{
		uint16 mShadingModelMaskRelevance;
		uint16 bInitializedThisFrame : 1;
		uint32 bStaticRelevance : 1;
		uint32 bDynamicRelevance : 1;
		uint32 bDrawRelevance : 1;
		uint32 bUsesGlobalDistanceField : 1;
		uint32 bRenderInMainPass : 1;
	};
}