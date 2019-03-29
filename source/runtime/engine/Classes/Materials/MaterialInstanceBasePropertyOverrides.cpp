#include "Classes/Materials/MaterialInstanceBasePropertyOverrides.h"
namespace Air
{
	MaterialInstanceBasePropertyOverrides::MaterialInstanceBasePropertyOverrides()
		:mTwoSided(0)
		,mDitheredLODTransition(0)
	{}

	bool MaterialInstanceBasePropertyOverrides::operator==(const MaterialInstanceBasePropertyOverrides& other) const
	{
		return
			bOverride_OpacityMaskClipValue == other.bOverride_OpacityMaskClipValue &&
			bOverride_BlendMode == other.bOverride_BlendMode&&
			bOverride_ShadingMode == other.bOverride_ShadingMode&&
			bOverride_TwoSided == other.bOverride_TwoSided &&
			bOverride_DitheredLODTransition == other.bOverride_DitheredLODTransition &&
			mOpacityMaskClipValue == other.mOpacityMaskClipValue &&
			mBlendMode == other.mBlendMode &&
			mShadingModel == other.mShadingModel &&
			mTwoSided == other.mTwoSided &&
			mDitheredLODTransition == other.mDitheredLODTransition;
	}

	bool MaterialInstanceBasePropertyOverrides::operator!=(const MaterialInstanceBasePropertyOverrides& other) const
	{
		return !(*this == other);
	}
}