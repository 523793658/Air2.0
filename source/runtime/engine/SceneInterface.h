#pragma once
#include "EngineMininal.h"
#include "RHI.h"
#include "Classes/Components/PrimitiveComponent.h"
#include "Math/SHMath.h"
namespace Air
{
	class FMaterial;
	class MaterialShaderMap;
	class LightComponent;
	class SkyLightSceneProxy;
	class SkyLightComponent;
	class RTextureCube;
	class Texture;
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

		virtual void setSkyLight(SkyLightSceneProxy* light) = 0;

		virtual void disableSkyLight(SkyLightSceneProxy* light) = 0;

		virtual void updateSkyCaptureContents(const SkyLightComponent* captureComponent, bool bCaptureEmissiveOnly, std::shared_ptr<RTextureCube> sourceCubemap, Texture* outProcessedTexture, float& outAverageBrightness, SHVectorRGB3& outIrradianceEnvironmentMap) {}

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

		EShaderPlatform getShaderPlatform() const { return GShaderPlatformForFeatureLevel[getFeatureLevel()]; }
	};
}