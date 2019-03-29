#include "MaterialExpression.h"
#include "MaterialExpressionCustom.h"
#include "SimpleReflection.h"
#include "MaterialExpressionMaterialFunctionCall.h"
#include "MaterialExpressionFunctionInput.h"
#include "MaterialExpressionFunctionOutput.h"
#include "MaterialShared.h"
#include "material.h"
#include "Misc/App.h"
#include "MaterialCompiler.h"
#include "Classes/Materials/XMLMaterialTranslator.h"
#include "MaterialExpressionScalarParameter.h"
namespace Air
{
	static bool readExpressionInputXML(ExpressionInput& input, XMLNode* node)
	{
		input.mOutputIndex = boost::lexical_cast<uint32>(node->first_node(TEXT("OutputIndex"))->value());
		input.mInputName = node->first_node(TEXT("InputName"))->value();
		input.mExpressionId = boost::lexical_cast<uint32>(node->first_node(TEXT("ExpressionID"))->value());
		return true;
	}

	static void readExpressionOutputXML(ExpressionOutput& output, XMLNode* node)
	{
		output.mOutputName = node->first_node(TEXT("Name"))->value();
		output.mMask = boost::lexical_cast<int32>(node->first_node(TEXT("Mask"))->value());
		output.mMaskR = boost::lexical_cast<int32>(node->first_node(TEXT("MaskR"))->value());
		output.mMaskG = boost::lexical_cast<int32>(node->first_node(TEXT("MaskG"))->value());
		output.mMaskB = boost::lexical_cast<int32>(node->first_node(TEXT("MaskB"))->value());
		output.mMaskA = boost::lexical_cast<int32>(node->first_node(TEXT("MaskA"))->value());

	}

	RMaterialExpression::RMaterialExpression(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		RMaterial* material = getTypedOuter<RMaterial>();
		material->mExpressions.add(this);
		mOutputs.add(ExpressionOutput(TEXT("")));
	}

	void RMaterialExpression::postLoad()
	{
		if (!mMaterial && getOuter()->isA(RMaterial::StaticClass()))
		{
			mMaterial = check_cast<RMaterial*>(getOuter());
		}
		if (!mFunction && getOuter()->isA(RMaterialFunction::StaticClass()))
		{
			mFunction = check_cast<RMaterialFunction*>(getOuter());
		}
		updateParameterGuid(false, false);
		updateMaterialExpressionGuid(false, false);
	}

	void RMaterialExpression::updateParameterGuid(bool bForceGeneration, bool bAllowMarkingPakageDirty)
	{
		if (bIsParameterExpression)
		{
			Guid& guid = getParameterExpressionId();
			if (bForceGeneration || !guid.isValid())
			{
				guid = Guid::newGuid();
			}
		}
	}

	void RMaterialExpression::updateMaterialExpressionGuid(bool bForceGeration, bool allowMarkingPacakagedirty)
	{
		Guid& guid = getMaterialExpressionId();
		if (bForceGeration || !guid.isValid())
		{
			guid = Guid::newGuid();
		}
	}

	Guid& RMaterialExpression::getParameterExpressionId()
	{
		BOOST_ASSERT(!bIsParameterExpression);
		static Guid mDummy;
		return mDummy;
	}

	Guid& RMaterialExpression::getMaterialExpressionId()
	{
#if WITH_EDITORONLY_DATA
		return mMaterialExpressionGuid;
#endif
		static Guid mDummy;
		return mDummy;
	}

#if WITH_EDITOR
	int32 RMaterialExpressionCustom::compile(class MaterialCompiler* compiler, int32 outputIndex)
	{
		TArray<int32> compiledInputs;
		for (int32 i = 0; i < mInputs.size(); i++)
		{
			if (mInputs[i].mInputName.length() == 0)
			{
				compiledInputs.add(INDEX_NONE);
			}
			else
			{
				if (!mInputs[i].mInput.getTracedInput().mExpression)
				{
					return compiler->errorf(TEXT("Custom material %s missing input %d (%s)"), mDescription.c_str(), i + 1, mInputs[i].mInputName.c_str());
				}
				int32 inputCode = mInputs[i].mInput.compile(compiler);
				if (inputCode < 0)
				{
					return inputCode;
				}
				compiledInputs.add(inputCode);
			}
		}
		return compiler->customExpression(this, compiledInputs);
	}
#endif

