#pragma once
#include "CoreMinimal.h"
#include "Classes/GameFramework/Info.h"
namespace Air
{
	class ENGINE_API APlayerState : public AInfo
	{
	public:
		uint32 bOnlySpectator : 1;
	};
}