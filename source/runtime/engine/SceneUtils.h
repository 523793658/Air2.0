#pragma once
#include "CoreMinimal.h"
#define WANTS_DRAW_MESH_EVENTS 0

namespace Air
{
#if WANTS_DRAW_MESH_EVENTS
#else
	template<typename TRHICmdList>
	struct ENGINE_API TDrawEvent
	{

	};

#define BEGIN_DRAW_EVENTF(...)
#define STOP_DRAW_EVENT(...)
#endif
	
}