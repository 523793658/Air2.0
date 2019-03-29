#include "MallocAnsi.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#endif

namespace Air
{
	MallocAnsi::MallocAnsi()
	{
#if PLATFORM_WINDOWS
		intptr_t	CrtHeapHandle = _get_heap_handle();
		ULONG		EnableLFH = 2;
		HeapSetInformation((void*)CrtHeapHandle, HeapCompatibilityInformation, &EnableLFH, sizeof(EnableLFH));
#endif
	}
}