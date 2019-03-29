#pragma once
#include <emmintrin.h>
typedef __m128 VectorRegister;
typedef __m128i VectorRegisterInt;
typedef __m128d VectorRegisterDouble;

#define DECLARE_VECTOR_REGISTER(x, y, z, w) {x, y, z, w}

#define SHUFFLEMASK(A0, A1, B2, B3) ((A0) | ((A1) <<2 ) | ((B2) <<4) | ((B3) << 6))

FORCEINLINE VectorRegister MakeVectorRegister(uint32 x, uint32 y, uint32 z, uint32 w)
{
	union { VectorRegister v; VectorRegisterInt i; } tmp;
	tmp.i = _mm_setr_epi32(x, y, z, w);
	return tmp.v;
}

FORCEINLINE VectorRegister MakeVectorRegister(float x, float y, float z, float w)
{
	return _mm_setr_ps(x, y, z, w);
}

FORCEINLINE VectorRegisterInt MakeVectorRegister(int32 x, int32 y, int32 z, int32 w)
{
	return _mm_setr_epi32(x, y, z, w);
}

#include "Math/MathVectorConstants.h"


#define VectorMultiply(Vec1, Vec2)	_mm_mul_ps(Vec1, Vec2)

#define VectorMultiplyAdd(Vec1, Vec2, Vec3) _mm_add_ps(_mm_mul_ps(Vec1, Vec2), Vec3)

#define VectorReplicate(Vec, ElementIndex)	_mm_shuffle_ps(Vec, Vec, SHUFFLEMASK(ElementIndex, ElementIndex, ElementIndex, ElementIndex))

#define VectorLoadAligned(ptr) _mm_load_ps((float*)(ptr))

#define VectorStoreAligned(Vec, ptr) _mm_store_ps((float*)(ptr), Vec)

#define VectorAdd(vec1, vec2)		_mm_add_ps(vec1, vec2)

#define VectorAbs(Vec)				_mm_and_ps(Vec, GlobalVectorConstants::SignMask)

#define VectorSubtract(Vec1, Vec2) _mm_sub_ps(Vec1, Vec2)

#define VectorAnyGreaterThan(vec1, vec2) _mm_movemask_ps(_mm_cmpgt_ps(vec1, vec2))

#define VectorMaskBits(VecMask)				_mm_movemask_ps(VecMask)

#define VectorCompareEQ(Vec1, Vec2)		_mm_cmpeq_ps(Vec1, Vec2)

#define VectorSwizzle(vec, x, y, z, w) _mm_shuffle_ps(vec, vec, SHUFFLEMASK(x, y, z, w))


#define VectorReplicate(Vec, ElementIndex) _mm_shuffle_ps(Vec, Vec, SHUFFLEMASK(ElementIndex, ElementIndex, ElementIndex, ElementIndex))

#define VectorShuffle(vec1, vec2, x, y, z, w) _mm_shuffle_ps(vec1, vec2, SHUFFLEMASK(x, y, z, w))

#define VectorLoadFloat1(ptr)		_mm_load1_ps((float*)(ptr))

#define VectorLoadFloat3_W0(ptr)	MakeVectorRegister(((const float*)(ptr))[0], ((const float*)(ptr))[1], ((const float*)(ptr))[2], 0.0f)

#define VectorMin(vec1, vec2)	_mm_min_ps(vec1, vec2)

#define VectorBitwiseAnd(vec1, vec2) _mm_and_ps(vec1, vec2)

#define VectorBitwiseOr(vec1, vec2) _mm_or_ps(vec1, vec2)

#define VectorBitwiseXor(vec1, vec2) _mm_xor_ps(vec1, vec2)


#define VectorOne()				(GlobalVectorConstants::FloatOne)
#define VectorZero()			(GlobalVectorConstants::FloatZero)

#define VectorSet_W0(vec)		_mm_and_ps(vec, GlobalVectorConstants::XYZMask)

#define VectorCompareGE(vec1, vec2)	_mm_cmpge_ps(vec1, vec2)

#define VectorReciprocalSqrt(vec)  _mm_rsqrt_ps(vec)

#define VectorReciprocal(vec)	_mm_rcp_ps(vec)

#define VectorDivide(vec1, vec2)	_mm_div_ps(vec1, vec2)

#define VectorLoadByte4(Ptr)	_mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(*(int32*)Ptr), _mm_setzero_si128()), _mm_setzero_si128()))

#define VectorResetFloatRegisters()

