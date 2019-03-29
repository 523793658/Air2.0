#pragma once
#include "EngineMininal.h"
#include <array>
namespace Air
{
	struct OutCode
	{
	private:
		uint32 mInside : 1;
		uint32 mOutside : 1;
	public:
		OutCode()
			:mInside(0), mOutside(0)
		{}

		OutCode(bool inInside, bool inOutside)
			:mInside(inInside), mOutside(inOutside)
		{}

		FORCEINLINE void setInside(bool newInside) { mInside = newInside; }

		FORCEINLINE void setOutside(bool newOutside) { mOutside = newOutside; }

		FORCEINLINE bool getInside() const { return mInside; }
		FORCEINLINE bool getOutside() const { return mOutside; }
	};


	struct ENGINE_API ConvexVolume
	{
	public:
		typedef TArray<Plane> PlaneArray;

		PlaneArray mPlanes;
		PlaneArray mPermutedPlanes;
		ConvexVolume()
		{

		}

		ConvexVolume(const TArray<Plane> &inPlanes)
			:mPlanes(inPlanes)
		{
			init();
		}

		~ConvexVolume()
		{
			mPlanes.clear();
			mPermutedPlanes.clear();
		}

		void init();

		//bool clipPolygon(class )
		OutCode getBoxIntersectionOutcode(const float3& origin, const float3& extent) const;
		
	};



	extern ENGINE_API void getViewFrustumBounds(ConvexVolume& outResult, const Matrix& viewProjectMatrix, bool bUseNearPlane);

	extern ENGINE_API void getViewFrustumBounds(ConvexVolume& outResult, const Matrix& viewProjectionMatrix, const Plane& inFarPlane, bool boverredeFarPlane, bool bUseNearPlane);
}