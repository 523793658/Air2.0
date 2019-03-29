#pragma once
#include "Math/Vector.h"
#include "Math/TransformCalculus.h"
#include <type_traits>
namespace Air
{
	inline float inverse(float scale)
	{
		return 1.0f / scale;
	}

	inline float2 inverse(const float2& transform)
	{
		return -transform;
	}

	class Matrix2x2;

	class Scale2D
	{
	public:
		Scale2D() :mScale(1.0f, 1.0f)
		{}
		explicit Scale2D(float inScale) :mScale(inScale, inScale) {}
		explicit Scale2D(float inScaleX, float inScaleY) :mScale(inScaleX, inScaleY)
		{}

		explicit Scale2D(const float2& inScale):mScale(inScale)
		{}

		float2 transformPoint(const float2& point) const
		{
			return mScale * point;
		}

		float2 transformVector(const float2& vector) const
		{
			return transformPoint(vector);
		}

		Scale2D concatenate(const Scale2D& rhs) const
		{
			return Scale2D(mScale * rhs.mScale);
		}

		Scale2D inverse()const
		{
			return Scale2D(1.0f / mScale.x, 1.0f / mScale.y);
		}
		bool operator == (const Scale2D& other) const
		{
			return mScale == other.mScale;
		}

		bool operator !=(const Scale2D& other)const
		{
			return !operator==(other);
		}

		const float2& getVector() const { return mScale; }

	private:
		float2 mScale;
	};

	template<> struct ConcatenateRules<float, Scale2D> { typedef Scale2D ResultType; };
	template<> struct ConcatenateRules<Scale2D, float> { typedef Scale2D ResultType; };

	class Shear2D
	{
	public:
		Shear2D() :mShear(0, 0) {}

		explicit Shear2D(float shearX, float shearY) :mShear(shearX, shearY){}

		explicit Shear2D(const float2& inShear) : mShear(inShear) {}
		
		static Shear2D fromShearAngles(const float2& inShearAngels)
		{
			float shearX = inShearAngels.x == 0 ? 0 : (1.0f / Math::tan(Math::degreesToRadians(90 - Math::clamp(inShearAngels.x, -89.0f, 89.0f))));

			float shearY = inShearAngels.y == 0 ? 0 : (1.0f / Math::tan(Math::degreesToRadians(90 - Math::clamp(inShearAngels.y, -89.0f, 89.0f))));
			return Shear2D(shearX, shearY);
		}

		float2 transformPoint(const float2& point) const
		{
			return point + float2(point.y, point.x) * mShear;
		}

		float2 transformVector(const float2& vector) const
		{
			return transformPoint(vector);
		}

		inline Matrix2x2 concatenate(const Shear2D& rhs) const;

		Matrix2x2 inverse() const;

		bool operator == (const Shear2D& other) const
		{
			return mShear == other.mShear;
		}

		bool operator != (const Shear2D& other) const
		{
			return mShear != other.mShear;
		}

		const float2& getVector() const { return mShear; }
	public:
		float2 mShear;
	};

	class Quat2D
	{
	public:
		Quat2D() :mRot(1.0f, 0.0f) {}
		explicit Quat2D(float rotRadians) :mRot(Math::cos(rotRadians), Math::sin(rotRadians)) {}
		explicit Quat2D(const float2& inRot) :mRot(inRot){}

		float2 transformPoint(const float2& point) const
		{
			return float2(point.x * mRot.x - point.y * mRot.y,
				point.x * mRot.y + point.y * mRot.x);
		}

		float2 transformVector(const float2& vector) const
		{
			return transformPoint(vector);
		}
		Quat2D concatenate(const Quat2D& rhs) const
		{
			return Quat2D(transformPoint(rhs.mRot));
		}

		Quat2D inverse() const
		{
			return Quat2D(float2(mRot.x, -mRot.y));
		}

		bool operator == (const Quat2D& other)const
		{
			return mRot == other.mRot;
		}

		bool operator != (const Quat2D& other) const
		{
			return mRot != other.mRot;
		}

		const float2& getVector() const { return mRot; }
	private:
		float2 mRot;
	};

	class Matrix2x2
	{
	public:
		Matrix2x2()
		{
			M[0][0] = 1; M[0][1] = 0;
			M[1][0] = 0; M[1][1] = 1;
		}

		Matrix2x2(float m00, float m01, float m10, float m11)
		{
			M[0][0] = m00; M[0][1] = m01;
			M[1][0] = m10; M[1][1] = m11;
		}

		explicit Matrix2x2(float uniformScale)
		{
			M[0][0] = uniformScale; M[0][1] = 0;
			M[1][0] = 0;	M[1][1] = uniformScale;
		}

		explicit Matrix2x2(const Scale2D& scale)
		{
			float scaleX = scale.getVector().x;
			float scaleY = scale.getVector().y;
			M[0][0] = scaleX; M[0][1] = 0;
			M[1][0] = 0;	M[1][1] = scaleY;
		}

