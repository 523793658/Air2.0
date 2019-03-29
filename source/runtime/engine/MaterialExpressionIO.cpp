#include "MaterialExpressionIO.h"
#include "MaterialCompiler.h"
#include "Classes/Materials/MaterialExpression.h"
namespace Air
{
#if WITH_EDITOR
	int32 ScalarMaterialInput::compileWithDefault(class MaterialCompiler* compiler, EMaterialProperty pro)
	{
		if (bUseConstant)
		{
			return compiler->constant(mConstant);
		}
		else if (mExpression)
		{
			int32 resultIndex = ExpressionInput::compile(compiler);
			if (resultIndex != INDEX_NONE)
			{
				return resultIndex;
			}
		}
		return compiler->forceCast(MaterialAttributeDefinationMap::compileDefaultExpression(compiler, pro), MCT_Float1);
	}

	int32 VectorMaterialInput::compileWithDefault(class MaterialCompiler* compiler, EMaterialProperty prop)
	{
		if (bUseConstant)
		{
			return compiler->constant3(mConstant.x, mConstant.y, mConstant.z);
		}
		else if (mExpression)
		{
			int32 resultIndex = ExpressionInput::compile(compiler);
			if (resultIndex != INDEX_NONE)
			{
				return resultIndex;
			}
		}
		return compiler->forceCast(MaterialAttributeDefinationMap::compileDefaultExpression(compiler, prop), MCT_Float3);
	}

	int32 Vector2MaterialInput::compileWithDefault(class MaterialCompiler* compiler, EMaterialProperty prop)
	{
		if (bUseConstant)
		{
			return compiler->constant2(mConstant.x, mConstant.y);

		}
		else if (mExpression)
		{
			int32 resultIndex = ExpressionInput::compile(compiler);
			if (resultIndex != INDEX_NONE)
			{
				return resultIndex;
			}

		}
		return compiler->forceCast(MaterialAttributeDefinationMap::compileDefaultExpression(compiler, prop), MCT_Float2);
	}

	int32 ColorMaterialInput::compileWithDefault(class MaterialCompiler* compiler, EMaterialProperty prop)
	{
		if (bUseConstant)
		{
			LinearColor linearColor(mConstant);
			return compiler->constant3(linearColor.R, linearColor.G, linearColor.B);

		}
		else if (mExpression)
		{
			int32 resultIndex = ExpressionInput::compile(compiler);
			if (resultIndex != INDEX_NONE)
			{
				return resultIndex;
			}

		}
		return compiler->forceCast(MaterialAttributeDefinationMap::compileDefaultExpression(compiler, prop), MCT_Float3);
	}

	int32 ExpressionInput::compile(class MaterialCompiler* compiler)
	{
		if (mExpression)
		{
			mExpression->validateState();
			int32 expressionResult = compiler->callExpression(MaterialExpressionKey(mExpression, mOutputIndex, compiler->getMaterialAttribute(), compiler->isCurrentlyCompilingForPreviousFrame()), compiler);
			if (mMask && expressionResult != INDEX_NONE)
			{
				return compiler->componentMask(expressionResult, !!mMaskR, !!mMaskG, !!mMaskB, !!mMaskA);
			}
			else
			{
				return expressionResult;
			}
		}
		else
		{
			return INDEX_NONE;
		}
	}


	void ExpressionInput::connect(int32 inOutputIndex, class RMaterialExpression* inExpression)
	{
		mOutputIndex = inOutputIndex;
		mExpression = inExpression;
		TArray<ExpressionOutput>& outputs = mExpression->getOutputs();
		ExpressionOutput* output = &outputs[mOutputIndex];
		mMask = output->mMask;
		mMaskR = output->mMaskR;
		mMaskG = output->mMaskG;
		mMaskB = output->mMaskB;
		mMaskA = output->mMaskA;

	}

	void ExpressionInput::connect(class RMaterialExpression* inExpression)
	{
		mExpression = inExpression;
		TArray<ExpressionOutput>& outputs = mExpression->getOutputs();
		ExpressionOutput* output = &outputs[mOutputIndex];
		mMask = output->mMask;
		mMaskR = output->mMaskR;
		mMaskG = output->mMaskG;
		mMaskB = output->mMaskB;
		mMaskA = output->mMaskA;
	}
#endif

	ExpressionInput ExpressionInput::getTracedInput() const
	{
		return *this;
	}
}