#include "Classes/Camera/CameraTypes.h"
#include "SceneView.h"
namespace Air
{
	void MinimalViewInfo::calculateProjectionMatrixGivenView(const MinimalViewInfo& viewInfo, TEnumAsByte<enum EAspectRatioAxisConstraint> aspectRatioAxisConstraint, class Viewport* viewport, struct SceneViewProjectionData& inOutProjectionData)
	{
		if (viewInfo.bConstrainAspectRatio)
		{
			inOutProjectionData.setConstrainedViewRectangle(viewport->calculateViewExtents(viewInfo.mAspectRatio, inOutProjectionData.getViewRect()));
			if (viewInfo.mProjectonMode == ECameraProjectionMode::Orthographic)
			{
				const float YScale = 1.0f / viewInfo.mAspectRatio;
				const float orthoWidth = viewInfo.mOrthoWidth / 2.0f;
				const float orthoHeight = orthoWidth * YScale;
				const float nearPlane = viewInfo.mOrthoNearClipPlane;
				const float farPlane = viewInfo.mOrthoFarClipPlane;
				const float zScale = 1.0f / (farPlane - nearPlane);
				const float zOffset = -nearPlane;

				inOutProjectionData.mProjectionMatrix = ReversedZOrthoMatrix(orthoWidth, orthoHeight, zScale, zOffset);

			}
			else
			{
				inOutProjectionData.mProjectionMatrix = ReversedZPerspectiveMatrix(std::max<float>(0.001f, viewInfo.mFOV) * (float)PI / 360.f, viewInfo.mAspectRatio, 1.0f, GNearClippingPlane);
			}
		}
		else
		{
			float MatrixFOV = std::max<float>(0.001f, viewInfo.mFOV) * (float)PI / 360.f;
			float xAxisMultiplier;
			float YAxisMultiplier;
			const IntRect& viewRect = inOutProjectionData.getViewRect();
			const int32 width = viewRect.width();
			const int32 height = viewRect.height();

			if (((width > height) && (aspectRatioAxisConstraint == AspectRatio_MajorAxisFOV)) || (aspectRatioAxisConstraint == AspectRatio_MaintainXFOV) || (viewInfo.mProjectonMode == ECameraProjectionMode::Orthographic))
			{
				xAxisMultiplier = 1.0f;
				YAxisMultiplier = width / (float)height;
			}
			else
			{
				xAxisMultiplier = height / (float)width;
				YAxisMultiplier = 1.0f;
			}
			if (viewInfo.mProjectonMode == ECameraProjectionMode::Orthographic)
			{
				const float orthoWidth = viewInfo.mOrthoWidth / 2.0f * xAxisMultiplier;
				const float orthoHeight = viewInfo.mOrthoWidth / 2.0f * YAxisMultiplier;

				const float nearPlane = viewInfo.mOrthoNearClipPlane;
				const float farPlane = viewInfo.mOrthoFarClipPlane;

				const float zScale = 1.0f / (farPlane - nearPlane);
				const float zOffset = -nearPlane;

				//inOutProjectionData.mProjectionMatrix = OrthoMatrix(orthoWidth, orthoHeight, zScale, zOffset);
			}
			else
			{
				inOutProjectionData.mProjectionMatrix = PerspectiveMatrix(MatrixFOV, MatrixFOV, xAxisMultiplier, YAxisMultiplier, GNearClippingPlane, GNearClippingPlane);
			}
		}
		if (!viewInfo.OffCenterProjectionOffset.isZero())
		{
			const float left = -1.0f + viewInfo.OffCenterProjectionOffset.x;
			const float right = left + 2.0f;
			const float bottom = -1.0f + viewInfo.OffCenterProjectionOffset.y;
			const float top = bottom + 2.0f;
			inOutProjectionData.mProjectionMatrix.M[2][0] = (left + right) / (left - right);
			inOutProjectionData.mProjectionMatrix.M[2][1] = (bottom + top) / (bottom - top);
		}
	}

	bool MinimalViewInfo::equals(const MinimalViewInfo& otherInfo) const
	{
		return (mLocation == otherInfo.mLocation) &&
			(mRotation == otherInfo.mRotation) &&
			(mFOV == otherInfo.mFOV) &&
			(mOrthoWidth == otherInfo.mOrthoWidth) &&
			(mOrthoNearClipPlane == otherInfo.mOrthoNearClipPlane) &&
			(mOrthoFarClipPlane == otherInfo.mOrthoFarClipPlane) &&
			(mAspectRatio == otherInfo.mAspectRatio) &&
			(bConstrainAspectRatio == otherInfo.bConstrainAspectRatio) &&
			(bUseFieldOfViewForLOD == otherInfo.bUseFieldOfViewForLOD) &&
			(mProjectonMode == otherInfo.mProjectonMode) &&
			(OffCenterProjectionOffset == otherInfo.OffCenterProjectionOffset);
	}
}