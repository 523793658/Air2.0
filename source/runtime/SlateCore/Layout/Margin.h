#pragma once

#include "CoreType.h"
#include "Math/Math.h"
#include "Type/SlateEnums.h"
namespace Air
{


	struct Margin 
	{
		float mLeft{ 0.0f };
		float mTop{ 0.0f };
		float mRight{ 0.0f };
		float mBottom{ 0.0f };

	public:
		Margin()
		{}

		Margin(float uniformMargin)
			:mLeft(uniformMargin)
			,mTop(uniformMargin)
			,mRight(uniformMargin)
			,mBottom(uniformMargin)
		{}
		Margin(float Horizontal, float Vertical)
			:mLeft(Horizontal)
			, mTop(Vertical)
			, mRight(Horizontal)
			, mBottom(Vertical)
		{}

		Margin(float inleft, float intop, float inright, float inButtom)
			:mLeft(inleft)
			, mTop(intop)
			, mRight(inright)
			, mBottom(inButtom)
		{}

	public:
		Margin operator* (float scale) const
		{
			return Margin(mLeft * scale, mTop * scale, mRight * scale, mBottom * scale);
		}

		Margin operator* (const Margin& inScale) const
		{
			return Margin(mLeft * inScale.mLeft, mTop * inScale.mTop, mRight * inScale.mRight, mBottom * inScale.mBottom);
		}

		Margin operator + (const Margin & inDelta) const
		{
			return Margin(mLeft + inDelta.mLeft, mTop + inDelta.mTop, mRight + inDelta.mRight, mBottom + inDelta.mBottom);
		}

		Margin operator - (const Margin& other) const
		{
			return Margin(mLeft - other.mLeft, mTop - other.mTop, mRight - other.mRight, mBottom - other.mBottom);
		}

		bool operator == (const Margin & other) const
		{
			return (mLeft == other.mLeft) && (mRight == other.mRight) && (mTop == other.mTop) && (mBottom == other.mBottom);
		}

		bool operator != (const Margin & other) const
		{
			return mLeft != other.mLeft || mRight != other.mRight || mTop != other.mTop || mBottom != other.mBottom;
		}

	public:
		float2 getDesiredSize() const
		{
			return float2(mLeft + mRight, mTop + mBottom);
		}

		template<EOrientation orientation>
		float getTotalSpaceAlong() const
		{
			return 0.0f;
		}
	};

	template<>
	inline float Margin::getTotalSpaceAlong<Orient_Horizontal>() const
	{
		return mLeft + mRight;
	}

	template<>
	inline float Margin::getTotalSpaceAlong<Orient_Vertical>() const
	{
		return mTop + mBottom;
	}
}