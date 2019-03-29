#pragma once
#include "CoreObject.h"

namespace Air
{
	class Object;
	class COREOBJECT_API ObjectAllocator
	{
	public:
		void allocatePermanentObjectPool(int32 inPermanentObjectPoolSize);
		void bootMessage();

		FORCEINLINE bool resizedsInPermanentPool(const Object* object) const
		{
			return ((const uint8*)object >= mPermanentObjectPool) && ((const uint8*)object < mPermanentObjectPoolTail);
		}

		Object* allocateObject(int32 size, int32 alignment, bool bAllowPermanent);

		void freeObject(Object* object) const;

	private:
		int32 mPermanentObjectPoolSize{ 0 };
		uint8*	mPermanentObjectPool{ nullptr };
		uint8*	mPermanentObjectPoolTail{ nullptr };
		uint8*	mPermanentObjectPoolExceededTail{ nullptr };
	};

	extern COREOBJECT_API ObjectAllocator GObjectAllocator;
}