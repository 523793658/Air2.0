#pragma once
#include "CoreType.h"
#include <algorithm>
#include "HAL/ThreadSingleton.h"
#include "Template/AlignmentTemplates.h"
#include "boost/align/align.hpp"
#include "Math/Math.h"
#include "Containers/LockFreeFixedSizeAllocator.h"
#include "Containers/ContainerAllocationPolicies.h"
using namespace Air;

	enum EMemZeroed
	{
		MEM_Zeroed = 1
	};
	enum EMemOned
	{
		MEM_Oned	=1
	};

	

	class CORE_API PageAllocator
	{
	public:
		enum
		{
			PageSize = 64 * 1024,
			SmallPageSize = 1024 - 16
		};

		typedef TLockFreeFixedSizeAllocator<PageSize, PLATFORM_CACHE_LINE_SIZE, ThreadSafeCounter> TPageAllocator;
		static void *alloc();
		static void free(void* mem);
		static void * allocSmall();
		static void freeSmall(void* mem);
		static uint64 bytesUsed();
		static uint64 bytesFree();
		static void latchProtectedMode();


	private:
		static TPageAllocator mAllocator;
	};


	class CORE_API MemStackBase
	{
	public:
		MemStackBase(int32 inMinMarksToAlloc = 1)
			:mTop(nullptr),
			mEnd(nullptr),
			mTopChunk(nullptr),
			mTopMark(nullptr),
			mNumMarks(0),
			mMinMarksToAlloc(inMinMarksToAlloc)
		{

		}

		~MemStackBase()
		{
			freeChunks(nullptr);
		}

		FORCEINLINE uint8* pushBytes(int32 AllocSize, int32 alignment)
		{
			return (uint8*)alloc(AllocSize, std::max<int32>(AllocSize > 16 ? (int32)16 : (int32)8, alignment));
		}

		FORCEINLINE void* alloc(int32 allocSize, int32 alignment)
		{
			uint8* result = align(mTop, alignment);
			uint8* newTop = result + allocSize;
			if (newTop <= mEnd)
			{
				mTop = newTop;
			}
			else
			{
				allocateNewChunk(allocSize + alignment);
				result = align(mTop, alignment);
				newTop = result + allocSize;
				mTop = newTop;
			}
			return result;
		}

		FORCEINLINE bool isEmpty() const
		{
			return mTopChunk == nullptr;
		}

		FORCEINLINE void flush()
		{
			freeChunks(nullptr);
		}

		FORCEINLINE int32 getNumMarks()
		{
			return mNumMarks;
		}

		int32 getByteCount() const;

		bool containsPointer(const void* pointer) const;

		friend class MemMark;
		friend void* operator new (size_t size, MemStackBase& mem, int32 count, int32 align);
		friend void* operator new(size_t Size, MemStackBase& Mem, EMemZeroed Tag, int32 Count, int32 Align);
		friend void* operator new(size_t Size, MemStackBase& Mem, EMemOned Tag, int32 Count, int32 Align);
		friend void* operator new[](size_t Size, MemStackBase& Mem, int32 Count, int32 Align);
		friend void* operator new[](size_t Size, MemStackBase& Mem, EMemZeroed Tag, int32 Count, int32 Align);
		friend void* operator new[](size_t Size, MemStackBase& Mem, EMemOned Tag, int32 Count, int32 Align);


		struct TaggedMemory
		{
			TaggedMemory* mNext;
			int32 mDataSize;
			uint8* data() const
			{
				return ((uint8*)this) + sizeof(TaggedMemory);
			}
		};

	private:
		void allocateNewChunk(int32 minSize);
		void freeChunks(TaggedMemory* newTopChunk);


		uint8*		mTop;
		uint8*		mEnd;
		TaggedMemory* mTopChunk;
		class MemMark* mTopMark;

		int32 mNumMarks;
		int32 mMinMarksToAlloc;
	};

	template <class T> inline T* New(MemStackBase& mem, int32 count = 1, int32 align = DEFAULT_ALIGNMENT)
	{
		return (T*)mem.pushBytes(count * sizeof(T), align);
	}

	template <class T> inline T* NewZeored(MemStackBase& mem, int32 count = 1, int32 align = DEFAULT_ALIGNMENT)
	{
		uint8* result = mem.pushBytes(count * sizeof(T), align);
		Memory::memzero(result, count * sizeof(T));
		return (T*)result;
	}

	template <class T> inline T* NewOned(MemStackBase& mem, int32 count = 1, int32 align = DEFAULT_ALIGNMENT)
	{
		uint8* result = mem.pushBytes(count * sizeof(T), align);
		Memory::Memset(result, 0xff, count * sizeof(T));
		return (T*)result;
	}

	class CORE_API MemStack : public TThreadSingleton<MemStack>, public MemStackBase
	{};

	class MemMark
	{
	public:
		MemMark(MemStackBase& inMem)
			:mMem(inMem)
			, mTop(inMem.mTop)
			, mSavedChunk(inMem.mTopChunk)
			, bPopped(false)
			, mNextTopmostMark(inMem.mTopMark)
		{
			mMem.mTopMark = this;
			mMem.mNumMarks++;
		}

		~MemMark()
		{
			pop();
		}

		void pop()
		{
			if (!bPopped)
			{
				BOOST_ASSERT(mMem.mTopMark == this);
				bPopped = true;
				--mMem.mNumMarks;
				if (mSavedChunk != mMem.mTopChunk)
				{
					mMem.freeChunks(mSavedChunk);
				}
				mMem.mTop = mTop;
				mMem.mTopMark = mNextTopmostMark;
				mTop = nullptr;
			}
		}
	private:
		MemStackBase& mMem;
		uint8* mTop;
		MemStackBase::TaggedMemory* mSavedChunk;
		bool bPopped;
		MemMark* mNextTopmostMark;
	};

	template<uint32 Alignment = DEFAULT_ALIGNMENT>
	class TMemStackAllocator
	{
	public:
		enum { NeedsElementType = true };
		enum { RequireRangeCheck = true };
		template<typename ElementType>
		class ForElementType
		{
		public:
			ForElementType():
				mData(nullptr)
			{}

			FORCEINLINE ElementType* getAllocation() const
			{
				return mData;
			}

			
			void resizeAllocation(int32 previousNumElements, int32 numElements, int32 numBytesPerElement)
			{
				void* oldData = mData;
				if (numElements)
				{
					mData = (ElementType*)MemStack::get().pushBytes(numElements * numBytesPerElement, Math::max(Alignment, (uint32)ALIGNOF(ElementType)));
					if (oldData && previousNumElements)
					{
						const int32 numCopiesElements = Math::min(numElements, previousNumElements);
						Memory::memcpy(mData, oldData, numCopiesElements * numBytesPerElement);
					}
				}
			}

			FORCEINLINE int32 calculateSlackReserve(int32 numElements, int32 numBytesPerElement) const
			{
				return defaultCalculateSlackReserve(numElements, numBytesPerElement, false, Alignment);
			}

			FORCEINLINE int32 calculateSlackShrink(int32 numElements, int32 numAllocatedElements, int32 numBytesPerElement) const
			{
				return defaultCalculateSlackShrink(numElements, numAllocatedElements, numBytesPerElement, false, Alignment);
			}

			FORCEINLINE int32 calculateSlackGrow(int32 numElements, int32 numAllocatedElements, int32 numBytesPerElement) const
			{
				return defaultCalculateSlackGrow(numElements, numAllocatedElements, numBytesPerElement, false, Alignment);
			}
		private:
			ElementType* mData;

		};

		typedef ForElementType<ScriptContainerElement> ForAnyElementType;

	};
	inline void* operator new(size_t size, MemStackBase& mem, int32 count = 1, int32 align = DEFAULT_ALIGNMENT)
	{
		return mem.pushBytes(size* count, align);
	}