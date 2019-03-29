#include "TransformVectorized.h"
namespace Air
{
	const Transform Transform::identity;

	void Transform::getRelativeTransformUsingMatrixWithScale(Transform* outTransform, const Transform* base, const Transform* relative)
	{
		Matrix am = base->toMatrixWithScale();
		Matrix bm = relative->toMatrixWithScale();
		static ScalarRegister STolerance(SMALL_NUMBER);
		VectorRegister vsafeScale3D = VectorSet_W0(getSafeScaleReciprocal(relative->mScale3D, STolerance));
		VectorRegister vscale3D = VectorMultiply(base->mScale3D, vsafeScale3D);
		constructTransformFromMatrixWithDesiredScale(am, bm.inverse(), vscale3D, *outTransform);
	}
}