#pragma once
#include "CoreType.h"
#include "boost/assert.hpp"
#include "Serialization/Archive.h"
#include "Template/MemoryOps.h"
#include "Template/Sorting.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "Template/AirTypeTraits.h"
namespace Air
{
#if BUILD_SHIPPING || BUILD_TEST
#define TARRAY_RANGED_FOR_CHECKS	0
#else
#define TARRAY_RANGED_FOR_CHECKS	1
#endif


#define ARGESSIVE_ARRAY_FORCEINLINE

#if TARRAY_RANGED_FOR_CHECKS
	template<typename ElementType>
	struct TCheckedPointerIterator
	{
		explicit TCheckedPointerIterator(const int32 & inNum, ElementType* inPtr)
			:ptr(inPtr)
			,currentNum(inNum)
			, initialNum(inNum)
		{

		}
		
		FORCEINLINE ElementType& operator*() const
		{
			return *ptr;
		}

		FORCEINLINE TCheckedPointerIterator& operator++()
		{
			++ptr;
			return *this;
		}
		FORCEINLINE TCheckedPointerIterator& operator --()
		{
			--ptr;
			return *this;
		}

	private:
		ElementType * ptr;
		const int32& currentNum;
		int32			initialNum;
		FORCEINLINE friend bool operator != (const TCheckedPointerIterator & lhs, const TCheckedPointerIterator & rhs)
		{
			BOOST_ASSERT(lhs.currentNum == lhs.initialNum);
			return lhs.ptr != rhs.ptr;
		}
	};
#endif

	template<typename ElementType, typename IteratorType>
	struct TDereferencingIterator
	{
		explicit TDereferencingIterator(IteratorType InIter)
			:mIter(InIter)
		{
		}
		FORCEINLINE ElementType& operator*() const
		{
			return *(ElementType*)*mIter;
		}

		FORCEINLINE TDereferencingIterator& operator ++()
		{
			++mIter;
			return *this;
		}
	private:
		IteratorType mIter;
		FORCEINLINE friend bool operator !=(const TDereferencingIterator& lhs, const TDereferencingIterator & rhs)
		{
			return lhs.mIter != rhs.mIter;
		}
	};
	template<typename ContainerType, typename ElementType, typename IndexType>
	class TIndexedContainerIterator
	{
	public:	 
		TIndexedContainerIterator(ContainerType& inContainter, IndexType startIndex = 0)
			:mContainer(inContainter)
			,mIndex(startIndex)
		{

		}

		TIndexedContainerIterator& operator ++()
		{
			++mIndex;
			return *this;
		}

		TIndexedContainerIterator operator ++(int)
		{
			TIndexedContainerIterator tmp(*this);
			++mIndex;
			return tmp;
		}

		TIndexedContainerIterator & operator --()
		{
			--mIndex;
			return *this;
		}
		TIndexedContainerIterator operator --(int)
		{
			TIndexedContainerIterator tmp(*this);
			--mIndex;
			return tmp;
		}

		TIndexedContainerIterator& operator += (int32 offset)
		{
			mIndex += offset;
			return *this;
		}

		TIndexedContainerIterator operator + (int32 offset) const
		{
			TIndexedContainerIterator tmp(*this);
			return tmp += offset;
		}

		TIndexedContainerIterator& operator -=(int32 offset)
		{
			mIndex -= offset;
			return *this;
		}
		TIndexedContainerIterator operator -(int32 offset)
		{
			TIndexedContainerIterator tmp(*this);
			return tmp += offset;	
		}

		ElementType& operator*() const 
		{
			return mContainer[mIndex];
		}

		ElementType* operator->() const
		{
			return &mContainer[mIndex];
		}

		FORCEINLINE explicit operator bool() const
		{
			return mContainer.isValidIndex(mIndex);
		}

		FORCEINLINE bool operator !() const
		{
			return !(bool)*this;
		}

		IndexType getIndex() const
		{
			return mIndex;
		}

		void reset()
		{
			mIndex = 0;
		}

		FORCEINLINE friend bool operator == (const TIndexedContainerIterator& lhs, const TIndexedContainerIterator& rhs)
		{
			return &lhs.mContainer == &rhs.mContainer && lhs.mIndex == lhs.mIndex;
		}

