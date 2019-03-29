#pragma once
#include "EngineMininal.h"
#include "Object.h"
namespace Air
{
	class ENGINE_API Player : public Object
	{
		GENERATED_RCLASS_BODY(Player, Object)
	public:

		class APlayerController* mPlayerComtroller;
	};
}