	void RMaterialExpressionCustom::serialize(Archive& ar)
	{
		ParentType::serialize(ar);
	}

	void RMaterialExpression::serialize(Archive& ar)
	{
#if WITH_EDITORONLY_DATA
		const TArray<ExpressionInput*> inputs = getInputs();
		for (int32 inputIndex = 0; inputIndex < inputs.size(); ++inputIndex)
		{
			ExpressionInput* input = inputs[inputIndex];
			doMaterialAttributeReorder(input, 0);
		}
#endif
	}

	void RMaterialExpression::serialize(XMLNode* node)
	{
		bIsParameterExpression = boost::lexical_cast<bool>(node->first_attribute(TEXT("isParameterExpression"))->value());
		mID = boost::lexical_cast<uint32>(node->first_attribute(TEXT("id"))->value());
		XMLNode* outputRoot = node->first_node(TEXT("ExpressionOutputs"));
		XMLNode* outputNode = outputRoot->first_node(TEXT("ExpressionOutput"));
		while (outputNode)
		{
			mOutputs.addDefaulted();
			readExpressionOutputXML(mOutputs.top(), outputNode);
			outputNode = outputNode->next_sibling(TEXT("ExpressionOutput"));
		}
	}

	const TArray<ExpressionInput*> RMaterialExpression::getInputs()
	{
		TArray<ExpressionInput*> result;
		innerGetInputs(result);
		return result;
	}

	const void RMaterialExpression::innerGetInputs(TArray<ExpressionInput *>& result)
	{

	}

	ExpressionInput* RMaterialExpression::getInput(int32 inputIndex)
	{
		int32 index = 0; 
		return nullptr;
	}

	wstring RMaterialExpression::getInputName(uint32 inputInde) const
	{
		return TEXT("");
	}
	DECLARE_SIMPLER_REFLECTION(RMaterialExpression);

	RMaterialExpressionMaterialFunctionCall::RMaterialExpressionMaterialFunctionCall(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		struct ConstructorStatics
		{
			wstring Name_Functions;
			ConstructorStatics()
				:Name_Functions(TEXT("Functions"))
			{}
		};
		static ConstructorStatics mConstructorStatics;
	}

	DECLARE_SIMPLER_REFLECTION(RMaterialExpressionMaterialFunctionCall);

	RMaterialExpressionFunctionInput::RMaterialExpressionFunctionInput(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		struct ConstructStatics
		{
			wstring Name_Functions;
			ConstructStatics()
				:Name_Functions(TEXT("Functions"))
			{}
		};
		static ConstructStatics mConstructorStatics;
	}

	DECLARE_SIMPLER_REFLECTION(RMaterialExpressionFunctionInput);

	RMaterialExpressionFunctionOutput::RMaterialExpressionFunctionOutput(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		struct ConstructorStatics
		{
			wstring Name_Functions;
			ConstructorStatics()
				:Name_Functions(TEXT("Functions"))
			{}
		};

		static ConstructorStatics mConstructorStatics;
	}

	RMaterialExpressionCustom::RMaterialExpressionCustom(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		struct ConstructorStatics
		{
			wstring Name_Custom;
			ConstructorStatics()
				:Name_Custom(TEXT("Custom"))
			{

			}
		};

		static ConstructorStatics mConstructorStatics;
		mDescription = TEXT("Custom");
		mCode = TEXT("1");
		mOutputTypes.add(CMOT_Float3);
		mInputs.add(CustomInput());
		mInputs[0].mInputName = TEXT("");
	}