FORCEINLINE VectorRegister VectorSet_W1(const VectorRegister& vector)
{
	VectorRegister temp = _mm_movehl_ps(VectorOne(), vector);
	return _mm_shuffle_ps(vector, temp, SHUFFLEMASK(0, 1, 0, 3));
}

FORCEINLINE VectorRegister VectorCross(const VectorRegister& vec1, const VectorRegister& vec2)
{
	VectorRegister A_YZXW = _mm_shuffle_ps(vec1, vec1, SHUFFLEMASK(1, 2, 0, 3));
	VectorRegister B_ZXYW = _mm_shuffle_ps(vec2, vec2, SHUFFLEMASK(2, 0, 1, 3));
	VectorRegister A_ZXYW = _mm_shuffle_ps(vec1, vec1, SHUFFLEMASK(2, 0, 1, 3));
	VectorRegister B_YZXW = _mm_shuffle_ps(vec2, vec2, SHUFFLEMASK(1, 2, 0, 3));
	return VectorSubtract(VectorMultiply(A_YZXW, B_ZXYW), VectorMultiply(A_ZXYW, B_YZXW));
}

FORCEINLINE VectorRegister VectorSelect(const VectorRegister& mask, const VectorRegister& vec1, const VectorRegister& vec2)
{
	return _mm_xor_ps(vec2, _mm_and_ps(mask, _mm_xor_ps(vec1, vec2)));
}

FORCEINLINE VectorRegister VectorReciprocalSqrtAccurate(const VectorRegister& vec)
{
	const VectorRegister oneHalf = GlobalVectorConstants::FloatOneHalf;
	const VectorRegister vecDivBy2 = VectorMultiply(vec, oneHalf);

	const VectorRegister x0 = VectorReciprocalSqrt(vec);
	VectorRegister x1 = VectorMultiply(x0, x0);
	x1 = VectorSubtract(oneHalf, VectorMultiply(vecDivBy2, x1));
	x1 = VectorMultiplyAdd(x0, x1, x0);

	VectorRegister x2 = VectorMultiply(x1, x1);
	x2 = VectorSubtract(oneHalf, VectorMultiply(vecDivBy2, x2));
	x2 = VectorMultiplyAdd(x1, x2, x1);

	return x2;

}



FORCEINLINE VectorRegister vectorQuaternionMultiply2(const VectorRegister& quat1, const VectorRegister& quat2)
{
	VectorRegister result = VectorMultiply(VectorReplicate(quat1, 3), quat2);
	result = VectorMultiplyAdd(VectorMultiply(VectorReplicate(quat1, 0), VectorSwizzle(quat2, 3, 2, 1, 0)), GlobalVectorConstants::QMULTI_SIGN_MASK0, result);
	result = VectorMultiplyAdd(VectorMultiply(VectorReplicate(quat1, 1), VectorSwizzle(quat2, 2, 3, 0, 1)), GlobalVectorConstants::QMULTI_SIGN_MASK1, result);
	result = VectorMultiplyAdd(VectorMultiply(VectorReplicate(quat1, 2), VectorSwizzle(quat2, 1, 0, 3, 2)), GlobalVectorConstants::QMULTI_SIGN_MASK2, result);
	return result;
}

FORCEINLINE void vectorQuaternionMultiply(void* RESTRICT result, const void* RESTRICT quat1, const void* RESTRICT quat2)
{
	*((VectorRegister*)result) = vectorQuaternionMultiply2(*((const VectorRegister*)quat1), *((const VectorRegister*)quat2));
}

FORCEINLINE void vectorMatrixMultiply(void* result, const void* matrix1, const void* matrix2)
{
	const VectorRegister * A = (const VectorRegister*)matrix1;

	const VectorRegister * B = (const VectorRegister*)matrix2;

	VectorRegister* R = (VectorRegister*)result;
	VectorRegister Temp, R0, R1, R2, R3;
	VectorReplicate(A[0], 0);
	Temp = VectorMultiply(VectorReplicate(A[0], 0), B[0]);
	Temp = VectorMultiplyAdd(VectorReplicate(A[0], 1), B[1], Temp);
	Temp = VectorMultiplyAdd(VectorReplicate(A[0], 2), B[2], Temp);
	R0 = VectorMultiplyAdd(VectorReplicate(A[0], 3), B[3], Temp);

	Temp = VectorMultiply(VectorReplicate(A[1], 0), B[0]);
	Temp = VectorMultiplyAdd(VectorReplicate(A[1], 1), B[1], Temp);
	Temp = VectorMultiplyAdd(VectorReplicate(A[1], 2), B[2], Temp);
	R1 = VectorMultiplyAdd(VectorReplicate(A[1], 3), B[3], Temp);

	Temp = VectorMultiply(VectorReplicate(A[2], 0), B[0]);
	Temp = VectorMultiplyAdd(VectorReplicate(A[2], 1), B[1], Temp);
	Temp = VectorMultiplyAdd(VectorReplicate(A[2], 2), B[2], Temp);
	R2 = VectorMultiplyAdd(VectorReplicate(A[2], 3), B[3], Temp);

	Temp = VectorMultiply(VectorReplicate(A[3], 0), B[0]);
	Temp = VectorMultiplyAdd(VectorReplicate(A[3], 1), B[1], Temp);
	Temp = VectorMultiplyAdd(VectorReplicate(A[3], 2), B[2], Temp);
	R3 = VectorMultiplyAdd(VectorReplicate(A[3], 3), B[3], Temp);

	R[0] = R0;
	R[1] = R1;
	R[2] = R2;
	R[3] = R3;
}

