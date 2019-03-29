#pragma once
#include "EngineMininal.h"
#include "RHI.h"
#include "Classes/Components/PrimitiveComponent.h"
namespace Air
{
	class FMaterial;
	class MaterialShaderMap;
	class LightComponent;

	enum EBasePassDrawListType
	{
		EBasePass_Default = 0,
		EBasePass_Masked,
		EBAsePass_MAX
	};

	enum class EShadingPath
	{
		Mobile,
		Deferred,
		Num
	};

	class SceneInterface
	{
	public:
		virtual class Scene* getRenderScene()
		{
			return nullptr;
		}

		virtual class World* getWorld() const = 0;

		virtual ERHIFeatureLevel::Type getFeatureLevel() const
		{
			return GMaxRHIFeatureLevel;
		}
		virtual void addPrimitive(PrimitiveComponent* primitive) = 0;

		virtual void addLight(LightComponent* light) = 0;

		virtual void setShaderMapsOnMaterialResources(const TMap<FMaterial*, MaterialShaderMap*>& materialsToUpdate) {}

		static EShadingPath getShadingPath(ERHIFeatureLevel::Type inFeatureLevel)
		{
			if (inFeatureLevel >= ERHIFeatureLevel::SM4)
			{
				return EShadingPath::Deferred;
			}
			else
			{
				return EShadingPath::Mobile;
			}
		}

		virtual void updatePrimitiveTransform(PrimitiveComponent* primitive) = 0;

		virtual void updateLightColorAndBrightness(LightComponent* light) = 0;

		EShadingPath getShadingPath()
		{
			return getShadingPath(getFeatureLevel());
		}

		virtual void updateParameterCollections(const TArray<class MaterialParameterCollectionInstanceResource*>& inParameterCollections) {}

	};
}