	void RMaterialExpressionCustom::serialize(XMLNode* node)
	{
		ParentType::serialize(node);
		const wstring preFixUp = mCode;
		bool bDidUpdate = false;
		XMLNode* parentNode = node->first_node(TEXT("Parent"));
		RMaterialExpression::serialize(node);

		mInputs.clear();
		XMLNode* inputsNode = node->first_node(TEXT("Inputs"));
		if (inputsNode)
		{
			XMLNode* customInputNode = inputsNode->first_node(TEXT("CustomInput"));
			while (customInputNode)
			{
				int index = mInputs.addDefaulted(1);
				customInputNode = customInputNode->next_sibling(TEXT("CustomInput"));

			}
		}
	}

	const void RMaterialExpressionCustom::innerGetInputs(TArray<ExpressionInput *>& result)
	{
		for (int32 i = 0; i < mInputs.size(); i++)
		{
			result.add(&mInputs[i].mInput);
		}
	}

	ExpressionInput* RMaterialExpressionCustom::getInput(int32 inputIndex)
	{
		if (inputIndex < mInputs.size())
		{
			return &mInputs[inputIndex].mInput;
		}
		return nullptr;
	}

	wstring RMaterialExpressionCustom::getInputName(uint32 inputInde) const
	{
		if (inputInde < mInputs.size())
		{
			return mInputs[inputInde].mInputName;
		}
		return TEXT("");
	}

#if WITH_EDITOR
	uint32 RMaterialExpressionCustom::getOutputType(int32 outputIndex)
	{
		switch (mOutputTypes[outputIndex])
		{
		case CMOT_Float1:
			return MCT_Float;
		case  CMOT_Float2:
			return MCT_Float2;
		case CMOT_Float3:
			return MCT_Float3;
		case CMOT_Float4:
			return MCT_Float4;
		default:
			return MCT_Unknown;
		}
	}
#endif

	void RMaterialExpression::validateState()
	{

	}
	bool RMaterialExpression::containsInputLoop(const bool bStopOnFunctionCall /* = true */)
	{
		TArray<MaterialExpressionKey> expressionStack;
		TSet<MaterialExpressionKey> visitedExpressions;
		return containsInputLoopInternal(expressionStack, visitedExpressions, bStopOnFunctionCall);
	}

	bool RMaterialExpression::containsInputLoopInternal(TArray<MaterialExpressionKey> &expressionStack, TSet<MaterialExpressionKey>& visitedExpressions, const bool bStopOnFunctionCall)
	{
#if WITH_EDITORONLY_DATA
		const TArray<ExpressionInput*> inputs = getInputs();
		for (int32 index = 0; index < inputs.size(); ++index)
		{
			ExpressionInput* input = inputs[index];
			if (input->mExpression)
			{
				RMaterialExpressionMaterialFunctionCall* functionCall = check_cast<RMaterialExpressionMaterialFunctionCall*>(input->mExpression);
				if (bStopOnFunctionCall && functionCall)
				{
					continue;
				}
				MaterialExpressionKey inputExpressionKey(input->mExpression, input->mOutputIndex);
				if (expressionStack.contains(inputExpressionKey))
				{
					return true;
				}
				else if (!visitedExpressions.contains(inputExpressionKey))
				{
					visitedExpressions.add(inputExpressionKey);
					expressionStack.add(inputExpressionKey);
					if (input->mExpression->containsInputLoopInternal(expressionStack, visitedExpressions, bStopOnFunctionCall))
					{
						return true;
					}
					expressionStack.pop();
				}
			}
		}
#endif
		return false;
	}

#if WITH_EDITOR
	TArray<ExpressionOutput>& RMaterialExpression::getOutputs()
	{
		return mOutputs;
	}

	uint32 RMaterialExpression::getInputType(int32 inputIndex)
	{
		return MCT_Float;
	}

