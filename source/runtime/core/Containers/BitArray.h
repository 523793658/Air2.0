#pragma once
#include "CoreType.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "Math/Math.h"
namespace Air
{
	struct BitSet
	{
		static FORCEINLINE uint32 getAndClearNextBit(uint32& mask)
		{
			const uint32 lowestBitMask = (mask)&(-(int32)mask);
			const uint32 bitIndex = Math::floorLog2(lowestBitMask);
			mask ^= lowestBitMask;
			return bitIndex;
		}
	};


	template<typename Allocator> class TBitArray;

	class BitReference
	{
	public:
		FORCEINLINE BitReference(uint32 & inData, uint32 inMask)
			:mData(inData)
			,mMask(inMask)
		{

		}
		FORCEINLINE operator bool() const
		{
			return (mData & mMask) != 0;
		}

		FORCEINLINE void operator =(const bool newValue)
		{
			if (newValue)
			{
				mData |= mMask;
			}
			else
			{
				mData &= ~mMask;
			}
		}

		FORCEINLINE void atomicSet(const bool newValue)
		{
			if (newValue)
			{
				if (!(mData & mMask))
				{
					while (1)
					{
						uint32 current = mData;
						uint32 desired = current | mMask;
						if (current == desired || PlatformAtomics::interlockedCompareExchange((volatile int32*)&mData, (int32)desired, (int32)current) == (int32)current)
						{
							return;
						}
					}
				}
			}
			else
			{
				if (mData & mMask)
				{
					while (1)
					{
						uint32 current = mData;
						uint32 desired = current &~mMask;
						if (current == desired || PlatformAtomics::interlockedCompareExchange((volatile int32*)&mData, (int32)desired, (int32)current) == (int32)current)
						{
							return;
						}
					}
				}
			}
		}

		FORCEINLINE BitReference & operator =(const BitReference& copy)
		{
			*this = (bool)copy;
			return *this;
		}

	private:  
		uint32& mData;
		uint32 mMask;
	};


	class RelativeBitReference
	{
	public:
		FORCEINLINE explicit RelativeBitReference(int32 bitIndex)
			:mDWORDIndex(bitIndex >> NumBitsPerDWORDLogTwo)
			,mMask(1 << (bitIndex & (NumBitsPerDWORD - 1)))
		{

		}

		int32 mDWORDIndex;
		uint32 mMask;
	};


	class ConstBitReference
	{
	public:	 
		FORCEINLINE ConstBitReference(const uint32 & inData, uint32 inMask)
			:mData(inData),
			mMask(inMask)
		{

		}
		FORCEINLINE operator bool()const
		{
			return (mData & mMask) != 0;
		}

	private:
		const uint32 & mData;
		uint32 mMask;
	};

	template<typename Allocator = DefaultBitArrayAllocator>
	class TBitArray;

	template<typename Allocator>
	class TBitArray
	{
		friend class ScriptBitArray;
	public:
		template<typename>
		friend class TConstSetBitIterator;

		template<typename, typename>
		friend class TConstDualSetBitIterator;

		class Iterator : public RelativeBitReference
		{
		public:
			FORCEINLINE Iterator(TBitArray<Allocator>& inArray, int32 startIndex = 0)
				:RelativeBitReference(startIndex)
				,mArray(inArray)
				,mIndex(startIndex)
			{

			}

			FORCEINLINE Iterator & operator ++()
			{
				mIndex++;
				this->mMask <<= 1;
				if (!this->mMask)
				{
					this->mMask = 1;
					++this->mDWORDIndex;
				}
				return *this;
			}

			FORCEINLINE explicit operator bool() const
			{
				return mIndex < mArray.count();
			}
			FORCEINLINE bool operator !() const
			{
				return !(bool)*this;
			}
			FORCEINLINE BitReference getValue() const { return BitReference(mArray.getData()[this->mDWORDIndex], this->mMask); }
			FORCEINLINE int32 getIndex() const { return mIndex; }
		private:
			TBitArray<Allocator>& mArray;
			int32 mIndex;
		};

		class ConstIterator : public RelativeBitReference
		{
		public:
			FORCEINLINE ConstIterator(const TBitArray<Allocator>& inArray, int32 startIndex = 0)
				:RelativeBitReference(startIndex)
				,mArray(inArray)
				,mIndex(startIndex)
			{

			}

			FORCEINLINE ConstIterator & operator ++()
			{
				++mIndex;
				this->mMask <<= 1;
				if (!this->mMask)
				{
					this->mMask = 1;
					++this->mDWORDIndex;
				}
				return *this;
			}

