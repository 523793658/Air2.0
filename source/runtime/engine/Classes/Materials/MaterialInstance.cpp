#include "Classes/Materials/MaterialInstance.h"
#include "MaterialShared.h"
#include "Classes/Materials/Material.h"
#include "Misc/App.h"
#include "SimpleReflection.h"
#include "Classes/Materials/MaterialInstanceConstant.h"
#include "AirEngine.h"
#include "Classes/Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInstanceSupport.h"
#include "RenderingThread.h"
#include "MaterialInterface.h"
namespace Air
{
	MaterialInstance::MaterialInstance(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		bHasStaticPermutationResource = false;
	}

	template<typename ParameterType>
	void GameThread_UpdateMIParameter(const MaterialInstance* instance, const ParameterType& parameter)
	{
		MaterialInstanceResource* resource = instance->mResources[0];
		const wstring parameterName = parameter.mParameterName;
		typename ParameterType::ValueType value = ParameterType::getValue(parameter);
		ENQUEUE_RENDER_COMMAND(SetMIParameterValue)([resource, parameterName, value](RHICommandListImmediate& RHICmdList) {
			resource->RenderThread_UpdateParameter(parameterName, value);
			});
	}

	template<typename ParameterType>
	void GameThread_InitMIParameters(const MaterialInstance* instance, const TArray<ParameterType>& parameters)
	{
		if (!instance->hasAnyFlags(RF_ClassDefaultObject))
		{
			for (int32 parameterIndex = 0; parameterIndex < parameters.size(); parameterIndex++)
			{
				GameThread_UpdateMIParameter(instance, parameters[parameterIndex]);
			}
		}
	}

	void MaterialInstance::setParentInternal(std::shared_ptr<class MaterialInterface>& newParent, bool recacheShaders)
	{
		if (!mParent || mParent != nullptr)
		{
			std::shared_ptr<MaterialInstance> parentAsMaterialInstance = std::dynamic_pointer_cast<MaterialInstance>(newParent);
			bool bSetParent = false;
			if (parentAsMaterialInstance && parentAsMaterialInstance->isChildOf(this))
			{
				AIR_LOG(LogMaterial, Warning, TEXT("%s is not a valid parent for %s as it is already a child for materialInstance"), newParent->getName().c_str(), getName().c_str());

			}
			else if (newParent && !newParent->isA(RMaterial::StaticClass()))
			{
				AIR_LOG(logMaterial, Warning, TEXT("%s is not a valid parent for %s, only material is valid parent for a materialInstance"), newParent->getName().c_str(), this->getName().c_str());
			}
			else
			{
				mParent = newParent;
				bSetParent = true;
				if (mParent)
				{
					mParent->conditionalPostLoad();
				}
			}
			if (bSetParent && recacheShaders)
			{
				initStaticPermutation();
			}
			else
			{
				initResources();
			}
		}
	}

	void MaterialInstance::initStaticPermutation()
	{
		updateOverridableBaseProperties();

		bHasStaticPermutationResource = (!mStaticParameters.isEmpty() || hasOverridenBaseProperties()) && mParent;

		updatePermutationAllocations();

		if (App::canEverRender())
		{
			cacheResourceShaderForRendering();
		}
	}

	void MaterialInstance::cacheResourceShaderForRendering()
	{
		BOOST_ASSERT(isInGameThread() || isAsyncLoading());

		updatePermutationAllocations();

		updateOverridableBaseProperties();

		if (bHasStaticPermutationResource && App::canEverRender())
		{
			BOOST_ASSERT(isA(MaterialInstanceConstant::StaticClass()));

			uint32 featureLevelsToCompile = getFeatureLevelsToCompileForRendering();

			EMaterialQualityLevel::Type activeQualityLevel = getCachedScalabilityCVars().mMaterialQualityLevel;

			TArray<MaterialResource*> resourceToCache;
			while (featureLevelsToCompile != 0)
			{
				ERHIFeatureLevel::Type featureLevel = (ERHIFeatureLevel::Type)BitSet::getAndClearNextBit(featureLevelsToCompile);
				EShaderPlatform shaderPlatform = GShaderPlatformForFeatureLevel[featureLevel];
				resourceToCache.reset();
				cacheShaderForResources(shaderPlatform, resourceToCache, true);

			}
		}
		initResources();
	}

