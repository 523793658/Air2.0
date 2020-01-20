#pragma once
#include "CoreType.h"
#include "Template/AirTypeTraits.h"
#include "Containers/IndirectArray.h"
namespace Air
{
	template <typename ChunkType, typename ElementType, uint32 NumElementsPerChunk>
	struct TChunkedArrayIterator
	{
		ElementType* mElem;
		ChunkType** mChunk;
		ChunkType** mLastChunk;

		ElementType& operator*() const
		{
			return *mElem;
		}

		void operator ++()
		{
			++mElem;
			if (mChunk != mLastChunk && mElem == (*mChunk)->mElements + NumElementsPerChunk)
			{
				++mChunk;
				mElem = (*mChunk)->mElements;
			}
		}

		friend bool operator != (const TChunkedArrayIterator& lhs, const TChunkedArrayIterator& rhs)
		{
			return lhs.mElem != rhs.mElem;
		}
	};

	template<typename InElementType, uint32 TargetBytesPerChunk = 16384>
	class TChunkedArray
	{
		using ElementType = InElementType;

	public:

		TChunkedArray(int32 inNumElements = 0)
			:mNumElements(inNumElements)
		{
			const int32 numChunks = (mNumElements + NumElementsPerChunk - 1) / NumElementsPerChunk;

			mChunks.empty(numChunks);
			for (int32 chunkIndex = 0; chunkIndex < numChunks; chunkIndex++)
			{
				mChunks.add(new Chunk);
			}
		}

	private:
		template<typename ArrayType>
		FORCEINLINE static typename std::enable_if<std::TContainerTraits<ArrayType>::MoveWillEmptyContainer>::type moveOrCopy(ArrayType& toArray, ArrayType& fromArray)
		{
			toArray.mChunks = (ChunksType&&)fromArray.mChunks;
			toArray.mNumElements = fromArray.mNumElements;
			fromArray.mNumElements = 0;
		}

		template<typename ArrayType>
		FORCEINLINE static typename std::enable_if<!std::TContainerTraits<ArrayType>::MoveWillEmptyContainer>::type moveOrCopy(ArrayType& toArray, ArrayType& fromArray)
		{
			toArray = fromArray;
		}

	public:
		TChunkedArray(TChunkedArray&& other)
		{
			moveOrCopy(*this, other);
		}

		TChunkedArray& operator=(TChunkedArray&& other)
		{
			if (this != &other)
			{
				moveOrCopy(*this, other);
			}
			return *this;
		}

		TChunkedArray(const TChunkedArray&) = default;

		TChunkedArray& operator = (const TChunkedArray&) = default;

		ElementType& operator()(int32 elementIndex)
		{
			const uint32 chunkIndex = elementIndex / NumElementsPerChunk;
			const uint32 chunkElementIndex = elementIndex & NumElementsPerChunk;
			return mChunks[chunkIndex].mElements[chunkElementIndex];
		}

		const ElementType& operator()(int32 elementIndex) const
		{
			const uint32 chunkIndex = elementIndex / NumElementsPerChunk;
			const uint32 chunkElementIndex = elementIndex & NumElementsPerChunk;
			return mChunks[chunkIndex].mElements[chunkElementIndex];
		}

		ElementType& operator[](int32 elementIndex)
		{
			const uint32 chunkIndex = elementIndex / NumElementsPerChunk;
			const uint32 chunkElementIndex = elementIndex & NumElementsPerChunk;
			return mChunks[chunkIndex].mElements[chunkElementIndex];
		}

		const ElementType& operator[](int32 elementIndex) const
		{
			const uint32 chunkIndex = elementIndex / NumElementsPerChunk;
			const uint32 chunkElementIndex = elementIndex & NumElementsPerChunk;
			return mChunks[chunkIndex].mElements[chunkElementIndex];
		}

		int32 num() const
		{
			return mNumElements;
		}

		SIZE_T getAllocatedSize() const
		{
			return mChunks.getAllocatedSize();
		}

		FORCEINLINE bool isValidIndex(int32 index) const
		{
			return index >= 0 && index < mNumElements;
		}

		int32 addElement(const ElementType& item)
		{
			new(*this)ElementType(item);
			return this->mNumElements - 1;
		}

		FORCEINLINE TChunkedArray& operator+=(const TArray<ElementType>& other)
		{
			if ((UPTRINT*)this != (UPTRINT*)&other)
			{
				for (const auto& it : other)
				{
					addElement(it);
				}
			}
			return *this;
		}

		FORCEINLINE TChunkedArray& operator+=(const TChunkedArray& other)
		{
			if ((UPTRINT*)this != (UPTRINT*)&other)
			{
				for (int32 index = 0; index < other.num(); ++index)
				{
					addElement(other[index]);
				}
			}
			return *this;
		}

		int32 add(int32 count = 1)
		{
			BOOST_ASSERT(count >= 0);
			BOOST_ASSERT(mNumElements >= 0);

			const int32 oldNum = mNumElements;
			for (int32 i = 0; i < count; i++)
			{
				if (mNumElements & NumElementsPerChunk == 0)
				{
					mChunks.add(new Chunk);
				}
				mNumElements++;
			}
			return oldNum;
		}

