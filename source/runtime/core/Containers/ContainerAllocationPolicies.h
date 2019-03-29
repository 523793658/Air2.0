#pragma once
#include "CoreType.h"
#include "Template/TypeCompatibleBytes.h"
#include "HAL/AirMemory.h"
#include "boost/assert.hpp"
namespace Air
{

	template<typename ReferencedType>
	ReferencedType* ifAThenAElseB(ReferencedType* A, ReferencedType* B);


	template<typename PredicateType, typename ReferencedType>
	ReferencedType* ifAThenAElseB(PredicateType predicate, ReferencedType* A, ReferencedType* B);

	template <uint32 NumInlineElements>
	class TFixedAllocator
	{
	public:
		enum { NeedsElementType = true };
		enum { RequireRangeCheck = true };
		template<typename ElementType>
		class ForElementType
		{
		public:	 
			ForElementType()
			{}
			FORCEINLINE void moveToEmpty(ForElementType& other)
			{
				BOOST_ASSERT(this != &other);
				relocateConstructItems<ElementType>((void*)mInlineData, other.getInlineElements(), NumInlineElements);
			}

			FORCEINLINE ElementType* getAllocation() const
			{
				return getInlineElements();
			}

			void resizeAllocation(int32 previousNumElements, int32 numElements, SIZE_T numBytesPerElement)
			{
				BOOST_ASSERT(numElements <= NumInlineElements);
			}

			FORCEINLINE int32 calculateSlackReserve(int32 numElements, SIZE_T numBytesperElement)
			{
				BOOST_ASSERT(numElements <= NumInlineElements);
				return NumInlineElements;
			}

			FORCEINLINE int32 calculateSlackShrink(int32 numElements, int32 numAllocatedElements, int32 numBytesPerElements) const
			{
				BOOST_ASSERT(numAllocatedElements <= NumInlineElements);
				return NumInlineElements;
			}
			FORCEINLINE int32 calculateSlackGrow(int32 numElements, int32 numAllocatedElements, int32 numBytesPerElement) const
			{
				BOOST_ASSERT(numElements <= NumInlineElements);
				return NumInlineElements;
			}
			SIZE_T getAllocatedSize(int32 NumAllocatedElements, SIZE_T numBytesPerElement) const
			{
				return 0;
			}
			bool hasAllocation()
			{
				return false;
			}
		private:
			ForElementType(const ForElementType&);
			ForElementType& operator = (const ForElementType&);
			TTypeCompatibleBytes<ElementType> mInlineData[NumInlineElements];
			ElementType* getInlineElements() const
			{
				return (ElementType*)mInlineData;
			}
		};
		typedef void ForAnyElementType;

	};
	class DefaultAllocator;

	template<uint32 NumInlineElements, typename SecondaryAllocator = DefaultAllocator>
	class TInlineAllocator
	{
	public:
		enum { NeedsElementType = true };
		enum { RequireRangeCheck = true };

		template<typename ElementType>
		class ForElementType
		{
		public:
			ForElementType()
			{}

			FORCEINLINE void moveToEmpty(ForElementType other)
			{
				BOOST_ASSERT(this != &other);
				if(!other.mSecondaryData.)
			}
			FORCEINLINE ElementType* getAllocation() const
			{
				return ifAThenAElseB<ElementType>(mSecondaryData.getAllocation(), getInlineElements());
			}

		private:
			ForElementType(const ForElementType&);
			ForElementType& operator = (const ForElementType&);

			TTypeCompatibleBytes<ElementType> mInlineData[NumInlineElements];
			typename SecondaryAllocator::template ForElementType<ElementType> mSecondaryData;
		public:
			ElementType* getInlineElements() const
			{
				return (ElementType*)mInlineData;
			}

			void resizeAllocation(int32 previousNumElements, int32 numElements, SIZE_T numBytesPerElement)
			{
				if (numElements <= NumInlineElements)
				{
					if (mSecondaryData.getAllocation())
					{
						relocateConstructItems<ElementType>((void*)mInlineData, (ElementType*)mSecondaryData.getAllocation(), previousNumElements);
						mSecondaryData.resizeAllocation(0, 0, numBytesPerElement);
					}
				}
				else
				{
					if (!mSecondaryData.getAllocation())
					{
						mSecondaryData.resizeAllocation(0, numElements, numBytesPerElement);
						relocateConstructItems<ElementType>((void*)mSecondaryData.getAllocation(), getInlineElements(), previousNumElements);
					}
					else
					{
						mSecondaryData.resizeAllocation(previousNumElements, numElements, numBytesPerElement);
					}
				}
			}

