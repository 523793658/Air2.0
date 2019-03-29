#pragma once
#include "CoreType.h"
#include "Vector.h"
namespace Air
{
	struct Box
	{
	public:
		float3 mMin;
		float3 mMax;
		uint8 isValid;

		Box(const float3& inMin, const float3 & inMax)
			:mMin(inMin)
			,mMax(inMax)
			,isValid(1)
		{}

		Box() 
		{
		}


		Box(int32)
		{
			init();
		}

		explicit Box(EForceInit)
		{
			init();
		}

		CORE_API Box(const float3* points, int32 count);

		CORE_API Box(const TArray<float3>& points);

		FORCEINLINE bool operator == (const Box& other) const
		{
			return (mMin == other.mMin) && (mMax == other.mMax);
		}

		FORCEINLINE Box& operator +=(const float3& other)
		{
			if (isValid)
			{
				mMin.x = Math::min(mMin.x, other.x);
				mMin.y = Math::min(mMin.y, other.y);
				mMin.z = Math::min(mMin.z, other.z);

				mMax.x = Math::max(mMax.x, other.x);
				mMax.y = Math::max(mMax.y, other.y);
				mMax.z = Math::max(mMax.z, other.z);

			}
			else
			{
				mMin = mMax = other;
				isValid = 1;
			}
			return *this;
		}

		FORCEINLINE Box operator +(const float3& other) const
		{
			return Box(*this) += other;
		}

		FORCEINLINE Box operator +=(const Box& other)
		{
			if (isValid)
			{
				mMin.x = Math::min(mMin.x, other.mMin.x);
				mMin.y = Math::min(mMin.y, other.mMin.y);
				mMin.z = Math::min(mMin.z, other.mMin.z);

				mMax.x = Math::max(mMax.x, other.mMax.x);
				mMax.y = Math::max(mMax.y, other.mMax.y);
				mMax.z = Math::max(mMax.z, other.mMax.z);
			}
			else if(other.isValid)
			{
				*this = other;
				
			}
			return *this;
		}

		FORCEINLINE Box operator + (const Box& other) const
		{
			return Box(*this) += other;
		}

		FORCEINLINE float3& operator[](int32 index)
		{
			BOOST_ASSERT((index >= 0) && (index < 2));
			if (index == 0)
			{
				return mMin;
			}
			return mMax;
		}



		FORCEINLINE float3 getExtent() const
		{
			return 0.5f * (mMax - mMin);
		}

		FORCEINLINE void getCenterAndExtents(float3& center, float3& extents) const
		{
			extents = getExtent();
			center = 0.5f *(mMin + mMax);
		}

		FORCEINLINE void init()
		{
			mMin = mMax = float3::Zero;
			isValid = 0;
		}
	};
}