FORCEINLINE void vectorMatrixInverse(void* dstMatrix, const void* srcMatrix)
{
	typedef float float4x4[4][4];
	const float4x4& M = *((const float4x4*)srcMatrix);
	float4x4 Result;
	float Det[4];
	float4x4 Tmp;
	Tmp[0][0] = M[2][2] * M[3][3] - M[2][3] * M[3][2];
	Tmp[0][1] = M[1][2] * M[3][3] - M[1][3] * M[3][2];
	Tmp[0][2] = M[1][2] * M[2][3] - M[1][3] * M[2][2];

	Tmp[1][0] = M[2][2] * M[3][3] - M[2][3] * M[3][2];
	Tmp[1][1] = M[0][2] * M[3][3] - M[0][3] * M[3][2];
	Tmp[1][2] = M[0][2] * M[2][3] - M[0][3] * M[2][2];

	Tmp[2][0] = M[1][2] * M[3][3] - M[1][3] * M[3][2];
	Tmp[2][1] = M[0][2] * M[3][3] - M[0][3] * M[3][2];
	Tmp[2][2] = M[0][2] * M[1][3] - M[0][3] * M[1][2];

	Tmp[3][0] = M[1][2] * M[2][3] - M[1][3] * M[2][2];
	Tmp[3][1] = M[0][2] * M[2][3] - M[0][3] * M[2][2];
	Tmp[3][2] = M[0][2] * M[1][3] - M[0][3] * M[1][2];

	Det[0] = M[1][1] * Tmp[0][0] - M[2][1] * Tmp[0][1] + M[3][1] * Tmp[0][2];
	Det[1] = M[0][1] * Tmp[1][0] - M[2][1] * Tmp[1][1] + M[3][1] * Tmp[1][2];
	Det[2] = M[0][1] * Tmp[2][0] - M[1][1] * Tmp[2][1] + M[3][1] * Tmp[2][2];
	Det[3] = M[0][1] * Tmp[3][0] - M[1][1] * Tmp[3][1] + M[2][1] * Tmp[3][2];

	float Determinant = M[0][0] * Det[0] - M[1][0] * Det[1] + M[2][0] * Det[2] - M[3][0] * Det[3];
	const float	RDet = 1.0f / Determinant;

	Result[0][0] = RDet * Det[0];
	Result[0][1] = -RDet * Det[1];
	Result[0][2] = RDet * Det[2];
	Result[0][3] = -RDet * Det[3];
	Result[1][0] = -RDet * (M[1][0] * Tmp[0][0] - M[2][0] * Tmp[0][1] + M[3][0] * Tmp[0][2]);
	Result[1][1] = RDet * (M[0][0] * Tmp[1][0] - M[2][0] * Tmp[1][1] + M[3][0] * Tmp[1][2]);
	Result[1][2] = -RDet * (M[0][0] * Tmp[2][0] - M[1][0] * Tmp[2][1] + M[3][0] * Tmp[2][2]);
	Result[1][3] = RDet * (M[0][0] * Tmp[3][0] - M[1][0] * Tmp[3][1] + M[2][0] * Tmp[3][2]);
	Result[2][0] = RDet * (
		M[1][0] * (M[2][1] * M[3][3] - M[2][3] * M[3][1]) -
		M[2][0] * (M[1][1] * M[3][3] - M[1][3] * M[3][1]) +
		M[3][0] * (M[1][1] * M[2][3] - M[1][3] * M[2][1])
		);
	Result[2][1] = -RDet * (
		M[0][0] * (M[2][1] * M[3][3] - M[2][3] * M[3][1]) -
		M[2][0] * (M[0][1] * M[3][3] - M[0][3] * M[3][1]) +
		M[3][0] * (M[0][1] * M[2][3] - M[0][3] * M[2][1])
		);
	Result[2][2] = RDet * (
		M[0][0] * (M[1][1] * M[3][3] - M[1][3] * M[3][1]) -
		M[1][0] * (M[0][1] * M[3][3] - M[0][3] * M[3][1]) +
		M[3][0] * (M[0][1] * M[1][3] - M[0][3] * M[1][1])
		);
	Result[2][3] = -RDet * (
		M[0][0] * (M[1][1] * M[2][3] - M[1][3] * M[2][1]) -
		M[1][0] * (M[0][1] * M[2][3] - M[0][3] * M[2][1]) +
		M[2][0] * (M[0][1] * M[1][3] - M[0][3] * M[1][1])
		);
	Result[3][0] = -RDet * (
		M[1][0] * (M[2][1] * M[3][2] - M[2][2] * M[3][1]) -
		M[2][0] * (M[1][1] * M[3][2] - M[1][2] * M[3][1]) +
		M[3][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1])
		);
	Result[3][1] = RDet * (
		M[0][0] * (M[2][1] * M[3][2] - M[2][2] * M[3][1]) -
		M[2][0] * (M[0][1] * M[3][2] - M[0][2] * M[3][1]) +
		M[3][0] * (M[0][1] * M[2][2] - M[0][2] * M[2][1])
		);
	Result[3][2] = -RDet * (
		M[0][0] * (M[1][1] * M[3][2] - M[1][2] * M[3][1]) -
		M[1][0] * (M[0][1] * M[3][2] - M[0][2] * M[3][1]) +
		M[3][0] * (M[0][1] * M[1][2] - M[0][2] * M[1][1])
		);
	Result[3][3] = RDet * (
		M[0][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1]) -
		M[1][0] * (M[0][1] * M[2][2] - M[0][2] * M[2][1]) +
		M[2][0] * (M[0][1] * M[1][2] - M[0][2] * M[1][1])
		);

	memcpy(dstMatrix, &Result, 16 * sizeof(float));
}

