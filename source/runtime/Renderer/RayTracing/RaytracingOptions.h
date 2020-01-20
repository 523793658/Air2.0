#pragma once
#include "RHIDefinitions.h"

namespace Air
{
	class ViewInfo;

#if RHI_RAYTRACING
	extern bool canOverlayRayTracingOutput(const ViewInfo& view);
#endif
}