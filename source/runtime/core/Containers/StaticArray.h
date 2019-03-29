#pragma once
#include "CoreType.h"
#include "Template/AlignmentTemplates.h"
#include "Template/TypeCompatibleBytes.h"
#include "Template/MemoryOps.h"
namespace Air
{
	template<typename TElement, uint32 NumElements, uint32 Alignment = ALIGNOF(TElement)>
	class TStaticArray
	{
	public:
		TStaticArray()
		{
			for (uint32 ElementIndex = 0; ElementIndex < NumElements; ++ElementIndex)
			{
				new(&(*this)[ElementIndex]) TElement;
			}
		}

		TStaticArray(TStaticArray&& other)
		{
			moveConstructItems((void*)mElements, (const TElement*)other.mElements, NumElements);
		}

		TStaticArray(const TStaticArray& other)
		{
			constructItems<TElement>((void*)mElements, (const TElement*)other.mElements, NumElements);
		}

		TStaticArray& operator = (TStaticArray&& other)
		{
			moveAssignItems((TElement*)mElements, (const TElement*)other.Elements, NumElements);
			return *this;
		}

		TStaticArray& operator = (const TStaticArray& other)
		{
			copyAssignItems((TElement*)mElements, (const TElement*)other.mElements, NumElements);
			return *this;
		}

		~TStaticArray()
		{
			destructItems((TElement*)mElements, NumElements);
		}

		TElement& operator [](uint32 index)
		{
			BOOST_ASSERT(index < NumElements);
			return *(TElement*)&mElements[index];
		}

		const TElement& operator[](uint32 index) const
		{
			BOOST_ASSERT(index < NumElements);
			return *(const TElement*)&mElements[index];
		}

		friend bool operator == (const TStaticArray& A, const TStaticArray& B)
		{
			for (uint32 ElementIndex = 0; ElementIndex < NumElementss; ElementIndex++)
			{
				if (!(A[ElementIndex] == B[ElementIndex]))
				{
					return false;
				}
			}
			return true;
		}


		friend bool operator != (const TStaticArray& A, const TStaticArray& B)
		{
			for (uint32 ElementIndex = 0; ElementIndex < NumElements; ++ElementIndex)
			{
				if (!(A[ElementIndex] == B[ElementIndex]))
				{
					return true;
				}
			}
			return false;
		}

		int32 size() const { return NumElements; }

		friend uint32 getTypeHash(const TStaticArray& Array)
		{
			uint32 result = 0;
			for (uint32 elementIndex = 0; elementIndex < NumElements; ++elementIndex)
			{
				result ^= getTypeHash(Array[elementIndex]);
			}
			return result;
		}

	private:
		TAlignedBytes<sizeof(TElement), Alignment> mElements[NumElements];
	};
}