FORCEINLINE VectorRegister vectorTransformVector(const VectorRegister& vecP, const void* matrixM)
{
	const VectorRegister *M = (const VectorRegister*)matrixM;
	VectorRegister VTempX, VTempY, VTempZ, VTempW;
	VTempX = VectorReplicate(vecP, 0);
	VTempY = VectorReplicate(vecP, 1);
	VTempZ = VectorReplicate(vecP, 2);
	VTempW = VectorReplicate(vecP, 3);

	VTempX = VectorMultiply(VTempX, M[0]);
	VTempY = VectorMultiply(VTempY, M[1]);
	VTempZ = VectorMultiply(VTempZ, M[2]);
	VTempW = VectorMultiply(VTempW, M[3]);

	VTempX = VectorAdd(VTempX, VTempY);
	VTempZ = VectorAdd(VTempZ, VTempW);
	VTempX = VectorAdd(VTempX, VTempZ);
	return VTempX;
}

FORCEINLINE VectorRegister VectorDot4(const VectorRegister& vec1, const VectorRegister& vec2)
{
	VectorRegister temp1, temp2;
	temp1 = VectorMultiply(vec1, vec2);
	temp2 = _mm_shuffle_ps(temp1, temp1, SHUFFLEMASK(2, 3, 0, 1));
	temp1 = VectorAdd(temp1, temp2);
	temp2 = _mm_shuffle_ps(temp1, temp1, SHUFFLEMASK(1, 2, 3, 0));
	return VectorAdd(temp1, temp2);
}

FORCEINLINE VectorRegister VectorReciprocalAccurate(const VectorRegister& vec)
{
	const VectorRegister x0 = VectorReciprocal(vec);
	const VectorRegister x0Squared = VectorMultiply(x0, x0);
	const VectorRegister x0Times2 = VectorAdd(x0, x0);
	const VectorRegister x1 = VectorSubtract(x0Times2, VectorMultiply(vec, x0Squared));
	const VectorRegister x1Squared = VectorMultiply(x1, x1);
	const VectorRegister x1Times2 = VectorAdd(x1, x1);
	const VectorRegister x2 = VectorSubtract(x1Times2, VectorMultiply(vec, x1Squared));
	return x2;
}

FORCEINLINE VectorRegister VectorTruncate(const VectorRegister& x)
{
	return _mm_cvtepi32_ps(_mm_cvttps_epi32(x));
}

FORCEINLINE VectorRegister vectorMod(const VectorRegister& x, const VectorRegister& y)
{
	VectorRegister temp = VectorTruncate(VectorDivide(x, y));
	return VectorSubtract(x, VectorMultiply(y, temp));
}

