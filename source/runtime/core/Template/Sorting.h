#pragma once
#include "CoreType.h"
#include "HAL/PlatformMath.h"
#include "Template/Less.h"
#include "Template/AirTemplate.h"
namespace Air
{
	class Sorting
	{

	public:
		template<typename T, class PREDICATE_CLASS>
		struct TDereferenceWrapper
		{
			const PREDICATE_CLASS& mPredicate;
			TDereferenceWrapper(const PREDICATE_CLASS& inPredicate)
				:mPredicate(inPredicate) {}
			FORCEINLINE bool operator ()(T& A, T& B) { return mPredicate(A, B); }

			FORCEINLINE bool operator()(const T& A, const T& B)const
			{
				return mPredicate(A, B);
			}
		};

		template<typename T, class PREDICATE_CLASS>
		struct TDereferenceWrapper<T*, PREDICATE_CLASS>
		{
			const PREDICATE_CLASS& mPredicate;
			TDereferenceWrapper(const PREDICATE_CLASS& InPredicate)
				:mPredicate(InPredicate) {}
			FORCEINLINE bool operator()(T* A, T* B)const
			{
				return mPredicate(*A, *B);
			}
		};

		template<class T, class PREDICATE_CLASS>
		static void sortInternal(T* first, const int32 num, const PREDICATE_CLASS& predicate)
		{
			struct FStack
			{
				T* mMin;
				T* mMax;
			};

			if (num < 2)
			{
				return;
			}
			FStack recursionStack[32] = { {first, first + num - 1} }, current, inner;
			for (FStack* stackTop = recursionStack; stackTop >= recursionStack; --stackTop)
			{
				current = *stackTop;
			Loop:
				PTRINT count = current.mMax - current.mMin + 1;
				if (count < 8)
				{
					while (current.mMax > current.mMin)
					{
						T* Max, *item;
						for (Max = current.mMin, item = current.mMin + 1; item <= current.mMax; item++)
						{
							if (predicate(*Max, *item))
							{
								Max = item;
							}
						}
						exchange(*Max, *current.mMax--);
					}
				}
				else
				{
					exchange(current.mMin[count / 2], current.mMin[0]);
					inner.mMin = current.mMin;
					inner.mMax = current.mMax + 1;
					for (; ; )
					{
						while (++inner.mMin <= current.mMax && !predicate(*current.mMin, *inner.mMin));
						while (--inner.mMax > current.mMin && !predicate(*inner.mMax, *current.mMin));
						if (inner.mMin > inner.mMax)
						{
							break;
						}
						exchange(*inner.mMin, *inner.mMax);
					}
					exchange(*current.mMin, *inner.mMax);
					if (inner.mMax - 1 - current.mMin >= current.mMax - inner.mMin)
					{
						if (current.mMin + 1 < inner.mMax)
						{
							stackTop->mMin = current.mMin;
							stackTop->mMax = inner.mMax - 1;
							stackTop++;
						}
						if (current.mMax > inner.mMin)
						{
							current.mMin = inner.mMin;
							goto Loop;
						}
					}
					else
					{
						if (current.mMax > inner.mMin)
						{
							stackTop->mMin = inner.mMin;
							stackTop->mMax = current.mMax;
							stackTop++;
						}
						if (current.mMin < inner.mMax)
						{
							current.mMax = inner.mMax - 1;
							goto Loop;
						}
					}
				}
			}
		}

		template<class T, class PREDICATE_CLASS>
		static void sort(T* first, const int32 num, const PREDICATE_CLASS& predicate)
		{
			sortInternal(first, num, TDereferenceWrapper<T, PREDICATE_CLASS>(predicate));
		}

		template<class T, class PREDICATE_CLASS>
		static void sort(T** first, const int32 num, const PREDICATE_CLASS& predicate)
		{
			sortInternal(first, num, TDereferenceWrapper<T*, PREDICATE_CLASS>(predicate));
		}

		template<class T>
		static void sort(T* first, const int32 num)
		{
			sortInternal(first, num, TDereferenceWrapper<T, TLess<T>>(TLess<T>()));
		}

		template<class T>
		static void sort(T** first, const int32 num)
		{
			sortInternal(first, num, TDereferenceWrapper<T*, TLess<T>>(TLess<T>()));
		}
	};
}