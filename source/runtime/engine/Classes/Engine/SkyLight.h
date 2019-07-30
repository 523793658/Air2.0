#pragma once
#include "CoreMinimal.h"
#include "Classes/GameFramework/Info.h"
#include "Classes/Components/SkyLightComponent.h"
namespace Air
{
	class ENGINE_API ASkyLight : public AInfo
	{
		GENERATED_RCLASS_BODY(ASkyLight, AInfo)
	public:
		std::shared_ptr<class SkyLightComponent> mLightComponent;

	public:
		uint32 bEnabled;

		class SkyLightComponent* getLightComponent() const;
	};
}