		FORCEINLINE friend bool operator != (const TIndexedContainerIterator& lhs, const TIndexedContainerIterator& rhs)
		{
			return &lhs.mContainer != &rhs.mContainer || lhs.mIndex != lhs.mIndex;
		}
	private:
		ContainerType& mContainer;
		IndexType	mIndex;
	};



	template<class InElementType, class InAllocator = DefaultAllocator>
	class TArray
	{
		template<typename OtherInElementType, typename OtherAllocator>
		friend class TArray;


	public:
		typedef InElementType ElementType;
		typedef InAllocator Allocator;

		typedef TIndexedContainerIterator<TArray, ElementType, int32> TIterator;
		typedef TIndexedContainerIterator<const TArray, const ElementType, int32> TConstIterator;

		TConstIterator createConstIterator()const
		{
			return TConstIterator(*this);
		}

		TIterator createIterator()
		{
			return TIterator(*this);
		}
		FORCEINLINE TArray()
			:mArrayNum(0)
			, mArrayMax(0)
		{}

		ARGESSIVE_ARRAY_FORCEINLINE ~TArray()
		{
			destructItems(getData(), mArrayNum);
#if defined(_MSC_VER) && !defined(__clang__)
			volatile const ElementType* dummy = &DebugGet(0);
#endif
		}

		TArray(std::initializer_list<InElementType> initList)
		{
			copyToEmpty(initList.begin(), (int32)initList.size(), 0, 0);
		}

		template<typename OtherElementType, typename OtherAllocation>
		FORCEINLINE explicit TArray(const TArray<OtherElementType, OtherAllocation>& other)
		{
			copyToEmpty(other.getData(), other.size(), 0, 0);
		}

		FORCEINLINE TArray(const TArray& other)
		{
			copyToEmpty(other.getData(), other.size(), 0, 0);
		}

		FORCEINLINE TArray(const TArray& other, int32 extraSlack)
		{
			copyToEmpty(other.getData(), other.size(), 0, extraSlack);
		}
#if defined(_MSC_VER) && !defined(__clang__)
	private:
		FORCENOINLINE const ElementType& DebugGet(int32 index) const
		{
			return getData()[index];
		}
#endif
	public:


		template<typename OtherElementType>
		ARGESSIVE_ARRAY_FORCEINLINE void copyToEmpty(const OtherElementType* otherData, int32 otherNum, int32 preMax, int32 extraSlack)
		{
			BOOST_ASSERT(extraSlack >= 0);
			mArrayNum = otherNum;
			if (otherNum || extraSlack || preMax)
			{
				resizeForCopy(otherNum + extraSlack, preMax);
				constructItems<ElementType>(getData(), otherData, otherNum);
			}
			else
			{
				mArrayMax = 0;
			}
		}
		FORCENOINLINE void resizeForCopy(int32 newMax, int32 prevMax)
		{
			if (newMax)
			{
				newMax = mAllocatorInstance.calculateSlackReserve(newMax, sizeof(ElementType));
			}
			if (newMax != prevMax)
			{
				mAllocatorInstance.resizeAllocation(0, newMax, sizeof(ElementType));
			}
			mArrayMax = newMax;
		}

#if TARRAY_RANGED_FOR_CHECKS
		typedef TCheckedPointerIterator<ElementType> RangedForIteratorType;
		typedef TCheckedPointerIterator<const ElementType> RangedForConstIteratorType;
#else
		typedef ElementType* RangedForIteratorType;
		typedef const ElementType* RangedForConstIteratorType;
#endif



#if TARRAY_RANGED_FOR_CHECKS
		FORCEINLINE friend RangedForIteratorType begin(TArray& inArray)
		{
			return RangedForIteratorType(inArray.mArrayNum, inArray.getData());
		
		}
		FORCEINLINE friend RangedForConstIteratorType begin(const TArray& inArray)
		{
			return RangedForConstIteratorType(inArray.mArrayNum, inArray.getData());
		}

