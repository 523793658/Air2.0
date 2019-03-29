#pragma once
#include "CoreType.h"
#include "Template/AirTypeTraits.h"
#include "Containers/BitArray.h"
namespace Air
{
	template<typename ElementType, typename Allocator = DefaultSparseArrayAllocator>
	class TSparseArray;

	struct SparseArrayAllocationInfo
	{
		int32 mIndex;
		void* mPointer;
	};


	template<typename ElementType>
	union TSparseArrayElementOrFreeListLink
	{
		ElementType mElementData;
		struct  
		{
			int32 mPrevFreeIndex;
			int32 mNextFreeIndex;
		};
	};



	template<typename ElementType, typename Allocator>
	class TSparseArray
	{

	private:
		typedef TBitArray<typename Allocator::BitArrayAllocator> AllocationBitArrayType;
		AllocationBitArrayType mAllocationFlags;

		typedef TSparseArrayElementOrFreeListLink<TAlignedBytes<sizeof(ElementType), ALIGNOF(ElementType)>> FElementOrFreeListLink;

		template<bool bConst>
		class TBaseIterator
		{
		public:	 
			typedef TConstSetBitIterator<typename Allocator::BitArrayAllocator> BitArrayItType;
		private:
			typedef typename TChooseClass<bConst, const TSparseArray, TSparseArray>::Result	ArrayType;
			typedef	typename TChooseClass<bConst, const ElementType, ElementType>::Result ItElementType;

		public:
			explicit TBaseIterator(ArrayType& inArray, const BitArrayItType& inBitArrayIt)
				:mArray(inArray)
				,mBitArrayIt(inBitArrayIt)
			{

			}

			FORCEINLINE TBaseIterator& operator ++()
			{
				++mBitArrayIt;
				return *this;
			}

			FORCEINLINE int32 getIndex() const { return mBitArrayIt.getIndex(); }

			FORCEINLINE friend bool operator == (const TBaseIterator& lhs, const TBaseIterator& rhs)
			{
				return lhs.mBitArrayIt == rhs.mBitArrayIt && &lhs.mArray == &rhs.mArray;
			}

			FORCEINLINE friend bool operator != (const TBaseIterator& lhs, const TBaseIterator& rhs)
			{
				return lhs.mBitArrayIt != rhs.mBitArrayIt || &lhs.mArray != &rhs.mArray;
			}

			FORCEINLINE explicit operator bool() const
			{
				return !!mBitArrayIt;
			}

			FORCEINLINE bool operator !() const
			{
				return !(bool)*this;
			}

			FORCEINLINE ItElementType& operator*() const { return mArray[getIndex()]; }
			FORCEINLINE ItElementType* operator->() const { return &mArray[getIndex()]; }

			FORCEINLINE const RelativeBitReference& getRelativeBitReference() const { return mBitArrayIt; }
		

		protected:
			ArrayType& mArray;
			BitArrayItType mBitArrayIt;
		};
	public:
		class TIterator : public TBaseIterator<false>
		{
		public:
			TIterator(TSparseArray& inArray)
				:TBaseIterator<false>(inArray, TConstSetBitIterator<typename Allocator::BitArrayAllocator>(inArray.mAllocationFlags))
			{

			}

			TIterator(TSparseArray& inArray, const typename TBaseIterator<false>::BitArrayItType& inBitArrayIt)
				:TBaseIterator<false>(inArray, inBitArrayIt)
			{

			}
			void removeCurrent()
			{
				this->mArray.removeAt(this->getIndex());
			}
		};


		class TConstIterator : public TBaseIterator<true>
		{
		public:
			TConstIterator(const TSparseArray& inArray)
				:TBaseIterator<true>(inArray, TConstSetBitIterator<typename Allocator::BitArrayAllocator>(inArray.mAllocationFlags))
			{

			}

			TConstIterator(const TSparseArray& inArray, const typename TBaseIterator<true>::BitArrayItType& inBitArrayIt)
				:TBaseIterator<true>(inArray, inBitArrayIt)
			{

			}
			TIterator createIterator()
			{
				return TIterator(*this);
			}

