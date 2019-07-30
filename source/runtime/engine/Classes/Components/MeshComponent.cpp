#include "Classes/Components/MeshComponent.h"
#include "Classes/Materials/MaterialInterface.h"
#include "Classes/GameFramework/Actor.h"
#include "SimpleReflection.h"
#include "Classes/Materials/MaterialInstanceDynamic.h"

namespace Air
{
	MeshComponent::MeshComponent(const ObjectInitializer& objectInitializer /* = ObjectInitializer::get() */)
		:PrimitiveComponent(objectInitializer)
	{
		bCastShadow = true;
		bCachedMaterialParameterIndicesAreDirty = true;
	}

	void MeshComponent::markCachedMaterialParameterNameIndicesDirty()
	{
		bCachedMaterialParameterIndicesAreDirty = true;
	}

	std::shared_ptr<MaterialInterface> MeshComponent::getMaterial(int32 elementIndex) const
	{
		if (mOverrideMaterials.isValidIndex(elementIndex))
		{
			return mOverrideMaterials[elementIndex];
		}
		else
		{
			return nullptr;
		}
	}


	void MeshComponent::setMaterial(int32 elementIndex, class MaterialInterface* material)
	{
		if (elementIndex >= 0)
		{
			if (mOverrideMaterials.isValidIndex(elementIndex) && (mOverrideMaterials[elementIndex].get() == material))
			{

			}
			else
			{
				if (mOverrideMaterials.size() <= elementIndex)
				{
					mOverrideMaterials.addZeroed(elementIndex + 1 - mOverrideMaterials.size());
				}
				if (material != nullptr && mOverrideMaterials[elementIndex] != nullptr)
				{
					RMaterialInstanceDynamic* dynamicMaterial = dynamic_cast<RMaterialInstanceDynamic*>(material);
					if (dynamicMaterial && dynamicMaterial->mParent != mOverrideMaterials[elementIndex])
					{
						markCachedMaterialParameterNameIndicesDirty();
					}
				}
				mOverrideMaterials[elementIndex] = std::dynamic_pointer_cast<MaterialInterface>(material->shared_from_this());
				markRenderStateDirty();
			}
		}
	}

	DECLARE_SIMPLER_REFLECTION(MeshComponent);
}