			FORCEINLINE int32 calculateSlackGrow(int32 numElements, int32 numAllocatedElements, int32 numBytesPerElement) const
			{
				return numElements <= NumInlineElements ? NumInlineElements : mSecondaryData.calculateSlackGrow(numElements, numAllocatedElements, numBytesPerElement);
			}

			FORCEINLINE int32 calculateSlackReserve(int32 numElements, SIZE_T numBytesPerElement)
			{
				return numElements <= NumInlineElements ? NumInlineElements : mSecondaryData.calculateSlackReserve(numElements, numBytesPerElement);
			}

			FORCEINLINE int32 calculateSlackShrink(int32 numElements, int32 numAllocatedElements, int32 numBytesPerElement) const
			{
				return numElements <= NumInlineElements ? NumInlineElements : mSecondaryData.calculateSlackShrink(numElements, numAllocatedElements, numBytesPerElement);
			}
		};

		typedef void ForAnyElementType;
	};

	FORCEINLINE int32 defautCalculateSlackReserve(int32 numElements, SIZE_T bytesPerElement, bool bAllowQuantize, uint32 alignment = DEFAULT_ALIGNMENT)
	{
		int32 retval = numElements;
		BOOST_ASSERT(numElements > 0);
		if (bAllowQuantize)
		{
			retval = Memory::quantizeSize(SIZE_T(retval) * SIZE_T(bytesPerElement), alignment) / bytesPerElement;
			if (numElements > retval)
			{
				retval = std::numeric_limits<int32>::max();
			}
		}
		return retval;
	}


	FORCEINLINE int32 defaultCalculateSlackGrow(int32 numElements, int32 numAllocatedElements, SIZE_T bytesPerElement, bool bAllowQuantize, uint32 alignment = DEFAULT_ALIGNMENT)
	{
		int32 retvel;
		BOOST_ASSERT(numElements > numAllocatedElements && numElements > 0);
		SIZE_T grow = 4;
		if (numAllocatedElements || SIZE_T(numElements) > grow)
		{
			grow = SIZE_T(numElements) + 3 * SIZE_T(numElements) / 8 + 16;
		}
		if (bAllowQuantize)
		{
			retvel = Memory::quantizeSize(grow * bytesPerElement, alignment) / bytesPerElement;
		}
		else
		{
			retvel = grow;
		}
		if (numElements > retvel)
		{
			retvel = std::numeric_limits<int32>::max();
		}
		return retvel;
	}

	FORCEINLINE int32 defaultCalculateSlackShrink(int32 numElements, int32 numAllocatedElements, SIZE_T bytesPerElement, bool bAllowQuantize, uint32 aligment = DEFAULT_ALIGNMENT)
	{
		int32 retvel;
		BOOST_ASSERT(numElements < numAllocatedElements);
		const uint32 currentSlackElements = numAllocatedElements - numElements;
		const SIZE_T currentSlackBytes = currentSlackElements * bytesPerElement;
		const bool bTooManySlackBytes = currentSlackBytes >= 16384;
		const bool bTooManySlackElements = 3 * numElements < 2 * numAllocatedElements;
		if ((bTooManySlackBytes || bTooManySlackElements) && (currentSlackElements > 64 || !numElements))
		{
			retvel = numElements;
			if (retvel > 0)
			{
				if (bAllowQuantize)
				{
					retvel = Memory::quantizeSize(retvel * bytesPerElement, aligment) / bytesPerElement;
				}
			}
		}
		else
		{
			retvel = numAllocatedElements;
		}
		return retvel;
	}

	FORCEINLINE int32 defaultCalculateSlackReserve(int32 numElements, SIZE_T bytesPerElement, bool bAllowQuantize, uint32 alignment = DEFAULT_ALIGNMENT)
	{
		int32 retval = numElements;
		BOOST_ASSERT(numElements > 0);
		if (bAllowQuantize)
		{
			retval = Memory::quantizeSize(SIZE_T(retval) * SIZE_T(bytesPerElement), alignment) / bytesPerElement;
			if (numElements > retval)
			{
				retval = std::numeric_limits<int32>::max();
			}
		}
		return retval;
	}
#define NumBitsPerDWORD ((int32)32)
#define NumBitsPerDWORDLogTwo ((int32)5)

