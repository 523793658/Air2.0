#pragma once
#include "MaterialShared.h"
#include "Classes/Materials/MaterialInstance.h"
namespace Air
{
	class MaterialInstanceResource : public MaterialRenderProxy
	{
	public:
		template<typename ValueType>
		struct TNamedParameter
		{
			wstring mName;
			ValueType mValue;
		};

		MaterialInstanceResource(MaterialInstance* inOwner, bool bInSelected, bool bInHovered);

		void GameThread_setParent(MaterialInterface* parentMaterialInterface);

		template<typename ValueType>
		void RenderThread_UpdateParameter(const wstring name, ValueType& value)
		{
			invalidateConstantExpressionCache();
			TArray<TNamedParameter<ValueType>>& valueArray = getValueArray<ValueType>();
			const int32 parameterCount = valueArray.size();
			for (int32 parameterIndex = 0; parameterIndex < parameterCount; ++parameterIndex)
			{
				TNamedParameter<ValueType>& parameter = valueArray[parameterIndex];
				if (parameter.mName == name)
				{
					parameter.mValue = value;
					return;
				}
			}
			TNamedParameter<ValueType> newParameter;
			newParameter.mValue = value;
			newParameter.mName = name;
			valueArray.add(newParameter);
		}

		virtual FMaterial* getMaterialNoFallback(ERHIFeatureLevel::Type inFeatureLevel) const override;

		const FMaterial* getMaterial(ERHIFeatureLevel::Type inFeatureLevel) const override;

		bool getScalarValue(const wstring parameterName, float* outValue, const MaterialRenderContext& context) const override;

		bool getVectorValue(const wstring parameterName, LinearColor* outValue, const MaterialRenderContext& context) const override;

		bool getTextureValue(const wstring parameterName, const RTexture** outValue, const MaterialRenderContext& context) const override;

		template<typename ValueType>
		const ValueType* RenderThread_findParameterByName(wstring name) const
		{
			const TArray<TNamedParameter<ValueType>>& valueArray = getValueArray<ValueType>();
			const int32 parameterCount = valueArray.size();
			for (int32 parameterIndex = 0; parameterIndex < parameterCount; parameterIndex++)
			{
				const TNamedParameter<ValueType>& parameter = valueArray[parameterIndex];
				if (parameter.mName == name)
				{
					return &parameter.mValue;
				}
			}
			return nullptr;
		}

	private:
		template<typename ValueType>
		TArray<TNamedParameter<ValueType>>& getValueArray();

		template<typename ValueType>
		const TArray<TNamedParameter<ValueType>>& getValueArray() const;

		TArray<TNamedParameter<float>> mScalarParameterArray;
		TArray<TNamedParameter<const RTexture*>> mTextureParameterArray;
		TArray<TNamedParameter<LinearColor>> mVectorParameterArray;

		MaterialInterface* mParent;
		MaterialInstance* mOwner;
		MaterialInterface* mGameThreadParent;
	};

	template<> FORCEINLINE TArray<MaterialInstanceResource::TNamedParameter<float>>& MaterialInstanceResource::getValueArray() { return mScalarParameterArray; }

	template<> FORCEINLINE TArray<MaterialInstanceResource::TNamedParameter<const RTexture*>>& MaterialInstanceResource::getValueArray() { return mTextureParameterArray; }

	template<> FORCEINLINE TArray<MaterialInstanceResource::TNamedParameter<LinearColor>>& MaterialInstanceResource::getValueArray() { return mVectorParameterArray; }

	template<> FORCEINLINE const TArray<MaterialInstanceResource::TNamedParameter<float>>& MaterialInstanceResource::getValueArray() const { return mScalarParameterArray; }

	template<> FORCEINLINE const TArray<MaterialInstanceResource::TNamedParameter<const RTexture*>>& MaterialInstanceResource::getValueArray() const { return mTextureParameterArray; }

	template<> FORCEINLINE const TArray<MaterialInstanceResource::TNamedParameter<LinearColor>>& MaterialInstanceResource::getValueArray() const { return mVectorParameterArray; }

	class MICReentranceGuard
	{
	public:
		MICReentranceGuard(const MaterialInstance* inMaterial)
			:mMaterial(inMaterial)
		{
			BOOST_ASSERT(isInGameThread() || isAsyncLoading());
			if (mMaterial->mReentrantflag == 1)
			{
				AIR_LOG(LogMaterial, Warning, TEXT("InMaterial: %s GameThread: %d RenderThread: %d"), inMaterial->getName().c_str(), isInGameThread(), isInRenderingThread());
				BOOST_ASSERT(!mMaterial->mReentrantflag);
			}
			const_cast<MaterialInstance*>(mMaterial)->mReentrantflag = 1;
		}

		~MICReentranceGuard()
		{
			BOOST_ASSERT(isInGameThread() || isAsyncLoading());
			const_cast<MaterialInstance*>(mMaterial)->mReentrantflag = 0;
		}
	private:
		const MaterialInstance* mMaterial;
	};

	void cacheMaterialInstanceConstantExpressions(const MaterialInstance* materialInstance);

	template<typename ParameterType>
	ParameterType* GameThread_findParameterByName(TArray<ParameterType>& parameters, wstring parameterName)
	{
		for (int32 parameterIndex = 0; parameterIndex < parameters.size(); parameterIndex++)
		{
			ParameterType* parameter = &parameters[parameterIndex];
			if (parameter->mParameterName == parameterName)
			{
				return parameter;
			}
		}
		return nullptr;
	}

	template<typename ParameterType>
	ParameterType* GameThread_findParameterByIndex(TArray<ParameterType>& parameters, int32 parameterIndex)
	{
		if (!parameters.isValidIndex(parameterIndex))
		{
			return nullptr;
		}
		return &parameters[parameterIndex];
	}
}