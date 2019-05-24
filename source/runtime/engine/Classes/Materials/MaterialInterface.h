#pragma once
#include "EngineMininal.h"
#include "Object.h"
#include "SceneTypes.h"
#include "RHIDefinitions.h"
#include "Classes/Engine/EngineType.h"
#include "RHI.h"
#include "RenderCommandFence.h"
#include "PrimitiveViewRelevance.h"
namespace Air
{
	class MaterialResource;
	class MaterialCompiler;

	struct ENGINE_API MaterialRelevance
	{
		uint16 mShadingModelMask;
		uint32 bOpaque : 1;
		uint32 bMasked : 1;
		uint32 bDistortion;
		uint32 bSeparateTranslucency : 1;
		uint32 bMobileSeparateTranslucency : 1;
		uint32 bNormalTranslucency : 1;
		uint32 bDisableDepthTest : 1;
		uint32 bOutputsVelocityInBasePass : 1;
		uint32 bUsesGlobalDistanceField : 1;
		uint32 bUsesWorldPositionOffset : 1;
		uint32 bDecal : 1;
		uint32 bTranslucentSurfaceLighting : 1;
		uint32 bUsersSceneDpeth;

		MaterialRelevance()
		{
			uint8* RESTRICT p = (uint8*)this;
			for (uint32 i = 0; i < sizeof(*this); ++i)
			{
				*p++ = 0;
			}
		}

		MaterialRelevance& operator |= (const MaterialRelevance& b)
		{
			const uint8 * RESTRICT s = (const uint8*)&b;
			uint8* RESTRICT d = (uint8*)this;
			for (uint32 i = 0; i < sizeof(*this); ++i)
			{
				*d = *d | *s;
				++s; ++d;
			}
			return *this;
		}

		void setPrimitiveViewRelevance(PrimitiveViewRelevance& outViewRelevance) const;
	};

	class MaterialInterface : public Object
	{
		GENERATED_RCLASS_BODY(MaterialInterface, Object)
	public:
	

		virtual class MaterialRenderProxy* getRenderProxy(bool selected, bool bHovered = false) const PURE_VIRTRUAL(MaterialInterface::getRenderProxy, return nullptr;);

		virtual MaterialResource* getMaterialResource(ERHIFeatureLevel::Type inFeatureLvel, EMaterialQualityLevel::Type qualityLevel = EMaterialQualityLevel::Num) PURE_VIRTRUAL(MaterialInterface::getMaterialResource, return nullptr;)

			virtual const MaterialResource* getMaterialResource(ERHIFeatureLevel::Type inFeatureLvel, EMaterialQualityLevel::Type qualityLevel = EMaterialQualityLevel::Num) const PURE_VIRTRUAL(MaterialInterface::getMaterialResource, return nullptr;)
		ENGINE_API static void assertDefaultMaterialsPostLoaded();

		ENGINE_API virtual void postLoad() override;

		ENGINE_API virtual EBlendMode getBlendMode() const;

		ENGINE_API virtual EMaterialShadingModel getShadingModel() const;

		ENGINE_API static void initDefaultMaterials();

		ENGINE_API static void assertDefaultMaterialsExist();

		ENGINE_API virtual void compileProperty(MaterialCompiler* compiler, wstring** properties, uint32 forceCastFlags = 0);

		ENGINE_API int32 compileProperty(MaterialCompiler* compiler, EMaterialProperty prop, uint32 forceCastFlags = 0);

		ENGINE_API virtual int32 compilePropertyEx(class MaterialCompiler* compiler, const Guid& attributeId);

		ENGINE_API virtual bool isPropertyActive(EMaterialProperty inProperty) const;

		ENGINE_API MaterialRelevance getRelevance(ERHIFeatureLevel::Type inFeatureLevel) const;

		virtual class RMaterial* getMaterial() PURE_VIRTRUAL(MaterialInterface::getMaterial, return nullptr;);

		virtual const class RMaterial* getMaterial() const PURE_VIRTRUAL(MaterialInterface::getMaterial, return nullptr;);

		virtual void recacheConstantExpressions() const {};

		ENGINE_API static uint32 getFeatureLevelsToCompileForAllMaterials() { return mFeatureLevelsForAllMaterials | (1 << GMaxRHIFeatureLevel); }

		ENGINE_API virtual bool isTwoSided() const;

		ENGINE_API virtual bool isMasked() const;

		ENGINE_API virtual float getOpacityMaskClipValue() const;

		ENGINE_API virtual bool isDitheredLODTransition() const;

		virtual bool isDependent(MaterialInterface* testDependency) { return testDependency == this; }

		template<typename FunctionType>
		static void IterateOverActiveFeatureLevels(FunctionType inHandler)
		{
			uint32 featureLevels = getFeatureLevelsToCompileForAllMaterials();
			while (featureLevels != 0)
			{
				inHandler((ERHIFeatureLevel::Type)BitSet::getAndClearNextBit(featureLevels));
			}
		}

	protected:
		void updateMaterialRenderProxy(MaterialRenderProxy& proxy);

		ENGINE_API uint32 getFeatureLevelsToCompileForRendering() const;
	private:
		static void postLoadDefaultMaterials();

		MaterialRelevance getRelevance_Internal(const RMaterial* material, ERHIFeatureLevel::Type inFeatureLevel) const;
	private: 
		uint32 mFeatureLevelsToForceCompile;

		static uint32 mFeatureLevelsForAllMaterials;

	public:
		RenderCommandFence mParentReference;
	};

	
}