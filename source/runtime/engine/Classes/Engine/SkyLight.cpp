#include "SkyLight.h"
#include "Classes/Components/SkyLightComponent.h"
#include "SimpleReflection.h"
namespace Air
{
	ASkyLight::ASkyLight(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		mLightComponent = createDefaultSubObject<SkyLightComponent>(TEXT("SkyLightComponent0"));
		mRootComponent = mLightComponent;

#if WITH_EDITORONLY_DATA
		
#endif
	}

	SkyLightComponent* ASkyLight::getLightComponent() const { return mLightComponent.get(); }

	DECLARE_SIMPLER_REFLECTION(ASkyLight);
}