		FORCEINLINE friend RangedForIteratorType end(TArray& inArray)
		{
			return RangedForIteratorType(inArray.mArrayNum, inArray.getData() + inArray.size());
		}
		FORCEINLINE friend RangedForConstIteratorType end(const TArray& inArray)
		{
			return RangedForConstIteratorType(inArray.mArrayNum, inArray.getData() + inArray.size());
		}
#else
		FORCEINLINE friend RangedForIteratorType begin(TArray& inArray)
		{
			return inArray.getData();
		}

		FORCEINLINE friend RangedForConstIteratorType begin(const TArray& inArray)
		{
			return inArray.getData();
		}

		FORCEINLINE friend RangedForIteratorType end(TArray& inArray)
		{
			return inArray.getData() + inArray.size();
		}

		FORCEINLINE friend RangedForConstIteratorType end(const TArray& inArray)
		{
			return inArray.getData() + inArray.size();
		}

#endif
		FORCEINLINE void erase(TIterator& it)
		{
			this->rangeCheck(it.mIndex)
				destructItems(getData() + it.mIndex, 1);
			if (it.mIndex < mArrayNum - 1)
			{
				Memory::memmove(&getData()[it.mIndex], &getData()[it.mIndex + 1], sizeof(ElementType));
			}
			mArrayNum--;
		}

		template<typename OtherElementType, typename OtherAllocator>
		ARGESSIVE_ARRAY_FORCEINLINE void append(const TArray<OtherElementType, OtherAllocator>& source)
		{
			BOOST_ASSERT((void*)this != (void*)&source);
			int32 sourceCount = source.size();
			if (!sourceCount)
			{
				return;
			}
			reserve(mArrayNum + sourceCount);
			constructItems<ElementType>(getData() + mArrayNum, source.getData(), sourceCount);
			mArrayNum += sourceCount;
		}

		template<typename OtherElementType, typename OtherAllocator>
		ARGESSIVE_ARRAY_FORCEINLINE void append(TArray<OtherElementType, OtherAllocator>&& source)
		{
			BOOST_ASSERT((void*)this != (void*)&source);
			int32 sourceCount = source.size();
			if (!sourceCount)
			{
				return;
			}
			reserve(mArrayNum + sourceCount);
			relocateConstructItems<ElementType>(getData() + mArrayNum, source.getData(), sourceCount);
			source.mArrayNum = 0;

			mArrayNum += sourceCount;
		}

		ARGESSIVE_ARRAY_FORCEINLINE void append(const ElementType* ptr, int32 count)
		{
			BOOST_ASSERT(ptr != nullptr);
			int32 pos = addUninitialized(count);
			constructItems<ElementType>(getData() + pos, ptr, count);
		}

		FORCEINLINE void append(std::initializer_list<ElementType> initList)
		{
			int32 count = (int32)initList.size();
			int32 pos = addUninitialized(count);
			constructItems<ElementType>(getData() + pos, initList.begin(), count);
		}

		FORCEINLINE bool isValidIndex(int32 index) const
		{
			return index >= 0 && index < mArrayNum;
		}

		FORCEINLINE void swapMemory(int32 firstIndexToSwap, int32 secondIndexToSwap)
		{
			Memory::memswap((uint8*)mAllocatorInstance.getAllocation() + (sizeof(ElementType)*firstIndexToSwap), (uint8*)mAllocatorInstance.getAllocation() + (sizeof(ElementType)*secondIndexToSwap), sizeof(ElementType));
		}

		FORCEINLINE void swap(int32 firstindexToSwap, int32 secondIndexToSwap)
		{
			BOOST_ASSERT((firstindexToSwap >= 0) && (secondIndexToSwap >= 0));
			BOOST_ASSERT((mArrayNum > firstindexToSwap) && (mArrayNum > secondIndexToSwap));
			if (firstindexToSwap != secondIndexToSwap)
			{
				swapMemory(firstindexToSwap, secondIndexToSwap);
			}
		}

		void setNum(int32 newNum, bool bAllowShrinking = true)
		{
			if (newNum > mArrayNum)
			{
				const int32 diff = newNum - mArrayNum;
				const int32 index = addUninitialized(diff);
				defaultConstructItems<ElementType>((uint8*)mAllocatorInstance.getAllocation() + index * sizeof(ElementType), diff);
			}
			else if (newNum < mArrayNum)
			{
				removeAt(newNum, mArrayNum - newNum, bAllowShrinking);
			}
		}

