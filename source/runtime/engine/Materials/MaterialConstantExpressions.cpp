#include "Materials/MaterialConstantExpressions.h"
#include "Classes/Materials/MaterialParameterCollection.h"
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
	IMPLEMENT_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionScalarParameter)

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
}