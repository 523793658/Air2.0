#pragma once
#include "CoreType.h"
#include "Math/Rotator.h"
#include "Math/Matrix.hpp"
namespace Air
{
	MS_ALIGN(16) struct Quaternion
	{
	public:
		float x;
		float y;
		float z;
		float w;
	public:
		static CORE_API const Quaternion identity;
	public:

		FORCEINLINE Quaternion() {}



		explicit FORCEINLINE Quaternion(EForceInit);

		FORCEINLINE Quaternion(float inX, float inY, float inZ, float inW);

		FORCEINLINE Quaternion(const Quaternion& rhs);

		FORCEINLINE Quaternion(const Rotator& r);

		inline explicit Quaternion(const Matrix& m)
		{
			if (m.getScaleAxis(EAxis::X).isNearlyZero() || m.getScaleAxis(EAxis::Y).isNearlyZero() || m.getScaleAxis(EAxis::Z).isNearlyZero())
			{
				*this = Quaternion::identity;
				return;
			}

			float s;
			const float tr = m.M[0][0] + m.M[1][1] + m.M[2][2];
			if (tr > 0.0f)
			{
				float invs = Math::InvSqrt(tr + 1.0f);
				this->w = 0.5f * (1.f / invs);
				s = 0.5f * invs;
				this->x = (m.M[1][2] - m.M[2][1]) * s;
				this->y = (m.M[2][0] - m.M[0][2]) * s;
				this->z = (m.M[0][1] - m.M[1][0]) * s;

			}
			else
			{
				int32 i = 0; 
				if (m.M[1][1] > m.M[0][0])
				{
					i = 1;
				}
				if (m.M[2][2] > m.M[i][i])
				{
					i = 2;
				}

				static const int32 nxt[3] = { 1, 2, 0 };
				const int32 j = nxt[i];
				const int32 k = nxt[j];
				s = m.M[i][j] - m.M[j][j] - m.M[k][k] + 1.0f;
				float invs = Math::InvSqrt(s);

				float qt[4];
				qt[i] = 0.5f * (1.0f / invs);
				s = 0.5f * invs;

				qt[3] = (m.M[j][k] - m.M[k][j]) * s;
				qt[j] = (m.M[i][j] - m.M[j][i]) * s;
				qt[k] = (m.M[i][k] - m.M[k][i]) * s;


				this->x = qt[0];
				this->y = qt[1];
				this->z = qt[2];
				this->w = qt[3];
			}
		}
		
		FORCEINLINE Quaternion inverse() const;

		CORE_API Rotator rotator() const;

		FORCEINLINE float3 rotateVector(float3 v) const;

		FORCEINLINE void normalize(float tolerance = SMALL_NUMBER)
		{
#if PLATFORM_ENABLE_VECTORINTRINSICS
			const VectorRegister vector = VectorLoadAligned(this);
			const VectorRegister squareSum = VectorDot4(vector, vector);
			const VectorRegister nozeroMask = VectorCompareGE(squareSum, VectorLoadFloat1(&tolerance));

			const VectorRegister invLength = VectorReciprocalSqrtAccurate(squareSum);
			const VectorRegister normalizedVector = VectorMultiply(invLength, vector);

			VectorRegister result = VectorSelect(nozeroMask, normalizedVector, GlobalVectorConstants::Float0001);
			VectorStoreAligned(result, this);
#endif
		}


		FORCEINLINE float3 getAxisX() const
		{
			return rotateVector(float3::Right);
		}

		FORCEINLINE float3 getAxisY() const
		{
			return rotateVector(float3::Up);
		}

		FORCEINLINE float3 getAxisZ() const
		{
			return rotateVector(float3::Forward);
		}

		FORCEINLINE Quaternion getNormalized(float tolerance = SMALL_NUMBER) const
		{
			Quaternion result(*this);
			result.normalize(tolerance);
			return result;
		}

		FORCEINLINE bool equals(const Quaternion& q, float tolerance) const;

		FORCEINLINE bool operator !=(const Quaternion& q) const;

		FORCEINLINE bool operator ==(const Quaternion& q) const
		{
#if PLATFORM_ENABLE_VECTORINTRINSICS
			const VectorRegister A = VectorLoadAligned(this);
			const VectorRegister B = VectorLoadAligned(&q);
			return VectorMaskBits(VectorCompareEQ(A, B)) == 0x0f;
#endif
		}

		FORCEINLINE Quaternion operator*(const Quaternion& q) const
		{
			Quaternion result;
			vectorQuaternionMultiply(&result, this, &q);
			return result;
		}
	};

	FORCEINLINE bool Quaternion::operator !=(const Quaternion& q) const
	{
#if PLATFORM_ENABLE_VECTORINTRINSICS
		const VectorRegister a = VectorLoadAligned(this);
		const VectorRegister b = VectorLoadAligned(&q);
		return VectorMaskBits(VectorCompareGE(a, b)) == 0x0f;
#else
		return x == q.x && y == q.y && z == q.z && w == q.w;
#endif
	}

	FORCEINLINE Quaternion::Quaternion(const Rotator& r)
	{
		*this = r.quaternion();
	}



	FORCEINLINE Quaternion::Quaternion(EForceInit)
		:x(0), y(0), z(0), w(0)
	{

	}

	FORCEINLINE Quaternion::Quaternion(float inX, float inY, float inZ, float inW)
		: x(inX), y(inY), z(inZ), w(inW)
	{

	}

	FORCEINLINE Quaternion::Quaternion(const Quaternion& rhs)
		: x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w)
	{
	}

	FORCEINLINE Quaternion Quaternion::inverse() const
	{
		return Quaternion(-x, -y, -z, w);
	}

	FORCEINLINE float3 Quaternion::rotateVector(float3 v) const
	{
		const float3 q(x, y, z);
		const float3 t = 2.0f * float3::crossProduct(q, v);
		const float3 result = v + (w * t) + float3::crossProduct(q, t);
		return result;
	}

	FORCEINLINE bool Quaternion::equals(const Quaternion& q, float tolerance) const
	{
#if PLATFORM_ENABLE_VECTORINTRINSICS
		const VectorRegister toleranceV = VectorLoadFloat1(&tolerance);
		const VectorRegister A = VectorLoadAligned(this);
		const VectorRegister B = VectorLoadAligned(&q);

		const VectorRegister rotationSub = VectorAbs(VectorSubtract(A, B));
		const VectorRegister rotationAdd = VectorAbs(VectorAdd(A, B));
		return !VectorAnyGreaterThan(rotationSub, toleranceV) || !VectorAnyGreaterThan(rotationAdd, toleranceV);
#endif
	}

}