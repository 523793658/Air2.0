#pragma once
#include "CoreType.h"
#include "Vector.h"
#include "Math/Plane.h"
#include "HAL/AirMemory.h"
#include "Math/Axis.h"
#include "Math/Rotator.h"
namespace Air
{

	struct Matrix
	{
	public:
		union
		{
			MS_ALIGN(16) float M[4][4] GCC_ALIGN(16);
		};
		MS_ALIGN(16) static CORE_API const Matrix Identity GCC_ALIGN(16);

		FORCEINLINE Matrix();

		FORCEINLINE Matrix(float v[16]);

		FORCEINLINE Matrix(
			float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23,
			float m30, float m31, float m32, float m33
		);

		explicit FORCEINLINE Matrix(EForceInit)
		{
			Memory::memzero(this, sizeof(*this));
		}

		FORCEINLINE Matrix(const Plane& x, const Plane& y, const Plane& z, const Plane& w);

		FORCEINLINE float4 transformVector4(const float4 &v) const;

		FORCEINLINE float4 transformPosition(const float3& v) const;

		FORCEINLINE float4 transformVector(const float3& v) const;

		inline float3 getColomn(int32 i) const
		{
			return float3(M[0][i], M[1][i], M[2][i]);
		}

		FORCEINLINE void operator *= (float other);

		FORCEINLINE Matrix operator * (const Matrix& rhs) const;

		FORCEINLINE bool getFrustumFarPlane(Plane& outPlane) const;

		FORCEINLINE bool getFrustumNearPlane(Plane& outPlane) const;

		FORCEINLINE bool getFrustumLeftPlane(Plane& outPlane) const;

		FORCEINLINE bool getFrustumRightPlane(Plane& outPlane) const;

		FORCEINLINE bool getFrustumTopPlane(Plane& outPlane) const;

		FORCEINLINE bool getFrustumBottomPlane(Plane& outPlane) const;

		FORCEINLINE float3 inverseTransformPosition(const float3& v) const;

		FORCEINLINE Matrix getTransposed() const;

		inline float3 getOrigin() const;

		inline float determinant() const;

		inline Matrix inverseFast() const;

		inline Matrix inverse() const;

		inline float3 getScaleAxis(EAxis::Type inAxis) const;

		inline void setIdentity();

		inline float rotDeterminant() const;

		inline float3 getUnitAxis(EAxis::Type inAxis) const;

		inline Matrix removeTranslation() const;

		inline void removeScaling(float tolerance = SMALL_NUMBER);

		inline void setAxis(int32 i, const float3& axis)
		{
			BOOST_ASSERT(i >= 0 && i <= 2);
			M[i][0] = axis.x;
			M[i][1] = axis.y;
			M[i][2] = axis.z;

		}

		inline void setOrigin(const float3& newOrigin)
		{
			M[3][0] = newOrigin.x;
			M[3][1] = newOrigin.y;
			M[3][2] = newOrigin.z;
		}
	
	};

	class TranslationMatrix
		: public Matrix
	{
	public:
		TranslationMatrix(const float3& delta);

		static Matrix make(float3 const & delta)
		{
			return TranslationMatrix(delta);
		}
	};




	FORCEINLINE TranslationMatrix::TranslationMatrix(const float3& delta)
		:Matrix(
			Plane(1.0f, 0.0f, 0.0f, 0.0f),
			Plane(0.0f, 1.0f, 0.0f, 0.0f),
			Plane(0.0f, 0.0f, 1.0f, 0.0f),
			Plane(delta.x, delta.y, delta.z, 1.0f)
		)
	{
	}


	class InverseRotationMatrix : public Matrix
	{
	public:
		FORCEINLINE InverseRotationMatrix(const Rotator& rot)
			:Matrix(
				Matrix(
					Plane(Math::cos(rot.mRoll * ANGLE2RADIAN), -Math::sin(rot.mRoll* ANGLE2RADIAN), 0.0f, 0.0f),
					Plane(Math::sin(rot.mRoll * ANGLE2RADIAN) , Math::cos(rot.mRoll * ANGLE2RADIAN), 0.0f, 0.0f),
					Plane(0.0f, 0.0f, 1.0f, 0.0f),
					Plane(0.0f, 0.0f, 0.0f, 1.0f)) *
				Matrix(
					Plane(Math::cos(rot.mYaw * ANGLE2RADIAN), 0.0f, Math::sin(rot.mYaw* ANGLE2RADIAN), 0.0f),
					Plane(0.0f, 1.0f, 0.0f, 0.0f),
					Plane(-Math::sin(rot.mYaw * ANGLE2RADIAN), 0.0f, Math::cos(rot.mYaw * ANGLE2RADIAN), 0.0f),
					Plane(0.0f, 0.0f, 0.0f, 1.0f))*
				Matrix(
					Plane(1.0f, 0.0f, 0.0f, 0.0f),
					Plane(0.0f, Math::cos(rot.mPitch* ANGLE2RADIAN) , -Math::sin(rot.mPitch * ANGLE2RADIAN), 0.0f),
					Plane(0.0f, Math::sin(rot.mPitch* ANGLE2RADIAN), Math::cos(rot.mPitch * ANGLE2RADIAN), 0.0f),
					Plane(0.0f, 0.0f, 0.0f, 1.0f)))
		{

		}
	};

