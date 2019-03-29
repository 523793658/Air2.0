#include "ObjectAllocator.h"
#include "Template/AlignmentTemplates.h"
namespace Air
{

	COREOBJECT_API ObjectAllocator GObjectAllocator;

	void ObjectAllocator::allocatePermanentObjectPool(int32 inPermanentObjectPoolSize)
	{
		mPermanentObjectPoolSize = inPermanentObjectPoolSize;
		mPermanentObjectPool = (uint8*)Memory::malloc(mPermanentObjectPoolSize);
		mPermanentObjectPoolTail = mPermanentObjectPool;
		mPermanentObjectPoolExceededTail = mPermanentObjectPool;
	}

	Object* ObjectAllocator::allocateObject(int32 size, int32 alignment, bool bAllowPermanent)
	{
		alignment = 16;
		int32 alignedSize = align(size, alignment);
		Object* result = nullptr;
		const bool bPlaceInPerm = bAllowPermanent && (align(mPermanentObjectPoolTail, alignment) + size) <= (mPermanentObjectPool + mPermanentObjectPoolSize);
		if (bAllowPermanent && !bPlaceInPerm)
		{
			uint8* alignedPtr = align(mPermanentObjectPoolExceededTail, alignment);
			mPermanentObjectPoolExceededTail = alignedPtr + size;
		}
		if (bPlaceInPerm)
		{
			uint8* alignedPtr = align(mPermanentObjectPoolTail, alignment);
			mPermanentObjectPoolTail = alignedPtr + size;
			result = (Object*)alignedPtr;
			if (mPermanentObjectPoolExceededTail < mPermanentObjectPoolTail)
			{
				mPermanentObjectPoolExceededTail = mPermanentObjectPoolTail;
			}
		}
		else
		{
			result = (Object*)(Memory::malloc(alignedSize));
		}
		return result;
	}

	void ObjectAllocator::freeObject(Object* object) const
	{
		if (resizedsInPermanentPool(object) == false)
		{
			Memory::free(object);
		}
		else
		{

		}
	}
}