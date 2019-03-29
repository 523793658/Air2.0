#pragma once
#include "CoreType.h"
#include "Math/Vector.h"

namespace Air
{
	struct Matrix;


	MS_ALIGN(16) struct Plane
		: public float3
	{
	public:
		float w;

	public:
		FORCEINLINE Plane() {}

		FORCEINLINE Plane(const Plane& p)
			:float3(p),
			w(p.w)
		{}

		FORCEINLINE Plane(const float4& v)
			:float3(v),
			w(v.w)
		{}

		FORCEINLINE Plane(float inX, float inY, float inZ, float inW)
			:float3(inX, inY, inZ),
			w(inW)
		{}

		FORCEINLINE Plane(const float3 normal, float d)
			:float3(normal),
			w(d)
		{}

		FORCEINLINE Plane(float3 base, const float3 &normal)
			:float3(normal),w( base | normal)
		{

		}

		FORCEINLINE Plane(float3 a, float3 b, float3 c)
			:float3(((b - a) ^(c -a )).getSafeNormal())
		{
			w = a | (Vector3)(*this);
		}

		FORCEINLINE Plane(EForceInit)
			:Vector3(ForceInit), w(0.f)
		{}

		FORCEINLINE float dot(float3 &p) const
		{
			return x * p.x + y * p.y + z * p.z - w;
		}

		FORCEINLINE Plane flip() const
		{
			return Plane(-x, -y, -z, -w);
		}
	}GCC_ALIGN(16);


	
}