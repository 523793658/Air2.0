#pragma once
#include "CoreMinimal.h"
#include "Math/TransformCalculus2D.h"
namespace Air
{
	class SlateLayoutTransform;

	

	class SlateLayoutTransform
	{
	public:
		explicit SlateLayoutTransform(float inScale = 1.0f, const float2& inTranslation = float2(ForceInit))
			:mScale(inScale)
			,mTranlation(inTranslation)
		{
		}
		explicit SlateLayoutTransform(const float2& inTranslation)
			:mScale(1.0f)
			, mTranlation(inTranslation)
		{}

		const float2& getTranslation() const
		{
			return mTranlation;
		}

		float getScale() const
		{
			return mScale;
		}

		Matrix toMatrix() const
		{
			Matrix matrix = ScaleMatrix(getScale());
			matrix.setOrigin(getTranslation());
			return matrix;
		}

		float2 transformPoint(const float2& point) const
		{
			return Air::transformPoint(mTranlation, Air::transformPoint(mScale, point));
		}

		float2 transformVector(const float2& vector) const
		{
			return Air::transformVector(mTranlation, Air::transformVector(mScale, vector));
		}

		SlateLayoutTransform concatenate(const SlateLayoutTransform& rhs) const
		{
			return SlateLayoutTransform(Air::concatenate(mScale, rhs.mScale), rhs.transformPoint(mTranlation));
		}

		SlateLayoutTransform inverse() const
		{
			return SlateLayoutTransform(Air::inverse(mScale), Air::inverse(mTranlation) * Air::inverse(mScale));
		}

		bool operator==(const SlateLayoutTransform& other) const
		{
			return mScale == other.mScale && mTranlation == other.mTranlation;
		}

		bool operator !=(const SlateLayoutTransform& other) const
		{
			return mScale != other.mScale || mTranlation != other.mTranlation;
		}
	private:
		float mScale;
		float2 mTranlation;
	};
	inline SlateLayoutTransform concatenate(float scale, const float2& translation)
	{
		return SlateLayoutTransform(scale, translation);
	}

	inline SlateLayoutTransform concatenate(const float2& translation, float scale)
	{
		return SlateLayoutTransform(scale, translation);
	}

	template<> struct ConcatenateRules<SlateLayoutTransform, float> { typedef SlateLayoutTransform ResultType; };
	template<> struct ConcatenateRules<SlateLayoutTransform, float2> { typedef SlateLayoutTransform ResultType; };
	template<> struct ConcatenateRules<SlateLayoutTransform, Matrix> { typedef Matrix ResultType; };
	template<> struct ConcatenateRules<SlateLayoutTransform, Scale2D> { typedef Transform2D ResultType; };
	template<> struct ConcatenateRules<SlateLayoutTransform, Shear2D> { typedef Transform2D ResultType; };
	template<> struct ConcatenateRules<SlateLayoutTransform, Quat2D> { typedef Transform2D ResultType; };
	template<> struct ConcatenateRules<SlateLayoutTransform, Matrix2x2> { typedef Transform2D ResultType; };

	template<> struct ConcatenateRules<float, SlateLayoutTransform> { typedef SlateLayoutTransform ResultType; };
	template<> struct ConcatenateRules<float2, SlateLayoutTransform> { typedef SlateLayoutTransform ResultType; };
	template<> struct ConcatenateRules<Matrix, SlateLayoutTransform> { typedef Matrix ResultType; };
	template<> struct ConcatenateRules<Scale2D, SlateLayoutTransform> { typedef Transform2D ResultType; };
	template<> struct ConcatenateRules<Shear2D, SlateLayoutTransform> { typedef Transform2D ResultType; };
	template<> struct ConcatenateRules<Quat2D, SlateLayoutTransform> { typedef Transform2D ResultType; };
	template<> struct ConcatenateRules<Matrix2x2, SlateLayoutTransform> { typedef Transform2D ResultType; };


	template<> template<> inline Transform2D TransformConverter<Transform2D>::convert<SlateLayoutTransform>(const SlateLayoutTransform& transform)
	{
		return Transform2D(Scale2D(transform.getScale()), transform.getTranslation());
	}
}