		template<class PREDICATE_CLASS>
		int32 removeAll(const PREDICATE_CLASS& predicate)
		{
			const int32 originalNum = mArrayNum;
			if (!originalNum)
			{
				return 0;
			}

			int32 writeIndex = 0;
			int32 readIndex = 0;
			bool notMatch = !predicate(getData()[readIndex]);
			do 
			{
				int32 runStartIndex = readIndex;
				while (readIndex < originalNum && notMatch == !predicate(getData()[readIndex]))
				{
					readIndex++;
				}
				int32 runLength = readIndex - runStartIndex;
				BOOST_ASSERT(runLength > 0);
				if (notMatch)
				{
					if (writeIndex != runStartIndex)
					{
						Memory::memmove(&getData()[writeIndex], &getData()[runStartIndex], sizeof(ElementType)* runLength);

					}
					writeIndex += runLength;
				}
				else
				{
					destructItems(getData() + runStartIndex, runLength);
				}
				notMatch = !notMatch;
			} while (readIndex < originalNum);
			mArrayNum = writeIndex;
			return originalNum - mArrayNum;
		}

		int32 remove(const ElementType& item)
		{
			checkAddress(&item);
			return removeAll([&item](ElementType& element) {return element == item; });
		}

	private:
		void removeAtImpl(int32 index, int32 count, bool bAllowShrinking)
		{
			if (count)
			{
				checkInvariants();
				BOOST_ASSERT((count >= 0) & (index >= 0) & (index + count <= mArrayNum));
				destructItems(getData() + index, count);
				int32 numToMove = mArrayNum - index - count;
				if (numToMove)
				{
					Memory::memmove((uint8*)mAllocatorInstance.getAllocation() + (index) * sizeof(ElementType), (uint8*)mAllocatorInstance.getAllocation() + (index + count) * sizeof(ElementType), numToMove * sizeof(ElementType));
				}
				mArrayNum -= count;
				if (bAllowShrinking)
				{
					resizeShrink();
				}
			}
		}

	public:

		
		FORCEINLINE int32 addUnique(InElementType&& item)
		{
			return addUniqueImpl(std::move(item));
		}

		FORCEINLINE int32 addUnique(const InElementType& item)
		{
			return addUniqueImpl(item);
		}

		FORCEINLINE int32 add(ElementType&& item) 
		{
			checkAddress(&item);
			return emplace(std::move(item));
		}

		FORCEINLINE int32 add(const ElementType& item)
		{
			checkAddress(&item);
			return emplace(item);
		}

		FORCEINLINE void shrink()
		{
			checkInvariants();
			if (mArrayMax != mArrayNum)
			{
				resizeTo(mArrayNum);
			}
		}
	

		FORCEINLINE void checkInvariants() const
		{
			BOOST_ASSERT((this->size() >= 0));
		}

		FORCEINLINE int32 addUninitialized(int32 count = 1)
		{
			checkInvariants();
			BOOST_ASSERT(count >= 0);
			const int32 oldNum = mArrayNum;
			if ((mArrayNum += count) > mArrayMax)
			{
				resizeGrow(oldNum);
			}
			return oldNum;
		}

		FORCEINLINE int32 addDefaulted(int32 count = 1)
		{
			const int32 index = addUninitialized(count);
			defaultConstructItems<ElementType>((uint8*)mAllocatorInstance.getAllocation() + index * sizeof(ElementType), count);
			return index;
		}

		FORCEINLINE int32 addZeroed(int32 count = 1)
		{
			const int32 index = addUninitialized(count);
			Memory::memzero((uint8*)&(*this)[index], count * sizeof(InElementType));
			return index;
		}

		FORCEINLINE uint32 getTypeSize() const
		{
			return sizeof(InElementType);
		}

		
		/*FORCEINLINE void removeAtSwap(int32 index)
		{

		}*/

