#pragma once
#include "CoreType.h"
#include "Math/Math.h"
namespace Air
{
	struct Quaternion;

	struct Rotator
	{
	public:
		//¸©Ñö£¬xÖá
		float mPitch;

		//Æ«º½£¬YÖá
		float mYaw;
		//·­¹ö£¬ZÖá
		float mRoll;
	public:
		static CORE_API const Rotator ZeroRotator;
	public:
		FORCEINLINE Rotator() {}

		explicit FORCEINLINE Rotator(float inF);

		FORCEINLINE Rotator(float inPitch, float inYaw, float inRoll)
			:mYaw(inYaw), mPitch(inPitch), mRoll(inRoll)
		{

		}

		explicit FORCEINLINE Rotator(EForceInit)
			:mRoll(0), mYaw(0), mPitch(0)
		{

		}

		explicit CORE_API Rotator(const Quaternion& quat);

		FORCEINLINE bool operator == (const Rotator& rhs) const
		{
			return mPitch == rhs.mPitch && mRoll == rhs.mRoll && mYaw == rhs.mYaw;
		}

		static FORCEINLINE float clampAxis(float angle)
		{
			angle = Math::fmod(angle, 360.f);
			if (angle < 0.f)
			{
				angle += 360.f;
			}
			return angle;
		}

		FORCEINLINE void normalize()
		{
#if PLATFORM_ENABLE_VECTORINTRINSICS
			VectorRegister vRotator = VectorLoadFloat3_W0(this);
			vRotator = vectorNormalizeRotator(vRotator);
			VectorStoreFloat3(vRotator, this);
#endif
		}

		FORCEINLINE Rotator getNormalized() const
		{
			Rotator rot = *this;
			rot.normalize();
			return rot;
		}

		FORCEINLINE bool operator != (const Rotator& v) const
		{
			return mPitch != v.mPitch || mYaw != v.mYaw || mRoll != v.mRoll;
		}

		FORCEINLINE Rotator operator +=(const Rotator& r)
		{
			mPitch += r.mPitch;
			mYaw += r.mYaw;
			mRoll += r.mRoll;
			return *this;
		}

		FORCEINLINE Rotator clamp() const
		{
			return Rotator(clampAxis(mPitch), clampAxis(mYaw), clampAxis(mRoll));
		}

		static FORCEINLINE float normalizeAxis(float angle)
		{
			angle = clampAxis(angle);
			if (angle > 180.f)
			{
				angle -= 360.f;
			}
			return angle;
		}

		bool equals(const Rotator& r, float tolerance = KINDA_SMALL_NUMBER) const;

		CORE_API Quaternion quaternion() const;
	};

	FORCEINLINE bool Rotator::equals(const Rotator& r, float tolerance /* = KINDA_SMALL_NUMBER */)const
	{
#if PLATFORM_ENABLE_VECTORINTRINSICS
		const VectorRegister regA = VectorLoadFloat3_W0(this);
		const VectorRegister regB = VectorLoadFloat3_W0(&r);
		const VectorRegister normDelta = vectorNormalizeRotator(VectorSubtract(regA, regB));
		const VectorRegister absNormDelta = VectorAbs(normDelta);
		return !VectorAnyGreaterThan(absNormDelta, VectorLoadFloat1(&tolerance));
#endif
	}
}