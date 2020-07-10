#pragma once
#include "CoreType.h"
#include "EngineMininal.h"
namespace Air
{
	struct ENGINE_API PerPlatformInt
#if CPP
		:public tperplatform
#endif
	{

	};
}