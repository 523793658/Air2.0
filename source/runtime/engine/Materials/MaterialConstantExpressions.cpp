#include "Materials/MaterialConstantExpressions.h"
#include "Classes/Materials/MaterialParameterCollection.h"
#include "SceneManagement.h"
#include "RenderUtils.h"
#include "Texture.h"
namespace Air
{
	IMPLEMENT_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionComponentSwizzle);

	IMPLEMENT_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionVectorParameter);

	IMPLEMENT_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionFoldedMath);

	IMPLEMENT_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionMin);
	IMPLEMENT_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionMax);
	IMPLEMENT_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionClamp);
	IMPLEMENT_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionSaturate);
	IMPLEMENT_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionLogarithm2);
	IMPLEMENT_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionSquareRoot);
	IMPLEMENT_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionLength);
	IMPLEMENT_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionAppendVector);
	IMPLEMENT_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionConstant);
	IMPLEMENT_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionScalarParameter);
	IMPLEMENT_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionTexture);
	IMPLEMENT_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionTextureParameter);

	MaterialConstantExpressionTexture::MaterialConstantExpressionTexture()
		:mTextureIndex(INDEX_NONE)
		,mSamplerSource(SSM_FromTextureAsset)
		,mTransientOverrideValue_GameThread(nullptr)
		,mTransientOverrideValue_RenderThread(nullptr)
	{

	}

	MaterialConstantExpressionTexture::MaterialConstantExpressionTexture(int32 inTextureIndex, ESamplerSourceMode inSamplerSource)
		:mTextureIndex(inTextureIndex)
		,mSamplerSource(inSamplerSource)
		,mTransientOverrideValue_GameThread(nullptr)
		,mTransientOverrideValue_RenderThread(nullptr)
	{}

	void ConstantExpressionSet::setParameterCollections(const TArray<class MaterialParameterCollection *>& collections)
	{
		mParameterCollections.empty(collections.size());
		for (int32 collectionIndex = 0; collectionIndex < collections.size(); collectionIndex++)
		{
			mParameterCollections.add(collections[collectionIndex]->mStateId);
		}
	}

	const ShaderParametersMetadata& ConstantExpressionSet::getConstantBufferStruct() const
	{
		return mConstantBufferStruct.getValue();
	}

	void ConstantExpressionSet::fillConstantBuffer(const MaterialRenderContext& materialRenderContext, const ConstantExpressionCache& constantExpressionCache, uint8* tempBuffer, int tempBufferSize) const
	{
		BOOST_ASSERT(mConstantBufferStruct);
		BOOST_ASSERT(isInParallelRenderingThread());

		if (mConstantBufferStruct->getSize() > 0)
		{
			void* bufferCursor = tempBuffer;
			BOOST_ASSERT(bufferCursor <= tempBuffer + tempBufferSize);


			for (int32 vectorIndex = 0; vectorIndex < mConstantVectorExpressions.size(); ++vectorIndex)
			{
				LinearColor vectorValue(0, 0, 0, 0);
				mConstantVectorExpressions[vectorIndex]->getNumberValue(materialRenderContext, vectorValue);

				LinearColor* destAddress = (LinearColor*)bufferCursor;
				*destAddress = vectorValue;
				bufferCursor = destAddress + 1;
				BOOST_ASSERT(bufferCursor <= tempBuffer + tempBufferSize);
			}

			for (int32 scalarIndex = 0; scalarIndex < mConstantScalarExpressions.size(); scalarIndex++)
			{
				LinearColor vectorValue(0, 0, 0, 0);
				mConstantScalarExpressions[scalarIndex]->getNumberValue(materialRenderContext, vectorValue);
				float* destAddress = (float*)bufferCursor;
				*destAddress = vectorValue.R;
				bufferCursor = destAddress + 1;
				BOOST_ASSERT(bufferCursor <= tempBufferSize + tempBuffer);
			}

			bufferCursor = ((float*)bufferCursor) + ((4 - mConstantScalarExpressions.size() % 4) % 4);
			BOOST_ASSERT(bufferCursor <= tempBuffer + tempBufferSize);

			for (int32 expressionIndex = 0; expressionIndex < mConstant2DTextureExpression.size(); expressionIndex++)
			{
				std::shared_ptr<RTexture> value;
				mConstant2DTextureExpression[expressionIndex]->getTextureValue(materialRenderContext, materialRenderContext.mMaterial, value);
				if (value)
				{
					const MaterialConstantExpressionTextureParameter* textureParameter = (mConstant2DTextureExpression[expressionIndex]->getType() == &MaterialConstantExpressionTextureParameter::mStaticType) ? &static_cast<const MaterialConstantExpressionTextureParameter&>(*mConstant2DTextureExpression[expressionIndex]) : nullptr;

					if (value->getMaterialType() == MCT_TextureExternal)
					{

					}
				}
				void** resourceTableTexturePtr = (void**)((uint8*)bufferCursor + 0 * SHADER_PARAMETER_POINTER_ALIGNMENT);
				void** resourceTableSamplerPtr = (void**)((uint8*)bufferCursor + 1 * SHADER_PARAMETER_POINTER_ALIGNMENT);

				bufferCursor = ((uint8*)bufferCursor) + (SHADER_PARAMETER_POINTER_ALIGNMENT * 2);

				BOOST_ASSERT(bufferCursor <= tempBuffer + tempBufferSize);

				const uint32 validTextureTypes = MCT_Texture2D;

				if (value && value->mResource && value->mTextureReference.mTextureReferenceRHI && (value->getMaterialType() & validTextureTypes) != 0u)
				{
					BOOST_ASSERT(value->isA(RTexture::StaticClass()));
					*resourceTableTexturePtr = value->mTextureReference.mTextureReferenceRHI;
					SamplerStateRHIRef* samplerSource = &value->mResource->mSamplerStateRHI;

					ESamplerSourceMode sourceMode = mConstant2DTextureExpression[expressionIndex]->getSamplerSource();
					if (sourceMode == SSM_Wrap_WorldGroupSettings)
					{
						samplerSource = &Wrap_WorldGroupSettings->mSamplerStateRHI;
					}
					else if (sourceMode == SSM_Clamp_WorldGroupSettings)
					{
						samplerSource = &Clamp_WorldGroupSettings->mSamplerStateRHI;
					}

					BOOST_ASSERT(*samplerSource);
					*resourceTableSamplerPtr = *samplerSource;
				}
				else
				{
					BOOST_ASSERT(GWhiteTexture->mTextureRHI);
					*resourceTableTexturePtr = GWhiteTexture->mTextureRHI;
					BOOST_ASSERT(GWhiteTexture->mSamplerStateRHI);
					*resourceTableSamplerPtr = GWhiteTexture->mSamplerStateRHI;
				}

			}
			for (int32 expressinIndex = 0; expressinIndex < mConstantCubeTextureExpressions.size(); expressinIndex++)
			{
				std::shared_ptr<RTexture> value;
				mConstantCubeTextureExpressions[expressinIndex]->getTextureValue(materialRenderContext, materialRenderContext.mMaterial, value);
				void** resourceTableTexturePtr = (void**)((uint8*)bufferCursor + 0 * SHADER_PARAMETER_POINTER_ALIGNMENT);
				void** resourceTableSamplerPtr = (void**)((uint8*)bufferCursor + 1 * SHADER_PARAMETER_POINTER_ALIGNMENT);
				bufferCursor = ((uint8*)bufferCursor) + (SHADER_PARAMETER_POINTER_ALIGNMENT * 2);

				BOOST_ASSERT(bufferCursor <= tempBufferSize + tempBuffer);

				if (value && value->mResource && (value->getMaterialType() & MCT_TextureCube) != 0u)
				{
					BOOST_ASSERT(value->mTextureReference.mTextureReferenceRHI);
					*resourceTableTexturePtr = value->mTextureReference.mTextureReferenceRHI;
					SamplerStateRHIRef* samplerSource = &value->mResource->mSamplerStateRHI;
					ESamplerSourceMode sourceMode = mConstantCubeTextureExpressions[expressinIndex]->getSamplerSource();
					if (sourceMode == SSM_Wrap_WorldGroupSettings)
					{
						samplerSource = &Wrap_WorldGroupSettings->mSamplerStateRHI;
					}
					else if (sourceMode == SSM_Clamp_WorldGroupSettings)
					{
						samplerSource = &Clamp_WorldGroupSettings->mSamplerStateRHI;
					}
					BOOST_ASSERT(*samplerSource);
					*resourceTableSamplerPtr = *samplerSource;
				}
				else
				{
					BOOST_ASSERT(GWhiteTextureCube->mTextureRHI);
					*resourceTableTexturePtr = GWhiteTextureCube->mTextureRHI;
					BOOST_ASSERT(GWhiteTextureCube->mSamplerStateRHI);
					*resourceTableSamplerPtr = GWhiteTextureCube->mSamplerStateRHI;
				}
			}

			{
				void** Wrap_WorldGroupSettingsSamplerPtr = (void**)((uint8*)bufferCursor + 0 * SHADER_PARAMETER_POINTER_ALIGNMENT);
				BOOST_ASSERT(Wrap_WorldGroupSettings->mSamplerStateRHI);
				*Wrap_WorldGroupSettingsSamplerPtr = Wrap_WorldGroupSettings->mSamplerStateRHI;

				void** Clamp_WorldGroupSettingsSamplerPtr = (void**)((uint8*)bufferCursor + 1 * SHADER_PARAMETER_POINTER_ALIGNMENT);
				BOOST_ASSERT(Clamp_WorldGroupSettings->mSamplerStateRHI);
				*Clamp_WorldGroupSettingsSamplerPtr = Clamp_WorldGroupSettings->mSamplerStateRHI;

				bufferCursor = ((uint8*)bufferCursor) + (SHADER_PARAMETER_POINTER_ALIGNMENT * 2);
				BOOST_ASSERT(bufferCursor <= tempBuffer + tempBufferSize);
			}
		}
	}
}