			FORCEINLINE explicit operator bool() const
			{
				return mIndex < mArray.count();
			}

			FORCEINLINE bool operator !() const
			{
				return !(bool)*this;
			}

			FORCEINLINE ConstBitReference getValue() const { return ConstBitReference(mArray.getData()[this->mDWORDIndex], this->mMask); }
			FORCEINLINE int32 getIndex() const {
				return mIndex;
			}
		private:
			const TBitArray<Allocator>& mArray;
			int32 mIndex;
		};

		explicit TBitArray(const bool value = false, const int32 inNumBits = 0)
			: mNumBits(0)
			, mMaxBits(0)
		{
			init(value, inNumBits);
		}
		FORCEINLINE TBitArray(TBitArray&& other)
		{
			moveOrCopy(*this, other);
		}

		FORCEINLINE TBitArray(const TBitArray& copy)
			:mNumBits(0)
			,mMaxBits(0)
		{
			*this = copy;
		}

		void init(bool value, int32 inNumBits)
		{
			empty(inNumBits);
			if (inNumBits)
			{
				mNumBits = inNumBits;
				Memory::Memset(getData(), value ? 0xff : 0, Math::divideAndRoundUp(mNumBits, NumBitsPerDWORD) * sizeof(uint32));
			}
		}

		void empty(int32 expectedNumBits = 0)
		{
			mNumBits = 0;
			expectedNumBits = Math::divideAndRoundUp(expectedNumBits, NumBitsPerDWORD) * NumBitsPerDWORD;
			if (mMaxBits != expectedNumBits)
			{
				mMaxBits = expectedNumBits;
				realloc(0);
			}

		}

		FORCEINLINE int32 size() const { return mNumBits; }

		int32 findAndSetFirstZeroBit()
		{

			uint32* RESTRICT dwordArray = getData();
			const int32 localNumBits = mNumBits;
			const int32 DwordCount = Math::divideAndRoundUp(localNumBits, NumBitsPerDWORD);
			int32 DwordIndex = 0;
			while (DwordIndex < DwordCount && dwordArray[DwordIndex] == (uint32)-1)
			{
				++DwordIndex;
			}
			if (DwordIndex < DwordCount)
			{
				const uint32 bits = ~(dwordArray[DwordIndex]);
				ASSUME(bits != 0);
				const uint32 lowestBit = (bits) &(-(int32)bits);
				const int32 lowestBitIndex = Math::countTrainlingZeros(bits) + (DwordIndex << NumBitsPerDWORDLogTwo);
				if (lowestBitIndex < localNumBits)
				{
					dwordArray[DwordIndex] |= lowestBit;
					return lowestBitIndex;
				}
			}
			return INDEX_NONE;
		}

		FORCEINLINE TBitArray& operator =(const TBitArray& copy)
		{
			if (this == &copy)
			{
				return *this;
			}
			empty(copy.size());
			mNumBits = mMaxBits = copy.mNumBits;
			if (mNumBits)
			{
				const int32 NumDWORDs = Math::divideAndRoundUp(mMaxBits, NumBitsPerDWORD);
				realloc(0);
				Memory::memcpy(getData(), copy.getData(), NumDWORDs * sizeof(uint32));
			}
			return *this;
		}
		
		int32 add(const bool value)
		{
			const int32 index = mNumBits;
			const bool bReallocate = (mNumBits + 1) > mMaxBits;
			mNumBits++;
			if (bReallocate)
			{
				const uint32 maxDowds = mAllocatorInstance.calculateSlackGrow(Math::divideAndRoundUp(mNumBits, NumBitsPerDWORD),
					Math::divideAndRoundUp(mMaxBits, NumBitsPerDWORD), sizeof(uint32));
				mMaxBits = maxDowds * NumBitsPerDWORD;
				realloc(mNumBits - 1);
			}
			(*this)[index] = value;
			return index;
		}


		FORCEINLINE void realloc(int32 previousNumBits)
		{
			const int32 previousNumDWORDs = Math::divideAndRoundUp(previousNumBits, NumBitsPerDWORD);
			const int32 maxDWORDs = Math::divideAndRoundUp(mMaxBits, NumBitsPerDWORD);
			mAllocatorInstance.resizeAllocation(previousNumBits, maxDWORDs, sizeof(uint32));
			if (maxDWORDs)
			{
				Memory::memzero((uint32*)mAllocatorInstance.getAllocation() + previousNumBits, (maxDWORDs - previousNumBits) * sizeof(uint32));
			}
		}

