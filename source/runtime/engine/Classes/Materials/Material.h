#pragma once
#include "Classes/Materials/MaterialInterface.h"
#include "Containers/EnumAsByte.h"
#include "Classes/Engine/EngineType.h"
#include "ResLoader/ResLoader.h"
#include "MaterialShared.h"
#include "Misc/Guid.h"
#include "Classes/Materials/MaterialExpression.h"
namespace Air
{
	enum EMaterialDomain
	{
		MD_Surface,
		MD_DeferredDecal,
		MD_LightFunction,
		MD_PostProcess,
		MD_UI,
		MD_MAX
	};

	class RTexture;

	class RMaterial : public MaterialInterface
	{
		GENERATED_RCLASS_BODY(RMaterial, MaterialInterface)
	public:
		

		ENGINE_API virtual class MaterialRenderProxy* getRenderProxy(bool selected, bool bHovered = false) const override;

		virtual void postInitProperties() override;

		ENGINE_API virtual MaterialResource* getMaterialResource(ERHIFeatureLevel::Type inFeatureLvel, EMaterialQualityLevel::Type qualityLevel = EMaterialQualityLevel::Num) override;

		ENGINE_API virtual const MaterialResource* getMaterialResource(ERHIFeatureLevel::Type inFeatureLvel, EMaterialQualityLevel::Type qualityLevel = EMaterialQualityLevel::Num) const override;

		ENGINE_API virtual std::shared_ptr<RMaterial> getMaterial() override;

		ENGINE_API virtual std::shared_ptr<const RMaterial> getMaterial() const override;

		ENGINE_API void postLoad() override;

		ENGINE_API bool isDefaultMaterial() const;

		ENGINE_API static std::shared_ptr<RMaterial>& getDefaultMaterial(EMaterialDomain domain);

		ENGINE_API static ResLoadingDescPtr createLoadingDesc(wstring const & path);

		ENGINE_API virtual MaterialResource* allocateResource();

		ENGINE_API void appendReferencedTextures(TArray<std::shared_ptr<RTexture>>& inOutTextures) const;

		ENGINE_API virtual EMaterialShadingModel getShadingModel() const override;

		ENGINE_API virtual EBlendMode getBlendMode() const override;

		ENGINE_API virtual void recacheConstantExpressions() const override;

		ENGINE_API virtual bool isTwoSided() const override;

		ENGINE_API virtual bool isMasked() const override;

		ENGINE_API virtual float getOpacityMaskClipValue() const override;

		ENGINE_API virtual bool isDitheredLODTransition() const override;

		ENGINE_API virtual bool isPropertyActive(EMaterialProperty inProperty) const override;
#if WITH_EDITOR
		ENGINE_API virtual int32 compilePropertyEx(class MaterialCompiler* compiler, const Guid& attributeId)override;
#endif

		void cacheExpressionTextureReferences();

	private:
		void propagateDataToMaterialProxy();

		void rebuildExpressionTextureReferences();

		void cacheResourceShadersForRendering(bool bRegenerateId);

		ENGINE_API virtual void flushResourceShaderMaps();

		void updateResourceAllocations();

		void getQualityLevelUsage(TArray<bool, TInlineAllocator<EMaterialQualityLevel::Num>>& qualityLevelsUsed, EShaderPlatform shaderPlatform);

		void cacheShadersForResource(EShaderPlatform shaderPlatform, const TArray<MaterialResource*>& resourceToCache, bool bApplyCompletedShaderMapForRendering);

		ENGINE_API virtual void compileProperty(MaterialCompiler* compiler, wstring** properties, uint32 forceCastFlags = 0) override;

		void initMaterialPropertyTable();

		void _connect();

		class RMaterialExpression* findExpression(uint32 id);

		void rebuildMaterialFunctionInfo();

		void rebuildMaterialParameterCollectionInfo();

	private:
		TEnumAsByte<enum EBlendMode>	mBlendMode{ BLEND_Opaque };
		TEnumAsByte<enum EMaterialShadingModel> mShadingMode{ MSM_DefaultLit };

		MaterialResource* mMaterialResources[EMaterialQualityLevel::Num][ERHIFeatureLevel::Num];

		TArray<MaterialResource> mLoadedMaterialResources;

		TArray <std::shared_ptr<RTexture>> mExpressionTextureReferences;

		uint32 bUseMaterialAttributes : 1;

		MaterialAttributesInput mMaterialAttributes;

		std::shared_ptr<void> mLocalData;
	public:
		class DefaultMaterialInstance* mDefaultMaterialInstances[3];

		friend class DefaultMaterialInstance;

		Guid mStateId;

		int32 mNumCustomizedUVs{ 1 };

		uint32 bWireFrame : 1;

		uint32 mTwoSided : 1;

		uint32 bUsedAsSpecialEngineMaterial : 1;

		uint32 mDitheredLODTransition : 1;

		TEnumAsByte<enum EMaterialDomain> mMaterialDomain;

		float mOpacityMaskClipValue{ 0 };

		TArray<std::shared_ptr<class RMaterialExpression>> mExpressions;
		ScalarMaterialInput mMetallic;
		
		ScalarMaterialInput mSpecular;

		ScalarMaterialInput mRoughness;
		
		ColorMaterialInput mBaseColor;

		VectorMaterialInput mNormal;

		Vector2MaterialInput mCustomizedUVs[8];

		ExpressionInput* mMaterialPropertyTable[MP_Max];
	private:
		friend class MaterialResource;
		friend class MaterialLoadingDesc;
	};
}