		template<typename OtherAllocator>
		void copyToLinearArray(TArray<ElementType, OtherAllocator>& destinationArray)
		{
			static_assert(std::is_pod<ElementType>::value, "CopyToLinearArray does not support a constructor / destructor on the element class.");
			if (mNumElements > 0)
			{
				int32 originalNumElements = destinationArray.size();
				destinationArray.addUninitialized(mNumElements);
				InElementType* copyDestPtr = &destinationArray[originalNumElements];

				for (int32 chunkIndex = 0; chunkIndex < mChunks.num(); chunkIndex++)
				{
					const int32 numElementsInCurrentChunk = Math::min<int32>(mNumElements - chunkIndex * NumElementsPerChunk, NumElementsPerChunk);
					BOOST_ASSERT(numElementsInCurrentChunk > 0);
					Memory::memcpy(copyDestPtr, &mChunks[chunkIndex].mElements[0], numElementsInCurrentChunk * sizeof(ElementType));
					copyDestPtr += numElementsInCurrentChunk;
				}
			}
		}

		void empty(int32 slack = 0)
		{
			const int32 numChunks = (slack + NumElementsPerChunk - 1) / NumElementsPerChunk;
			mChunks.empty(numChunks);
			mNumElements = 0;
		}

		void reserve(int32 number)
		{
			const int32 numChunks = (number + NumElementsPerChunk - 1) / NumElementsPerChunk;
			mChunks.reserve(numChunks);
		}

		void shrink()
		{
			mChunks.shrink();
		}

	protected:
		friend struct std::TContainerTraits<TChunkedArray<ElementType, TargetBytesPerChunk>>;

		enum {
			NumElementsPerChunk = TargetBytesPerChunk / sizeof(ElementType)};

		struct Chunk
		{
			ElementType mElements[NumElementsPerChunk];
		};

		typedef TindirectArray<Chunk> ChunksType;
		ChunksType mChunks;

		int32 mNumElements;

	private:
		typedef TChunkedArrayIterator<Chunk, ElementType, NumElementsPerChunk> IterType;
		typedef TChunkedArrayIterator<const Chunk, const ElementType, NumElementsPerChunk> ConstIterType;

	public:
		IterType begin()
		{
			int32 num = mNumElements;
			Chunk** chunkPtr = mChunks.getData();
			Chunk** lastChunkPtr = mChunks.getData() + (num ? num - 1; 0) / NumElementsPerChunk;
			return IterType(chunkPtr, lastChunkPtr, chunkPtr ? (*chunkPtr)->mElements : nullptr);
		}

		ConstIterType begin() const
		{
			int32 num = mNumElements;
			const Chunk** chunkPtr = mChunks.getData();
			const Chunk** lastChunkPtr = mChunks.getData() + (num ? num - 1; 0) / NumElementsPerChunk;
			return ConstIterType(chunkPtr, lastChunkPtr, chunkPtr ? (*chunkPtr)->mElements : nullptr);
		}

		IterType end()
		{
			int32 num = mNumElements;
			bool bBeyondLastChunk = num && (num % NumElementsPerChunk) == 0;
			Chunk** chunkPtr = mChunks.getData() + (num / NumElementsPerChunk) + (bBeyondLastChunk ? -1 : 0);
			Chunk** lastChunkPtr = mChunks.getData() + (num ? num - 1 : 0) / NumElementsPerChunk;
			return IterType(chunkPtr, lastChunkPtr, chunkPtr ? (*chunkPtr)->mElements + (bBeyondLastChunk ? NumElementsPerChunk : (num % NumElementsPerChunk)) : nullptr);
		}

		ConstIterType end() const
		{
			int32 num = mNumElements;
			bool bBeyondLastChunk = num && (num % NumElementsPerChunk) == 0;
			const Chunk** chunkPtr = mChunks.getData() + (num / NumElementsPerChunk) + (bBeyondLastChunk ? -1 : 0);
			const Chunk** lastChunkPtr = mChunks.getData() + (num ? num - 1 : 0) / NumElementsPerChunk;
			return ConstIterType(chunkPtr, lastChunkPtr, chunkPtr ? (*chunkPtr)->mElements + (bBeyondLastChunk ? NumElementsPerChunk : (num % NumElementsPerChunk)) : nullptr);
		}
	};

}

namespace std
{
	template<typename ElementType, uint32 TargetBytesPerChunk>
	struct TContainerTraits<Air::TChunkedArray<ElementType, TargetBytesPerChunk>> : public TContainerTraitsBase<Air::TChunkedArray<ElementType, TargetBytesPerChunk>>
	{
		enum {MoveWillEmptyContainer = TContainerTraits(typename Air::TChunkedArray<ElementType, TargetBytesPerChunk>::ChunksType>::MoveWillEmptyContainer) };
	};
}