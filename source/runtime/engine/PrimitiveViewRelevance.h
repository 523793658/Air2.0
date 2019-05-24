#pragma once
#include "CoreMinimal.h"
namespace Air
{
	struct PrimitiveViewRelevance
	{
		uint16 mShadingModelMaskRelevance;
		uint32 bOpaqueRelevance : 1;
		uint32 bMaskedRelevance : 1;
		uint16 bInitializedThisFrame : 1;
		uint32 bStaticRelevance : 1;
		uint32 bDynamicRelevance : 1;
		uint32 bDistortionRelevance : 1;
		uint32 bDrawRelevance : 1;
		uint32 bUsesGlobalDistanceField : 1;
		uint32 bUsesWorldPositionOffset : 1;
		uint32 bRenderInMainPass : 1;
		uint32 bSeparateTranslucencyRelevance : 1;
		uint32 bMobileSeparateTranslucencyRelevance : 1;
		uint32 bNormalTranslucencyRelevance : 1;
		uint32 bDecal : 1;
		uint32 bTranslucentSurfaceLighting : 1;
		uint32 bUsesSceneDepth : 1;
		
		bool hasTranslucency() const
		{
			return bSeparateTranslucencyRelevance || bNormalTranslucencyRelevance || bMobileSeparateTranslucencyRelevance;
		}
	};
}