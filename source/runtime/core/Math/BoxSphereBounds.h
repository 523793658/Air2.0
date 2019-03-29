#pragma once
#include "Math/Vector.h"
namespace Air
{
	struct BoxSphereBounds
	{
		float3 mOrigin;
		float3 mBoxExtent;
		float mSphereRadius;
	};
}