#include "Classes/Materials/MaterialInterface.h"
#include "Classes/Materials/Material.h"
#include "MaterialCompiler.h"
namespace Air
{

	uint32 MaterialInterface::mFeatureLevelsForAllMaterials = 0;

	MaterialInterface::MaterialInterface(const ObjectInitializer& objectInitializer /* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{

	}

	void MaterialInterface::postLoad()
	{
		ParentType::postLoad();
		postLoadDefaultMaterials();
	}

	

	EMaterialShadingModel MaterialInterface::getShadingModel() const
	{
		return EMaterialShadingModel::MSM_DefaultLit;
	}

	void MaterialInterface::updateMaterialRenderProxy(MaterialRenderProxy& proxy)
	{
		BOOST_ASSERT(&proxy);
		EMaterialShadingModel mateiralShadingMode = getShadingModel();
	}

	uint32 MaterialInterface::getFeatureLevelsToCompileForRendering() const
	{
		return mFeatureLevelsToForceCompile | getFeatureLevelsToCompileForAllMaterials();
	}

	bool MaterialInterface::isTwoSided() const
	{
		return false;
	}

	EBlendMode MaterialInterface::getBlendMode() const
	{
		return BLEND_Opaque;
	}
	bool MaterialInterface::isMasked() const
	{
		return false;
	}

	float MaterialInterface::getOpacityMaskClipValue() const
	{
		return 0.0f;
	}

	bool MaterialInterface::isDitheredLODTransition() const
	{
		return false;
	}

	int32 MaterialInterface::compileProperty(MaterialCompiler* compiler, EMaterialProperty prop, uint32 forceCastFlags /* = 0 */)
	{
		int32 result = INDEX_NONE;
		if (isPropertyActive(prop))
		{
			result = compilePropertyEx(compiler, MaterialAttributeDefinationMap::getID(prop));

		}
		else
		{
			result = MaterialAttributeDefinationMap::compileDefaultExpression(compiler, prop);
		}
		if (forceCastFlags & MFCF_ForceCast)
		{
			result = compiler->forceCast(result, MaterialAttributeDefinationMap::getValueType(prop), forceCastFlags);
		}
		return result;
	}

	bool MaterialInterface::isPropertyActive(EMaterialProperty inProperty) const
	{
		return false;
	}

#if WITH_EDITOR
	int32 MaterialInterface::compilePropertyEx(class MaterialCompiler* compiler, const Guid& attributeId)
	{
		return INDEX_NONE;
	}
#endif

	MaterialRelevance MaterialInterface::getRelevance(ERHIFeatureLevel::Type inFeatureLevel) const
	{
		const RMaterial* material = getMaterial();
		return getRelevance_Internal(material, inFeatureLevel);
	}

	MaterialRelevance MaterialInterface::getRelevance_Internal(const RMaterial* material, ERHIFeatureLevel::Type inFeatureLevel) const
	{
		if (material)
		{
			const MaterialResource* materialResource = material->getMaterialResource(inFeatureLevel);
			const bool bIsTranslucent = isTranslucentBlendMode((EBlendMode)getBlendMode());
			EMaterialShadingModel shadingModel = getShadingModel();
			EMaterialDomain domain = (EMaterialDomain)materialResource->getMaterialDomain();
			bool bDecal = (domain == MD_DeferredDecal);

			MaterialRelevance materialRelevance;
			materialRelevance.mShadingModelMask = 1 << shadingModel;
			if (bDecal)
			{
				materialRelevance.bDecal = bDecal;
			}
			else
			{
				materialRelevance.bOpaque = !bIsTranslucent;
				materialRelevance.bMasked = isMasked();
				//materialRelevance.bDistortion= materialResource->isdisto
			}
			return materialRelevance;
		}
		else
		{
			return MaterialRelevance();
		}
	}

	void MaterialRelevance::setPrimitiveViewRelevance(PrimitiveViewRelevance& outViewRelevance) const
	{
		outViewRelevance.bOpaqueRelevance = bOpaque;
		outViewRelevance.bMaskedRelevance = bMasked;
		outViewRelevance.bDistortionRelevance = bDistortion;
		outViewRelevance.bSeparateTranslucencyRelevance = bSeparateTranslucency;
		outViewRelevance.bMobileSeparateTranslucencyRelevance = bMobileSeparateTranslucency;
		outViewRelevance.bNormalTranslucencyRelevance = bNormalTranslucency;
		outViewRelevance.mShadingModelMaskRelevance = mShadingModelMask;
		outViewRelevance.bUsesGlobalDistanceField = bUsesGlobalDistanceField;
		outViewRelevance.bUsesWorldPositionOffset = bUsesWorldPositionOffset;
		outViewRelevance.bDecal = bDecal;
		outViewRelevance.bTranslucentSurfaceLighting = bTranslucentSurfaceLighting;
		outViewRelevance.bUsesSceneDepth = bUsersSceneDpeth;
	}
}