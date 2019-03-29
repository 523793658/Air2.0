#pragma once
#include "Classes/GameFramework/Actor.h"
#include "Classes/Components/LightComponent.h"
namespace Air
{
	class MaterialInterface;
	class ENGINE_API ALight: public AActor
	{
		GENERATED_RCLASS_BODY(ALight, AActor)

	public:
		LightComponent * getLightComponent() const;

		void setLightColor(LinearColor newLightColor);
	public:
		class LightComponent* mLightComponent;
		uint32 bEnabled : 1;
	};
}