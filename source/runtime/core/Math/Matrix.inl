#pragma once
#include "CoreType.h"
#include "CoreFwd.h"
namespace Air
{

	FORCEINLINE Matrix::Matrix()
	{}

	FORCEINLINE Matrix::Matrix(float v[16])
	{
		Memory::memcpy(M, v, sizeof(float) * 16);
	}

	FORCEINLINE Matrix::Matrix(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
	{
		M[0][0] = m00; M[0][1] = m01; M[0][2] = m02; M[0][3] = m03;
		M[1][0] = m10; M[1][1] = m11; M[1][2] = m12; M[1][3] = m13;
		M[2][0] = m20; M[2][1] = m21; M[2][2] = m22; M[2][3] = m23;
		M[3][0] = m30; M[3][1] = m31; M[3][2] = m32; M[3][3] = m33;
	}

	FORCEINLINE bool makeFrustumPlane(float a, float b, float c, float d, Plane& outPlane)
	{
		const float LengthSquared = a * a + b * b + c * c;
		if (LengthSquared > DELTA * DELTA)
		{
			const float invLength = Math::InvSqrt(LengthSquared);
			outPlane = Plane(-a * invLength, -b * invLength, -c * invLength, d * invLength);
			return 1;
		}
		else
		{
			return 0;
		}
	}

	FORCEINLINE bool Matrix::getFrustumNearPlane(Plane& outPlane) const
	{
		return makeFrustumPlane(M[0][2], M[1][2], M[2][2], M[3][2], outPlane);
	}

	FORCEINLINE bool Matrix::getFrustumFarPlane(Plane& outPlane) const
	{
		return makeFrustumPlane(
			M[0][3] - M[0][2],
			M[1][3] - M[1][2],
			M[2][3] - M[2][2],
			M[3][3] - M[3][2],
			outPlane
			);
	}

	FORCEINLINE bool Matrix::getFrustumLeftPlane(Plane& outPlane) const
	{
		return makeFrustumPlane(
			M[0][3] + M[0][0],
			M[1][3] + M[1][0],
			M[2][3] + M[2][0],
			M[3][3] + M[3][0],
			outPlane
			);
	}

	FORCEINLINE bool Matrix::getFrustumRightPlane(Plane& outPlane) const
	{
		return makeFrustumPlane(
			M[0][3] - M[0][0],
			M[1][3] - M[1][0],
			M[2][3] - M[2][0],
			M[3][3] - M[3][0],
			outPlane
		);
	}

	FORCEINLINE bool Matrix::getFrustumTopPlane(Plane& outPlane) const
	{
		return makeFrustumPlane(
			M[0][3] - M[0][1],
			M[1][3] - M[1][1],
			M[2][3] - M[2][1],
			M[3][3] - M[3][1],
			outPlane
		);
	}

	FORCEINLINE bool Matrix::getFrustumBottomPlane(Plane& outPlane)const
	{
		return makeFrustumPlane(
			M[0][3] + M[0][1],
			M[1][3] + M[1][1],
			M[2][3] + M[2][1],
			M[3][3] + M[3][1],
			outPlane
		);
	}

	FORCEINLINE float4 Matrix::transformVector4(const float4 &v) const
	{
		float4 result;
		VectorRegister vecP = VectorLoadAligned(&v);
		VectorRegister vecR = vectorTransformVector(vecP, this);
		VectorStoreAligned(vecR, &result);
		return result;
	}

	FORCEINLINE float4 Matrix::transformPosition(const float3& v) const
	{
		return transformVector4(float4(v.x, v.y, v.z, 1.0f));
	}

	FORCEINLINE float3 Matrix::inverseTransformPosition(const float3& v) const
	{
		Matrix invSelf = this->inverseFast();
		return invSelf.transformPosition(v);
	}

	FORCEINLINE float4 Matrix::transformVector(const float3& v) const
	{
		return transformVector4(float4(v.x, v.y, v.z, 0.0f));
	}

	inline float3 Matrix::getScaleAxis(EAxis::Type inAxis) const
	{
		switch (inAxis)
		{
		case Air::EAxis::X:
			return float3(M[0][0], M[0][1], M[0][2]);
		case Air::EAxis::Y:
			return float3(M[1][0], M[1][1], M[1][2]);
		case Air::EAxis::Z:
			return float3(M[2][0], M[2][1], M[2][2]);
		default:
			return float3::Zero;
		}
	}

	inline Matrix Matrix::inverseFast() const
	{
		Matrix result;
		vectorMatrixInverse(&result, this);
		return result;
	}

	inline float Matrix::determinant() const
	{
		return M[0][0] * (
			M[1][1] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
			M[2][1] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) +
			M[3][1] * (M[1][2] * M[2][3] - M[1][3] * M[2][2])
			) -
			M[1][0] * (
				M[0][1] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
				M[2][1] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
				M[3][1] * (M[0][2] * M[2][3] - M[0][3] * M[2][2])
				) +
			M[2][0] * (
				M[0][1] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) -
				M[1][1] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
				M[3][1] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
				) -
			M[3][0] * (
				M[0][1] * (M[1][2] * M[2][3] - M[1][3] * M[2][2]) -
				M[1][1] * (M[0][2] * M[2][3] - M[0][3] * M[2][2]) +
				M[2][1] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
				);
	}

	inline float3 Matrix::getOrigin() const
	{
		return float3(M[3][0], M[3][1], M[3][2]);
	}

	inline void Matrix::setIdentity()
	{
		Memory::memcpy(M, Identity.M, sizeof(float) * 16);
	}

	inline Matrix Matrix::removeTranslation() const
	{
		Matrix result = *this;
		result.M[3][0] = 0.0f;
		result.M[3][1] = 0.0f;
		result.M[3][2] = 0.0f;
		return result;
	}

	inline void Matrix::removeScaling(float tolerance /* = SMALL_NUMBER */)
	{
		const float squareSum0 = (M[0][0] * M[0][0] + M[0][1] * M[0][1] + M[0][2] * M[0][2]);
		const float squareSum1 = (M[1][0] * M[1][0] + M[1][1] * M[1][1] + M[1][2] * M[1][2]);
		const float squareSum2 = (M[2][0] * M[2][0] + M[2][1] * M[2][1] + M[2][2] * M[2][2]);
		const float scale0 = Math::floatSelect(squareSum0 - tolerance, Math::InvSqrt(squareSum0), 1.0f);
		const float scale1 = Math::floatSelect(squareSum1 - tolerance, Math::InvSqrt(squareSum1), 1.0f);
		const float scale2 = Math::floatSelect(squareSum2 - tolerance, Math::InvSqrt(squareSum2), 1.0f);
		M[0][0] *= scale0;
		M[0][1] *= scale0;
		M[0][2] *= scale0;
		M[1][0] *= scale1;
		M[1][1] *= scale1;
		M[1][2] *= scale1;
		M[2][0] *= scale2;
		M[2][1] *= scale2;
		M[2][2] *= scale2;

	}

	inline Matrix Matrix::inverse() const
	{
		Matrix result;
		if (getScaleAxis(EAxis::X).isNearlyZero(SMALL_NUMBER) &&
			getScaleAxis(EAxis::Y).isNearlyZero(SMALL_NUMBER) &&
			getScaleAxis(EAxis::Z).isNearlyZero(SMALL_NUMBER))
		{
			result = Matrix::Identity;
		}
		else
		{
			const float det = determinant();
			if (det == 0.0f)
			{
				result = Matrix::Identity;
			}
			else
			{
				vectorMatrixInverse(&result, this);
			}
		}
		return result;
	}

	FORCEINLINE Matrix Matrix::getTransposed() const
	{
		Matrix Result;
		Result.M[0][0] = M[0][0];
		Result.M[0][1] = M[1][0];
		Result.M[0][2] = M[2][0];
		Result.M[0][3] = M[3][0];

		Result.M[1][0] = M[0][1];
		Result.M[1][1] = M[1][1];
		Result.M[1][2] = M[2][1];
		Result.M[1][3] = M[3][1];

		Result.M[2][0] = M[0][2];
		Result.M[2][1] = M[1][2];
		Result.M[2][2] = M[2][2];
		Result.M[2][3] = M[3][2];

		Result.M[3][0] = M[0][3];
		Result.M[3][1] = M[1][3];
		Result.M[3][2] = M[2][3];
		Result.M[3][3] = M[3][3];

		return Result;
	}

	FORCEINLINE Matrix::Matrix(const Plane& x, const Plane& y, const Plane& z, const Plane& w)
	{
		M[0][0] = x.x;	M[0][1] = x.y; M[0][2] = x.z; M[0][3] = x.w;
		M[1][0] = y.x;	M[1][1] = y.y; M[1][2] = y.z; M[1][3] = y.w;
		M[2][0] = z.x;	M[2][1] = z.y; M[2][2] = z.z; M[2][3] = z.w;
		M[3][0] = w.x;	M[3][1] = w.y; M[3][2] = w.z; M[3][3] = w.w;
	}

	FORCEINLINE Matrix Matrix::operator *(const Matrix& other) const
	{
		Matrix result;
		vectorMatrixMultiply(&result, this, &other);
		return result;
	}

	inline float Matrix::rotDeterminant() const
	{
		return M[0][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1]) - M[1][0] * (M[0][1] * M[2][2] - M[0][2] * M[2][1]) +
			M[2][0] * (M[0][1] * M[1][2] - M[0][2] * M[1][1]);
	}

	inline float3 Matrix::getUnitAxis(EAxis::Type inAxis) const
	{
		return getScaleAxis(inAxis).getSafeNormal();
	}

	FORCEINLINE ReversedZPerspectiveMatrix::ReversedZPerspectiveMatrix(float halfFOVX, float halfFOVY, float multFOVX, float MultFOVY, float minZ, float maxZ)
		:Matrix(
			Plane(multFOVX / Math::tan(halfFOVX), 0.0f, 0.0f, 0.0f),
			Plane(0.0f, MultFOVY / Math::tan(halfFOVY) , 0.0f, 0.0f),
			Plane(0.0f, 0.0f, ((minZ == maxZ) ? 0.0f : minZ / (minZ - maxZ)), 1.0f),
			Plane(0.0f, ((minZ == maxZ) ? minZ : -maxZ * (minZ / (minZ - maxZ))), 0.0f)
		)
	{

	}

	FORCEINLINE ReversedZPerspectiveMatrix::ReversedZPerspectiveMatrix(float halfFOV, float width, float height, float minZ, float maxZ)
		: Matrix(
			Plane(1.0f / Math::tan(halfFOV), 0.0f, 0.0f, 0.0f),
			Plane(0.0f, width / Math::tan(halfFOV) / height, 0.0f, 0.0f),
			Plane(0.0f, 0.0f, ((minZ == maxZ) ? 0.0f : minZ / (minZ - maxZ)), 1.0f),
			Plane(0.0f, 0.0f, ((minZ == maxZ) ? minZ : -maxZ * minZ / (minZ - maxZ)), 0.0f)
		)
	{

	}

	FORCEINLINE ReversedZPerspectiveMatrix::ReversedZPerspectiveMatrix(float halfFOV, float width, float height, float minZ)
		: Matrix(
			Plane(1.0f / Math::tan(halfFOV), 0.0f, 0.0f, 0.0f),
			Plane(0.0f, width / Math::tan(halfFOV) / height, 0.0f, 0.0f),
			Plane(0.0f, 0.0f, 0.0f, 1.0f),
			Plane(0.0f, 0.0f, minZ, 0.0f)
		)
	{

	}

	FORCEINLINE PerspectiveMatrix::PerspectiveMatrix(float halfFOVX, float halfFOVY, float multFOVX, float multFOVY, float minZ, float maxZ)
		:Matrix(
			Plane(multFOVX / Math::tan(halfFOVX), 0.0f, 0.0f, 0.0f),
			Plane(0.0f, multFOVY / Math::tan(halfFOVY), 0.0f, 0.0f),
			Plane(0.0f, 0.0f, ((minZ == maxZ) ? (1.0f - Z_PRECISION) : maxZ / (maxZ - minZ)), 1.0f),
			Plane(0.0f, 0.0f, -minZ * ((minZ == maxZ) ? (1.0f - Z_PRECISION) : maxZ / (maxZ - minZ)), 0.0f)
		)
	{

	}

	FORCEINLINE PerspectiveMatrix::PerspectiveMatrix(float halfFOV, float width, float height, float minZ, float maxZ)
		:Matrix(
			Plane(1.0f / Math::tan(halfFOV), 0.0f, 0.0f, 0.0f),
			Plane(0.0f, 1.0f / Math::tan(halfFOV), 0.0f, 0.0f),
			Plane(0.0f, 0.0f, ((minZ == maxZ) ? (1.0f - Z_PRECISION) : maxZ / (maxZ - minZ)), 1.0f),
			Plane(0.0f, 0.0f, -minZ *((minZ == maxZ) ? (1.0f - Z_PRECISION) : maxZ / (maxZ - minZ)), 0.0f)
		)
	{

	}

	FORCEINLINE PerspectiveMatrix::PerspectiveMatrix(float halfFOV, float width, float height, float minZ)
		:Matrix(
			Plane(1.0f / Math::tan(halfFOV), 0.0f, 0.0f, 0.0f),
			Plane(0.0f, width / Math::tan(halfFOV) / height, 0.0f, 0.0f),
			Plane(0.0f, 0.0f, (1.0f - Z_PRECISION), 1.0f),
			Plane(0.0f, 0.0f, -minZ * (1.0f - Z_PRECISION), 0.0f)
		)
	{
		
	}

	FORCEINLINE RotationMatrix::RotationMatrix(float pitch, float yaw, float roll)
		:RotationTranslationMatrix(Rotator(pitch, yaw, roll), float3::Zero)
	{

	}

	FORCEINLINE RotationMatrix::RotationMatrix(const Rotator& rot)
		:RotationTranslationMatrix(rot, float3::Zero)
	{}
}
