#pragma once
#include "CoreType.h"
#include "Math/Matrix.hpp"
#include "Math/Vector.h"
#include "Math/TransformVectorized.h"
#include "Serialization/Archive.h"
namespace Air
{
	class Sphere
	{
	public:
		float3 mCenter;
		float W;
	public:
		Sphere() {}
		Sphere(int32)
			:mCenter(0.0f, 0.0f, 0.0f)
			,W(0)
		{}

		Sphere(float3 inV, float inW)
			:mCenter(inV)
			,W(inW)
		{}

		explicit FORCEINLINE Sphere(EForceInit)
			:mCenter(ForceInit)
			,W(0.0f)
		{}
		CORE_API Sphere(const float3* pts, int32 count);

	public:
		bool equals(const Sphere& sphere, float tolerance = KINDA_SMALL_NUMBER) const
		{
			return mCenter.equals(sphere.mCenter, tolerance) && Math::abs(W - sphere.W) < tolerance;
		}

		bool isInside(const Sphere& other, float tolerance = KINDA_SMALL_NUMBER) const
		{
			if (W > other.W + tolerance)
			{
				return false;
			}
			return (mCenter - other.mCenter).sizeSquared() <= Math::square(other.W + tolerance - W);
		}

		bool isInside(const float3& in, float tolerance = KINDA_SMALL_NUMBER) const
		{
			return (mCenter - in).sizeSquared() <= Math::square(W + tolerance);
		}

		FORCEINLINE bool intersects(const Sphere& other, float tolerance = KINDA_SMALL_NUMBER) const
		{
			return (mCenter - other.mCenter).sizeSquared() <= Math::square(Math::max(0.f, other.W + W + tolerance));
		}

		CORE_API Sphere transformBy(const Matrix& m) const;

		CORE_API Sphere transformBy(const Transform& m) const;

		CORE_API float getVolume() const;

		CORE_API Sphere& operator +=(const Sphere& other);

		Sphere operator + (const Sphere& other) const
		{
			return Sphere(*this) += other;
		}

	public:
		friend Archive& operator <<(Archive& ar, Sphere & sphere)
		{
			ar << sphere.mCenter << sphere.W;
			return ar;
		}
	};
}