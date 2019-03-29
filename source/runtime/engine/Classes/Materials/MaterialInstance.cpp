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
namespace Air
{
	MaterialInstance::MaterialInstance(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		bHasStaticPermutationResource = false;
	}

	ENQUEUE_UNIQUE_RENDER_COMMAND_FIVEPARAMETER_DECLARE_TEMPLATE(
		SetMIParameterValue, ParameterType,
		MaterialInstanceResource*, Resource0, Resource0,
		MaterialInstanceResource*, Resource1, Resource1,
		MaterialInstanceResource*, Resource2, Resource2,
		wstring, ParameterName, Parameter.mParameterName,
		typename ParameterType::ValueType, Value, ParameterType::getValue(Parameter),
		{
			Resource0->RenderThread_UpdateParameter(ParameterName, Value);
			if (Resource1)
			{
				Resource1->RenderThread_UpdateParameter(ParameterName, Value);
			}
			if (Resource2)
			{
				Resource2->RenderThread_UpdateParameter(ParameterName, Value);
			}
		});

	template<typename ParameterType>
	void GameThread_UpdateMIParameter(const MaterialInstance* instance, const ParameterType& parameter)
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_FIVEPARAMETER_CREATE_TEMPLATE(SetMIParameterValue, ParameterType,
			MaterialInstanceResource*, instance->mResources[0],
			MaterialInstanceResource*, instance->mResources[1],
			MaterialInstanceResource*, instance->mResources[2],
			wstring, parameter.mParameterName,
			typename ParameterType::ValueType, ParameterType::getValue(parameter));
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

	void MaterialInstance::setParentInternal(class MaterialInterface* newParent, bool recacheShaders)
	{
		if (!mParent || mParent != nullptr)
		{
			MaterialInstance* parentAsMaterialInstance = static_cast<MaterialInstance*>(newParent);
			bool bSetParent = false;
			if (parentAsMaterialInstance != nullptr && parentAsMaterialInstance->isChildOf(this))
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
			RMaterial* baseMaterial = getMaterial();
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
					currentResource->setMaterial(baseMaterial, (EMaterialQualityLevel::Type)qualityLevelIndex, bQualityLevelHasDifferentNodes, (ERHIFeatureLevel::Type)featureLevelIndex, this);
				}
			}
		}
	}

	void MaterialInstance::cacheShaderForResources(EShaderPlatform shaderPlatform, const TArray<MaterialResource *>& resourcesToCache, bool bApplyCompltedShaderMapForRendering)
	{
		RMaterial* baseMaterial = getMaterial();
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

		const RMaterial* material = getMaterial();
		if (mParent && material && material->bUsedAsSpecialEngineMaterial == false && ((Math::abs(getOpacityMaskClipValue() - mParent->getOpacityMaskClipValue()) > SMALL_NUMBER) ||
			(getBlendMode() != mParent->getBlendMode()) || (getShadingModel() != mParent->getShadingModel()) || (isTwoSided() != mParent->isTwoSided())))
		{
			return true;
		}
		return false;
	}

	void MaterialInstance::initResources()
	{
		MaterialInterface* safeParent = nullptr;
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
			material = (materialInstance != nullptr) ? materialInstance->mParent : nullptr;
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

	const RMaterial* MaterialInstance::getMaterial() const
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

	RMaterial* MaterialInstance::getMaterial() 
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


	DECLARE_SIMPLER_REFLECTION(MaterialInstance);
}