		friend Archive& operator << (Archive& ar, TArray& a)
		{
			a.countBytes(ar);
			if (sizeof(ElementType) == 1)
			{
				ar << a.mArrayNum;
				BOOST_ASSERT(a.mArrayNum >= 0);
				if ((a.mArrayNum || a.mArrayMax) && ar.isLoading())
				{
					a.resizeForCopy(a.mArrayNum, a.mArrayMax);
				}
				ar.serialize(a.getData(), a.size());
			}
			else if (ar.isLoading())
			{
				int32 newNum;
				ar << newNum;
				a.empty(newNum);
				for (int32 i = 0; i < newNum; i++)
				{
					ar << *::new(a)ElementType;
				}
			}
			else
			{
				ar << a.mArrayNum;
				for (int32 i = 0; i < a.mArrayNum; i++)
				{
					ar << a[i];
				}
			}
			return ar;
		}

		void countBytes(Archive& ar)
		{

		}
		

		FORCEINLINE void removeAt(int32 index)
		{
			removeAtImpl(index, 1, true);
		}
		 
		int32 findLast(const ElementType& item) const
		{
			for (const ElementType* RESTRICT start = getData(), *RESTRICT data = start + mArrayNum; data != start;)
			{
				--data;
				if (*data == item)
				{
					return static_cast<int32>(data - start);
				}
			}
			return INDEX_NONE;
		}

		FORCEINLINE bool findLast(const ElementType& item, int32& index) const
		{
			index = this->findLast(item);
			return index != INDEX_NONE;
		}

		template<typename CountType>
		FORCEINLINE void removeAt(int32 index, CountType count = 1, bool bAllowShrinking = true)
		{
			static_assert(!std::are_types_equal<CountType, bool>::value, "TArray::removeAt: unexpected bool passed as the count argument");
			removeAtImpl(index, count, bAllowShrinking);
		}

		FORCEINLINE bool find(const ElementType& item, int32& index)const
		{
			index = this->find(item);
			return index != INDEX_NONE;
		}


		ARGESSIVE_ARRAY_FORCEINLINE int32 find(const InElementType & value) const
		{
			const ElementType* RESTRICT start = getData();
			for (const ElementType* RESTRICT data = start, *RESTRICT dataEnd = data + mArrayNum; data != dataEnd; ++data)
			{
				if (*data == value)
				{
					return static_cast<int32>(data - start);
				}
			}
			return INDEX_NONE;
		}

		int32 insert(std::initializer_list<ElementType> initList, const int32 inIndex)
		{
			insertUninitialized(inIndex, (int32)initList.size());
			ElementType* data = (ElementType*)mAllocatorInstance.getAllocation();
			int32 index = inIndex;
			for (const ElementType& element : initList)
			{
				new (data + index++)ElementType(element);
			}
			return inIndex;
		}

		int32 insert(const TArray<ElementType>& items, const int32 inIndex)
		{
			BOOST_ASSERT(this != &items);
			insertUninitialized(inIndex, items.size());
			ElementType* data = (ElementType*)mAllocatorInstance.getAllocation();
			int32 index = inIndex;
			for (auto it = items.createConstIterator(); it; ++it)
			{
				new (data + index++)ElementType(std::move(*it));
			}
			return inIndex;
		}

		int32 insert(const ElementType* ptr, int32 count, int32 index)
		{
			BOOST_ASSERT(ptr != nullptr);
			insertUninitialized(index, count);
			constructItems<ElementType>(getData() + index, ptr, count);
			return index;
		}


		int32 insert(ElementType&& item, int32 index)
		{
			checkAddress(&item);
			insertUninitialized(index, 1);
			new(getData() + index)ElementType(std::move(item));
			return index;
		}

		int32 insert(const ElementType& item, int32 index)
		{
			checkAddress(&item);
			insertUninitialized(index, 1);
			new(getData() + index)ElementType(item);
			return index;
		}

		template<typename CountType>
		FORCEINLINE void removeAtSwap(int32 it, CountType count, bool bAllowShrinking = true)
		{
			static_assert(!std::are_types_equal<CountType, bool>::value, "TArray::removeAtSwap: unexpected bool passed as the count argument");
			removeAtSwapImpl(it, count, bAllowShrinking);
		}

		FORCEINLINE void removeAtSwap(int32 index)
		{
			removeAtSwapImpl(index, 1, true);
		}