	void MaterialInstance::updatePermutationAllocations()
	{
		if (bHasStaticPermutationResource)
		{
			std::shared_ptr<RMaterial>& baseMaterial = getMaterial();
			for (int32 featureLevelIndex = 0; featureLevelIndex < ERHIFeatureLevel::Num; featureLevelIndex++)
			{
				EShaderPlatform shaderPlatform = GShaderPlatformForFeatureLevel[featureLevelIndex];
				TArray<bool, TInlineAllocator<EMaterialQualityLevel::Num>> qualityLevelUsed;
				for (int32 qualityLevelIndex = 0; qualityLevelIndex < EMaterialQualityLevel::Num; qualityLevelIndex++)
				{
					MaterialResource*& currentResource = mStaticPermutationMaterialResources[qualityLevelIndex][featureLevelIndex];
					if (!currentResource)
					{
						currentResource = allocatePermutationResource();
					}

					const bool bQualityLevelHasDifferentNodes = qualityLevelUsed[qualityLevelIndex];
					currentResource->setMaterial(baseMaterial.get(), (EMaterialQualityLevel::Type)qualityLevelIndex, bQualityLevelHasDifferentNodes, (ERHIFeatureLevel::Type)featureLevelIndex, std::dynamic_pointer_cast<MaterialInstance>( this->shared_from_this()));
				}
			}
		}
	}

	void MaterialInstance::cacheShaderForResources(EShaderPlatform shaderPlatform, const TArray<MaterialResource *>& resourcesToCache, bool bApplyCompltedShaderMapForRendering)
	{
		std::shared_ptr<RMaterial>& baseMaterial = getMaterial();
		baseMaterial->cacheExpressionTextureReferences();

		for (int32 resourceIndex = 0; resourceIndex < resourcesToCache.size(); resourceIndex++)
		{
			MaterialResource* currentRsource = resourcesToCache[resourceIndex];
			MaterialShaderMapId shaderMapId;
			currentRsource->getShaderMapId(shaderPlatform, shaderMapId);
			const bool bSuccess = currentRsource->cacheShaders(shaderMapId, shaderPlatform, bApplyCompltedShaderMapForRendering);
			if (!bSuccess)
			{
				AIR_LOG(LogMaterial, Warning, TEXT("Failed to compile Material instance %s"), getName().c_str());
				const TArray<wstring>& compileError = currentRsource->getCompileErrors();
				for (int32 errorIndex = 0; errorIndex < compileError.size(); errorIndex++)
				{
					AIR_LOG(logMaterial, Warning, TEXT("%s"), compileError[errorIndex].c_str());
				}
			}
		}
	}

	float MaterialInstance::getOpacityMaskClipValue() const
	{
		return mOpacityMaskClipValue;
	}

	bool MaterialInstance::hasOverridenBaseProperties() const
	{
		BOOST_ASSERT(isInGameThread());

		std::shared_ptr<const RMaterial>& material = getMaterial();
		if (mParent && material && material->bUsedAsSpecialEngineMaterial == false && ((Math::abs(getOpacityMaskClipValue() - mParent->getOpacityMaskClipValue()) > SMALL_NUMBER) ||
			(getBlendMode() != mParent->getBlendMode()) || (getShadingModel() != mParent->getShadingModel()) || (isTwoSided() != mParent->isTwoSided())))
		{
			return true;
		}
		return false;
	}

	void MaterialInstance::initResources()
	{
		std::shared_ptr<MaterialInterface> safeParent;
		if (mParent)
		{
			safeParent = mParent;
		}

		if (safeParent && safeParent->isDependent(this))
		{
			safeParent = nullptr;
		}
		if (safeParent && safeParent->isA(RMaterialInstanceDynamic::StaticClass()))
		{
			safeParent = nullptr;
		}

		if (!safeParent)
		{
			safeParent = RMaterial::getDefaultMaterial(MD_Surface);
		}

		BOOST_ASSERT(safeParent);
		for (int32 curResourceIndex = 0; curResourceIndex < ARRAY_COUNT(mResources); ++curResourceIndex)
		{
			if (mResources[curResourceIndex] != nullptr)
			{
				mResources[curResourceIndex]->GameThread_setParent(safeParent);
			}
		}

		GameThread_InitMIParameters(this, mScalarParameterValues);
		GameThread_InitMIParameters(this, mVectorParameterValues);
		GameThread_InitMIParameters(this, mTextureParameterValues);

		propagateDataToMaterialProxy();

		cacheMaterialInstanceConstantExpressions(this);


	}

