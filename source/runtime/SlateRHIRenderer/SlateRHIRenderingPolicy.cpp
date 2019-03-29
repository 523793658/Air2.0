#include "SlateRHIRenderingPolicy.h"
namespace Air
{
	void SlateRHIRenderingPolicy::endDrawingWindows()
	{
		BOOST_ASSERT(isInRenderingThread());
	}

	void SlateRHIRenderingPolicy::beginDrawingWindows()
	{
		BOOST_ASSERT(isInParallelRenderingThread());
	}
}