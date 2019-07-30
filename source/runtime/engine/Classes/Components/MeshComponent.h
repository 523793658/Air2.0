#pragma once
#include "Classes/Components/PrimitiveComponent.h"
namespace Air
{
	class ENGINE_API MeshComponent : public PrimitiveComponent
	{
		GENERATED_RCLASS_BODY(MeshComponent, PrimitiveComponent)
	public:

		virtual std::shared_ptr<class MaterialInterface> getMaterial(int32 elementIndex) const override;

		virtual void setMaterial(int32 elementIndex, class MaterialInterface* material) override;

	protected:
		void markCachedMaterialParameterNameIndicesDirty();

	protected:
		uint32 bCachedMaterialParameterIndicesAreDirty : 1;

	private:
		TArray<std::shared_ptr<class MaterialInterface>> mOverrideMaterials;
	};
}