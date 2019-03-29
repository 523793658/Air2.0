#include "GPUProfiler.h"
#include "Template/RefCounting.h"
namespace Air
{
	bool GPUTiming::GAreGlobalsInitialized = false;
	bool GPUTiming::GIsSupported = false;
	uint64 GPUTiming::GTimingFrequency = 0;
}