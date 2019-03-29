#include "LockFreeVoidPointerListBase.h"
namespace Air
{
	LockFreeVoidPointerListBase::LinkAllocator::LinkAllocator()
		:mFreeLinks(nullptr)
		, mSpecialClosedLink(new Link())
	{
		closedLink()->mLockCount.increment();
	}
	LockFreeVoidPointerListBase::LinkAllocator::~LinkAllocator()
	{}
}