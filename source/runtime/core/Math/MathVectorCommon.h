#pragma once
#include "MathSSE.h"

FORCEINLINE VectorRegister vectorQuaternionRotateVector(const VectorRegister& quat, const VectorRegister& vectorW0)
{
	const VectorRegister qw = VectorReplicate(quat, 3);
	VectorRegister t = VectorCross(quat, vectorW0);
	t = VectorAdd(t, t);
	const VectorRegister vTemp0 = VectorMultiplyAdd(qw, t, vectorW0);
	const VectorRegister vTemp1 = VectorCross(quat, t);
	const VectorRegister rotated = VectorAdd(vTemp0, vTemp1);
	return rotated;
}



FORCEINLINE VectorRegister vectorQuaternionInverse(const VectorRegister& normalizedQuat)
{
	return VectorMultiply(GlobalVectorConstants::QINV_SIGN_MASK, normalizedQuat);
}

FORCEINLINE VectorRegister vectorQuaternionInverseRotateVector(const VectorRegister& quat, const VectorRegister& vectorW0)
{
	const VectorRegister QInv = vectorQuaternionInverse(quat);
	return vectorQuaternionRotateVector(QInv, vectorW0);
}

FORCEINLINE VectorRegister vectorNormalizeRotator(const VectorRegister& unnormalizedRotator)
{
	VectorRegister v0 = vectorMod(unnormalizedRotator, GlobalVectorConstants::Float360);
	VectorRegister v1 = VectorAdd(v0, GlobalVectorConstants::Float360);
	VectorRegister v2 = VectorSelect(VectorCompareGE(v0, VectorZero()), v0, v1);
	VectorRegister v3 = VectorSubtract(v2, GlobalVectorConstants::Float360);
	VectorRegister v4 = VectorSelect(VectorCompareGE(v2, GlobalVectorConstants::Float180), v3, v2);
	return v4;
}