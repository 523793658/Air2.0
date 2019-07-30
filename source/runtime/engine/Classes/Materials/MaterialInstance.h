#pragma once
#include "Classes/Materials/MaterialInterface.h"
#include "Containers/EnumAsByte.h"
#include "Classes/Materials/MaterialInstanceBasePropertyOverrides.h"
#include "StaticParameterSet.h"
namespace Air
{

	class RTexture;

	struct ScalarParameterValue
	{
		wstring mParameterName;
		float mParameterValue;
		Guid mExpressionGUID;
		ScalarParameterValue()
			:mParameterValue(0)
		{}
		typedef float ValueType;
		static ValueType getValue(const ScalarParameterValue& parameter)
		{
			return parameter.mParameterValue;
		}

	};

	struct TextureParameterValue
	{
		wstring mParameterName;
		RTexture* mParameterValue;
		Guid mExpressionGUID;
		TextureParameterValue()
			:mParameterValue(nullptr)
		{}
		typedef const RTexture* ValueType;
		static ValueType getValue(const TextureParameterValue& parameter)
		{
			return parameter.mParameterValue;
		}

	};

	struct VectorParameterValue
	{
		wstring mParameterName;
		LinearColor mParameterValue;
		Guid mExpressionGUID;
		VectorParameterValue()
			:mParameterValue(ForceInit)
		{}
		typedef LinearColor ValueType;
		static ValueType getValue(const VectorParameterValue& parameter)
		{
			return parameter.mParameterValue;
		}

	};



	class MaterialInstance : public MaterialInterface
	{
		GENERATED_RCLASS_BODY(MaterialInstance, MaterialInterface)

	public:
		ENGINE_API virtual EBlendMode getBlendMode() const override;

		ENGINE_API virtual std::shared_ptr<RMaterial> getMaterial() override;

		ENGINE_API virtual std::shared_ptr<const RMaterial> getMaterial() const override;

		ENGINE_API virtual bool isMasked() const override;

		ENGINE_API virtual MaterialResource* getMaterialResource(ERHIFeatureLevel::Type inFeatureLvel, EMaterialQualityLevel::Type qualityLevel = EMaterialQualityLevel::Num) override;

		ENGINE_API virtual const MaterialResource* getMaterialResource(ERHIFeatureLevel::Type inFeatureLvel, EMaterialQualityLevel::Type qualityLevel = EMaterialQualityLevel::Num) const override;

		ENGINE_API virtual MaterialResource* allocatePermutationResource();

		ENGINE_API virtual EMaterialShadingModel getShadingModel() const override;

		ENGINE_API bool isChildOf(const MaterialInterface* material) const;

		ENGINE_API virtual bool isDitheredLODTransition() const override;

		ENGINE_API void initStaticPermutation();

		ENGINE_API void initResources();

		ENGINE_API virtual bool isDependent(MaterialInterface* testDependency) override;

		ENGINE_API void updateOverridableBaseProperties();

		ENGINE_API virtual float getOpacityMaskClipValue() const override;

		ENGINE_API virtual bool isPropertyActive(EMaterialProperty inProperty) const override;

		virtual ENGINE_API MaterialRenderProxy* getRenderProxy(bool selected, bool bHovered = false) const override;

		void updatePermutationAllocations();

		void cacheResourceShaderForRendering();

	protected:
		ENGINE_API void setParentInternal(std::shared_ptr<class MaterialInterface>& newParent, bool recacheShaders);

		ENGINE_API virtual bool hasOverridenBaseProperties() const;

		void cacheShaderForResources(EShaderPlatform shaderPlatform, const TArray<MaterialResource*>& resourcesToCache, bool bApplyCompltedShaderMapForRendering);

		void setScalarParameterValueInternal(wstring parameterName, float value);

		bool setScalarParameterByIndexInternal(int32 ParameterIndex, float value);

		void setVectorParameterValueInternal(wstring parameterName, LinearColor& value);

		bool setVectorParameterByIndexInternal(int32 parameterIndex, LinearColor& value);

		void setTextureParameterValueInternal(wstring parameterName, class RTexture* value);

		virtual ENGINE_API void postInitProperties() override;

	private:
		void propagateDataToMaterialProxy();

	public:
		StaticParameterSet mStaticParameters;

		class MaterialInstanceResource* mResources[3];

		TArray<struct ScalarParameterValue> mScalarParameterValues;

		TArray<struct TextureParameterValue> mTextureParameterValues;

		TArray<struct VectorParameterValue> mVectorParameterValues;

		struct MaterialInstanceBasePropertyOverrides mBasePropertyOverrides;

	public:
		MaterialResource * mStaticPermutationMaterialResources[EMaterialQualityLevel::Num][ERHIFeatureLevel::Num];

	public:
		std::shared_ptr<class MaterialInterface> mParent;

		float mOpacityMaskClipValue;

		TEnumAsByte<EMaterialShadingModel> mShadingModel;
		TEnumAsByte<EBlendMode> mBlendMode;

		uint32 mTwoSided : 1;

		uint32 mDitheredLODTransition : 1;

		uint32 bHasStaticPermutationResource : 1;

		uint32 mReentrantflag : 1;
	};
}