	class DefaultBitArrayAllocator;


	class DefaultBitArrayAllocator : public TInlineAllocator<4> { public: typedef TInlineAllocator<4> Typedef; };

	template<typename InElementAllocator = DefaultAllocator, typename InBitArrayAllocator = DefaultBitArrayAllocator>
	class TSparseArrayAllocator
	{
	public:
		typedef InElementAllocator ElementAllocator;
		typedef InBitArrayAllocator BitArrayAllocator;
	};

	template<uint32 NumInlineElements,
		typename SecondaryAllocator = TSparseArrayAllocator<DefaultAllocator, DefaultAllocator>>
		class TInlineSparseArrayAllocator
	{
	private:
		enum {
			InlineBitArrayDWORDs = (NumInlineElements + NumBitsPerDWORD - 1) / NumBitsPerDWORD
		};

	public:
		typedef TInlineAllocator<NumInlineElements, typename SecondaryAllocator::ElementAllocator> ElementAllocator;
		typedef TInlineAllocator<InlineBitArrayDWORDs, typename SecondaryAllocator::BitArrayAllocator> BitArrayAllocator;
	};


#define DEFAULT_NUMBER_OF_ELEMENTS_PER_HASH_BUCKET	2
#define DEFAULT_BASE_NUMBER_OF_HASH_BUCKETS			8
#define DEFAULT_MIN_NUMBER_OF_HASHED_ELEMENTS		4



	template<typename InSparseArrayAllocator = TSparseArrayAllocator<>, typename InHashAllocator = TInlineAllocator<1, DefaultAllocator>, uint32 AverageNumberOfElementsPerHashBucket = DEFAULT_NUMBER_OF_ELEMENTS_PER_HASH_BUCKET, uint32 BaseNumberOfHashBuckets = DEFAULT_BASE_NUMBER_OF_HASH_BUCKETS,
		uint32 MinNumberOfHashBuckets = DEFAULT_MIN_NUMBER_OF_HASHED_ELEMENTS>
		class TSetAllocator
	{
	public:
		static FORCEINLINE uint32 getNumberOfHashBuckets(uint32 numHashedElements)
		{
			if (numHashedElements >= MinNumberOfHashBuckets)
			{
				return PlatformMath::roundUpToPowerOfTwo(numHashedElements / AverageNumberOfElementsPerHashBucket + BaseNumberOfHashBuckets);
			}
			return 1;
		}

		typedef InSparseArrayAllocator SparseArrayAllocator;
		typedef InHashAllocator HashAllocator;
	};

	struct ScriptContainerElement
	{
	};


	class CORE_API HeapAllocator
	{
	public:	 
		enum { NeedsElementType = false };
		enum { RequireRangeCheck = true };
		class CORE_API ForAnyElementType
		{
		public:
			ForAnyElementType()
				:mData(nullptr)
			{

			}

			FORCEINLINE void moveToEmpty(ForAnyElementType & other)
			{
				BOOST_ASSERT(this != &other);
				if (mData)
				{
					Memory::free(mData);
				}
				mData = other.mData;
				other.mData = nullptr;
			}

			FORCEINLINE ~ForAnyElementType()
			{
				if (mData)
				{
					Memory::free(mData);
				}
			}

			FORCEINLINE ScriptContainerElement* getAllocation() const
			{
				return mData;
			}

			FORCEINLINE void resizeAllocation(int32 previousNumElements, int32 numElements, SIZE_T numBytesPerElement)
			{
				if (mData || numElements)
				{
					mData = (ScriptContainerElement *)Memory::realloc(mData, numElements * numBytesPerElement);
				}

			}

			FORCEINLINE int32 calculateSlackReserve(int32 numElements, int32 numBytesPerElement)
			{
				return defautCalculateSlackReserve(numElements, numBytesPerElement, true);
			}

			FORCEINLINE int32 calculateSlackGrow(int32 numElements, int32 numAllocatedElements, int32 numBytesPerElement) const
			{
				return defaultCalculateSlackGrow(numElements, numAllocatedElements, numBytesPerElement, true);
			}

			FORCEINLINE int32 calculateSlackShrink(int32 numElements, int32 numAllocatedElements, int32 numBytesPerElement) const
			{
				return defaultCalculateSlackShrink(numElements, numAllocatedElements, numBytesPerElement, true);
			}

