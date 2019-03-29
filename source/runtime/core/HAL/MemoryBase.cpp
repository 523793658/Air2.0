#include "MemoryBase.h"
namespace Air
{
	uint32 Malloc::mTotalMallocCalls = 0;
	uint32 Malloc::mTotalReallocCalls = 0;
	uint32 Malloc::mTotalFreeCalls = 0;

	void Malloc::initializeStatsMetadata()
	{

	}


}