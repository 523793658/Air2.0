#pragma once
#include "CoreMinimal.h"
#include "Misc/Guid.h"
#include "HAL/PlatformMisc.h"
#include "ConstantBuffer.h"
#include "Object.h"
namespace Air
{
	struct CollectionParameterBase
	{
		Guid mId;
		wstring mParameterName;

		CollectionParameterBase()
		{
			PlatformMisc::createGuid(mId);
		}
	};

	struct CollectionScalarParameter : public CollectionParameterBase
	{
		float mDefaultValue;

		CollectionScalarParameter()
		{
			mParameterName = TEXT("Scalar");
		}

	};

	struct CollectionVectorParameter : public CollectionParameterBase
	{
		CollectionVectorParameter()
		{
			mParameterName = TEXT("Vector");
		}
		LinearColor mDefaultValue;
	};

	class MaterialParameterCollection : public Object
	{
		GENERATED_RCLASS_BODY(MaterialParameterCollection, Object)
	public:

		Guid mStateId;
		TArray<CollectionScalarParameter> mScalarParameters;

		TArray<CollectionVectorParameter> mVectorParameters;
	public:
		const ShaderParametersMetadata& getCosntantBufferStruct() const
		{
			BOOST_ASSERT(mShaderParametersMetadata);
			return *mShaderParametersMetadata;
		}

		virtual void postLoad() override;

	
	private:
		std::unique_ptr<ShaderParametersMetadata> mShaderParametersMetadata;
	
		class MaterialParameterCollectionInstanceResource* mDefaultResource;

		void createBufferStruct();

		void getDefaultParametrData(TArray<float4>& parameterData) const;

		void updateDefaultResource(bool bRecreateConstantBuffer);
	};
}