		FORCEINLINE uint32* getData()
		{
			return (uint32*)mAllocatorInstance.getAllocation();
		}

		FORCEINLINE const uint32* getData() const
		{
			return (uint32*)mAllocatorInstance.getAllocation();
		}

		FORCEINLINE BitReference operator[](int32 index)
		{
			BOOST_ASSERT(index >= 0 && index < mNumBits);
			return BitReference(getData()[index / NumBitsPerDWORD], 1 << (index & (NumBitsPerDWORD - 1)));
		}

		FORCEINLINE const ConstBitReference operator[] (int32 index) const
		{
			BOOST_ASSERT(index >= 0 && index < mNumBits);
			return ConstBitReference(getData()[index / NumPoolBuckets], 1 << (index & (NumBitsPerDWORD - 1)));
		}

		void reset()
		{
			Memory::Memset(getData(), 0, Math::divideAndRoundUp(mNumBits, NumBitsPerDWORD) * sizeof(uint32));
			mNumBits = 0;
		}

		void removeAt(int32 baseIndex, int32 numBitsToRemove = 1)
		{
			BOOST_ASSERT(baseIndex >= 0 && baseIndex + numBitsToRemove <= mNumBits);
			Iterator writeIt(*this);
			for (ConstIterator readIt(*this); readIt; ++readIt)
			{
				if (readIt.getIndex() < baseIndex || readIt.getIndex() >= baseIndex + numBitsToRemove)
				{
					if (writeIt.getIndex() != readIt.getIndex())
					{
						writeIt.getValue() = (bool)readIt.getValue();
					}
					++writeIt;
				}
			}
			mNumBits -= numBitsToRemove;
		}
		void removeAtSwap(int32 baseIndex, int32 numBitsToRemove = 1)
		{
			BOOST_ASSERT(baseIndex >= 0 && baseIndex + numBitsToRemove <= mNumBits);
			if (baseIndex < mNumBits - numBitsToRemove)
			{
				for (int32 index = 0; index < numBitsToRemove; index++)
				{
#if PLATFORM_MAC || PLATFORM_LINUX
#else
					(*this)[baseIndex + index] = (bool)(*this)[mNumBits - numBitsToRemove + index];
#endif
				}
			}
			removeAt(mNumBits - numBitsToRemove, numBitsToRemove);
		}

		uint32 getAllocationSize()const
		{
			return Math::divideAndRoundUp(mMaxBits, NumBitsPerDWORD) * sizeof(uint32);
		}

		void countBytes(Archive& ar)
		{
			ar.countBytes(Math::divideAndRoundUp(mNumBits, NumBitsPerDWORD) * sizeof(uint32), Math.divideAndRoundUp(mMaxBits, NumBitsPerDWORD) * sizeof(uint32));
		}

		int32 find(bool bValue) const
		{
			const uint32 test = bValue ? 0u : (uint32)-1;
			const uint32 *RESTRICT dwordArray = getData();
			const int32 localNumBits = mNumBits;
			const int32 dwordCount = Math::divideAndRoundUp(localNumBits, NumBitsPerDWORD);
			int32 dwordIndex = 0;
			while (dwordIndex < dwordCount && dwordArray[dwordIndex] == test)
			{
				++dwordIndex;
			}
			if (dwordIndex < dwordCount)
			{
				const uint32 bits = bValue ? (dwordArray[dwordIndex]) : ~(dwordArray[dwordIndex]);
				BOOST_ASSERT(bits != 0);
				const int32 lowestBitIndex = Math::countTrainlingZeros(bits) + (dwordIndex << NumBitsPerDWORDLogTwo);
				if (lowestBitIndex < localNumBits)
				{
					return localNumBits;
				}
			}
			return INDEX_NONE;
		}

		FORCEINLINE bool contains(bool bValue) const
		{
			return find(bValue) != INDEX_NONE;
		}



		FORCEINLINE const ConstBitReference accessCorrespondingBit(const RelativeBitReference& relativeReference) const
		{
			BOOST_ASSERT(relativeReference.mMask);
			BOOST_ASSERT(relativeReference.mDWORDIndex >= 0);
			BOOST_ASSERT(((uint32)relativeReference.mDWORDIndex + 1) * NumBitsPerDWORD - 1 - Math::countLeadingZeros(relativeReference.mMask) < (uint32)mNumBits);
			return ConstBitReference(getData()[relativeReference.mDWORDIndex], relativeReference.mMask);
		}

