#pragma once
namespace GlobalVectorConstants
{
	static const VectorRegister FloatOne = MakeVectorRegister(1.0f, 1.0f, 1.0f, 1.0f);

	static const VectorRegister FloatZero = MakeVectorRegister(0.0f, 0.0f, 0.0f, 0.0f);

	static const VectorRegister DEG_TO_RAD_HALF = MakeVectorRegister(PI / (180.f) * 0.5f, PI / (180.f) * 0.5f, PI / (180.f) * 0.5f, PI / (180.f) * 0.5f);

	static const VectorRegister DEG_TO_RAD = MakeVectorRegister(PI / 180.f, PI / 180.f, PI / 180.f, PI / 180.f);

	static const VectorRegister RAD_TO_DEG = MakeVectorRegister(180.f / PI, 180.f / PI, 180.f / PI, 180.f / PI);

	static const VectorRegister OneOverTwoPi = MakeVectorRegister(1.0f / (2.0f * PI), 1.0f / (2.0f * PI), 1.0f / (2.0f * PI), 1.0f / (2.0f * PI));

	static const VectorRegister OneOverPi = MakeVectorRegister(1.0f / PI, 1.0f / PI, 1.0f / PI, 1.0f / PI);

	static const VectorRegister Pi = MakeVectorRegister(PI, PI, PI, PI);

	static const VectorRegister TWOPi = MakeVectorRegister(2.0f * PI, 2.0f * PI, 2.0f * PI, 2.0f * PI);

	static const VectorRegister PiByTwo = MakeVectorRegister(0.5f * PI, 0.5f * PI, 0.5f * PI, 0.5f * PI);

	static const VectorRegister KindaSmallNumber = MakeVectorRegister(KINDA_SMALL_NUMBER, KINDA_SMALL_NUMBER, KINDA_SMALL_NUMBER, KINDA_SMALL_NUMBER);

	static const VectorRegister FloatMinusOne = MakeVectorRegister(-1.0f, -1.0f, -1.0f, -1.0f);

	static const VectorRegister Float360 = MakeVectorRegister(360.f, 360.f, 360.f, 360.f);

	static const VectorRegister Float180 = MakeVectorRegister(180.f, 180.f, 180.f, 180.f);

	static const VectorRegister SmallNumber = MakeVectorRegister(SMALL_NUMBER, SMALL_NUMBER, SMALL_NUMBER, SMALL_NUMBER);

	static const VectorRegister Float0001 = MakeVectorRegister(0.0f, 0.0f, 0.0f, 1.0f);

	static const VectorRegister QINV_SIGN_MASK = MakeVectorRegister(-1.0f, -1.f, -1.f, 1.f);

	static const VectorRegister SmallLengthThreshold = MakeVectorRegister(1.e-8f, 1.e-8f, 1.e-8f, 1.e-8f);

	static const VectorRegister FloatOneHundredth = MakeVectorRegister(0.01f, 0.01f, 0.01f, 0.01f);

	static const VectorRegister FloatOneHalf = MakeVectorRegister(0.5f, 0.5f, 0.5f, 0.5f);

	static const VectorRegister QMULTI_SIGN_MASK0 = MakeVectorRegister(1.f, -1.f, 1.f, -1.f);
	static const VectorRegister QMULTI_SIGN_MASK1 = MakeVectorRegister(1.f, 1.f, -1.f, -1.f);
	static const VectorRegister QMULTI_SIGN_MASK2 = MakeVectorRegister(-1.f, 1.f, 1.f, -1.f);

	static const VectorRegister XYZMask = MakeVectorRegister((uint32)0xffffffff, (uint32)0xffffffff, (uint32)0xffffffff, (uint32)0x00000000);

#define SIGN_BIT ((1 << 31))
	static const VectorRegister SignBit = MakeVectorRegister((uint32)SIGN_BIT, (uint32)SIGN_BIT, (uint32)SIGN_BIT, (uint32)SIGN_BIT);
	static const VectorRegister SignMask = MakeVectorRegister((uint32)(~SIGN_BIT), (uint32)(~SIGN_BIT), (uint32)(~SIGN_BIT), (uint32)(~SIGN_BIT));
#undef SIGN_BIT

	static const VectorRegister ThreshQuatNormalized = MakeVectorRegister(THRESH_QUAT_NORMALIZED, THRESH_QUAT_NORMALIZED, THRESH_QUAT_NORMALIZED, THRESH_QUAT_NORMALIZED);
}