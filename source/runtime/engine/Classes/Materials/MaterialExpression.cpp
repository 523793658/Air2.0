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
#include "MaterialExpressionVectorParameter.h"
#include "MaterialExpressionTextureSampleParameter.h"
#include "MaterialExpressionTextureSampleParameter2D.h"
#include "MaterialExpressionTextureObject.h"
#include "MaterialExpressionTextureObjectParameter.h"
#include "Texture.h"
#include "Texture2D.h"
#include "Misc/SecureHash.h"
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
		material->mExpressions.add(std::dynamic_pointer_cast<RMaterialExpression>(this->shared_from_this()));
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
		XMLNode* ThisNode = node->first_node(TEXT("RMaterialExpression"));
		auto* idNode = node->first_attribute(TEXT("id"));
		BOOST_ASSERT(idNode);
		mID = boost::lexical_cast<uint32>(idNode->value());
		
		if (ThisNode)
		{
			auto* isParameterExpressionNode = node->first_attribute(TEXT("isParameterExpression"));
			if (isParameterExpressionNode)
			{
				bIsParameterExpression = boost::lexical_cast<bool>(isParameterExpressionNode->value());
				XMLNode* outputRoot = node->first_node(TEXT("ExpressionOutputs"));
				if (outputRoot)
				{
					XMLNode* outputNode = outputRoot->first_node(TEXT("ExpressionOutput"));
					while (outputNode)
					{
						mOutputs.addDefaulted();
						readExpressionOutputXML(mOutputs.top(), outputNode);
						outputNode = outputNode->next_sibling(TEXT("ExpressionOutput"));
					}
				}
			}
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
		ParentType::serialize(node);
		auto* ThisNode = node->first_node(TEXT("RMaterialExpressionParameter"));
		if (ThisNode)
		{
			mParameterName = ThisNode->first_node(TEXT("ParameterName"))->value();
		}
	}
	RMaterialExpressionScalarParameter::RMaterialExpressionScalarParameter(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{}

	void RMaterialExpressionScalarParameter::serialize(XMLNode* node)
	{
		ParentType::serialize(node);
		auto* thisNode = node->first_node(TEXT("RMaterialExpressionScalarParameter"));
		if (thisNode)
		{
			auto* valueNode = thisNode->first_node(TEXT("DefaultValue"));
			if (valueNode)
			{
				mDefaultValue = boost::lexical_cast<float>(valueNode->value());
			}
			auto* sliderMinNode = thisNode->first_node(TEXT("SliderMin"));
			if (sliderMinNode)
			{
				mSliderMin = boost::lexical_cast<float>(sliderMinNode->value());
			}

			auto* sliderMaxNode = thisNode->first_node(TEXT("SliderMax"));
			if (sliderMaxNode)
			{
				mSliderMax = boost::lexical_cast<float>(sliderMaxNode->value());
			}
		
		}
		

	}

	int32 RMaterialExpressionScalarParameter::compile(class MaterialCompiler* compiler, int32 outputIndex)
	{
		return compiler->scalarParameter(mParameterName, mDefaultValue);
	}

	RMaterialExpressionVectorParameter::RMaterialExpressionVectorParameter(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{}

#if WITH_EDITOR
	int32 RMaterialExpressionVectorParameter::compile(class MaterialCompiler* compiler, int32 outputIndex)
	{
		return compiler->vectorParameter(mParameterName, mDefaultValue);
	}

	void RMaterialExpressionVectorParameter::getCaption(TArray<wstring>& outCaptions) const
	{
		outCaptions.add(printf(TEXT("Param (%.3g,%.3g,%.3g,%.3g)"), mDefaultValue.R, mDefaultValue.G, mDefaultValue.B, mDefaultValue.A));
		outCaptions.add(printf(TEXT("'%s'"), mParameterName.c_str()));
	}

	void RMaterialExpressionVectorParameter::serialize(XMLNode* node)
	{
		ParentType::serialize(node);
		auto* thisNode = node->first_node(TEXT("RMaterialExpressionVectorParameter"));
		if (thisNode)
		{
			auto* n = thisNode->first_node(TEXT("DefaultValue"));
			if (n)
			{
				mDefaultValue = LinearColor::fromString(n->value());
			}
		}


	}

	int32 RMaterialExpression::compilerError(class MaterialCompiler* compiler, const TCHAR* pcMessage)
	{
		TArray<wstring> captions;
		getCaption(captions);
		return compiler->errorf(TEXT("%s> %s"), mDesc.length() > 0 ? mDesc.c_str() : captions[0].c_str(), pcMessage);
	}

	const TCHAR* RMaterialExpressionTextureSampleParameter::getRequirements()
	{
		return TEXT("Invalid texture type");
	}

	bool RMaterialExpressionTextureSampleParameter::textureIsValid(std::shared_ptr<RTexture> inTexture)
	{
		return false;
	}

	static bool verifySamplerType(
		MaterialCompiler* compiler,
		const TCHAR* expressionDesc,
		const std::shared_ptr<RTexture> texture,
		EMaterialSamplerType samplerType)
	{
		BOOST_ASSERT(compiler);
		BOOST_ASSERT(expressionDesc);
		if (texture)
		{
			EMaterialSamplerType correctSamplerType = RMaterialExpressionTextureBase::getSamplerTypeForTexture(texture);
			if (samplerType != correctSamplerType)
			{
				return false;
			}

			if ((samplerType == SamplerType_Normal || samplerType == SamplerType_Masks) && texture->bSRGB)
			{
				compiler->errorf(TEXT("%s> To use as sampler type, SRGB must be disabled for %s"), expressionDesc, texture->getPathName());
				return false;
			}
		}
		return true;
	}

	int32 RMaterialExpressionTextureSampleParameter::compile(class MaterialCompiler* compiler, int32 outputIndex)
	{
		if (!mTexture)
		{
			return compilerError(compiler, getRequirements());
		}
		if (mTexture)
		{
			if (!textureIsValid(mTexture))
			{
				return compilerError(compiler, getRequirements());
			}

			if (!verifySamplerType(compiler, (mDesc.length() > 0 ? mDesc.c_str() : TEXT("TextureSamplerParameter")), mTexture, mSamplerType))
			{
				return INDEX_NONE;
			}
		}

		if (mParameterName.empty())
		{
			return RMaterialExpressionTextureSample::compile(compiler, outputIndex);
		}

		int32 mipValue0Index = compileMipValue0(compiler);
		int32 mipValue1Index = compileMipValue1(compiler);
		int32 textureReferenceIndex = INDEX_NONE;
		return compiler->textureSample(compiler->textureParameter(mParameterName, mTexture, textureReferenceIndex, mSamplerSource), mCoordinates.getTracedInput().mExpression ? mCoordinates.compile(compiler) : compiler->textureCoordinate(mConstCoordinate, false, false), (EMaterialSamplerType)mSamplerType, mipValue0Index, mipValue1Index, mMipValueMode, mSamplerSource, textureReferenceIndex);
	}


	void RMaterialExpressionTextureSampleParameter::serialize(XMLNode* node)
	{
		ParentType::serialize(node);
		auto* thisNode = node->first_node(TEXT("RMaterialExpressionTextureSampleParameter"));
		if (thisNode)
		{
			XMLNode* parameterNameNode = thisNode->first_node(TEXT("ParameterName"));
			if (parameterNameNode)
			{
				mParameterName = parameterNameNode->value();
			}
		}
		
	}

#endif

	RMaterialExpressionTextureBase::RMaterialExpressionTextureBase(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
		, isDefaultMeshpaintTexture(false)
	{

	}

	void RMaterialExpressionTextureBase::autoSetSamplerType()
	{
		if (mTexture)
		{
			mSamplerType = getSamplerTypeForTexture(mTexture);
		}
	}

	wstring RMaterialExpressionTextureBase::getDescription() const
	{
		wstring result = ParentType::getDescription();
		result += TEXT(" (");
		result += mTexture ? mTexture->getName() : TEXT("None");
		result += TEXT(")");
		return result;
	}

	EMaterialSamplerType RMaterialExpressionTextureBase::getSamplerTypeForTexture(std::shared_ptr<const RTexture> texture)
	{
		if (texture)
		{
			switch (texture->mCompressionSettings)
			{
			case TC_NormalMap:
				return SamplerType_Normal;
			case TC_Grayscale:
				return texture->bSRGB ? SamplerType_Grayscale : SamplerType_LinearGrayscale;
			case TC_Alpha:
				return SamplerType_Alpha;
			case TC_Masks:
				return SamplerType_Masks;
			case TC_DistanceFieldFont:
				return SamplerType_DistanceFieldFont;
			default:
				return texture->bSRGB ? SamplerType_Color : SamplerType_LinearColor;
			}
		}
		return SamplerType_Color;
	}

	RMaterialExpressionTextureSample::RMaterialExpressionTextureSample(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
		,bShowTextureInputPin(true)
	{}

	const TArray<ExpressionInput*> RMaterialExpressionTextureSample::getInputs()
	{
		TArray<ExpressionInput*> outputs;
		uint32 inputIndex = 0;
		while (ExpressionInput* ptr = getInput(inputIndex++))
		{
			outputs.add(ptr);
		}
		return outputs;
	}

#define IF_INPUT_RETURN(item) if(!inputIndex) return &item; -- inputIndex

	ExpressionInput* RMaterialExpressionTextureSample::getInput(int32 inputIndex)
	{
		IF_INPUT_RETURN(mCoordinates);

		if (bShowTextureInputPin)
		{
			IF_INPUT_RETURN(mTextureObject);
		}

		if (mMipValueMode == TMVM_Derivative)
		{
			IF_INPUT_RETURN(mCoordinatesDX);
			IF_INPUT_RETURN(mCoordinatesDY);
		}
		else if (mMipValueMode != TMVM_None)
		{
			IF_INPUT_RETURN(mMipValue);
		}
		return nullptr;
	}
#undef IF_INPUT_RETURN

#define IF_INPUT_RETURN(item, name) if(!inputIndex) return name; --inputIndex
	wstring RMaterialExpressionTextureSample::getInputName(uint32 inputIndex) const
	{
		IF_INPUT_RETURN(mCoordinates, TEXT("Coordinates"));

		if (bShowTextureInputPin)
		{
			IF_INPUT_RETURN(mTextureObject, TEXT("TextureObject"));
		}

		if (mMipValueMode == TMVM_MipLevel)
		{
			IF_INPUT_RETURN(mMipValue, TEXT("MipLevel"));
		}
		else if (mMipValueMode == TMVM_MipBias)
		{
			IF_INPUT_RETURN(mMipValue, TEXT("MipBias"));
		}
		else if (mMipValueMode == TMVM_Derivative)
		{
			IF_INPUT_RETURN(mCoordinatesDX, TEXT("DDX(UVs)"));
			IF_INPUT_RETURN(mCoordinatesDY, TEXT("DDY(UVs)"));

		}
		return TEXT("");
	}

#undef IF_INPUT_RETURN
#if WITH_EDITOR
	int32 RMaterialExpressionTextureSample::compile(class MaterialCompiler* compiler, int32 outputIndex)
	{
		if (mTexture || mTextureObject.mExpression)
		{
			int32 textureReferenceIndex = INDEX_NONE;
			int32 textureCodeIndex = mTextureObject.mExpression ? mTextureObject.compile(compiler) : compiler->texture(mTexture, textureReferenceIndex, mSamplerSource, mMipValueMode);

			std::shared_ptr<RTexture> effectiveTexture = mTexture;
			EMaterialSamplerType effectiveSamplerType = (EMaterialSamplerType)mSamplerType;
			if (mTextureObject.mExpression)
			{
				RMaterialExpression* inputExpression = mTextureObject.mExpression;
				RMaterialExpressionFunctionInput* functionInput = check_cast<RMaterialExpressionFunctionInput*>(inputExpression);
				if (functionInput)
				{
					RMaterialExpressionFunctionInput* nestedFunctionInput = functionInput;
					while (true)
					{
						RMaterialExpression* previewExpression = nestedFunctionInput->getEffectivePreviewExpression();
						if (previewExpression && previewExpression->isA(RMaterialExpressionFunctionInput::StaticClass()))
						{
							nestedFunctionInput = check_cast<RMaterialExpressionFunctionInput*>(previewExpression);
						}
						else
						{
							break;
						}
					}
					inputExpression = nestedFunctionInput->getEffectivePreviewExpression();
				}

				RMaterialExpressionTextureObject* textureObjectExpression = check_cast<RMaterialExpressionTextureObject*>(inputExpression);
				RMaterialExpressionTextureObjectParameter* textureObjectParameter = check_cast<RMaterialExpressionTextureObjectParameter*>(inputExpression);
				if (textureObjectExpression)
				{
					effectiveTexture = textureObjectExpression->mTexture;
					effectiveSamplerType = textureObjectExpression->mSamplerType;
				}
				else if (textureObjectParameter)
				{
					effectiveTexture = textureObjectParameter->mTexture;
					effectiveSamplerType = textureObjectParameter->mSamplerType;
				}
				textureReferenceIndex = compiler->getTextureReferenceIndex(effectiveTexture);
			}
			if (effectiveTexture && verifySamplerType(compiler, (mDesc.length() > 0 ? mDesc.c_str() : TEXT("TextureSample")), effectiveTexture, effectiveSamplerType))
			{
				return compiler->textureSample(
					textureCodeIndex,
					mCoordinates.getTracedInput().mExpression ? mCoordinates.compile(compiler) : compiler->textureCoordinate(mConstCoordinate, false, false),
					(EMaterialSamplerType)effectiveSamplerType,
					compileMipValue0(compiler),
					compileMipValue1(compiler),
					mMipValueMode,
					mSamplerSource,
					textureReferenceIndex);
			}
			else
			{
				return INDEX_NONE;
			}
		}
		else
		{
			if (mDesc.length() > 0)
			{
				return compiler->errorf(TEXT("%s> Missing input texture"), mDesc.c_str());
			}
			else
			{
				return compiler->errorf(TEXT("TextureSample> Missing input texture"));
			}
		}
	}
#endif


	RMaterialExpressionTextureSampleParameter::RMaterialExpressionTextureSampleParameter(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		: ParentType(objectInitializer)
	{
		struct ConstructStatics
		{
			wstring NAME_Obsolete;
			ConstructStatics()
				:NAME_Obsolete(TEXT("Obsolete"))
			{}
		};

		static ConstructStatics constructorStatics;
		bIsParameterExpression = true;
		bShowTextureInputPin = false;

#if WITH_EDITOR
		
#endif
		
	}

	void RMaterialExpressionTextureSample::getCaption(TArray<wstring>& outCaptions) const
	{
		outCaptions.add(TEXT("Texture Sample"));
	}

#if WITH_EDITOR

#define IF_INPUT_RETURN(item, type) if(!inputIndex) return type; -- inputIndex

	uint32 RMaterialExpressionTextureSample::getInputType(int32 inputIndex)
	{
		IF_INPUT_RETURN(mCoordinates, MCT_Float);

		if (bShowTextureInputPin)
		{
			IF_INPUT_RETURN(mTextureObject, MCT_Texture);
		}
		if (mMipValueMode == TMVM_MipLevel || mMipValueMode == TMVM_MipBias)
		{
			IF_INPUT_RETURN(mMipValue, MCT_Float);
		}
		else if (mMipValueMode == TMVM_Derivative)
		{
			IF_INPUT_RETURN(mCoordinatesDX, MCT_Float);
			IF_INPUT_RETURN(mCoordinatesDY, MCT_Float);
		}
		return MCT_Unknown;
	}
#undef IF_INPUT_RETURN

	void RMaterialExpressionTextureSample::serialize(XMLNode* node)
	{
		ParentType::serialize(node);

		auto* thisNode = node->first_node(TEXT("RMaterialExpressionTextureSample"));
		if (thisNode)
		{
			XMLNode* mipValueModeNode = thisNode->first_node(TEXT("MipValueMode"));
			if (mipValueModeNode)
			{
				wstring s = mipValueModeNode->value();
				size_t h = RT_HASH(TCHAR_TO_ANSI(s.c_str()));
				switch (h)
				{
				case CT_HASH("TMVM_None"):
					mMipValueMode = TMVM_None;
					break;
				case CT_HASH("TMVM_MipLevel"):
					mMipValueMode = TMVM_MipLevel;
					break;
				case CT_HASH("TMVM_MipBias"):
					mMipValueMode = TMVM_MipBias;
					break;
				case CT_HASH("TMVM_Derivative"):
					mMipValueMode = TMVM_Derivative;
					break;
				}
			}

			XMLNode* sampleSourceNode = thisNode->first_node(TEXT("SampleSource"));
			if (sampleSourceNode)
			{
				wstring s = sampleSourceNode->value();
				size_t h = RT_HASH(TCHAR_TO_ANSI(s.c_str()));
				if (h == CT_HASH("SSM_FromTextureAsset"))
				{
					mSamplerSource = SSM_FromTextureAsset;
				}
				else if (h == CT_HASH("SSM_Wrap_WorldGroupSettings"))
				{
					mSamplerSource = SSM_Wrap_WorldGroupSettings;
				}
				else if (h == CT_HASH("SSM_Clamp_WorldGroupSettings"))
				{
					mSamplerSource = SSM_Clamp_WorldGroupSettings;
				}
			}
		}
	}

	int32 RMaterialExpressionTextureSample::compileMipValue0(class MaterialCompiler* compiler)
	{
		if (mMipValueMode == TMVM_Derivative)
		{
			if (mCoordinatesDX.getTracedInput().isConnected())
			{
				return mCoordinatesDX.compile(compiler);
			}
		}
		else if (mMipValue.getTracedInput().isConnected())
		{
			return mMipValue.compile(compiler);
		}
		else
		{
			return compiler->constant(mConstCoordinate);
		}
		return INDEX_NONE;
	}
	int32 RMaterialExpressionTextureSample::compileMipValue1(class MaterialCompiler* compiler)
	{
		if (mMipValueMode == TMVM_Derivative && mCoordinatesDY.getTracedInput().isConnected())
		{
			return mCoordinatesDY.compile(compiler);
		}
		return INDEX_NONE;
	}

	void RMaterialExpressionTextureSampleParameter::getCaption(TArray<wstring>& outCaptions) const
	{
		outCaptions.add(TEXT("Texture Param"));
		outCaptions.add(printf(TEXT("'%s'"), mParameterName.c_str()));
	}

	void RMaterialExpressionTextureSampleParameter::setDefaultTexture()
	{

	}

#endif


	bool ExpressionInput::serialize(XMLNode* node)
	{
		return readExpressionInputXML(*this, node);
	}


	RMaterialExpressionTextureSampleParameter2D::RMaterialExpressionTextureSampleParameter2D(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		struct ConstructorStatics
		{
			
		};

		mTexture = loadObjectSync<RTexture>(TEXT("assets/EngineResources/DefaultTexture.dds"));
		autoSetSamplerType();
	}

#if WITH_EDITOR
	void RMaterialExpressionTextureSampleParameter2D::getCaption(TArray<wstring>& outCaptions) const
	{
		outCaptions.add(TEXT("Param2D"));
		outCaptions.add(printf(TEXT("'%s'"), mParameterName.c_str()));
	}
#endif

	bool RMaterialExpressionTextureSampleParameter2D::textureIsValid(std::shared_ptr<RTexture> inTexture)
	{
		bool result = false;
		if (inTexture)
		{
			if (inTexture->getClass() == RTexture2D::StaticClass())
			{
				result = true;
			}
		}
		return result;
	}

	void RMaterialExpressionTextureSampleParameter2D::setDefaultTexture()
	{
		mTexture = loadObjectAsync<RTexture2D>(TEXT("assets/EngineResources/DefaultTexture.dds"));
		autoSetSamplerType();
	}

	const TCHAR* RMaterialExpressionTextureSampleParameter2D::getRequirements()
	{
		return TEXT("Requires Texture2D");
	}


	DECLARE_SIMPLER_REFLECTION(RMaterialExpressionParameter);
	DECLARE_SIMPLER_REFLECTION(RMaterialExpressionScalarParameter);
	DECLARE_SIMPLER_REFLECTION(RMaterialExpressionVectorParameter);

	DECLARE_SIMPLER_REFLECTION(RMaterialExpressionTextureBase);
	DECLARE_SIMPLER_REFLECTION(RMaterialExpressionTextureSample);
	DECLARE_SIMPLER_REFLECTION(RMaterialExpressionTextureSampleParameter);
	DECLARE_SIMPLER_REFLECTION(RMaterialExpressionTextureSampleParameter2D);
}