		FORCEINLINE const BitReference accessCorrespondingBit(const RelativeBitReference& relativeReference)
		{
			BOOST_ASSERT(relativeReference.mMask);
			BOOST_ASSERT(relativeReference.mDWORDIndex >= 0);
			BOOST_ASSERT(((uint32)relativeReference.mDWORDIndex + 1) * NumBitsPerDWORD - 1 - Math::countLeadingZeros(relativeReference.mMask) < (uint32)mNumBits);
			return BitReference(getData()[relativeReference.mDWORDIndex], relativeReference.mMask);
		}

		FORCEINLINE int32 count() const { return mNumBits; }

	private:
		template<typename BitArrayType>
		static FORCEINLINE typename std::enable_if<std::TContainerTraits<BitArrayType>::MoveWillEmptyContainer>::type moveOrCopy(BitArrayType& toArray, BitArrayType& fromArray)
		{
			toArray.mAllocatorInstance.moveToEmpty(fromArray.mAllocatorInstance);
			toArray.mNumBits = fromArray.mNumBits;
			toArray.mMaxBits = fromArray.mMaxBits;
			fromArray.mNumBits = 0;
			fromArray.mMaxBits = 0;
		}


		template<typename BitArrayType>
		static FORCEINLINE typename std::enable_if<!std::TContainerTraits<BitArrayType>::MoveWillEmptyContainer>::type moveOrCopy(BitArrayType& toArray, BitArrayType& fromArray)
		{
			toArray = fromArray;
		}
	private:
		typedef typename Allocator::template ForElementType<uint32> AllocatorType;
		AllocatorType mAllocatorInstance;
		int32 mNumBits;
		int32 mMaxBits;

	};

	template<typename Allocator>
	class TConstSetBitIterator : public RelativeBitReference
	{
	public:
		TConstSetBitIterator(const TBitArray<Allocator>&  inArray, int32 startIndex = 0)
			:RelativeBitReference(startIndex)
			,mArray(inArray)
			,mUnvisibtedMask((~0) << (startIndex & (NumBitsPerDWORD - 1)))
			,mCurrentBitIndex(startIndex)
			,mBaseBitIndex(startIndex & ~(NumBitsPerDWORD - 1))
		{
			BOOST_ASSERT(startIndex >= 0 && startIndex <= mArray.count());
			if (startIndex != mArray.count())
			{
				findFirstSetBit();
			}
		}
		

		FORCEINLINE TConstSetBitIterator& operator ++()
		{
			mUnvisibtedMask &= ~this->mMask;
			findFirstSetBit();
			return *this;
		}

		FORCEINLINE friend bool operator == (const TConstSetBitIterator & lhs, const TConstSetBitIterator& rhs)
		{
			return lhs.mCurrentBitIndex == rhs.mCurrentBitIndex && &lhs.mArray == &rhs.mArray;
		}

		FORCEINLINE friend bool operator !=(const TConstSetBitIterator & lhs, const TConstSetBitIterator& rhs)
		{
			return !(lhs == rhs);
		}

		FORCEINLINE explicit operator bool() const
		{
			return mCurrentBitIndex < mArray.count();
		}

		FORCEINLINE bool operator !() const
		{
			return !(bool)*this;
		}

		FORCEINLINE int32 getIndex() const
		{
			return mCurrentBitIndex;
		}


	private:
		const TBitArray<Allocator>& mArray;
		uint32 mUnvisibtedMask;
		uint32 mCurrentBitIndex;
		uint32 mBaseBitIndex;

		void findFirstSetBit()
		{
			const uint32* arrayData = mArray.getData();
			const int32 arrayNum = mArray.count();
			const int32 lastDWORDIndex = (arrayNum - 1) / NumBitsPerDWORD;


			int32 remainingBitMask = arrayData[this->mDWORDIndex] & mUnvisibtedMask;
			while (!remainingBitMask)
			{
				++this->mDWORDIndex;
				mBaseBitIndex += NumBitsPerDWORD;
				if (this->mDWORDIndex > lastDWORDIndex)
				{
					mCurrentBitIndex = arrayNum;
					return;
				}
				remainingBitMask = arrayData[this->mDWORDIndex];
				mUnvisibtedMask = ~0;
			}

			const uint32 newRemainingBitMask = remainingBitMask & (remainingBitMask - 1);
			this->mMask = newRemainingBitMask ^ remainingBitMask;
			mCurrentBitIndex = mBaseBitIndex + NumBitsPerDWORD - 1 - Math::countLeadingZeros(this->mMask);
			if (mCurrentBitIndex > arrayNum)
			{
				mCurrentBitIndex = arrayNum;
			}
		}
	};
}