		explicit Matrix2x2(const Shear2D& shear)
		{
			float xx = shear.getVector().x;
			float yy = shear.getVector().y;
			M[0][0] = 1; M[0][1] = yy;
			M[1][0] = xx; M[1][1] = 1;
		}

		explicit Matrix2x2(const Quat2D& rotation)
		{
			float cosAngle = rotation.getVector().x;
			float sinAngle = rotation.getVector().y;
			M[0][0] = cosAngle, M[0][1] = sinAngle;
			M[1][0] = -sinAngle, M[1][1] = cosAngle;
		}

		float2 transformPoint(const float2& point) const
		{
			return float2(
				point.x * M[0][0] + point.y * M[0][1],
				point.x * M[1][0] + point.y * M[1][1]
			);
		}

		float2 transformVector(const float2& vector) const
		{
			return transformPoint(vector);
		}

		void getMatrix(float& a, float &b, float& c, float& d) const
		{
			a = M[0][0]; b = M[0][1];
			c = M[1][0]; d = M[1][1];
		}

		Matrix2x2 concatenate(const Matrix2x2& rhs) const
		{
			float a, b, c, d;
			getMatrix(a, b, c, d);
			float e, f, g, h;
			rhs.getMatrix(e, f, g, h);
			return Matrix2x2(
				a * e + b * g, a * f + b * h,
				c * e + d * g, c * f + d * h
			);
		}

		float determinant() const
		{
			float a, b, c, d;
			getMatrix(a, b, c, d);
			return (a * d - b * c);
		}

		float inverseDeterminant() const
		{
			float det = determinant();
			BOOST_ASSERT(det != 0.0f);
			return 1.0f / det;
		}

		Matrix2x2 inverse() const
		{
			float a, b, c, d;
			getMatrix(a, b, c, d);
			float invDet = inverseDeterminant();
			return Matrix2x2(d * invDet, -b * invDet, -c * invDet, a * invDet);
		}

		bool operator == (const Matrix2x2& rhs) const
		{
			float a, b, c, d;
			getMatrix(a, b, c, d);
			float e, f, g, h;
			rhs.getMatrix(e, f, g, h);
			return Math::isNearlyEqual(a, e, KINDA_SMALL_NUMBER) &&
				Math::isNearlyEqual(b, f, KINDA_SMALL_NUMBER) &&
				Math::isNearlyEqual(c, g, KINDA_SMALL_NUMBER) &&
				Math::isNearlyEqual(d, h, KINDA_SMALL_NUMBER);
		}
		bool operator != (const Matrix2x2& rhs) const
		{
			return !operator==(rhs);
		}

		Scale2D getScaleSquared() const
		{
			float a, b, c, d;
			getMatrix(a, b, c, d);
			return Scale2D(a * a + b * b, c * c + d * d);
		}

		Scale2D getScale() const
		{
			Scale2D scaleSquared = getScaleSquared();
			return Scale2D(Math::sqrt(scaleSquared.getVector().x), Math::sqrt(scaleSquared.getVector().y));
		}

		bool isIdentity() const
		{
			return M[0][0] == 1.0f && M[0][1] == 0.0f
				&& M[1][0] == 0.0f && M[1][1] == 1.0f;
		}



	private:
		float M[2][2] ;
	};

	inline Matrix2x2 Shear2D::concatenate(const Shear2D& rhs) const
	{
		float xxa = mShear.x;
		float yya = mShear.y;
		float xxb = rhs.mShear.x;
		float yyb = rhs.mShear.y;
		return Matrix2x2(1 + yya * xxb, yyb * yya, xxa + xxb, xxa * xxb + 1);
	}

	inline Matrix2x2 Shear2D::inverse() const
	{
		float invDet = 1.0f / (1.0f - mShear.x * mShear.y);
		return Matrix2x2(invDet, -mShear.y * invDet, -mShear.x * invDet, invDet);
	}

	template<typename TransformType> struct ConcatenateRules<typename std::enable_if<!std::are_types_equal<Matrix2x2, TransformType>::value, Matrix2x2>::type, TransformType> { typedef Matrix2x2 ResultType; };

	template<typename TransformType> struct ConcatenateRules < TransformType, typename std::enable_if<!std::are_types_equal<Matrix2x2, TransformType>::value, Matrix2x2>::type>{ typedef Matrix2x2 ResultType; };

	template<> struct ConcatenateRules<Scale2D, Shear2D> { typedef Matrix2x2 ResultType; };
	template<> struct ConcatenateRules<Scale2D, Quat2D> { typedef Matrix2x2 ResultType; };
	template<> struct ConcatenateRules<Shear2D, Scale2D> { typedef Matrix2x2 ResultType; };
	template<> struct ConcatenateRules<Quat2D, Scale2D> { typedef Matrix2x2 ResultType; };
	template<> struct ConcatenateRules<Shear2D, Quat2D> { typedef Matrix2x2 ResultType; };
	template<> struct ConcatenateRules<Quat2D, Shear2D> { typedef Matrix2x2 ResultType; };