		void sort()
		{
			Sorting::sort(getData(), size());
		}

		template<class PREDICATE_CLASS>
		void sort(const PREDICATE_CLASS& predicate)
		{
			Sorting::sort(getData(), size(), predicate);
		}

		FORCEINLINE uint32 getAllocatedSize(void) const
		{
			return mAllocatorInstance.getAllocatedSize(mArrayMax, sizeof(ElementType));
		}

		int32 removeSingle(const ElementType& item)
		{
			int32 index = find(item);
			if (index == INDEX_NONE)
			{
				return 0;
			}
			auto * removePtr = getData() + index;
			destructItems(removePtr, 1);
			const int32 nextIndex = index + 1;
			relocateConstructItems<ElementType>(removePtr, removePtr + 1, mArrayNum - (index + 1));
			--mArrayNum;
			return 1;
		}

		int32 removeSingleSwap(const InElementType& item, bool bAllowShrinking = true)
		{
			auto it = find(item);
			if (it == INDEX_NONE)
			{
				return 0;
			}
			removeAtSwap(it, 1, bAllowShrinking);
			return 1;
		}

		int32 removeSwap(const ElementType& item)
		{
			checkAddress(&item);
			const int32 originalNum = mArrayNum;
			for (int32 index = 0; index < mArrayNum; index++)
			{
				if ((*this)[index] == item)
				{
					removeAtSwap(index--);
				}
			}
			return originalNum - mArrayNum;
		}

		template<typename ComparisonType>
		FORCEINLINE bool contains(const ComparisonType& item) const
		{
			for (const ElementType* RESTRICT data = getData(), *RESTRICT dataEnd = data + mArrayNum; data != dataEnd; ++data)
			{
				if (*data == item)
				{
					return true;
				}
			}
			return false;
		}

		template<typename Predicate>
		FORCEINLINE const ElementType* findByPredicate(Predicate pred) const
		{
			return const_cast<TArray*>(this)->findByPredicate(pred);
		}

		template<typename Predicate>
		ElementType* findByPredicate(Predicate pred)
		{
			for (ElementType* RESTRICT data = getData(), *RESTRICT dataEnd = data + mArrayNum; data != dataEnd; ++data)
			{
				if (pred(*data))
				{
					return data;
				}
			}
			return nullptr;
		}

		template<typename Predicate> 
		FORCEINLINE bool containsByPredicate(Predicate pred) const
		{
			return findByPredicate(pred) != nullptr;
		}

		FORCEINLINE void resizeShrink()
		{
			const int32 newArrayMax = mAllocatorInstance.calculateSlackShrink(mArrayNum, mArrayMax, sizeof(ElementType));
			if (newArrayMax != mArrayMax)
			{
				mArrayMax = newArrayMax;
				BOOST_ASSERT(mArrayMax >= mArrayNum);
				mAllocatorInstance.resizeAllocation(mArrayNum, mArrayMax, sizeof(ElementType));
			}
		}

		FORCEINLINE void resizeGrow(int32 oldNum)
		{
			mArrayMax = mAllocatorInstance.calculateSlackGrow(mArrayNum, mArrayMax, sizeof(ElementType));
			mAllocatorInstance.resizeAllocation(oldNum, mArrayMax, sizeof(ElementType));
		}

		void insertUninitialized(int32 index, int32 count = 1)
		{
			checkInvariants();
			BOOST_ASSERT((count >= 0) & (index >= 0)&(index <= mArrayNum));
			const int32 oldNum = mArrayNum;
			if ((mArrayNum += count) > mArrayMax)
			{
				resizeGrow(oldNum);
			}
			ElementType* data = getData() + index;
			relocateConstructItems<ElementType>(data + count, data, oldNum - index);
		}

		FORCEINLINE int32 size() const
		{
			return mArrayNum;
		}

		FORCEINLINE ElementType& operator [](int32 index)
		{
			rangeCheck(index);
			return getData()[index];
		}

		FORCEINLINE const ElementType& operator [](int32 index) const
		{
			rangeCheck(index);
			return getData()[index];

		}

