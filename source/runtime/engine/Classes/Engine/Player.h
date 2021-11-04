#pragma once
#include "EngineMininal.h"
#include "UObject/Object.h"
namespace Air
{
	class ENGINE_API Player : public Object
	{
		GENERATED_RCLASS_BODY(Player, Object)
	public:

		std::shared_ptr<class APlayerController> mPlayerComtroller;
	};
}