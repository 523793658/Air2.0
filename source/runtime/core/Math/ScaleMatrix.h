#pragma once
#include "Math/Matrix.hpp"
namespace Air
{
	struct ScaleMatrix
		: public Matrix
	{
	public:
		FORCEINLINE ScaleMatrix(float scale)
			:Matrix(
				Plane(scale, 0.0f, 0.0f, 0.0f),
				Plane(0.0f, scale, 0.0f, 0.0f),
				Plane(0.0f, 0.0f, scale, 0.0f),
				Plane(0.0f, 0.0f, 0.0f, 1.0f)
			)
		{

		}

		FORCEINLINE ScaleMatrix(const float3& scale)
			:Matrix(
				Plane(scale.x, 0.0f, 0.0f, 0.0f),
				Plane(0.0f, scale.y, 0.0f, 0.0f),
				Plane(0.0f, 0.0f, scale.z, 0.0f),
				Plane(0.0f, 0.0f, 0.0f, 1.0f)
			)
		{

		}

		static Matrix make(float scale)
		{
			return ScaleMatrix(scale);
		}

		static Matrix make(const float3& scale)
		{
			return ScaleMatrix(scale);
		}
	};


}