	class ReversedZOrthoMatrix : public Matrix
	{
	public:
		ReversedZOrthoMatrix(float width, float height, float zScale, float zOffset)
			:Matrix(
				Plane((width) ? (1.0f / width) : 1.0f, 0.0f, 0.0f, 0.0f),
				Plane(0.0f, (height) ? (1.0f / height) : 1.0f, 0.0f, 0.0f),
				Plane(0.0f, 0.0f, -zScale, 0.0f),
				Plane(0.0f, 0.0f, 1.0f - zOffset * zScale, 1.0f)
			)
		{

		}
	};

	class ReversedZPerspectiveMatrix : public Matrix
	{
	public:
		ReversedZPerspectiveMatrix(float halfFOVX, float halfFOVY, float multFOVX, float MultFOVY, float minZ, float maxZ);

		ReversedZPerspectiveMatrix(float halfFOV, float width, float height, float minZ, float maxZ);

		ReversedZPerspectiveMatrix(float halfFOV, float width, float height, float minz);
	};

	class PerspectiveMatrix : public Matrix
	{
	public:
#define Z_PRECISION 0.0f
		PerspectiveMatrix(float halfFOVX, float halfFOVY, float multFOVX, float multFOVY, float minZ, float maxZ);

		PerspectiveMatrix(float halfFOV, float width, float height, float minZ, float maxZ);

		PerspectiveMatrix(float halfFOV, float width, float height, float minZ);
	};


	//已修改
	class RotationTranslationMatrix : public Matrix
	{
	public:
		RotationTranslationMatrix(const Rotator& rot, const float3& origin);

		static Matrix make(const Rotator& rot, const float3& origin)
		{
			return RotationTranslationMatrix(rot, origin);
		}
	};


	//已修改
	class RotationMatrix : public RotationTranslationMatrix
	{
	public:
		FORCEINLINE RotationMatrix(float pitch, float yaw, float roll);

		FORCEINLINE RotationMatrix(const Rotator& rot);
	};

	FORCEINLINE RotationTranslationMatrix::RotationTranslationMatrix(const Rotator& rot, const float3& origin)
	{
#if PLATFORM_ENABLE_VECTORINTRINSICS
		const VectorRegister angles = MakeVectorRegister(rot.mPitch, rot.mYaw, rot.mRoll, 0.0f);
		const VectorRegister radius = VectorMultiply(angles, GlobalVectorConstants::DEG_TO_RAD);
		union
		{
			VectorRegister v; float f[4];
		}SinAngles, CosAngles;
		VectorSinCos(&SinAngles.v, &CosAngles.v, &radius);
		float sp = SinAngles.f[0];
		float sy = SinAngles.f[1];
		float sr = SinAngles.f[2];

		float cp = CosAngles.f[0];
		float cy = CosAngles.f[1];
		float cr = CosAngles.f[2];
#else
		float sp, sy, sr;
		float cp, cy, cr;

		Math::sinCos(&sp, &cp, Math::degreesToRadians(rot.mPitch));
		Math::sinCos(&sy, &cy, Math::degreesToRadians(rot.mYaw));
		Math::sinCos(&sr, &cr, Math::degreesToRadians(rot.mRoll));
#endif

		M[0][0] = cy * cr;
		M[0][1] = cy * sr;
		M[0][2] = -sy;
		M[0][3] = 0.0f;
		M[1][0] = sp * sy * cr - cp * sr;
		M[1][1] = sp * sy * sr + cp * cr;
		M[1][2] = sp * cy;
		M[1][3] = 0.f;
		M[2][0] = cp * sy * cr + sp * sr;
		M[2][1] = cp * sy * sr - sp * cr;
		M[2][2] = cp * cy;
		M[2][3] = 0.f;
		M[3][0] = origin.x;
		M[3][1] = origin.y;
		M[3][2] = origin.z;
		M[3][3] = 1.0f;
	}
}
#include "Math/Matrix.inl"
