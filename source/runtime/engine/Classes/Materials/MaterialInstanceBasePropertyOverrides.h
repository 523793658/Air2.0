#pragma once
#include "CoreMinimal.h"
#include "Classes/Engine/EngineType.h"
namespace Air
{
	struct ENGINE_API MaterialInstanceBasePropertyOverrides
	{
		bool bOverride_OpacityMaskClipValue{ false };

		bool bOverride_BlendMode{ false };

		bool bOverride_ShadingMode{ false };

		bool bOverride_DitheredLODTransition{ false };

		bool bOverride_TwoSided{ false };

		float mOpacityMaskClipValue{ 0.333333f };

		TEnumAsByte<EBlendMode> mBlendMode{ BLEND_Opaque };

		TEnumAsByte<EMaterialShadingModel> mShadingModel{ MSM_DefaultLit };

		uint32 mTwoSided : 1;

		uint32 mDitheredLODTransition : 1;

		MaterialInstanceBasePropertyOverrides();

		bool operator == (const MaterialInstanceBasePropertyOverrides& other) const;

		bool operator != (const MaterialInstanceBasePropertyOverrides& other) const;
	};
}