	inline auto transformPoint(const float transform, const float2& point)->decltype(transform * point)
	{
		return transform * point;
	}

	inline float2 transformPoint(const float2& transform, const float2& point)
	{
		return transform + point;
	}


	inline const float2 transformVector(const float2& transform, const float2& vector)
	{
		return vector;
	}

	inline auto transformVector(float transform, const float2& vector)->decltype(transform * vector)
	{
		return transform * vector;
	}

	inline float concatenate(float lhs, float rhs)
	{
		return lhs * rhs;
	}

	inline float2 concatenate(const float2& lhs, const float2& rhs)
	{
		return lhs + rhs;
	}

	class Transform2D
	{
	public:
		Transform2D(const float2& translation = float2::Zero)
			:mTrans(translation)
		{
		}
		explicit Transform2D(float uniformScale, const float2& translation = float2::Zero)
			:m(Scale2D(uniformScale))
			,mTrans(translation)
		{}

		explicit Transform2D(const Scale2D& scale, const float2& translation = float2::Zero)
			:m(scale)
			,mTrans(translation)
		{

		}



		explicit Transform2D(const Shear2D& shear, const float2& translation = float2::Zero)
			:m(shear)
			,mTrans(translation)
		{

		}
		explicit Transform2D(const Quat2D& rot, const float2& translation = float2::Zero)
			:m(rot)
			,mTrans(translation)
		{

		}
		
		explicit Transform2D(const Matrix2x2& transform, const float2& translation = float2::Zero)
			:m(transform)
			,mTrans(translation)
		{}

		float2 transformPoint(const float2& point) const
		{
			return Air::transformPoint(mTrans, Air::transformPoint(m, point));
		}

		float2 transformVector(const float2& vector) const
		{
			return Air::transformVector(m, vector);
		}

		Transform2D concatenate(const Transform2D& rhs) const
		{
			return Transform2D(Air::concatenate(m, rhs.m), Air::concatenate(Air::transformPoint(rhs.m, mTrans), rhs.mTrans));
		}

		Transform2D inverse() const
		{
			Matrix2x2 invM = Air::inverse(m);
			float2 invTrans = Air::transformPoint(invM, Air::inverse(mTrans));
			return Transform2D(invM, invTrans);
		}

		bool operator == (const Transform2D& other) const
		{
			return m == other.m && mTrans == other.mTrans;
		}

		bool operator != (const Transform2D& other) const
		{
			return !operator==(other);
		}

		const Matrix2x2& getMatrix() const { return m; }

		const float2& getTranslation() const { return mTrans; }

		bool isIdentity() const
		{
			return m.isIdentity() && mTrans == float2::Zero;
		}
	private:
		Matrix2x2 m;
		float2 mTrans;
	};

	

	

	inline Transform2D concatenate(const Scale2D& scale, const float2& translation)
	{
		return Transform2D(scale, translation);
	}
	inline Transform2D concatenate(const Shear2D& shear, const float2& translation)
	{
		return Transform2D(shear, translation);
	}

	inline Transform2D concatenate(const Quat2D& rot, const float2& translation)
	{
		return Transform2D(rot, translation);
	}
	inline Transform2D concatenate(const Matrix2x2& transform, const float2& translation)
	{
		return Transform2D(transform, translation);
	}

	inline Transform2D concatenate(const float2& translation, const Scale2D& scale)
	{
		return Transform2D(scale, translation);
	}
	inline Transform2D concatenate(const float2& translation, const Shear2D& shear)
	{
		return Transform2D(shear, translation);
	}

	inline Transform2D concatenate(const float2& translation, const Quat2D& rot)
	{
		return Transform2D(rot, translation);
	}
	inline Transform2D concatenate(const float2& translation, const Matrix2x2& transform)
	{
		return Transform2D(transform, translation);
	}

	inline Transform2D concatenate(const float2& translation, const Transform2D& transform)
	{
		return Transform2D(transform.getMatrix(), concatenate(transformPoint(transform.getMatrix(), translation), transform.getTranslation()));
	}

	template<> struct ConcatenateRules<float, float2> { typedef float2 ResultType; };

	template<> struct ConcatenateRules<float2, float> { typedef float2 ResultType; };


	template<typename TransformType> struct ConcatenateRules<typename std::enable_if<!std::are_types_equal<Transform2D, TransformType>::value, Transform2D>::type, TransformType> { typedef Transform2D ResultType; };

	template<typename TransformType> struct ConcatenateRules<TransformType, typename std::enable_if<!std::are_types_equal<Transform2D, TransformType>::value, Transform2D>::type> { typedef Transform2D ResultType; };

	template<> struct ConcatenateRules<Matrix2x2, Transform2D> { typedef Transform2D ResultType; };

	template<> struct ConcatenateRules<Transform2D, Matrix2x2> { typedef Transform2D ResultType; };
}