FORCEINLINE void VectorSinCos(VectorRegister* RESTRICT vSinAngle, VectorRegister * RESTRICT vCosAngle, const VectorRegister* RESTRICT vAngles)
{
	VectorRegister quotient = VectorMultiply(*vAngles, GlobalVectorConstants::OneOverTwoPi);
	quotient = _mm_cvtepi32_ps(_mm_cvtps_epi32(quotient));
	VectorRegister x = VectorSubtract(*vAngles, VectorMultiply(GlobalVectorConstants::TWOPi, quotient));
	VectorRegister sign = VectorBitwiseAnd(x, GlobalVectorConstants::SignBit);
	VectorRegister C = VectorBitwiseOr(GlobalVectorConstants::Pi, sign);
	VectorRegister absx = VectorAbs(x);
	VectorRegister rflx = VectorSubtract(C, x);
	VectorRegister comp = VectorCompareGE(absx, GlobalVectorConstants::PiByTwo);
	x = VectorSelect(comp, rflx, x);
	sign = VectorSelect(comp, GlobalVectorConstants::FloatMinusOne, GlobalVectorConstants::FloatOne);
	const VectorRegister xSquared = VectorMultiply(x, x);
	const VectorRegister sincoeff0 = MakeVectorRegister(1.0f, -0.166666667f, 0.0083333310f, -0.00019840874f);
	const VectorRegister sincoeff1 = MakeVectorRegister(2.7525562e-06f, -2.3889859e-08f, 0.f, 0.f);
	VectorRegister s;
	s = VectorReplicate(sincoeff1, 1);
	s = VectorMultiplyAdd(xSquared, s, VectorReplicate(sincoeff0, 3));
	s = VectorMultiplyAdd(xSquared, s, VectorReplicate(sincoeff0, 2));
	s = VectorMultiplyAdd(xSquared, s, VectorReplicate(sincoeff0, 1));
	s = VectorMultiplyAdd(xSquared, s, VectorReplicate(sincoeff0, 0));
	*vSinAngle = VectorMultiply(s, x);

	const VectorRegister cosCoeff0 = MakeVectorRegister(1.0f, -0.5f, 0.041666638f, -0.0013888378f);

	const VectorRegister cosCoeff1 = MakeVectorRegister(2.4760495e-05f, -2.6051615e-07f, 0.f, 0.f);
	VectorRegister c;
	c = VectorReplicate(cosCoeff1, 1);
	c = VectorMultiplyAdd(xSquared, c, VectorReplicate(cosCoeff1, 0));
	c = VectorMultiplyAdd(xSquared, c, VectorReplicate(cosCoeff0, 3));
	c = VectorMultiplyAdd(xSquared, c, VectorReplicate(cosCoeff0, 2));
	c = VectorMultiplyAdd(xSquared, c, VectorReplicate(cosCoeff0, 1));
	c = VectorMultiplyAdd(xSquared, c, VectorReplicate(cosCoeff0, 0));
	*vCosAngle = VectorMultiply(c, sign);

}

FORCEINLINE float VectorGetComponent(VectorRegister vec, uint32 componentIndex)
{
	return (((float*)&(vec))[componentIndex]);
}

FORCEINLINE uint32 VectorAnyLesserThan(VectorRegister vec1, VectorRegister vec2)
{
	return VectorAnyGreaterThan(vec2, vec1);
}

FORCEINLINE void VectorStoreFloat3(const VectorRegister& vec, void* ptr)
{
	union { VectorRegister v; float f[4]; } tmp;
	tmp.v = vec;
	float* floatPtr = (float*)(ptr);
	floatPtr[0] = tmp.f[0];
	floatPtr[1] = tmp.f[1];
	floatPtr[2] = tmp.f[2];
}

FORCEINLINE void VectorStore(const VectorRegister& vec, void* ptr)
{
	union { VectorRegister v; float f[4]; } tmp;
	tmp.v = vec;
	float* floatPtr = (float*)(ptr);
	floatPtr[0] = tmp.f[0];
	floatPtr[1] = tmp.f[1];
	floatPtr[2] = tmp.f[2];
	floatPtr[3] = tmp.f[3];

}

FORCEINLINE VectorRegister VectorSign(const VectorRegister& x)
{
	VectorRegister mask = VectorCompareGE(x, (GlobalVectorConstants::FloatZero));
	return VectorSelect(mask, (GlobalVectorConstants::FloatOne), (GlobalVectorConstants::FloatMinusOne));
}