#pragma once
#include "Math/Vector.h"
namespace Air
{
	template<typename TransformType>
	inline auto concatenate(const TransformType& lhs, const TransformType& rhs)->decltype(lhs.concatenate(rhs))
	{
		return lhs.concatenate(rhs);
	}

	template<typename TransformType>
	struct TransformConverter
	{
		static const TransformType& convert(const TransformType& transform)
		{
			return transform;
		}

		template<typename OtherTransformType>
		static TransformType convert(const OtherTransformType& transform)
		{
			return TransformType(transform);
		}
	};

	template<typename ResultType, typename TransformType>
	inline auto transformCast(const TransformType& transform)->decltype(TransformConverter<ResultType>::convert(transform))
	{
		return TransformConverter<ResultType>::convert(transform);
	}

	template<typename TransformType>
	inline auto inverse(const TransformType& transform)->decltype(transform.inverse())
	{
		return transform.inverse();
	}

	template<typename TransformType>
	inline float2 transformVector(const TransformType& transform, const float2& vector)
	{
		return transform.transformVector(vector);
	}

	template<typename TransformType>
	inline float2 transformPoint(const TransformType& transform, const float2& point)
	{
		return transform.transformPoint(point);
	}

	template<typename TransformTypeA, typename TransformTypeB>
	struct ConcatenateRules
	{

	};

	template<typename TransformType>
	struct ConcatenateRules<TransformType, TransformType>
	{
		typedef TransformType resultType;
	};

	template<typename TransformTypeA, typename TransformTypeB>
	inline typename ConcatenateRules<TransformTypeA, TransformTypeB>::ResultType concatenate(const TransformTypeA& lhs, const TransformTypeB& rhs)
	{
		typedef typename ConcatenateRules<TransformTypeA, TransformTypeB>::ResultType ReturnType;

		return concatenate(transformCast<ReturnType>(lhs), transformCast<ReturnType>(rhs));
	}
}