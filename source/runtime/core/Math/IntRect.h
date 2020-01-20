#pragma once
#include "CoreType.h"
#include "Math/Vector.h"
namespace Air
{
	struct IntRect
	{
		int2	min;
		int2	max;

		FORCEINLINE IntRect()
			:min(ForceInit),
			max(ForceInit)
		{
		}
		FORCEINLINE IntRect(int32 x0, int32 y0, int32 x1, int32 y1)
			:min(x0, y0),
			max(x1, y1)
		{
		}

		FORCEINLINE IntRect(int2 inMin, int2 inMax)
			:min(inMin),
			max(inMax)
		{}

		FORCEINLINE int32 width() const
		{
			return max.x - min.x;
		}

		FORCEINLINE int32 height() const
		{
			return max.y - min.y;
		}

		FORCEINLINE int32 area() const
		{
			return (max.x - min.x) * (max.y - min.y);
		}

		FORCEINLINE int2 size() const
		{
			return int2(max.x - min.x, max.y - min.y);
		}

		FORCEINLINE IntRect& operator -=(const int2& point)
		{
			min -= point;
			max -= point;
			return *this;
		}
	};
}