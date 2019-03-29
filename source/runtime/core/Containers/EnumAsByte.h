#pragma once
#include "CoreType.h"
#include <type_traits>
namespace Air
{
	template <bool> struct TEnumAsByte_EnumClass;

	template<> struct TEnumAsByte_EnumClass<true> {};
	template<> struct TEnumAsByte_EnumClass<false> {};

	template<class TEnum>
	class TEnumAsByte
	{
		typedef  TEnumAsByte_EnumClass<std::is_enum<TEnum>::value> Check;
	public:
		typedef TEnum EnumType;
		FORCEINLINE TEnumAsByte() {}

		FORCEINLINE TEnumAsByte(const TEnumAsByte& inValue)
			:mValue(inValue.mValue)
		{}

		FORCEINLINE TEnumAsByte(TEnum inValue)
			: mValue(static_cast<uint8>(inValue))
		{}

		explicit FORCEINLINE TEnumAsByte(int32 inValue)
			: mValue(static_cast<uint8>(inValue))
		{}

		explicit FORCEINLINE TEnumAsByte(uint8 inValue)
			: mValue(inValue)
		{}

	public:
		FORCEINLINE TEnumAsByte & operator =(TEnumAsByte inValue)
		{
			mValue = inValue.mValue;
			return *this;
		}

		FORCEINLINE TEnumAsByte & operator = (TEnum inValue)
		{
			mValue = static_cast<int8>(inValue);
			return *this;
		}

		bool operator == (TEnum inValue) const
		{
			return mValue == static_cast<int8>(inValue);
		}

		bool operator == (TEnumAsByte inValue) const
		{
			return mValue == inValue.mValue;
		}

		operator TEnum() const
		{
			return (TEnum)mValue;
		}

	public:
		TEnum getValue() const
		{
			return (TEnum)mValue;
		}



	private:
		uint8 mValue;
	};
}