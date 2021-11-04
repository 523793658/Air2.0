#pragma once
#include "CoreMinimal.h"

namespace Air
{
	struct ShaderCompilerEnvironment;

	struct ShaderPermutationBool
	{
		using Type = bool;
		static constexpr int32 mPermutationCount = 2;

		static constexpr bool bIsMultiDimensional = false;

		static int32 toDimensionValueId(Type e)
		{
			return e ? 1 : 0;
		}

		static bool toDefineValue(Type e)
		{
			return e;
		}

		static Type fromDimensionValueId(int32 permutationId)
		{
			BOOST_ASSERT(permutationId == 0 || permutationId == 1);
			return permutationId == 1;
		}
	};

	template<typename TType, int32 TDimensionSize, int32 TFirstValue = 0>
	struct TShaderPermutationInt
	{
		using Type = TType;

		static constexpr int32 mPermutationCount = TDimensionSize;

		static constexpr bool bIsMultiDimensional = false;

		static constexpr Type MinValue = static_cast<Type>(TFirstValue);
		static constexpr Type MaxValue = static_cast<Type>(TFirstValue + TDimensionSize - 1);

		static int32 toDimensionValueId(Type e)
		{
			int32 permutationId = static_cast<int32>(e) - TFirstValue;
			BOOST_ASSERT(permutationId < mPermutationCount && permutationId >= 0);
			return permutationId;
		}

		static int32 toDefineValue(Type e)
		{
			return toDimensionValueId(e) + TFirstValue;
		}

		static Type fromDimensionValueId(int32 permutationId)
		{
			BOOST_ASSERT(permutationId < mPermutationCount && permutationId >= 0);
			return static_cast<Type>(permutationId + TFirstValue);
		}
	};

	template<typename... Ts>
	struct TShaderPermutationDomain
	{
		using Type = TShaderPermutationDomain<Ts...>;
		static constexpr bool bIsMultiDimensional = true;

		static constexpr int32 mPermutationCount = 1;

		TShaderPermutationDomain<Ts...>() {}

		explicit TShaderPermutationDomain<Ts...>(int32 permutationId)
		{
			BOOST_ASSERT(permutationId == 0);
		}

		template<class DimensionToSet>
		void set(typename DimensionToSet::Type)
		{
			static_assert(sizeof(typename DimensionToSet::Type) == 0, "Unknown shader permutation dimension.");
			return DimensionToSet::Type();
		}

		void modifyCompilationEnvironment(ShaderCompilerEnvironment& outEnvironment) const {}

		static int32 toDimensionValueId(const Type& permutationVector)
		{
			return 0;
		}

		int32 toDimensionValueId() const
		{
			return toDimensionValueId(*this);
		}

		static Type fromDimensionValueId(const int32 permutationId)
		{
			return Type(permutationId);
		}

		bool operator==(const Type& other) const
		{
			return true;
		}
	};

	template<bool BooleanSpetialization>
	class TShaderPermutationDomainSpetialization
	{
	public:
		template<typename TPermutationVector, typename TDimension>
		static void modifyCompilationEnvironment(const TPermutationVector& permutationVector, ShaderCompilerEnvironment& outEnvironment)
		{
			outEnvironment.setDefine(TDimension::DefineName, TDimension::toDefineValue(permutationVector.mDimensionValue));
			return permutationVector.mTail.modifyCompilationEnvironment(outEnvironment);
		}

		template<typename TPermutationVector, typename TDimensionToGet>
		static const typename TDimensionToGet::Type& getDimension(const TPermutationVector& permutationVector)
		{
			return permutationVector.mTail.template get<TDimensionToGet>();
		}

		template<typename TPermutationVector, typename TDimensionToSet>
		static void setDimension(TPermutationVector& permutationVector, const typename TDimensionToSet::Type& value)
		{
			return permutationVector.mTail.template set<TDimensionToSet>(value);
		}
	};