			SIZE_T getAllocatedSize(int32 numAllocatedElements, SIZE_T numBytesPerElement)const
			{
				return numAllocatedElements * numBytesPerElement;
			}

		protected:
			ScriptContainerElement * mData;
		};

		template<typename ElementType>
		class ForElementType : public ForAnyElementType
		{
		public:
			ForElementType()
			{

			}

			FORCEINLINE void resizeAllocation(int32 previousNumElements, int32 numElements, SIZE_T numBytesPerElement)
			{
				void* newData = Memory::malloc(numElements* numBytesPerElement);
				tryMoveConstructItems<ElementType>(newData, (ElementType*)mData, previousNumElements);
				destructItems<ElementType>((ElementType*)mData, previousNumElements);
				Memory::free(mData);
				mData = (ScriptContainerElement*)newData;
			}

			FORCEINLINE ElementType* getAllocation() const
			{
				return (ElementType*)ForAnyElementType::getAllocation();
			}
		};
	};




	class DefaultSetAllocator : public TSetAllocator<>
	{
	public:
		typedef TSetAllocator<> Typedef;
	};

	class DefaultAllocator : public HeapAllocator { public: typedef HeapAllocator Typedef; };

	class DefaultBitArrayAllocation : public TInlineAllocator<4> { public: typedef TInlineAllocator<4> Typedef; };


	class DefaultSparseArrayAllocator : public TSparseArrayAllocator<> { public: typedef TSparseArrayAllocator<> Typedef; };

	template<uint32 Alignment = DEFAULT_ALIGNMENT>
	class TAlignedHeapAllocator
	{
	public:
		enum { NeedsElementType = false };
		enum { RequireRangeCheck = true };
		class ForAnyElementType
		{
		public:
			ForAnyElementType()
				:mData(nullptr)
			{}

			FORCEINLINE void moveToEmpty(ForAnyElementType& other)
			{
				BOOST_ASSERT(this != other);
				if (mData)
				{
#if PLATFORM_HAS_UMA
#else
					Memory::free(mData);
#endif
				}
				mData = other.mData;
				other.mData = nullptr;
			}

			FORCEINLINE ~ForAnyElementType()
			{
				if (mData)
				{
#if PLATFORM_HAS_UMA
#else
					Memory::free(mData);
#endif
				}
			}

			FORCEINLINE ScriptContainerElement* getAllocation() const
			{
				return mData;
			}

			void resizeAllocation(int32 previousNumElements, int32 numElements,
				SIZE_T numBytesPerElement)
			{
				if (mData || numElements)
				{
#if PLATFORM_HAS_UMA

#else
					mData = (ScriptContainerElement*)Memory::realloc(mData, numElements * numBytesPerElement, Alignment);
#endif
				}
			}

			FORCEINLINE int32 calculateSlackReserve(int32 numElements, int32 numBytesPerElement) const
			{
				return defaultCalculateSlackReserve(numElements, numBytesPerElement, true, Alignment);
			}

			FORCEINLINE int32 calculateSlackShrink(int32 numElements, int32 numAllocatedElements, int32 numBytesPerElement) const
			{

				return defaultCalculateSlackShrink(numElements, numAllocatedElements, numBytesPerElement, true, Alignment);
			}

			FORCEINLINE int32 calculateSlackGrow(int32 numElements, int32 numAllocatedElements, int32 numBytesPerElement) const
			{
				return defaultCalculateSlackGrow(numElements, numAllocatedElements, numBytesPerElement, true, Alignment);
			}

			SIZE_T getAllocatedSize(int32 numAllocationElements, SIZE_T numBytesPerElement) const
			{
				return numAllocationElements * numBytesPerElement;
			}

			bool hasAllocation()
			{
				return !!mData;
			}



		private:
			ForAnyElementType(const ForAnyElementType&);
			ForAnyElementType& operator = (const ForAnyElementType&);

			ScriptContainerElement* mData;
		};

		template<typename ElementType>
		class ForElementType : public ForAnyElementType
		{
		public:
			ForElementType() {}

			FORCEINLINE ElementType* getAllocation() const
			{
				return (ElementType*)ForAnyElementType::getAllocation();
			}
			
		};
	};

	//template<uint32 Alignment>
	//struct TAllocatorTraits<TAlignedHeapAllocator<Alignment>> : TAllocatorTraitsBase<TAlignedHeapAllocator<Alignment>>
	//{
	//	enum { SupportsMove = true };
	//	enum { IsZeroConstruct = true };
	//};
}