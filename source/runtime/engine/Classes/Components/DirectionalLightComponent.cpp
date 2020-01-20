#include "Classes/Components/DirectionalLightComponent.h"
#include "SceneManagement.h"
#include "RenderUtils.h"
#include "SimpleReflection.h"
namespace Air
{
	class DirectionalLightSceneProxy : public LightSceneProxy
	{
	public:
		DirectionalLightSceneProxy(const DirectionalLightComponent* component)
			:LightSceneProxy(component)
			,mAtmosphereTransmittanceFactor(LinearColor::White)
		{

		}

		virtual void getLightShaderParameters(LightShaderParameters& lightParameters) const override
		{
			lightParameters.Position = float3::Zero;
			lightParameters.InvRadius = 0.0f;
			lightParameters.Color = float3(getColor() * mAtmosphereTransmittanceFactor);
			lightParameters.FallofExponent = 0.0f;
			lightParameters.Direction = -getDirection();
			lightParameters.Tangent = -getDirection();
			lightParameters.SpotAngles = float2::Zero;
			lightParameters.SpecularScale = mSpecularScale;
			lightParameters.SourceRadius = Math::sin(0.5f * Math::degreesToRadians(mLightSourceAngle));
			lightParameters.SoftSourceRadius = Math::sin(0.5f * Math::degreesToRadians(mLightSourceSoftAngle));
			lightParameters.SourceLength = 0.0f;
			lightParameters.SourceTexture = GWhiteTexture->mTextureRHI;
		}


		LinearColor mAtmosphereTransmittanceFactor;

		float mLightSourceAngle;

		float mLightSourceSoftAngle;
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