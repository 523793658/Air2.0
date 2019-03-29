#pragma once
#include "Object.h"
#include "EngineMininal.h"
#include "Containers/Map.h"
namespace Air
{
	class MaterialParameterCollectionInstanceResource;
	class MaterialParameterCollection;
	class ENGINE_API RMaterialParameterCollectionInstance : public Object
	{
		GENERATED_RCLASS_BODY(RMaterialParameterCollectionInstance, Object)
	public:
		const MaterialParameterCollection* getCollection() const
		{
			return mCollection;
		}

		void setCollection(MaterialParameterCollection* collection, class World* inWorld);

		MaterialParameterCollectionInstanceResource* getResource()
		{
			return mResource;
		}

		void updateRenderState();
	protected:
		void getParameterData(TArray<float4>& parameterData) const;

	protected:
		MaterialParameterCollection * mCollection;

		class World* mWorld;

		MaterialParameterCollectionInstanceResource* mResource;
		TMap<wstring, float> mScalarParameterValues;

		TMap<wstring, LinearColor> mVectorParameterValues;
	};
}