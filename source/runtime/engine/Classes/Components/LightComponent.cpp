#include "Classes/Components/LightComponent.h"
#include "SceneManagement.h"
#include "SimpleReflection.h"
#include "Classes/Engine/World.h"
#include "Classes/Materials/Material.h"
namespace Air
{
	struct LightAndChannel
	{
		LightComponent* mLight;
		int32 mChannel;
		LightAndChannel(LightComponent* inLight)
			:mLight(inLight)
			,mChannel(INDEX_NONE)
		{}
	};


	LightComponent::LightComponent(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
		,bAddedToSceneVisible(false)
	{}



	LightSceneProxy::LightSceneProxy(const LightComponent* inLightComponent)
		:mLightComponent(std::dynamic_pointer_cast<const LightComponent>(inLightComponent->shared_from_this()))
		,mLightType(inLightComponent->getLightType())
		,bStaticLighting(inLightComponent->hasStaticLighting())
		,bStaticShadowing(inLightComponent->hasStaticShadow())
		,bCastDynamicShadow(inLightComponent->bCastShadows && inLightComponent->bCastDynamicShadows)
		,bCastStaticShadow(inLightComponent->bCastShadows && inLightComponent->bCastStaticShadows)
		, bUsedAsAtmosphereSunLight(inLightComponent->isUsedAsAtmosphereSunLight())
		,mMinRoughness(inLightComponent->mMinRoughness)
		,mLightGuid(inLightComponent->mLightGuid)
		,bMovable(inLightComponent->isMovable())
	{
		float lightBrightness = inLightComponent->computeLightBrightness();
		mColor = LinearColor(inLightComponent->mLightColor) * lightBrightness;
		if (mLightComponent->mLightFunctionMaterial && mLightComponent->mLightFunctionMaterial->getMaterial()->mMaterialDomain == MD_LightFunction)
		{
			mLightFunctionMaterial = mLightComponent->mLightFunctionMaterial->getRenderProxy();
		}
		else
		{
			mLightFunctionMaterial = nullptr;
		}

	}

	void LightComponent::createRenderState_Concurrent()
	{
		ParentType::createRenderState_Concurrent();

		if (bAffectsWorld)
		{
			World* world = getWorld();
			const bool bHidden = !shouldComponentAddToScene() || !shouldRender() || mIntensity <= 0.f;
			if (!bHidden)
			{
				world->mScene->addLight(this);
				bAddedToSceneVisible = true;
				
			}
		}
	}

	void LightSceneProxy::setColor(const LinearColor& inColor)
	{
		mColor = inColor;
	}

	void LightComponent::setLightColor(LinearColor newLightColor, bool bSRGB /* = true */)
	{
		Color newColor(newLightColor.toColor(bSRGB));
		if (areDynamicDataChangeAllowed() && mLightColor != newColor)
		{
			mLightColor = newColor;
			World* world = getWorld();
			if (world && world->mScene)
			{
				world->mScene->updateLightColorAndBrightness(this);
			}
		}
	}

	float3 LightComponent::getDirection() const
	{
		return mComponentToWorld.getUnitAxis(EAxis::Z);
	}

	void LightSceneProxy::setTransform(const Matrix& inLightToWorld, const float4& inPosition)
	{
		mLightToWorld = inLightToWorld;
		mWorldToLight = inLightToWorld.inverseFast();
		mPosition = inPosition;
	}

	float LightComponent::computeLightBrightness() const
	{
		float lightBrightness = mIntensity;
		
		return lightBrightness;
	}

	DECLARE_SIMPLER_REFLECTION(LightComponent);
}