			TConstIterator createConstIterator() const
			{
				return TConstIterator(*this);
			}
		};
	private:
		FORCEINLINE friend TIterator begin(TSparseArray& inArray) { return TIterator(inArray, TConstSetBitIterator<typename Allocator::BitArrayAllocator>(inArray.mAllocationFlags)); }

		FORCEINLINE friend TConstIterator begin(const TSparseArray& inArray) { return TConstIterator(inArray, TConstSetBitIterator<typename Allocator::BitArrayAllocator>(inArray.mAllocationFlags)); }


		FORCEINLINE friend TIterator end(TSparseArray& inArray) { return TIterator(inArray, TConstSetBitIterator<typename Allocator::BitArrayAllocator>(inArray.mAllocationFlags, inArray.mAllocationFlags.size())); }

		FORCEINLINE friend TConstIterator end(const TSparseArray& inArray) { return TConstIterator(inArray, TConstSetBitIterator<typename Allocator::BitArrayAllocator>(inArray.mAllocationFlags, inArray.mAllocationFlags.size())); }
	public:
		~TSparseArray()
		{
			empty();
		}



		void empty(int32 ExpectedNumElements = 0)
		{
			if (!std::is_trivially_destructible<ElementType>::value)
			{
				for (TIterator it(*this); it; ++it)
				{
					ElementType& element = *it;
					element.~ElementType();
				}
			}
			mData.empty(ExpectedNumElements);
			mFirstFreeIndex = -1;
			mNumFreeIndices = 0;
			mAllocationFlags.empty(ExpectedNumElements);
		}

		void reset()
		{
			if (!std::is_trivially_destructible<ElementType>::value)
			{
				for (TIterator it(*this); it; ++it)
				{
					ElementType& element = *it;
					element.~ElementType();
				}
			}
			mData.reset();
			mFirstFreeIndex = -1;
			mNumFreeIndices = 0;
			mAllocationFlags.reset();
		}

		void reserve(int32 expectedNumElements)
		{
			if (expectedNumElements > mData.count())
			{
				const int32 elementsToAdd = expectedNumElements - mData.count();
				int32 elementIndex = mData.addUninitialized(elementsToAdd);
				for (int32 freeIndex = expectedNumElements - 1; freeIndex >= elementIndex; --freeIndex)
				{
					if (mNumFreeIndices)
					{
						getData(mFirstFreeIndex).mPrevFreeIndex = freeIndex;
					}
					getData(freeIndex).mPrevFreeIndex = -1;
					getData(freeIndex).mNextFreeIndex = mNumFreeIndices > 0 ? mFirstFreeIndex : INDEX_NONE;
					mFirstFreeIndex = freeIndex;
					++mNumFreeIndices;
				}
				for (int32 i = 0; i < elementsToAdd; i++)
				{
					mAllocationFlags.add(false);
				}
			}
		}

		void shrink()
		{
			int32 maxAllocatedIndex = INDEX_NONE;
			for (TConstSetBitIterator<typename Allocator::BitArrayAllocator> allocatedIndexIt(mAllocationFlags); allocatedIndexIt; ++allocatedIndexIt)
			{
				maxAllocatedIndex = Math::max(maxAllocatedIndex, allocatedIndexIt.getIndex());
			}
			const int32 firstIndexToRemove = maxAllocatedIndex + 1;
			if (firstIndexToRemove < mData.size())
			{
				if (mNumFreeIndices > 0)
				{
					int32 freeIndex = mFirstFreeIndex;
					while (freeIndex != INDEX_NONE)
					{
						if (freeIndex >= firstIndexToRemove)
						{
							const int32 prevFreeIndex = getData(freeIndex).mPrevFreeIndex;
							const int32 nextFreeIndex = getData(freeIndex).mPrevFreeIndex;
							if (nextFreeIndex != INDEX_NONE)
							{
								getData(nextFreeIndex).mPrevFreeIndex = prevFreeIndex;
							}
							if (prevFreeIndex != INDEX_NONE)
							{
								getData(prevFreeIndex).mNextFreeIndex = nextFreeIndex;
							}
							else
							{
								mFirstFreeIndex = nextFreeIndex;
							}
							--mNumFreeIndices;
						}
						else
						{
							freeIndex = getData(freeIndex).mNextFreeIndex;
						}
					}
				}
				mData.removeAt(firstIndexToRemove, mData.size() - firstIndexToRemove);
				mAllocationFlags.removeAt(firstIndexToRemove, mAllocationFlags.count() - firstIndexToRemove);
			}
			mData.shrink_to_fit();
		}

