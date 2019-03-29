#pragma once
#include "EngineMininal.h"
#include "Containers/EnumAsByte.h"
#include "Classes/Camera/CameraTypes.h"
#include "EngineDefines.h"
#include "scene.h"
namespace Air
{
	namespace ECameraProjectionMode
	{
		enum Type
		{
			Perspective,
			Orthographic
		};
	}


	struct MinimalViewInfo
	{
		float3 mLocation;
		Rotator mRotation;
		float mFOV;
		float mOrthoWidth;
		float mOrthoNearClipPlane;
		float mOrthoFarClipPlane;
		float mAspectRatio;
		uint32 bConstrainAspectRatio : 1;
		uint32 bUseFieldOfViewForLOD : 1;
		TEnumAsByte<ECameraProjectionMode::Type> mProjectonMode;

		float mPostProcessBlendWeight;

		struct PostProcessSettings mPostProcessSettings;

		float2 OffCenterProjectionOffset;

		MinimalViewInfo()
			:mLocation(ForceInit)
			,mRotation(ForceInit)
			,mFOV(90.0f)
			,mOrthoWidth(512.0f)
			,mOrthoNearClipPlane(0.0f)
			,mOrthoFarClipPlane(WORLD_MAX)
			,mAspectRatio(1.33333333f)
			,bConstrainAspectRatio(false)
			,bUseFieldOfViewForLOD(true)
			,mProjectonMode(ECameraProjectionMode::Perspective)
			,mPostProcessBlendWeight(0.0f)
			,OffCenterProjectionOffset(ForceInitToZero)
		{
		}

		ENGINE_API static void calculateProjectionMatrixGivenView(const MinimalViewInfo& viewInfo, TEnumAsByte<enum EAspectRatioAxisConstraint> aspectRatioAxisConstraint, class Viewport* viewport, struct SceneViewProjectionData& inOutProjectionData);

		ENGINE_API bool equals(const MinimalViewInfo& otherInfo) const;
	};
}