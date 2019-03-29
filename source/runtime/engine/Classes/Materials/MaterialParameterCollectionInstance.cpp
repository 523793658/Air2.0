#include "Classes/Materials/MaterialParameterCollectionInstance.h"
#include "Classes/Materials/MaterialParameterCollection.h"
#include "ParameterCollection.h"
#include "Classes/Engine/World.h"
#include "SimpleReflection.h"
namespace Air
{
	RMaterialParameterCollectionInstance::RMaterialParameterCollectionInstance(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		mResource = nullptr;
	}

	void RMaterialParameterCollectionInstance::setCollection(MaterialParameterCollection* collection, class World* inWorld)
	{
		mCollection = collection;
		mWorld = inWorld;
		updateRenderState();
	}

	void RMaterialParameterCollectionInstance::updateRenderState()
	{
		TArray<float4> parameterData;
		getParameterData(parameterData);
		mResource->GameThread_UpdateContents(mCollection ? mCollection->mStateId : Guid(), parameterData);
		mWorld->updateParameterCollectionInstances(false);
	}

	void RMaterialParameterCollectionInstance::getParameterData(TArray<float4>& parameterData) const
	{
		if (mCollection)
		{

			parameterData.empty(Math::divideAndRoundUp(mCollection->mScalarParameters.size(), 4) + mCollection->mVectorParameters.size());
			for (int32 parameterIndex = 0; parameterIndex < mCollection->mScalarParameters.size(); parameterIndex++)
			{
				const CollectionScalarParameter& parameter = mCollection->mScalarParameters[parameterIndex];
				if (parameterIndex % 4 == 0)
				{
					parameterData.add(float4(0, 0, 0, 0));
				}
				float4& currentVector = parameterData.last();
				auto& it = mScalarParameterValues.find(parameter.mParameterName);
				currentVector[parameterIndex % 4] = it == mScalarParameterValues.end() ? parameter.mDefaultValue : it->second;

			}
			for (int32 parameterIndex = 0; parameterIndex < mCollection->mVectorParameters.size(); ++parameterIndex)
			{
				const CollectionVectorParameter& parameter = mCollection->mVectorParameters[parameterIndex];
				auto& it = mVectorParameterValues.find(parameter.mParameterName);
				parameterData.add(it != mVectorParameterValues.end() ? it->second : parameter.mDefaultValue);
			}
		}
	}
	DECLARE_SIMPLER_REFLECTION(RMaterialParameterCollectionInstance);
}