#pragma once
#include "Math/Vector.h"
namespace Air
{
	struct BoxSphereBounds
	{
		float3 mOrigin;
		float3 mBoxExtent;
		float mSphereRadius;

		BoxSphereBounds() {}

		explicit FORCEINLINE BoxSphereBounds(EForceInit)
			:mOrigin(ForceInit)
			, mBoxExtent(ForceInit)
			, mSphereRadius(0.f)
		{
			diagnosticCheckNan();
		}

#if 0
#else
		FORCEINLINE void diagnosticCheckNan()const {}
#endif
	};
}