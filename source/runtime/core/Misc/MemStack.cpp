#include "Misc/MemStack.h"


PageAllocator::TPageAllocator PageAllocator::mAllocator;

void* PageAllocator::alloc()
{
	void *result = mAllocator.allocate();
	return result;
}

void PageAllocator::free(void* mem)
{
	mAllocator.free(mem);
}

void* PageAllocator::allocSmall()
{
	return Memory::malloc(SmallPageSize);
}

void PageAllocator::freeSmall(void* mem)
{
	Memory::free(mem);
}

uint64 PageAllocator::bytesUsed()
{
	return uint64(mAllocator.getNumUsed().getValue()) * PageSize;
}

uint64 PageAllocator::bytesFree()
{
	return uint64(mAllocator.getNumFree().getValue()) * PageSize;
}

void PageAllocator::latchProtectedMode()
{

}

int32 MemStackBase::getByteCount() const
{
	int32 count = 0;
	for (TaggedMemory* chunk = mTopChunk; chunk; chunk = chunk->mNext)
	{
		if (chunk != mTopChunk)
		{
			count += chunk->mDataSize;
		}
		else
		{
			count += mTop - chunk->data();
		}
	}
	return count;
}

void MemStackBase::allocateNewChunk(int32 minSize)
{
	TaggedMemory* chunk = nullptr;
	int32 totalSize = minSize + (int32) sizeof(TaggedMemory);
	uint32 allocSize;
	if (mTopChunk || totalSize > PageAllocator::SmallPageSize)
	{
		allocSize = alignArbitrary<int32>(totalSize, PageAllocator::PageSize);
		if (allocSize == PageAllocator::PageSize)
		{
			chunk = (TaggedMemory*)PageAllocator::alloc();
		}
		else
		{
			chunk = (TaggedMemory*)Memory::malloc(allocSize);
		}
	}
	else
	{
		allocSize = PageAllocator::SmallPageSize;
		chunk = (TaggedMemory*)PageAllocator::allocSmall();
	}
	chunk->mDataSize = allocSize - sizeof(TaggedMemory);
	chunk->mNext = mTopChunk;
	mTopChunk = chunk;
	mTop = chunk->data();
	mEnd = mTop + chunk->mDataSize;
}

void MemStackBase::freeChunks(TaggedMemory* newTopChunk)
{
	while (mTopChunk != newTopChunk)
	{
		TaggedMemory* removeChunk = mTopChunk;
		mTopChunk = mTopChunk->mNext;
		if (removeChunk->mDataSize + sizeof(TaggedMemory) == PageAllocator::PageSize)
		{
			PageAllocator::free(removeChunk);
		}
		else if (removeChunk->mDataSize + sizeof(TaggedMemory) == PageAllocator::SmallPageSize)
		{
			PageAllocator::freeSmall(removeChunk);
		}
		else
		{
			Memory::free(removeChunk);
		}
	}
	mTop = nullptr;
	mEnd = nullptr;
	if (mTopChunk)
	{
		mTop = mTopChunk->data();
		mEnd = mTop + mTopChunk->mDataSize;
	}
}

bool MemStackBase::containsPointer(const void* pointer) const 
{
	const uint8* ptr = (const uint8*)pointer;
	for (const TaggedMemory* chunk = mTopChunk; chunk; chunk = chunk->mNext)
	{
		if (ptr >= chunk->data() && ptr < chunk->data() + chunk->mDataSize)
		{
			return true;
		}
	}
	return false;
}