	public:
		FElementOrFreeListLink& getData(int32 index)
		{
			return ((FElementOrFreeListLink*)mData.data())[index];
		}

		const FElementOrFreeListLink& getData(int32 index) const
		{
			return ((FElementOrFreeListLink*)mData.data())[index];
		}

		ElementType& operator[](int32 index)
		{
			BOOST_ASSERT(index >= 0 && index < mData.size() && index < mAllocationFlags.count());
			return *(ElementType*)&getData(index).mElementData;
		}

		const ElementType& operator[](int32 index) const
		{
			BOOST_ASSERT(index >= 0 && index < mData.size() && index < mAllocationFlags.count());
			return *(ElementType*)&getData(index).mElementData;

		}

		int32 getMaxIndex() const { return mData.size(); }

		SparseArrayAllocationInfo allocateIndex(int32 index)
		{
			BOOST_ASSERT(index >= 0);
			BOOST_ASSERT(index < getMaxIndex());
			BOOST_ASSERT(!mAllocationFlags[index]);
			mAllocationFlags[index] = true;
			SparseArrayAllocationInfo result;
			result.mIndex = index;
			result.mPointer = &getData(result.mIndex).mElementData;
			return result;
		}

		SparseArrayAllocationInfo addUninitialized()
		{
			int32 index;
			if (mNumFreeIndices)
			{
				index = mFirstFreeIndex;
				mFirstFreeIndex = getData(mFirstFreeIndex).mNextFreeIndex;
				--mNumFreeIndices;
				if (mNumFreeIndices)
				{
					getData(mFirstFreeIndex).mPrevFreeIndex = -1;
				}
			}
			else
			{
				index = mData.addUninitialized(1);
				mAllocationFlags.add(false);
			}
			return allocateIndex(index);
		}

		void removeAt(int32 index, int32 count = 1)
		{
			if (!std::is_trivially_destructible<ElementType>::value)
			{
				for (int32 it = index, itCount = count; itCount; ++it, --itCount)
				{
					((ElementType&)getData(it).mElementData).~ElementType();
				}
			}
			removeAtUninitialized(index, count);
		}

		int32 count() const { return mData.size() - mNumFreeIndices; }

		int32 size() const { return mData.size() - mNumFreeIndices; }

		void removeAtUninitialized(int32 index, int32 count = 1)
		{
			for (; count; --count)
			{
				BOOST_ASSERT(mAllocationFlags[index]);
				if (mNumFreeIndices)
				{
					getData(mFirstFreeIndex).mPrevFreeIndex = index;
				}
				auto & indexData = getData(index);
				indexData.mPrevFreeIndex = -1;
				indexData.mNextFreeIndex = mNumFreeIndices > 0 ? mFirstFreeIndex : INDEX_NONE;
				mFirstFreeIndex = index;
				++mNumFreeIndices;
				mAllocationFlags[index] = false;
				++index;
			}

		}

		int32 add(typename std::type_traits<ElementType>::const_init_type element)
		{
			SparseArrayAllocationInfo allocation = addUninitialized();
			new(allocation)ElementType(element);
			return allocation.mIndex;
		}

	private:
		

		typedef TArray<FElementOrFreeListLink> DataType;
		DataType mData;

		int32 mFirstFreeIndex{ -1 };
		int32 mNumFreeIndices{ 0 };
	};

	
}

inline void* operator new(size_t size, const Air::SparseArrayAllocationInfo& allocation)
{
	return allocation.mPointer;
}