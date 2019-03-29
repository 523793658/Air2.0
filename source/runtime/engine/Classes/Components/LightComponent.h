#pragma once
#include "Classes/Components/LightComponentBase.h"
#include "SceneTypes.h"
namespace Air
{
	class ENGINE_API LightComponent : public LightComponentBase
	{
		GENERATED_RCLASS_BODY(LightComponent, LightComponentBase)
	public:

		virtual ELightComponentType getLightType() const PURE_VIRTRUAL(LightComponent::getLightType, return LightType_Max;);

		virtual float4 getLightPosition() const PURE_VIRTRUAL(LightComponent::getPosition, return float4(););

		virtual void createRenderState_Concurrent() override;

		virtual class LightSceneProxy* createSceneProxy() const
		{
			return nullptr;
		}

		float3 getDirection() const;

		void setLightColor(LinearColor newLightColor, bool bSRGB = true);

		float computeLightBrightness() const;

		virtual bool isUsedAsAtmosphereSunLight() const
		{
			return false;
		}

	public:
		class LightSceneProxy* mSceneProxy;

		class MaterialInterface* mLightFunctionMaterial{ nullptr };

		uint32 bAddedToSceneVisible : 1;

		float mMinRoughness{ 0.04f };
	};
}