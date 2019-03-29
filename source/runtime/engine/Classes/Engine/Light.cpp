#include "Classes/Engine/Light.h"
#include "SimpleReflection.h"
namespace Air
{
	ALight::ALight(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		mLightComponent = createAbsoluteDefaultSubobject<LightComponent>(TEXT("LightComponent0"));

	}

	LightComponent* ALight::getLightComponent() const
	{
		return mLightComponent;
	}

	void ALight::setLightColor(LinearColor newLightColor)
	{
		if (mLightComponent)
		{
			mLightComponent->setLightColor(newLightColor);
		}
	}

	DECLARE_SIMPLER_REFLECTION(ALight);
}