#include "Classes/Components/DirectionalLightComponent.h"
#include "SceneManagement.h"
#include "SimpleReflection.h"
namespace Air
{
	class DirectionalLightSceneProxy : public LightSceneProxy
	{
	public:
		DirectionalLightSceneProxy(const DirectionalLightComponent* component)
			:LightSceneProxy(component)
		{

		}

		virtual void getParameters(float4& lightPositionAndInvRadius, float4& lightColorAndFalloffExponent, float3& normalizedLightDirection, float2& spotAngles, float& lightSourceRadius, float& lightSourceLength, float& lightMinRoughness) const override
		{
			lightPositionAndInvRadius = float4(0, 0, 0, 0);
			lightColorAndFalloffExponent = float4(getColor().R, getColor().G, getColor().B, 0);
			normalizedLightDirection = -getDirection();

			spotAngles = float2(0, 0);
			lightSourceRadius = 0.0f;
			lightSourceLength = 0.0f;
			lightMinRoughness = Math::max(mMinRoughness, .04f);
		}

	};

	DirectionalLightComponent::DirectionalLightComponent(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
		, bUsedAsAtmosphereSunLight(false)
	{}

	ELightComponentType DirectionalLightComponent::getLightType() const
	{
		return LightType_Directional;
	}

	float4 DirectionalLightComponent::getLightPosition() const
	{
		return float4(-getDirection());
	}

	LightSceneProxy* DirectionalLightComponent::createSceneProxy() const
	{
		return new DirectionalLightSceneProxy(this);
	}

	DECLARE_SIMPLER_REFLECTION(DirectionalLightComponent);
}