		ARGESSIVE_ARRAY_FORCEINLINE void setNumUninitialized(int32 newNum, bool bAllowShrinking = true)
		{
			if (newNum > size())
			{
				addUninitialized(newNum - size());
			}
			else if (newNum < size())
			{
				removeAt(newNum, size() - newNum, bAllowShrinking);
			}
		}

		FORCEINLINE ElementType pop(bool bAllowShrinking = true)
		{
			rangeCheck(0);
			ElementType result = std::move(getData()[mArrayNum - 1]);
			removeAt(mArrayNum - 1, 1, bAllowShrinking);
			return result;
		}

		FORCEINLINE void push(ElementType&& item)
		{
			add(std::move(item));
		}

		FORCEINLINE void push(const ElementType& item)
		{
			add(item);
		}


		FORCEINLINE void checkAddress(const ElementType* addr) const
		{
			BOOST_ASSERT(addr < getData() || addr >= (getData() + mArrayMax));
		}

		template<typename... ArgsType>
		FORCEINLINE int32 emplace(ArgsType&&... args)
		{
			const int32 index = addUninitialized(1);
			new(getData() + index) ElementType(std::forward<ArgsType>(args)...);
			return index;
		}

		void insertZeroed(int32 index, int32 count = 1)
		{
			insertUninitialized(index, count);
			Memory::memzero((uint8*)mAllocatorInstance.getAllocation() + index * sizeof(ElementType), count * sizeof(ElementType));
		}


		FORCEINLINE int32 push_back(const ElementType& item)
		{
			checkAddress(&item);
			return emplace(item);
		}

		FORCEINLINE int32 push_back(ElementType&& item)
		{
			checkAddress(&item);
			return emplace(std::move(item));
		}

		FORCEINLINE ElementType& last(int32 indexFromTheEnd = 0)
		{
			rangeCheck(mArrayNum - indexFromTheEnd - 1);
			return getData()[mArrayNum - indexFromTheEnd - 1];
		}

		FORCEINLINE const ElementType& last(int32 indexFromTheEnd = 0) const
		{
			rangeCheck(mArrayNum - indexFromTheEnd - 1);
			return getData()[mArrayNum - indexFromTheEnd - 1];
		}

		FORCEINLINE ElementType& top()
		{
			return last();
		}

		ARGESSIVE_ARRAY_FORCEINLINE void reserve(int32 slack = 0)
		{
			if (mArrayMax != slack)
			{
				resizeTo(slack);
			}
		}

		ARGESSIVE_ARRAY_FORCEINLINE void empty(int32 slack = 0)
		{
			destructItems(getData(), mArrayNum);
			BOOST_ASSERT(slack >= 0);
			mArrayNum = 0;
			if (mArrayMax != slack)
			{
				resizeTo(slack);
			}
		}

		FORCEINLINE void resizeTo(int32 newMax)
		{
			if (newMax)
			{
				newMax = mAllocatorInstance.calculateSlackReserve(newMax, sizeof(ElementType));
			}
			if (newMax != mArrayMax)
			{
				mArrayMax = newMax;
				mAllocatorInstance.resizeAllocation(mArrayNum, mArrayMax, sizeof(ElementType));
			}
		}

		ARGESSIVE_ARRAY_FORCEINLINE void clear()
		{
			destructItems(getData(), mArrayNum);
			mArrayNum = 0;
		}

		FORCEINLINE ElementType& back()
		{
			return last();
		}

		FORCEINLINE const ElementType* getData() const
		{
			return (const ElementType*)mAllocatorInstance.getAllocation();
		}

		ARGESSIVE_ARRAY_FORCEINLINE bool operator == (const TArray& otherArray) const
		{
			int32 count = size();
			return count == otherArray.size() && compareItems(getData(), otherArray.getData(), count);
		}

		ARGESSIVE_ARRAY_FORCEINLINE TArray& operator=(std::initializer_list<InElementType> initList)
		{
			destructItems(getData(), mArrayNum);
			copyToEmpty(initList.begin(), (int32)initList.size(), mArrayMax, 0);
			return *this;
		}

