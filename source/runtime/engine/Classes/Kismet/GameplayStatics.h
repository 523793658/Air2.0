#pragma once
#include "EngineMininal.h"
#include "Object.h"
namespace Air
{
	class ENGINE_API GameplayStatics : public Object
	{
		GENERATED_RCLASS_BODY(GameplayStatics, Object)
	public:
		static class APlayerController* getPlayerController(const Object* worldContextObject, int32 playerIndex);
	};
}