#pragma once
#include "CoreType.h"
#include "Template/AirTypeTraits.h"
#include "boost/noncopyable.hpp"
#include <type_traits>

namespace Air
{
	template <typename T, uint32 N>
	char(&ArrayCounterHelper(const T(&)[N]))[N];

	template<typename ReferencedType>
	FORCEINLINE ReferencedType* ifAThenAElseB(ReferencedType* A, ReferencedType* B)
	{
		const PTRINT IntA = reinterpret_cast<PTRINT>(A);
		const PTRINT IntB = reinterpret_cast<PTRINT>(B);
		const PTRINT MaskB = -(!IntA);
		return reinterpret_cast<ReferencedType*>(IntA | (MaskB & IntB));
	}

	template<typename Type>
	struct TGuardValue : private boost::noncopyable
	{
		TGuardValue(Type& referenceValue, const Type& newValue)
			:mRefValue(referenceValue), mOldValue(referenceValue)
		{
			mRefValue = newValue;
		}
		~TGuardValue()
		{
			mRefValue = mOldValue;
		}

		FORCEINLINE const Type& operator *() const
		{
			return mOldValue;
		}

	private:
		Type& mRefValue;
		Type mOldValue;
	};




	template<typename T>
	inline typename std::enable_if<std::use_bitwise_swap<T>::value>::type Swap(T& A, T& B)
	{
		TTypeCompatibleBytes<T> temp;
		Memory::memcpy(&temp, &A, sizeof(T));
		Memory::memcpy(&A, &B, sizeof(T));
		Memory::memcpy(&B, &temp, sizeof(T));
	}

	template<typename T>
	inline typename std::enable_if<!std::use_bitwise_swap<T>::value>::type Swap(T& A, T& B)
	{
		T temp = std::move(A);
		A = std::move(B);
		B = std::move(temp);
	}

	template<typename T>
	inline void exchange(T& A, T& B)
	{
		Swap(A, B);
	}

#define ARRAY_COUNT(array) (sizeof(ArrayCounterHelper(array)) + 0)

#define STRUCT_OFFSET(struc, member) offsetof(struc, member)

	template<typename T>
	inline typename std::enable_if<!std::is_move_assignable<T>::value>::type move(T& A, T& B)
	{
		A.~T();
		new(&A)T(B);
	}


	template<typename T>
	inline typename std::enable_if<std::is_move_assignable<T>::value>::type move(T& A, T& B)
	{
		A.~T();
		new(&A)T(std::move(B));
	}

}