		template<typename OtherAllocator>
		ARGESSIVE_ARRAY_FORCEINLINE TArray& operator=(const TArray<ElementType, OtherAllocator>& other)
		{
			destructItems(getData(), mArrayNum);
			copyToEmpty(other.getData(), other.size(), mArrayMax, 0);
			return *this;
		}

		ARGESSIVE_ARRAY_FORCEINLINE TArray& operator=(const TArray& other)
		{
			if (this != &other)
			{
				destructItems(getData(), mArrayNum);
				copyToEmpty(other.getData(), other.size(), mArrayMax, 0);
			}
			return *this;
		}

		FORCEINLINE ElementType* getData()
		{
			return (ElementType*)mAllocatorInstance.getAllocation();
		}

		FORCEINLINE size_t getSlack() const
		{
			return mArrayMax - mArrayNum;
		}

		FORCEINLINE size_t max() const
		{
			return mArrayMax;
		}

		FORCEINLINE ElementType* data()
		{
			return (ElementType*)mAllocatorInstance.getAllocation();
		}

		FORCEINLINE const ElementType* data() const
		{
			return (ElementType*)mAllocatorInstance.getAllocation();
		}

		FORCEINLINE void rangeCheck(int32 index) const
		{
			checkInvariants();
			if (Allocator::RequireRangeCheck)
			{
				BOOST_ASSERT((index >= 0) & (index < mArrayNum));
			}
		}

		ARGESSIVE_ARRAY_FORCEINLINE void reset(int32 newSize = 0)
		{
			if (newSize <= mArrayMax)
			{
				destructItems(getData(), mArrayNum);
				mArrayNum = 0;
			}
			else
			{
				empty(newSize);
			}
		}
	private:

		

		template<typename ArgsType>
		int32 addUniqueImpl(ArgsType&& args)
		{
			int32 index;
			if (find(args, index))
			{
				return index;
			}
 			return add(std::forward<ArgsType>(args));
		}

		
		FORCEINLINE void removeAtSwapImpl(int32 index, int32 count = 1, bool bAllowShrinking = true)
		{
			if (count)
			{
				checkInvariants();
				BOOST_ASSERT((count >= 0) & (index >= 0) & (index + count <= mArrayNum));
				destructItems(getData() + index, count);
				const int32 numElemetsInHole = count;
				const int32 numElementsAfterHole = mArrayNum - (index + count);
				const int32 numElementsToMoveIntoHole = PlatformMath::min(numElemetsInHole, numElementsAfterHole);
				if (numElementsToMoveIntoHole)
				{
					Memory::memcpy((uint8*)mAllocatorInstance.getAllocation() + (index) * sizeof(ElementType), (uint8*)mAllocatorInstance.getAllocation() + (index + count) * sizeof(ElementType), numElementsToMoveIntoHole * sizeof(ElementType));
				}
				mArrayNum -= count;
				if (bAllowShrinking)
				{
					resizeShrink();
				}
			}
		}

		protected:
			typedef typename TChooseClass<Allocator::NeedsElementType || !std::is_pod<ElementType>::value, typename Allocator::template ForElementType<ElementType>,
				typename Allocator::ForAnyElementType>::Result ElementAllocatorType;
			ElementAllocatorType mAllocatorInstance;
			int32 mArrayNum;
			int32 mArrayMax;
	};



}
namespace std
{
	template<typename InElementType, typename Allocator>
	struct TContainerTraits<Air::TArray<InElementType, Allocator>> : public TContainerTraitsBase<Air::TArray<InElementType, Allocator>>
	{
		static_assert(TAllocatorTraits<Allocator>::SupportsMove, "TArray no longer supports move-unaware allocators");
		enum { MoveWillEmptyContainer = TAllocatorTraits<Allocator>::SupportsMove };
	};
}

template<typename T, typename Allocator> void* operator new(size_t size, Air::TArray<T, Allocator>& inArray)
{
	BOOST_ASSERT(size == sizeof(T));
	const int32 index = inArray.addUninitialized(1);
	return &inArray[index];
}

template<typename T, typename Allocator> void* operator new (size_t size, Air::TArray<T, Allocator>& inArray, int32 index)
{
	BOOST_ASSERT(size == sizeof(T));
	inArray.insertUninitialized(index, 1);
	return &inArray[index];
}