#pragma once
#include "SlateCore.h"
#include "Rendering/SlateLayoutTransform.h"
#include "Rendering/SlateRenderTransform.h"
#include "Math/TransformCalculus.h"

namespace Air
{
	struct SLATE_CORE_API Geometry
	{
	public:
		Geometry();

		FORCEINLINE_DEBUGGABLE static Geometry makeRoot(const float2& localSize, const SlateLayoutTransform& layoutTransform)
		{
			return Geometry(localSize, layoutTransform, SlateLayoutTransform(), SlateRenderTransform());
		}

		FORCEINLINE const SlateRenderTransform& getAccumulatedRenderTransform() const
		{
			return mAccumulatedRenderTransform;
		}

		FORCEINLINE_DEBUGGABLE float2 absoluteToLocal(float2 absoluteCoordinate) const
		{
			return transformPoint(inverse(getAccumulatedRenderTransform()), absoluteCoordinate);
		}


		bool operator == (const Geometry& other)const
		{
			return
				this->mSize == other.mSize &&
				this->mAbsolutePosition == other.mAbsolutePosition&&
				this->mPosition == other.mPosition&&
				this->mScale == other.mScale;
		}

		bool operator !=(const Geometry& other) const
		{
			return !operator==(other);
		}

		Geometry& operator=(const Geometry& rhs);


	private:


		Geometry(const float2& offsetFromParent, const float2& parentAbsolutePosition, const float2 inLocalSize, float inScale)
			:mSize(inLocalSize)
			, mScale(inScale)
			, mAbsolutePosition(0.0f, 0.0f)
		{
			float2 layoutOffset = inScale * offsetFromParent;
			SlateLayoutTransform parentAccumulatedLayoutTransform(inScale, parentAbsolutePosition);
			SlateLayoutTransform localLayoutTransform(layoutOffset);
			SlateLayoutTransform accumulatedLayoutTransform = concatenate(localLayoutTransform, parentAccumulatedLayoutTransform);
			mAccumulatedRenderTransform = transformCast<SlateRenderTransform>(accumulatedLayoutTransform);
			const_cast<float2&>(mAbsolutePosition) = accumulatedLayoutTransform.getTranslation();
			const_cast<float&>(mScale) = accumulatedLayoutTransform.getScale();
			const_cast<float2&>(mPosition) = localLayoutTransform.getTranslation();
		}

		Geometry(
			const float2& inLocalSize, 
			const SlateLayoutTransform& inLocalLayoutTransform, 
			const SlateLayoutTransform& parentAccumulatedLayoutTranform,
			const SlateRenderTransform& parentAccumulatedRenderTransform)
			:mSize(inLocalSize)
			, mScale(1.0f)
			, mAbsolutePosition(0.0f, 0.0f)
			, mAccumulatedRenderTransform(concatenate(inLocalLayoutTransform, parentAccumulatedRenderTransform))
		{
			SlateLayoutTransform accumulatedLayoutTransform = concatenate(inLocalLayoutTransform, parentAccumulatedLayoutTranform);
			const_cast<float2&>(mAbsolutePosition) = accumulatedLayoutTransform.getTranslation();
			const_cast<float&>(mScale) = accumulatedLayoutTransform.getScale();
			const_cast<float2&>(mPosition) = inLocalLayoutTransform.getTranslation();
		}
	public:
		const float2 mSize;
		const float mScale;
		const float2 mAbsolutePosition;

		const float2 mPosition;


	private:
		SlateRenderTransform mAccumulatedRenderTransform;
	};
}