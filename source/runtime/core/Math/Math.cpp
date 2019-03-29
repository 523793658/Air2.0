#include "Math/Math.h"
#include "Math/Rotator.h"
#include "Math/Quaternion.h"
#include "Math/Vector.h"
namespace Air
{
	Rotator Quaternion::rotator() const
	{
		
		const float singularTest = y * z - w * x;
		const float YawY = 2.0f * (w * y + z * x);
		const float YawX = (1.f - 2.f * (Math::square(x) + Math::square(y)));
		const float SINGULARITY_THRESHOLD = 0.4999995f;
		const float RAD_TO_DEG = (180.f) / PI;
		Rotator rotatorFromQuat;
		if (singularTest < -SINGULARITY_THRESHOLD)
		{
			rotatorFromQuat.mPitch = 90.f;
			rotatorFromQuat.mYaw = Math::atan2(YawY, YawX) * RAD_TO_DEG;
			rotatorFromQuat.mRoll = Rotator::normalizeAxis(rotatorFromQuat.mYaw + (2.0f * Math::atan2(z, w) * RAD_TO_DEG));
		}
		else if (singularTest > SINGULARITY_THRESHOLD)
		{
			rotatorFromQuat.mPitch = -90.f;
			rotatorFromQuat.mYaw = Math::atan2(YawY, YawX) * RAD_TO_DEG;
			rotatorFromQuat.mRoll = -Rotator::normalizeAxis(rotatorFromQuat.mYaw - (2.f * Math::atan2(z, w) * RAD_TO_DEG));
		}
		else
		{
			rotatorFromQuat.mPitch = -Math::fastAsin(2.f * (singularTest)) * RAD_TO_DEG;
			rotatorFromQuat.mYaw = Math::atan2(YawY, YawX) * RAD_TO_DEG;
			rotatorFromQuat.mRoll = Math::atan2(2.f * (w * z + x * y), (1.f - 2.f * (Math::square(z) + Math::square(x)))) * RAD_TO_DEG;
		}
		return rotatorFromQuat;
	}

	Quaternion Rotator::quaternion() const
	{
#if PLATFORM_ENABLE_VECTORINTRINSICS
		VectorRegister angles = MakeVectorRegister(mPitch, mYaw, mRoll, 0.0f);
		const VectorRegister halfAngles = VectorMultiply(angles, GlobalVectorConstants::DEG_TO_RAD_HALF);
		VectorRegister sinAngles, cosAngles;
		VectorSinCos(&sinAngles, &cosAngles, &halfAngles);
		const VectorRegister CP = VectorReplicate(cosAngles, 0);
		const VectorRegister SP = VectorReplicate(sinAngles, 0);
		const VectorRegister SR_SR_CR_CR_Temp = VectorShuffle(sinAngles, cosAngles, 2, 2, 2, 2);
		const VectorRegister SR_CR_SR_CR = VectorShuffle(SR_SR_CR_CR_Temp, SR_SR_CR_CR_Temp, 0, 2, 0, 2);
		const VectorRegister CR_SR_CR_SR = VectorShuffle(SR_SR_CR_CR_Temp, SR_SR_CR_CR_Temp, 2, 0, 2, 0);
		const VectorRegister SY_SY_CY_CY = VectorShuffle(sinAngles, cosAngles, 1, 1, 1, 1);
		const VectorRegister CY_CY_SY_SY = VectorShuffle(cosAngles, sinAngles, 1, 1, 1, 1);
		const uint32 neg = uint32(1 << 31);
		const uint32 pos = uint32(0);
		const VectorRegister signBitsLeft = MakeVectorRegister(pos, pos, pos, pos);
		const VectorRegister signBitsRight = MakeVectorRegister(pos, neg, neg, pos);

		const VectorRegister leftTerm = VectorBitwiseXor(signBitsLeft, VectorMultiply(CP, VectorMultiply(SY_SY_CY_CY, SR_CR_SR_CR)));
		const VectorRegister rightTerm = VectorBitwiseXor(signBitsRight, VectorMultiply(SP, VectorMultiply(CY_CY_SY_SY, CR_SR_CR_SR)));
		Quaternion rotationQuat;
		const VectorRegister result = VectorAdd(leftTerm, rightTerm);
		VectorStoreAligned(result, &rotationQuat);
#endif
		return rotationQuat;
	}

	const float3 float3::Zero(0);
	const int2 int2::Zero(EForceInit::ForceInit);

	float Math::clampAngle(float angleDegrees, float minAngleDegrees, float maxAngledegress)
	{
		float const maxDelta = Rotator::clampAxis(maxAngledegress - minAngleDegrees) * 0.5f;
		float const rangeCenter = Rotator::clampAxis(minAngleDegrees + maxDelta);
		float const deltaFromCenter = Rotator::normalizeAxis(angleDegrees - rangeCenter);

		if (deltaFromCenter > maxDelta)
		{
			return Rotator::normalizeAxis(rangeCenter + maxDelta);
		}
		else if (deltaFromCenter < -maxDelta)
		{
			return Rotator::normalizeAxis(rangeCenter - maxDelta);
		}
		return Rotator::normalizeAxis(angleDegrees);
	}
}