	uint32 RMaterialExpression::getOutputType(int32 outputIndex)
	{
		if (isResultMaterialAttributes(outputIndex))
		{
			return MCT_MaterialAttributes;
		}
		else
		{
			ExpressionOutput& output = getOutputs()[outputIndex];
			if (output.mMask)
			{
				int32 maskChannelCount = (output.mMaskR ? 1 : 0) + (output.mMaskG ? 1 : 0) + (output.mMaskB ? 1 : 0) + (output.mMaskA ? 1 : 0);
				switch (maskChannelCount)
				{
				case 1:
					return MCT_Float;
				case 2:
					return MCT_Float2;
				case 3:
					return MCT_Float3;
				case 4:
					return MCT_Float4;
				default:
					return MCT_Unknown;
				}
			}
			else
			{
				return MCT_Float;
			}
		}
	}

	void RMaterialExpression::getCaption(TArray<wstring>& outCaptions) const
	{
		outCaptions.add(TEXT("Expression"));
	}

	wstring RMaterialExpression::getDescription() const
	{
		TArray<wstring> captions;
		getCaption(captions);
		if (captions.size() > 1)
		{
			wstring result = captions[0];
			for (int32 index = 1; index < captions.size(); ++index)
			{
				result += TEXT(" ");
				result += captions[index];
			}
			return result;
		}
		else
		{
			return captions[0];
		}
	}

	int32 MaterialAttributesInput::compileWithDefault(class MaterialCompiler* compiler, const Guid& attributeID)
	{
		int32 ret = INDEX_NONE;
		if (mExpression)
		{
			ScopedMaterialCompilerAttribute scopedMaterialCompilerAttribute(compiler, attributeID);
			ret = ExpressionInput::compile(compiler);
			if (ret != INDEX_NONE && !mExpression->isResultMaterialAttributes(mOutputIndex))
			{
				compiler->error(TEXT("Cannot connect a non MaterialAttributes node to a MaterialAttributes pin."));
			}
		}
		EMaterialProperty prop = MaterialAttributeDefinationMap::getProperty(attributeID);
		setConnectedProperty(prop, ret != INDEX_NONE);
		if (ret == INDEX_NONE)
		{
			ret = MaterialAttributeDefinationMap::compileDefaultExpression(compiler, attributeID);
		}
		return ret;
	}
#endif

	DECLARE_SIMPLER_REFLECTION(RMaterialExpressionCustom);
	DECLARE_SIMPLER_REFLECTION(RMaterialExpressionFunctionOutput);

	RMaterialExpressionParameter::RMaterialExpressionParameter(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{

	}

	void RMaterialExpressionParameter::serialize(XMLNode* node)
	{
		XMLNode* parentNode = node->first_node(TEXT("RMaterialExpression"));
		ParentType::serialize(parentNode);
		mParameterName = node->first_node(TEXT("ParameterName"))->value();
	}
	RMaterialExpressionScalarParameter::RMaterialExpressionScalarParameter(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{}

	void RMaterialExpressionScalarParameter::serialize(XMLNode* node)
	{
		XMLNode* parentNode = node->first_node(TEXT("RMaterialExpressionParameter"));
		ParentType::serialize(parentNode);
		mDefaultValue = boost::lexical_cast<float>(node->first_node(TEXT("DefaultValue"))->value());
		mSliderMin = boost::lexical_cast<float>(node->first_node(TEXT("SliderMin"))->value());
		mSliderMax = boost::lexical_cast<float>(node->first_node(TEXT("SliderMax"))->value());

	}

	int32 RMaterialExpressionScalarParameter::compile(class MaterialCompiler* compiler, int32 outputIndex)
	{
		return compiler->scalarParameter(mParameterName, mDefaultValue);
	}

	bool ExpressionInput::serialize(XMLNode* node)
	{
		return readExpressionInputXML(*this, node);
	}

	DECLARE_SIMPLER_REFLECTION(RMaterialExpressionParameter);
	DECLARE_SIMPLER_REFLECTION(RMaterialExpressionScalarParameter);
}