#pragma once
#include "Containers/Array.h"
namespace Air
{
	template<typename T, typename Allocator = DefaultAllocator>
	class TindirectArray
	{
	public:
		typedef T			ElementType;
		typedef TArray<void*, Allocator> InternalArrayType;
#if PLATFORM_COMPILER_HAS_DEFAULTED_FUNCTIONS 
		TindirectArray() = default;
		TindirectArray(TindirectArray&&) = default;
		TindirectArray& operator = (TindirectArray&&) = default;
#else
		TindirectArray() {}
		FORCEINLINE TindirectArray(TindirectArray&& other) : mArray(std::move(other.mArray)) {}
		FORCEINLINE TindirectArray& operator = (TindirectArray&& other) {
			mArray = std::move(other.mArray); return *this;
		}
#endif
		~TindirectArray()
		{
			empty();
		}

		FORCEINLINE int32 size() const
		{
			return mArray.size();
		}

		FORCEINLINE T** getData()
		{
			return (T**)mArray.getData();
		}

		FORCEINLINE const T** getData() const
		{
			return (const T**)mArray.getData();
		}

		void shrink()
		{
			mArray.shrink();
		}

		void removeAt(int32 index, int32 count = 1, bool bAllowShrinking = true)
		{
			BOOST_ASSERT(index >= 0);
			BOOST_ASSERT(index <= mArray.size());
			BOOST_ASSERT(index + count < mArray.size());
			T** element = getData() + index;
			for (int32 elementId = count; elementId; --elementId)
			{
				typedef T IndirectArrayDestructElementType;
				(*element)->IndirectArrayDestructElementType::~IndirectArrayDestructElementType();
				Memory::free(*element);
				++element;
			}
			mArray.removeAt(index, count, bAllowShrinking);
		}


		void empty(int32 slack = 0)
		{
			destructAndFreeItems();
			mArray.empty(slack);
		}

		FORCEINLINE ElementType& last(int32 indexFromTheEnd = 0)
		{
			return *(T*)mArray.last(indexFromTheEnd);
		}

		FORCEINLINE int32 add(T* item)
		{
			return mArray.add(item);
		}

		FORCEINLINE T& operator[](int32 index)
		{
			return *(T*)mArray[index];
		}

		FORCEINLINE const T& operator[](int32 index) const
		{
			return *(T*)mArray[index];
		}

		FORCEINLINE void insert(T* item, int32 index)
		{
			mArray.insert(item, index);
		}

	private:
		void destructAndFreeItems()
		{
			T** element = getData();
			for (int32 index = mArray.size(); index; --index)
			{
				typedef T IndirectArrayDestructElementType;
				(*element)->IndirectArrayDestructElementType::~IndirectArrayDestructElementType();
				Memory::free(*element);
				++element;
			}
		}

	private:
		FORCEINLINE friend TDereferencingIterator<ElementType, typename InternalArrayType::RangedForIteratorType> begin(TindirectArray& indirectArray) { return TDereferencingIterator<ElementType, typename InternalArrayType::RangedForIteratorType>(begin(indirectArray.mArray)); }

		FORCEINLINE friend TDereferencingIterator<const ElementType, typename InternalArrayType::RangedForConstIteratorType> begin(const TindirectArray& indirectArray) { return TDereferencingIterator<const ElementType, typename InternalArrayType::RangedForConstIteratorType>(begin(indirectArray.mArray)); }

		FORCEINLINE friend TDereferencingIterator<ElementType, typename InternalArrayType::RangedForIteratorType> end(TindirectArray& indirectArray) { return TDereferencingIterator<ElementType, typename InternalArrayType::RangedForIteratorType>(end(indirectArray.mArray)); }

		FORCEINLINE friend TDereferencingIterator<const ElementType, typename InternalArrayType::RangedForConstIteratorType> end(const TindirectArray& indirectArray) { return TDereferencingIterator<const ElementType, typename InternalArrayType::RangedForConstIteratorType>(end(indirectArray.mArray)); }



		InternalArrayType mArray;
	};
}

template<typename T, typename Allocator> void* operator new(size_t size, Air::TindirectArray<T, Allocator>& inArray)
{
	BOOST_ASSERT(size == sizeof(T));
	const int32 index = inArray.add((T*)Memory::malloc(size));
	return &inArray[index];
}

template<typename T, typename Allocator> void* operator new(size_t size, Air::TindirectArray<T, Allocator>& inArray, int32 index)
{
	BOOST_ASSERT(size == sizeof(T));
	inArray.insert((T*)Memory::malloc(size), index);
	return &inArray[index];
}