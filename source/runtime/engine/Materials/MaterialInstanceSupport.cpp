#include "Materials/MaterialInstanceSupport.h"
#include "AirEngine.h"
#include "Classes/Materials/MaterialInterface.h"
#include "Classes/Materials/MaterialInstance.h"
#include "Classes/Materials/Material.h"
namespace Air
{
	void cacheMaterialInstanceConstantExpressions(const MaterialInstance* materialInstance)
	{
		if (materialInstance->mResources[0])
		{
			materialInstance->mResources[0]->cacheConstantExpressions_GameThread();
		}
	}

	MaterialInstanceResource::MaterialInstanceResource(MaterialInstance* inOwner, bool bInSelected, bool bInHovered)
		:MaterialRenderProxy(bInSelected, bInHovered)
		,mParent(nullptr)
		,mOwner(inOwner)
		,mGameThreadParent(nullptr)
	{}

	FMaterial* MaterialInstanceResource::getMaterialNoFallback(ERHIFeatureLevel::Type inFeatureLevel) const
	{
		BOOST_ASSERT(isInParallelRenderingThread());
		if (mParent)
		{
			if (mOwner->bHasStaticPermutationResource)
			{
				EMaterialQualityLevel::Type activeQualityLevel = getCachedScalabilityCVars().mMaterialQualityLevel;
				MaterialResource* staticPermutationResource = mOwner->mStaticPermutationMaterialResources[activeQualityLevel][inFeatureLevel];
				return staticPermutationResource;
			}
			else
			{
				MaterialRenderProxy* parentProxy = mParent->getRenderProxy(isSelected(), isHovered());
				if (parentProxy)
				{
					return parentProxy->getMaterialNoFallback(inFeatureLevel);
				}
			}
		}
		return nullptr;
	}

	void MaterialInstanceResource::GameThread_setParent(std::shared_ptr<MaterialInterface>& parentMaterialInterface)
	{
		BOOST_ASSERT(isInGameThread() || isAsyncLoading());
		if (mGameThreadParent != parentMaterialInterface)
		{
			std::shared_ptr<MaterialInterface>& oldParent = mGameThreadParent;
			mGameThreadParent = parentMaterialInterface;
			BOOST_ASSERT(parentMaterialInterface != nullptr);
			ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
				InitMaterialInstanceResource, MaterialInstanceResource*, resource, this, std::shared_ptr<MaterialInterface>, parent, parentMaterialInterface,
				{
					resource->mParent = parent;
					resource->invalidateConstantExpressionCache();
				}
			);
			if (oldParent)
			{
				oldParent->mParentReference.beginFence();
			}
		}
	}

	bool MaterialInstanceResource::getScalarValue(const wstring parameterName, float* outValue, const MaterialRenderContext& context) const
	{
		BOOST_ASSERT(isInParallelRenderingThread());
		static wstring nameSubsurfaceProfile(TEXT("__SubsurfaceProfile"));
		if (parameterName == nameSubsurfaceProfile)
		{

		}
		const float* value = RenderThread_findParameterByName<float>(parameterName);
		if (value)
		{
			*outValue = *value;
			return true;
		}

		else if (mParent)
		{
			return mParent->getRenderProxy(isSelected(), isHovered())->getScalarValue(parameterName, outValue, context);
		}
		else
		{
			return false;
		}
	}

	bool MaterialInstanceResource::getVectorValue(const wstring parameterName, LinearColor* outValue, const MaterialRenderContext& context) const
	{
		BOOST_ASSERT(isInParallelRenderingThread());
		const LinearColor* value = RenderThread_findParameterByName<LinearColor>(parameterName);
		if (value)
		{
			*outValue = *value;
			return true;
		}
		else if (mParent)
		{
			return mParent->getRenderProxy(isSelected(), isHovered())->getVectorValue(parameterName, outValue, context);
		}
		else
		{
			return false;
		}
	}

	bool MaterialInstanceResource::getTextureValue(const wstring parameterName, std::shared_ptr<const RTexture>& outValue, const MaterialRenderContext& context) const
	{
		BOOST_ASSERT(isInParallelRenderingThread());
		const RTexture* const * value = RenderThread_findParameterByName<const RTexture*>(parameterName);
		if (value && *value)
		{
			outValue.reset(*value);
			return true;
		}
		else if (mParent)
		{
			return mParent->getRenderProxy(isSelected(), isHovered())->getTextureValue(parameterName, outValue, context);
		}
		else
		{
			return false;
		}
	}

	const FMaterial* MaterialInstanceResource::getMaterial(ERHIFeatureLevel::Type inFeatureLevel) const
	{
		BOOST_ASSERT(isInParallelRenderingThread());
		if (mParent)
		{
			if (mOwner->bHasStaticPermutationResource)
			{
				EMaterialQualityLevel::Type activeQualityLevel = getCachedScalabilityCVars().mMaterialQualityLevel;
				MaterialResource* staticPermutationResource = mOwner->mStaticPermutationMaterialResources[activeQualityLevel][inFeatureLevel];
				if (staticPermutationResource->getRenderingThreadShaderMap())
				{
					BOOST_ASSERT(staticPermutationResource->getRenderingThreadShaderMap()->isCompilationFinalized());
					BOOST_ASSERT(staticPermutationResource->getRenderingThreadShaderMap()->compiledSuccessfully());
					return staticPermutationResource;
				}
				else
				{
					EMaterialDomain domain = (EMaterialDomain)staticPermutationResource->getMaterialDomain();
					std::shared_ptr<RMaterial>& fallbackMaterial = RMaterial::getDefaultMaterial(domain);
					return fallbackMaterial->getRenderProxy(isSelected(), isHovered())->getMaterial(inFeatureLevel);
				}

			}
			else
			{
				return mParent->getRenderProxy(isSelected(), isHovered())->getMaterial(inFeatureLevel);
			}
		}
		else
		{
			std::shared_ptr<RMaterial>& fallbackMaterial = RMaterial::getDefaultMaterial(MD_Surface);
			return fallbackMaterial->getRenderProxy(isSelected(), isHovered())->getMaterial(inFeatureLevel);
		}
	}
}