	void MaterialInstance::updateOverridableBaseProperties()
	{
		if (!mParent)
		{
			mOpacityMaskClipValue = 0.0f;
			mBlendMode = BLEND_Opaque;
			mShadingModel = MSM_DefaultLit;
			mTwoSided = 0;
			return;
		}
		if (mBasePropertyOverrides.bOverride_OpacityMaskClipValue)
		{
			mOpacityMaskClipValue = mBasePropertyOverrides.mOpacityMaskClipValue;
		}
		else
		{
			mOpacityMaskClipValue = mParent->getOpacityMaskClipValue();
		}

		if (mBasePropertyOverrides.bOverride_BlendMode)
		{
			mBlendMode = mBasePropertyOverrides.mBlendMode;
		}
		else
		{
			mBlendMode = mParent->getBlendMode();
		}

		if (mBasePropertyOverrides.bOverride_ShadingMode)
		{
			mShadingModel = mBasePropertyOverrides.mShadingModel;
		}
		else
		{
			mShadingModel = mParent->getShadingModel();
		}

		if (mBasePropertyOverrides.bOverride_TwoSided)
		{
			mTwoSided = mBasePropertyOverrides.mTwoSided;
		}
		else
		{
			mTwoSided = mParent->isTwoSided();
		}

		if (mBasePropertyOverrides.bOverride_DitheredLODTransition)
		{
			mDitheredLODTransition = mBasePropertyOverrides.mDitheredLODTransition;
		}
		else
		{
			mDitheredLODTransition = mParent->isDitheredLODTransition();
		}

	}
	void MaterialInstance::propagateDataToMaterialProxy()
	{
		for (int32 i = 0; i < ARRAY_COUNT(mResources); i++)
		{
			if (mResources[i])
			{
				updateMaterialRenderProxy(*mResources[i]);
			}
		}
	}

	bool MaterialInstance::isDependent(MaterialInterface* testDependency)
	{
		if (testDependency == this)
		{
			return true;
		}
		else if (mParent)
		{
			if (mReentrantflag)
			{
				return true;
			}
			MICReentranceGuard guard(this);
			return mParent->isDependent(testDependency);
		}
		else
		{
			return false;
		}
	}

	bool MaterialInstance::isChildOf(const MaterialInterface* parentMaterialInterface)const
	{
		const MaterialInterface* material = this;
		while (material != parentMaterialInterface && material != nullptr)
		{
			const MaterialInstance* materialInstance = dynamic_cast<const MaterialInstance*>(material);
			material = (materialInstance != nullptr) ? materialInstance->mParent.get() : nullptr;
		}
		return (material != nullptr);
	}

	MaterialResource* MaterialInstance::allocatePermutationResource()
	{
		return new MaterialResource();
	}

	MaterialResource* MaterialInstance::getMaterialResource(ERHIFeatureLevel::Type inFeatureLvel, EMaterialQualityLevel::Type qualityLevel /* = EMaterialQualityLevel::Num */)
	{
		BOOST_ASSERT(!isInActualRenderinThread());
		if (qualityLevel == EMaterialQualityLevel::Num)
		{
			qualityLevel = getCachedScalabilityCVars().mMaterialQualityLevel;
		}
		if (bHasStaticPermutationResource)
		{
			return mStaticPermutationMaterialResources[qualityLevel][inFeatureLvel];
		}

		return mParent ? mParent->getMaterialResource(inFeatureLvel, qualityLevel) : nullptr;
	}

	const MaterialResource* MaterialInstance::getMaterialResource(ERHIFeatureLevel::Type inFeatureLvel, EMaterialQualityLevel::Type qualityLevel /* = EMaterialQualityLevel::Num */) const
	{
		BOOST_ASSERT(!isInActualRenderinThread());
		if (qualityLevel == EMaterialQualityLevel::Num)
		{
			qualityLevel = getCachedScalabilityCVars().mMaterialQualityLevel;
		}
		if (bHasStaticPermutationResource)
		{
			return mStaticPermutationMaterialResources[qualityLevel][inFeatureLvel];
		}

		return mParent ? mParent->getMaterialResource(inFeatureLvel, qualityLevel) : nullptr;
	}

	std::shared_ptr<const RMaterial> MaterialInstance::getMaterial() const
	{
		BOOST_ASSERT(isInGameThread() || isAsyncLoading());
		if (mReentrantflag)
		{
			return RMaterial::getDefaultMaterial(MD_Surface);
		}
		MICReentranceGuard guard(this);
		if (mParent)
		{
			return mParent->getMaterial();
		}
		else
		{
			return RMaterial::getDefaultMaterial(MD_Surface);
		}
	}

