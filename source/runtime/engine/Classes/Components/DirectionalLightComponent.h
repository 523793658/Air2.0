#pragma once
#include "Classes/Components/LightComponent.h"
namespace Air
{
	class ENGINE_API DirectionalLightComponent : public LightComponent
	{
		GENERATED_RCLASS_BODY(DirectionalLightComponent, LightComponent)
	public:
		virtual ELightComponentType getLightType() const override;

		virtual float4 getLightPosition() const override;

		virtual bool isUsedAsAtmosphereSunLight() const override
		{
			return bUsedAsAtmosphereSunLight;
		}

		virtual LightSceneProxy* createSceneProxy() const override;

	private:
		uint32 bUsedAsAtmosphereSunLight : 1;
	};
}