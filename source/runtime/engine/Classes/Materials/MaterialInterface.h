#pragma once
#include "EngineMininal.h"
#include "Object.h"
#include "SceneTypes.h"
#include "RHIDefinitions.h"
#include "Classes/Engine/EngineType.h"
#include "RHI.h"
#include "RenderCommandFence.h"
namespace Air
{
	class MaterialResource;
	class MaterialCompiler;
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
	private: 
		uint32 mFeatureLevelsToForceCompile;

		static uint32 mFeatureLevelsForAllMaterials;

	public:
		RenderCommandFence mParentReference;
	};
}