	std::shared_ptr<RMaterial> MaterialInstance::getMaterial() 
	{
		BOOST_ASSERT(isInGameThread() || isAsyncLoading());
		if (mReentrantflag)
		{
			return RMaterial::getDefaultMaterial(MD_Surface);
		}
		MICReentranceGuard guard(this);
		if (mParent)
		{
			return mParent->getMaterial();
		}
		else
		{
			return RMaterial::getDefaultMaterial(MD_Surface);
		}
	}

	bool MaterialInstance::isDitheredLODTransition() const
	{
		return mDitheredLODTransition;
	}

	bool MaterialInstance::isPropertyActive(EMaterialProperty inProperty) const
	{
		return true;
	}

	void MaterialInstance::setScalarParameterValueInternal(wstring parameterName, float value)
	{
		ScalarParameterValue* parameterValue = GameThread_findParameterByName(mScalarParameterValues, parameterName);
		if (!parameterValue)
		{
			parameterValue = new (mScalarParameterValues)ScalarParameterValue();
			parameterValue->mParameterName = parameterName;
			parameterValue->mExpressionGUID.invalidate();
			parameterValue->mParameterValue = value - 1.0f;
		}
		if (parameterValue->mParameterValue != value)
		{
			parameterValue->mParameterValue = value;
			GameThread_UpdateMIParameter(this, *parameterValue);
			cacheMaterialInstanceConstantExpressions(this);
		}
	}

	bool MaterialInstance::setScalarParameterByIndexInternal(int32 ParameterIndex, float value)
	{
		ScalarParameterValue* parameterValue = GameThread_findParameterByIndex(mScalarParameterValues, ParameterIndex);
		if (parameterValue == nullptr)
		{
			return false;
		}
		parameterValue->mParameterValue = value;
		GameThread_UpdateMIParameter(this, *parameterValue);
		cacheMaterialInstanceConstantExpressions(this);
		return true;
	}

	void MaterialInstance::setVectorParameterValueInternal(wstring parameterName, LinearColor& value)
	{
		VectorParameterValue* parameterValue = GameThread_findParameterByName(mVectorParameterValues, parameterName);
		if (!parameterValue)
		{
			parameterValue = new (mVectorParameterValues)VectorParameterValue();
			parameterValue->mParameterName = parameterName;
			parameterValue->mExpressionGUID.invalidate();
			parameterValue->mParameterValue.B = value.B - 1.f;
		}
		if (parameterValue->mParameterValue != value)
		{
			parameterValue->mParameterValue = value;
			GameThread_UpdateMIParameter(this, *parameterValue);
			cacheMaterialInstanceConstantExpressions(this);
		}
	}

	bool MaterialInstance::setVectorParameterByIndexInternal(int32 parameterIndex, LinearColor& value)
	{
		VectorParameterValue* parameterValue = GameThread_findParameterByIndex(mVectorParameterValues, parameterIndex);
		if (!parameterValue)
		{
			return false;
		}
		parameterValue->mParameterValue = value;
		GameThread_UpdateMIParameter(this, *parameterValue);
		cacheMaterialInstanceConstantExpressions(this);
		return true;
	}

	void MaterialInstance::postInitProperties()
	{
		ParentType::postInitProperties();
		if (!hasAnyFlags(RF_ClassDefaultObject))
		{
			mResources[0] = new MaterialInstanceResource(this, false, false);
			if (GIsEditor)
			{
				mResources[1] = new MaterialInstanceResource(this, true, false);
				mResources[2] = new MaterialInstanceResource(this, false, true);
			}
		}
	}

	EMaterialShadingModel MaterialInstance::getShadingModel() const
	{
		return mShadingModel;
	}

	EBlendMode MaterialInstance::getBlendMode()const
	{
		return mBlendMode;
	}

	bool MaterialInstance::isMasked() const
	{
		return getBlendMode() == BLEND_Masked;
	}

	void MaterialInterface::compileProperty(class MaterialCompiler* compiler, wstring** properties, uint32 forceCastFlags /* = 0 */)
	{
		
	}

	MaterialRenderProxy* MaterialInstance::getRenderProxy(bool selected, bool bHovered /* = false */) const
	{
		BOOST_ASSERT(!(selected || bHovered) || GIsEditor);
		return mResources[selected ? 1 : (bHovered ? 2 : 0)];
	}


	DECLARE_SIMPLER_REFLECTION(MaterialInstance);
}