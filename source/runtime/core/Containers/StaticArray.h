#pragma once
#include "CoreType.h"
#include "Template/AlignmentTemplates.h"
#include "Template/TypeCompatibleBytes.h"
#include "Template/MemoryOps.h"
#include "Delegates/IntegerSequence.h"
namespace Air
{
	template<typename TElement, uint32 NumElements, uint32 Alignment = ALIGNOF(TElement)>
	class TStaticArray
	{
	public:
		TStaticArray()
			:mStorage()
		{
			
		}

		explicit TStaticArray(const TElement& defaultElement)
			:mStorage(TMakeIntegerSequence<uint32, NumElements>(), defaultElement)
		{}

		TStaticArray(TStaticArray&& other) = default;

		TStaticArray(const TStaticArray& other) = default;

		TStaticArray& operator = (TStaticArray&& other) = default;

		TStaticArray& operator = (const TStaticArray& other) = default;

		TElement& operator [](uint32 index)
		{
			BOOST_ASSERT(index < NumElements);
			return mStorage.mElements[index].mElement;
		}

		const TElement& operator[](uint32 index) const
		{
			BOOST_ASSERT(index < NumElements);
			return mStorage.mElements[index].mElement;
		}

		friend bool operator == (const TStaticArray& A, const TStaticArray& B)
		{
			for (uint32 ElementIndex = 0; ElementIndex < NumElements; ElementIndex++)
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
		struct alignas(Alignment) TArrayStorageElementAligned
		{
			TArrayStorageElementAligned() {}
			TArrayStorageElementAligned(const TElement & inElement)
				:mElement(inElement)
			{}
			TElement mElement;
		};

		struct TArrayStorage
		{
			TArrayStorage()
				:mElements()
			{}

			template<uint32... Indices>
			TArrayStorage(TIntegerSequence<uint32, Indices...>, const TElement& defaultElement)
				:mElements{((void)Indices, defaultElement)... }
			{}


			TArrayStorageElementAligned mElements[NumElements];
		};

		TArrayStorage mStorage;
	};
}