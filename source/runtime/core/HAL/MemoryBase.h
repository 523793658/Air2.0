#pragma once
#include "CoreType.h"
#include "Misc/Exec.h"
#include "PlatformAtomics.h"
namespace Air
{
	enum
	{
		DEFAULT_ALIGNMENT = 0,
		MIN_ALIGNMENT = 8,
	};

	CORE_API extern class Malloc* GMalloc;

	CORE_API extern class Malloc** GFixedMallocLocationPtr;

	class CORE_API UseSystemMallockForNew
	{
	public:
		void* operator new (size_t size);

		void operator delete(void* ptr);

		void* operator new[](size_t size);

		void operator delete[](void* ptr);
	};

	class CORE_API Malloc :
		public UseSystemMallockForNew,
		public Exec
	{
	public:
		virtual void* malloc(SIZE_T count, uint32 alignment = DEFAULT_ALIGNMENT) = 0;

		virtual void* realloc(void* original, SIZE_T count, uint32 alignment = DEFAULT_ALIGNMENT) = 0;

		virtual void free(void* original) = 0;

		virtual SIZE_T quantizeSize(SIZE_T count, uint32 alignment)
		{
			return count;
		}

		virtual bool getAllocationSize(void* original, SIZE_T & sizeOut)
		{
			return false;
		}

		virtual void trim()
		{

		}

		virtual void setupTLSCachesOnCurrentThread()
		{

		}

		virtual void clearAndDisableTLSCachesOnCurrentThread()
		{

		}

		virtual void initializeStatsMetadata();

	protected:
		FORCEINLINE void IncrementTotalMallocCalls()
		{
			PlatformAtomics::interlockedIncrement((volatile int32*)&Malloc::mTotalMallocCalls);
		}

		FORCEINLINE void incrementTotalFreeCalls()
		{
			PlatformAtomics::interlockedIncrement((volatile int32*)&Malloc::mTotalFreeCalls);
		}

		FORCEINLINE void incrementTotalReallocCalls()
		{
			PlatformAtomics::interlockedIncrement((volatile int32*)&Malloc::mTotalReallocCalls);
		}


		static uint32 mTotalMallocCalls;

		static uint32 mTotalFreeCalls;

		static uint32 mTotalReallocCalls;
	};


}