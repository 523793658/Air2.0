#include "Classes/Engine/DirectionalLight.h"
#include "Classes/Components/DirectionalLightComponent.h"
#include "SimpleReflection.h"
namespace Air
{

	ADirectionalLight::ADirectionalLight(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer.setDefaultSubobjectClass<DirectionalLightComponent>(TEXT("LightComponent0")))
	{
		struct ConstructorStatics
		{
			wstring ID_Lighting;
			wstring Name_Lighting;
			ConstructorStatics()
				:ID_Lighting(TEXT("Lighting"))
				, Name_Lighting(TEXT("Lighting"))
			{

			}
		};

		static ConstructorStatics constructorStatics;

		DirectionalLightComponent* directinalLightComponent = check_cast<DirectionalLightComponent*>(getLightComponent());
		directinalLightComponent->mMobility = EComponentMobility::Stationary;
		directinalLightComponent->mRelativeRotation = Rotator(46.0f, 0.0f, 0.0f);
		directinalLightComponent->setRelativeScale3D(float3(2.5f));
		
		mRootComponent = directinalLightComponent;


	}

	void ADirectionalLight::postLoad()
	{
		ParentType::postLoad();
		if (getLightComponent()->mMobility == EComponentMobility::Static)
		{
			getLightComponent()->mLightFunctionMaterial = nullptr;
		}
	}

	DECLARE_SIMPLER_REFLECTION(ADirectionalLight);
}