	template<>
	class TShaderPermutationDomainSpetialization<true>
	{
	public:
		template<typename TPermutationVector, typename TDimension>
		static void modifyCompilationEnvironment(const TPermutationVector& permutationVector, ShaderCompilerEnvironment& outEnvironment)
		{
			permutationVector.mDimensionValue.modifyCompilationEnvironment(outEnvironment);
			return permutationVector.mTail.modifyCompilationEnvironment(outEnvironment);
		}

		template<typename TPermutationVector, typename TDimensionToGet>
		static const typename TDimensionToGet::Type& getDimension(const TPermutationVector& permutationVector)
		{
			return permutationVector.mDimensionValue;
		}

		template<typename TPermutationVector, typename TDmimensionToSet>
		static void setDimension(TPermutationVector& permutationVector, const typename TDmimensionToSet::Type& value)
		{
			permutationVector.mDimensionValue = value;
		}
	};

	template<typename TDimension, typename... Ts>
	struct TShaderPermutationDomain<TDimension, Ts...>
	{
		using Type = TShaderPermutationDomain<TDimension, Ts...>;

		static constexpr bool bIsMultiDimensional = true;

		using Super = TShaderPermutationDomain<Ts...>;

		static constexpr int32 mPermutationCount = Super::mPermutationCount * TDimension::mPermutationCount;

		TShaderPermutationDomain<TDimension, Ts...>()
			: mDimensionValue(TDimension::fromDimensionValueId(0))
		{

		}

		explicit TShaderPermutationDomain<TDimension, Ts...>(int32 permutationId)
			: mDimensionValue(TDimension::fromDimensionValueId(permutationId% TDimension::mPermutationCount))
			, mTail(permutationId / TDimension::mPermutationCount)
		{
			BOOST_ASSERT(permutationId >= 0 && permutationId < mPermutationCount);
		}

		template<class DimensionToSet>
		void set(typename DimensionToSet::Type value)
		{
			return TShaderPermutationDomainSpetialization<std::is_same<TDimension, DimensionToSet>::value>::template setDimension<Type, DimensionToSet>(*this, value);
		}

		template<class DimensionToGet>
		const typename DimensionToGet::Type& get() const
		{
			return TShaderPermutationDomainSpetialization<std::is_same<TDimension, DimensionToGet>::value>::template getDimension<Type, DimensionToGet>(*this);
		}

		void modifyCompilationEnvironment(ShaderCompilerEnvironment& outEnvironment) const
		{
			TShaderPermutationDomainSpetialization<TDimension::bIsMultiDimensional>::template modifyCompilationEnvironment<Type, TDimension>(*this, outEnvironment);
		}

		static int32 toDimensionValueId(const Type& permutationVector)
		{
			return permutationVector.toDimensionValueId();
		}

		int32 toDimensionValueId() const
		{
			return TDimension::toDimensionValueId(mDimensionValue) + TDimension::mPermutationCount * mTail.toDimensionValueId();
		}

		static Type fromDimensionValueId(const int32 permutationId)
		{
			return Type(permutationId);
		}

		bool operator==(const Type& other) const
		{
			return mDimensionValue == other.mDimensionValue && mTail == other.mTail;
		}

		bool operator!=(const Type& other) const
		{
			return !(*this == other);
		}

	private:
		template<bool BooleanSpetialization>
		friend class TShaderPermutationDomainSpetialization;

		typename TDimension::Type mDimensionValue;
		Super mTail;
	};

	using ShaderPermutationNone = TShaderPermutationDomain<>;


#define DECLARE_SHADER_PERMUTATION_IMPL(ClassName, InDefineName, PermutationMetaType,...) \
	class ClassName : public PermutationMetaType<__VA_ARGS__>{\
	public:\
		static constexpr const TCHAR* DefineName = TEXT(InDefineName);\
	}

#define SHADER_PERMUTATION_BOOL(InDefineName) \
	public ShaderPermutationBool {\
	public: \
		static constexpr const TCHAR* DefineName = TEXT(InDefineName);\
	}

#define SHADER_PERMUTATION_ENUM_CLASS(ClassName, inDefinedName, EnumName) \
	DECLARE_SHADER_PERMUTATION_IMPL(ClassName, inDefinedName, TShaderPermutationInt, EnumName, static_cast<int32>(EnumName::MAX))
}