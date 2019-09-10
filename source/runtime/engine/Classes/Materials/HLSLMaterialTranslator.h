#pragma once
#include "MaterialCompiler.h"
#include "MaterialShared.h"
#include "MaterialExpressionCustom.h"
#include "boost/algorithm/algorithm.hpp"
#include "Containers/StringUtil.h"
#include "Classes/Materials/MaterialExpressionMaterialFunctionCall.h"
#include "Classes/Materials/MaterialExpressionFunctionInput.h"
#include "Classes/Materials/MaterialExpressionFunctionOutput.h"
#include "Materials/MaterialConstantExpressions.h"
#include "Classes/Materials/MaterialParameterCollection.h"

#include "Class.h"
#include "Texture.h"

namespace Air
{

	static inline int32 swizzleComponentToIndex(TCHAR component)
	{
		switch (component)
		{
		case TCHAR('x'): case TCHAR('X'): case TCHAR('r'): case TCHAR('R'):return 0;
		case TCHAR('y'): case TCHAR('Y'): case TCHAR('g'): case TCHAR('G'):return 1;
		case TCHAR('z'): case TCHAR('Z'): case TCHAR('b'): case TCHAR('B'):return 2;
		case TCHAR('w'): case TCHAR('W'): case TCHAR('a'): case TCHAR('A'):return 3;
		default:
			return -1;
		}
	}

	static inline uint32 getNumComponents(EMaterialValueType type)
	{
		switch (type)
		{
		case MCT_Float:
		case MCT_Float1:
			return 1;
		case MCT_Float2:
			return 2;
		case MCT_Float3:
			return 3;
		case MCT_Float4:
			return 4;
		default:
			return 0;
		}
	}
	static inline EMaterialValueType getVectorType(uint32 numComponents)
	{
		switch (numComponents)
		{
		case 1:return MCT_Float;
		case 2:return MCT_Float2;
		case 3:return MCT_Float3;
		case 4:return MCT_Float4;
		default:
			return MCT_Unknown;
		}
	}



	struct ShaderCodeChunk
	{
		wstring mDefinition;
		wstring mSymbolName;
		EMaterialValueType mType;
		bool bInline;
		TRefCountPtr<MaterialConstantExpression> mConstantExpression;
		ShaderCodeChunk(const TCHAR* indefinition, const wstring& inSymbolName, EMaterialValueType inType, bool bInInline = false)
			:mDefinition(indefinition)
			,mSymbolName(inSymbolName)
			,mConstantExpression(nullptr)
			,mType(inType)
			,bInline(bInInline)
		{}
		ShaderCodeChunk(MaterialConstantExpression* inConstantExpression, const TCHAR* indefinition, EMaterialValueType inType)
			: mConstantExpression(inConstantExpression)
			, mDefinition(indefinition)
			, mType(inType)
			, bInline(false)
		{}
	};

	class HLSLMaterialTranslator : public MaterialCompiler
	{
	protected:
		EShaderFrequency mShaderFrequency;
		FMaterial* mMaterial;
		MaterialCompilationOutput& mMaterialCompilationOutput;
		StaticParameterSet mStaticParamters;
		EShaderPlatform mPlatform;
		EMaterialQualityLevel::Type mQualityLevel;
		ERHIFeatureLevel::Type	mFeatureLevel;
		wstring mMaterialTemplate;
		uint32 bSuccess : 1;
		uint32 bCompilingPreviousFrame : 1;
		uint32 bUsesPixelDepthOffset : 1;
		uint32 bUsesEmissiveColor : 1;

		int32 mMaterialTemplateLineNumber;
		bool mSharedPixelProperties[CompiledMP_Max];
		wstring mTranslatedCodeChunkDefinitions[CompiledMP_Max];
		wstring mTranslatedCodeChunks[CompiledMP_Max];
		TArray<ShaderCodeChunk> * mCurrentScopeChuncks;

		TArray<wstring> mCustomExpressionImplementations;

		TArray<wstring> mCustomOutputImplementations;

		int32 mNextSymbolIndex{ INDEX_NONE };
		
		uint32 mNumUserTexCoords;
		
		uint32 mNumUserVertexTexCoords;

		wstring mResourcesString;

		bool bCompileForComputeShader = false;

		TArray<MaterialFunctionCompileState> mFunctionStacks[SF_NumFrequencies];

		TArray<ShaderCodeChunk> mSharedPropertyCodeChunks[SF_NumFrequencies];

		TArray<MaterialParameterCollection*> mParameterCollections;

		TArray<ShaderCodeChunk> mConstantExpressions;

		EMaterialProperty mMaterialProperty;

		TArray<Guid> mMaterialAttributesStack;

	public:
		HLSLMaterialTranslator(FMaterial* inMaterial, MaterialCompilationOutput& inMaterialCompilationOutput, const StaticParameterSet& inStaticParameters, EShaderPlatform inPlatform, EMaterialQualityLevel::Type inQualityLevel, ERHIFeatureLevel::Type inFeatureLevel)
			:mMaterial(inMaterial)
			, mMaterialCompilationOutput(inMaterialCompilationOutput)
			, mPlatform(inPlatform)
			, mQualityLevel(inQualityLevel)
			, mFeatureLevel(inFeatureLevel)
			, mNumUserTexCoords(0)
			, mNumUserVertexTexCoords(0)
		{
			Memory::memzero(mSharedPixelProperties);
			mSharedPixelProperties[MP_Normal] = true;
			mSharedPixelProperties[MP_BaseColor] = true;
			mSharedPixelProperties[MP_Metallic] = true;
			mSharedPixelProperties[MP_Specular] = true;
			mSharedPixelProperties[MP_Roughness] = true;
			mSharedPixelProperties[MP_AmbientOcclusion] = true;
			mSharedPixelProperties[MP_EmissiveColor] = true;
			{
				for (int32 frequency = 0; frequency < SF_NumFrequencies; ++frequency)
				{
					mFunctionStacks[frequency].add(MaterialFunctionCompileState(nullptr));
				}
			}
			const Guid& missingAttribute = MaterialAttributeDefinationMap::getID(MP_Max);
			mMaterialAttributesStack.add(missingAttribute);
		}

		virtual bool translate();

		const TCHAR* HLSLTypeString(EMaterialValueType type) const
		{
			switch (type)
			{
			case MCT_Float1:	return TEXT("MaterialFloat");
			case MCT_Float2:	return TEXT("MaterialFloat2");
			case MCT_Float3:	return TEXT("MaterialFloat3");
			case MCT_Float4:	return TEXT("MaterialFloat4");
			case MCT_Float:		return TEXT("MaterialFloat");
			
			case MCT_Texture2D:	return TEXT("texture2D");
			case MCT_TextureCube:	return TEXT("textureCube");
			case MCT_StaticBool:	return TEXT("static bool");
			case MCT_MaterialAttributes:	return TEXT("MaterialAttributes");

			default:
				return TEXT("unknown");
			};
		}

		void getSharedInputsMaterialCode(wstring& pixelMembersDeclaration, wstring & normalAssignment, wstring& pixelMembersInitializationEplilog);

		virtual int32 customExpression(class RMaterialExpressionCustom* custom, TArray<int32>& compiledInputs) override
		{
			/*int32 numOutputs = custom->mOutputTypes.size();
			int32 resultIdx;
			TArray<wstring> outputTypeStrings;*/
			int32 resultIndex = INDEX_NONE;
			return resultIndex;
		}

		int32 addCodeChunk(EMaterialValueType type, const TCHAR* format, ...)
		{
			int32 bufferSize = 256;
			TCHAR* formattedCode = nullptr;
			int32 result = -1;
			while (result == -1)
			{
				formattedCode = (TCHAR*)Memory::realloc(formattedCode, bufferSize * sizeof(TCHAR));
				GET_VARARGS_RESULT(formattedCode, bufferSize, bufferSize - 1, format, format, result);
				bufferSize *= 2;
			}
			formattedCode[result] = 0;

			const int32 codeIndex = addCodeChunkInner(formattedCode, type, false);
			Memory::free(formattedCode);
			return codeIndex;
		}

		int32 addCodeChunkInner(const TCHAR* formattedCode, EMaterialValueType type, bool bInline)
		{
			if (type == MCT_Unknown)
			{
				return INDEX_NONE;
			}
			if (bInline)
			{
				const int32 codeIndex = mCurrentScopeChuncks->size();
				new(*mCurrentScopeChuncks)ShaderCodeChunk(formattedCode, TEXT(""), type, true);
				return codeIndex;
			}
			else if (type & (MCT_Float))
			{
				const int32 codeIndex = mCurrentScopeChuncks->size();
				const wstring symbolName = createSymbolName(TEXT("Local"));
				const wstring localVariableDefinition = wstring(TEXT("	")) + HLSLTypeString(type) + TEXT(" ") + symbolName + TEXT(" = ") + formattedCode + TEXT(";") + LINE_TERMINATOR;
				new(*mCurrentScopeChuncks)ShaderCodeChunk(localVariableDefinition.c_str(), symbolName, type, false);
				return codeIndex;
			}
			else
			{
				if (type == MCT_MaterialAttributes)
				{
					return errorf(TEXT("Operation not supported on material attributes"));
				}
				if (type & MCT_Texture)
				{
					return errorf(TEXT("Operation not supported on a texture"));
				}
				if (type == MCT_StaticBool)
				{
					return errorf(TEXT("operation not supported on a static bool"));
				}
				return INDEX_NONE;
			}
		}

		wstring createSymbolName(const TCHAR* symbolNameHint)
		{
			mNextSymbolIndex++;
			return wstring(symbolNameHint) + boost::lexical_cast<wstring>(mNextSymbolIndex);
		}

		const TCHAR* describeType(EMaterialValueType type) const
		{
			switch (type)
			{
			case Air::MCT_Float1:
				return TEXT("float");
			case Air::MCT_Float2:
				return TEXT("float2");
			case Air::MCT_Float3:
				return TEXT("float3");
			case Air::MCT_Float4:
				return TEXT("float4");
			case Air::MCT_Texture2D:
				return TEXT("Texture2D");
			case Air::MCT_TextureCube:
				return TEXT("TextureCube");
			case Air::MCT_StaticBool:
				return TEXT("static bool");
			case Air::MCT_MaterialAttributes:
				return TEXT("MaterialAttributes");
			default:
				return TEXT("unknown");
			};
		}

		virtual wstring getParameterCode(int32 index, const TCHAR* def = 0)
		{
			if (index == INDEX_NONE && def)
			{
				return def;
			}

			BOOST_ASSERT(index >= 0 && index < mCurrentScopeChuncks->size());

			const ShaderCodeChunk& codeChunk = (*mCurrentScopeChuncks)[index];
			if ((codeChunk.mConstantExpression && codeChunk.mConstantExpression->isConstant()) || codeChunk.bInline)
			{
				return codeChunk.mDefinition;
			}
			else 
			{
				if (codeChunk.mConstantExpression) {

					const int32 accessedIndex = accessConstantExpression(index);
					const ShaderCodeChunk& accessedCodeChunk = (*mCurrentScopeChuncks)[accessedIndex];
					if (accessedCodeChunk.bInline)
					{
						return accessedCodeChunk.mDefinition;
					}
					BOOST_ASSERT(accessedCodeChunk.mSymbolName.length() > 0);
					return accessedCodeChunk.mSymbolName;
				}
				BOOST_ASSERT(codeChunk.mSymbolName.length() > 0);
				return codeChunk.mSymbolName;
			}
		}

		void getMaterialEnvironment(EShaderPlatform inPlatform, ShaderCompilerEnvironment& outEnvironment)
		{
			outEnvironment.setDefine(TEXT("WANT_PIXEL_DEPTH_OFFSET"), bUsesPixelDepthOffset);
			for (int32 collectionIndex = 0; collectionIndex < mParameterCollections.size(); collectionIndex++)
			{
				const wstring collectionName = printf(TEXT("MaterialCollection%u"), collectionIndex);
				ShaderConstantBufferParameter::modifyCompilationEnvironment(collectionName.c_str(), mParameterCollections[collectionIndex]->getCosntantBufferStruct(), inPlatform, outEnvironment);
			}
		}

		int32 accessConstantExpression(int32 index)
		{
			BOOST_ASSERT(index >= 0 && index < mCurrentScopeChuncks->size());
			const ShaderCodeChunk& codeChunk = (*mCurrentScopeChuncks)[index];
			BOOST_ASSERT(codeChunk.mConstantExpression && !codeChunk.mConstantExpression->isConstant());
			MaterialConstantExpressionTexture* textureConstantExpression = codeChunk.mConstantExpression->getTextureConstantExpression();
			BOOST_ASSERT(!(codeChunk.mType & MCT_Texture) || textureConstantExpression);
			TCHAR formattedCode[MAX_SPRINTF] = TEXT("");

			if (codeChunk.mType == MCT_Float)
			{
				if (codeChunk.mConstantExpression->isChangingPerFrame())
				{
					if (bCompilingPreviousFrame)
					{
						const int32 scalarInputIndex = mMaterialCompilationOutput.mConstantExpressionSet.mPerFramePrevConstantScalarExpressions.addUnique(codeChunk.mConstantExpression);
						printf_b(formattedCode, TEXT("Air_Material_PerFramePrevScalarExpression%u"), scalarInputIndex);
					}
					else
					{
						const int32 scalarInputIndex = mMaterialCompilationOutput.mConstantExpressionSet.mPerFrameConstantScalarExpressions.addUnique(codeChunk.mConstantExpression);
						printf_b(formattedCode, TEXT("Air_Material_PerFrameScalarExpression%u"), scalarInputIndex);
					}
				}
				else
				{
					const static TCHAR indexToMask[] = { 'x', 'y', 'z', 'w' };
					const int32 scalarInputIndex = mMaterialCompilationOutput.mConstantExpressionSet.mConstantScalarExpressions.addUnique(codeChunk.mConstantExpression);
					printf_b(formattedCode, TEXT("Material.ScalarExpressions[%u].%c"), scalarInputIndex / 4, indexToMask[scalarInputIndex % 4]);
				}
			}
			else if (codeChunk.mType & MCT_Float)
			{
				const TCHAR* mask;
				switch (codeChunk.mType)
				{
				case MCT_Float:
				case MCT_Float1: mask = TEXT(".r"); break;
				case MCT_Float2: mask = TEXT(".rg"); break;
				case MCT_Float3: mask = TEXT(".rgb"); break;
				default:
					mask = TEXT(""); break;
				};
				if (codeChunk.mConstantExpression->isChangingPerFrame())
				{
					if (bCompilingPreviousFrame)
					{
						const int32 vectorInputIndex = mMaterialCompilationOutput.mConstantExpressionSet.mPerFramePrevConstantVectorExpressions.addUnique(codeChunk.mConstantExpression);
						printf_b(formattedCode, TEXT("Air_Material_PerFramePrevVectorExpression%u%s"), vectorInputIndex, mask);
					}
					else
					{
						const int32 vectorInputIndex = mMaterialCompilationOutput.mConstantExpressionSet.mPerFrameConstantVectorExpressions.addUnique(codeChunk.mConstantExpression);
						printf_b(formattedCode, TEXT("Air_Material_PerFrameVectorExpression%u%s"), vectorInputIndex, mask);
					}
				}
				else
				{
const int32 vectorInputIndex = mMaterialCompilationOutput.mConstantExpressionSet.mConstantVectorExpressions.addUnique(codeChunk.mConstantExpression);
printf_b(formattedCode, TEXT("Material.VectorExpressions[%u]%s"), vectorInputIndex, mask);
				}
			}
			else if (codeChunk.mType & MCT_Texture)
			{
				BOOST_ASSERT(!codeChunk.mConstantExpression->isChangingPerFrame());
				int32 textureInputIndex = INDEX_NONE;
				const TCHAR* baseName = TEXT("");
				switch (codeChunk.mType)
				{
				case MCT_Texture2D:
					textureInputIndex = mMaterialCompilationOutput.mConstantExpressionSet.mConstant2DTextureExpression.addUnique(textureConstantExpression);
					baseName = TEXT("Texture2D");
					break;
				case MCT_TextureCube:
					textureInputIndex = mMaterialCompilationOutput.mConstantExpressionSet.mConstantCubeTextureExpressions.addUnique(textureConstantExpression);
					baseName = TEXT("TextureCube");
					break;
				default:
					AIR_LOG(LogMaterial, Fatal, TEXT("Unrecognized texture material value type : %u"), (int32)codeChunk.mType);
				}
				printf_b(formattedCode, TEXT("Material.%s_%u"), baseName, textureInputIndex);
			}
			else
			{
				AIR_LOG(logMaterial, Fatal, TEXT("User input of unknown type : %s"), describeType(codeChunk.mType));
			}
			return addInlinedCodeChunk((*mCurrentScopeChuncks)[index].mType, formattedCode);
		}

		int32 addInlinedCodeChunk(EMaterialValueType type, const TCHAR* format, ...)
		{
			int32 bufferSize = 256;
			TCHAR* formattedCode = nullptr;
			int32 result = -1;
			while (result == -1)
			{
				formattedCode = (TCHAR*)Memory::realloc(formattedCode, bufferSize * sizeof(TCHAR));
				GET_VARARGS_RESULT(formattedCode, bufferSize, bufferSize - 1, format, format, result);
				bufferSize *= 2;
			}
			formattedCode[result] = 0;
			const int32 codeIndex = addCodeChunkInner(formattedCode, type, true);
			Memory::free(formattedCode);
			return codeIndex;
		}

		EMaterialValueType getParameterType(int32 index) const
		{
			BOOST_ASSERT(index >= 0 && index < mCurrentScopeChuncks->size());
			return(*mCurrentScopeChuncks)[index].mType;
		}

		virtual int32 error(const TCHAR* text) override {
			wstring errorString;
			BOOST_ASSERT(mShaderFrequency < SF_NumFrequencies);
			auto& currentFunctionStack = mFunctionStacks[mShaderFrequency];
			if (currentFunctionStack.size() > 1)
			{
				RMaterialExpressionMaterialFunctionCall* errorFunction = currentFunctionStack[1].mFunctionCall;
				mMaterial->mErrorExpressions.add(errorFunction);
				errorFunction->mLastErrorText = text;
				errorString = wstring(TEXT("Function ")) + errorFunction->mMaterialFunction->getName() + TEXT(": ");
			}
			if (currentFunctionStack.last().mExpressionStack.size() > 0)
			{
				RMaterialExpression* errorExpression = currentFunctionStack.last().mExpressionStack.last().mExpression;
				BOOST_ASSERT(errorExpression);
				if (errorExpression->getClass() != RMaterialExpressionMaterialFunctionCall::StaticClass() && errorExpression->getClass() != RMaterialExpressionFunctionInput::StaticClass() && errorExpression->getClass() != RMaterialExpressionFunctionOutput::StaticClass())
				{
					mMaterial->mErrorExpressions.add(errorExpression);
					errorExpression->mLastErrorText = text;
					const int32 chopCount = CString::strlen(TEXT("MaterialExpression"));
					const wstring errorClassName = errorExpression->getClass()->getName();
					errorString += wstring(TEXT("(Node ")) + errorClassName.substr(errorClassName.length() - chopCount) + TEXT(") ");

				}
			}
			errorString += text;
			mMaterial->mCompileErrors.addUnique(errorString);
			bSuccess = false;
			return INDEX_NONE;
		}

		virtual EShaderFrequency getCurrentShaderFrequency() const override {
			return mShaderFrequency;
		}

		virtual int32 componentMask(int32 vector, bool r, bool g, bool b, bool a) override
		{
			if (vector == INDEX_NONE)
			{
				return INDEX_NONE;
			}
			EMaterialValueType vectorType = getParameterType(vector);
			if ((a && (vectorType & MCT_Float) < MCT_Float4) ||
				(b && (vectorType & MCT_Float) < MCT_Float3) ||
				(g && (vectorType & MCT_Float) < MCT_Float2) ||
				(r && (vectorType & MCT_Float) < MCT_Float1))
			{
				return errorf(TEXT("Not enough components in (%s %s) for component mask %u%u%u%u"), getParameterCode(vector).c_str(), describeType(getParameterType(vector)), r, g, b, a);
			}
			EMaterialValueType resultType;
			switch ((r ? 1: 0) + (g ? 1:0) + (b ? 1 : 0) + (a ? 1 : 0))
			{
			case 1 :
				resultType = MCT_Float1; break;
			case 2:
				resultType = MCT_Float2; break;
			case 3:
				resultType = MCT_Float3; break;
			case 4:
				resultType = MCT_Float4; break;
			default:
				return errorf(TEXT("Couldn't determin result type of component mask %u%u%u%u"), r, g, b, a);
				break;
			}

			wstring maskString = printf(TEXT("%s%s%s%s"),
				r ? TEXT("r") : TEXT(""),
				g ? (vectorType == MCT_Float ? TEXT("r") : TEXT("g")) : TEXT(""),
				b ? (vectorType == MCT_Float ? TEXT("r") : TEXT("b")) : TEXT(""),
				a ? (vectorType == MCT_Float ? TEXT("r") : TEXT("a")) : TEXT(""));
			auto* expression = getParameterConstantExpression(vector);
			if (expression)
			{
				int8 mask[4] = { -1, -1 - 1. - 1 };
				for (int32 index = 0; index < maskString.length(); ++index)
				{
					mask[index] = swizzleComponentToIndex(maskString[index]);
				}
				return addConstantExpression(new MaterialConstantExpressionComponentSwizzle(expression, mask[0], mask[1], mask[2], mask[3]), resultType, TEXT("%s.%s"), getParameterCode(vector).c_str(), maskString.c_str());

			}
			return addInlinedCodeChunk(resultType, TEXT("%s.%s"), getParameterCode(vector).c_str(), maskString.c_str());

		}

		int32 errorUnlessFeatureLevelSupported(ERHIFeatureLevel::Type requiredFeatureLevel)
		{
			if (mFeatureLevel < requiredFeatureLevel)
			{
				wstring featureLevelName;
				getFeatureLevelName(mFeatureLevel, featureLevelName);
				return errorf(TEXT("Node not supported in feature level %s"), featureLevelName.c_str());
			}
			return 0;
		}


		int32 addConstantExpression(MaterialConstantExpression* constantExpression, EMaterialValueType type, const TCHAR* format, ...)
		{
			if (type == MCT_Unknown)
			{
				return INDEX_NONE;
			}

			BOOST_ASSERT(constantExpression);
			if ((type & MCT_Texture) && !constantExpression->getTextureConstantExpression())
			{
				return errorf(TEXT("Operation not supported on a texture"));
			}
			if (type == MCT_StaticBool)
			{
				return errorf(TEXT("Operation not supported on a static bool"));
			}
			if (type == MCT_MaterialAttributes)
			{
				return errorf(TEXT("Operation not supported on a MaterialAttributes"));
			}
			bool bFoundExistingExpression = false;
			for (int32 expressionIndex = 0; expressionIndex < mConstantExpressions.size() & !bFoundExistingExpression; expressionIndex++)
			{
				MaterialConstantExpression* testExpression = mConstantExpressions[expressionIndex].mConstantExpression;
				BOOST_ASSERT(testExpression);
				if (testExpression->isIdentical(constantExpression))
				{
					bFoundExistingExpression = true;
					BOOST_ASSERT(type == mConstantExpressions[expressionIndex].mType);
					for (int32 chunkIndex = 0; chunkIndex < mCurrentScopeChuncks->size(); chunkIndex++)
					{
						MaterialConstantExpression* otherExpression = (*mCurrentScopeChuncks)[chunkIndex].mConstantExpression;
						if (otherExpression && otherExpression->isIdentical(constantExpression))
						{
							delete constantExpression;
							return chunkIndex;
						}
					}
					delete constantExpression;
					constantExpression = testExpression;
					break;
				}
			}
			int32 bufferSize = 256;
			TCHAR* formattedCode = nullptr;
			int32 result = -1;
			while (result == -1)
			{
				formattedCode = (TCHAR*)Memory::realloc(formattedCode, bufferSize * sizeof(TCHAR));
				GET_VARARGS_RESULT(formattedCode, bufferSize, bufferSize - 1, format, format, result);
				bufferSize *= 2;
			}
			formattedCode[result] = 0;
			const int32 returnIndex = mCurrentScopeChuncks->size();
			new(*mCurrentScopeChuncks)ShaderCodeChunk(constantExpression, formattedCode, type);
			if (bFoundExistingExpression)
			{
				new (mConstantExpressions)ShaderCodeChunk(constantExpression, formattedCode, type);
			}
			Memory::free(formattedCode);
			return returnIndex;
		}

		virtual void setMaterialProperty(EMaterialProperty inProperty, EShaderFrequency overrideShaderFrequency = SF_NumFrequencies, bool bUsePreviousframeTime = false) override;

		virtual void setBaseMaterialAttribute(const Guid& inAttributeId) override
		{
			BOOST_ASSERT(mMaterialAttributesStack.size() == 1);
			mMaterialAttributesStack.top() = inAttributeId;
		}

		MaterialConstantExpression* getParameterConstantExpression(int32 index)
		{
			BOOST_ASSERT(index >= 0 && index < mCurrentScopeChuncks->size());
			const ShaderCodeChunk& chunk = (*mCurrentScopeChuncks)[index];
			return chunk.mConstantExpression;
		}

		virtual int32 vectorParameter(wstring parameterName, const LinearColor& defaultValue)
		{
			return addConstantExpression(new MaterialConstantExpressionVectorParameter(parameterName, defaultValue), MCT_Float4, TEXT(""));
		}

		virtual int32 forceCast(int32 code, EMaterialValueType destType, int32 forceCastFlags = 0)
		{
			if (code == INDEX_NONE)
			{
				return INDEX_NONE;
			}
			if (getParameterConstantExpression(code) && !getParameterConstantExpression(code)->isConstant())
			{
				return forceCast(accessConstantExpression(code), destType, forceCastFlags);
			}
			EMaterialValueType sourceType = getParameterType(code);
			bool bExactMatch = (forceCastFlags & MFCF_ExactMatch) ? true : false;
			bool bReplicateValue = (forceCastFlags & MFCF_ReplicateValue) ? true : false;
			if (bExactMatch ? (sourceType == destType) : (sourceType & destType))
			{
				return code;
			}
			else if ((sourceType & MCT_Float) && (destType & MCT_Float))
			{
				const uint32 numSourceComponents = getNumComponents(sourceType);
				const uint32 numDestComponents = getNumComponents(destType);
				if (numSourceComponents > numDestComponents)
				{
					const TCHAR* mask;
					switch (numDestComponents)
					{
					case 1: mask = TEXT(".r"); break;
					case 2: mask = TEXT(".rg"); break;
					case 3: mask = TEXT(".rgb"); break;
					default:
						AIR_LOG(LogMaterial, Fatal, TEXT("should never get here!"));
						return INDEX_NONE;
					}
					return addInlinedCodeChunk(destType, TEXT("%s%s"), getParameterCode(code).c_str(), mask);
				}
				else if (numSourceComponents < numDestComponents)
				{
					if (numSourceComponents != 1)
					{
						bReplicateValue = false;
					}
					const uint32 numPadComponents = numDestComponents - numSourceComponents;
					wstring commaParameterColdeString = printf(TEXT(",%s"), getParameterCode(code).c_str());
					return addInlinedCodeChunk(destType,
						TEXT("%s(%s%s%s%s)"),
						HLSLTypeString(destType),
						getParameterCode(code).c_str(),
						numPadComponents >= 1 ? (bReplicateValue ? commaParameterColdeString.c_str() : TEXT(",0")) : TEXT(""),
						numPadComponents >= 2 ? (bReplicateValue ? commaParameterColdeString.c_str() : TEXT(",0")) : TEXT(""),
						numPadComponents >= 3 ? (bReplicateValue ? commaParameterColdeString.c_str() : TEXT(",0")) : TEXT(""));

				}
				else
				{
					return code;
				}
			}
			else
			{
				return errorf(TEXT("Cannot force a cast between non-numeric types."));
			}
		}

		EMaterialValueType getArithmeticResultType(EMaterialValueType typeA, EMaterialValueType typeB)
		{
			if (!(typeA & MCT_Float) || !(typeB & MCT_Float))
			{
				errorf(TEXT("Attemptin to perform arithmetic on non-numberic types: %s %s"), describeType(typeA), describeType(typeB));
				return MCT_Unknown;
			}
			if (typeA == typeB)
			{
				return typeA;
			}
			else if (typeA & typeB)
			{
				if (typeA == MCT_Float)
				{
					return typeB;
				}
				else
				{
					BOOST_ASSERT(typeB == MCT_Float);
					return typeA;
				}
			}
			else
			{
				errorf(TEXT("Aricthmetic between types %s and %s are undefined"), describeType(typeA), describeType(typeB));
				return MCT_Unknown;
			}
		}

		EMaterialValueType getArithmeticResultType(int32 a, int32 b)
		{
			BOOST_ASSERT(a >= 0 && a < mCurrentScopeChuncks->size());
			BOOST_ASSERT(b >= 0 && b < mCurrentScopeChuncks->size());
			EMaterialValueType typea = (*mCurrentScopeChuncks)[a].mType;
			EMaterialValueType typeB = (*mCurrentScopeChuncks)[b].mType;
			return getArithmeticResultType(typea, typeB);
		}

		virtual int32 add(int32 a, int32 b) override
		{
			if (a == INDEX_NONE || b == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			if (getParameterConstantExpression(a) && getParameterConstantExpression(b))
			{
				return addConstantExpression(new MaterialConstantExpressionFoldedMath(getParameterConstantExpression(a), getParameterConstantExpression(b), FMO_Add), getArithmeticResultType(a, b), TEXT("(%s + %s)"), getParameterCode(a).c_str(), getParameterCode(b).c_str());
			}
			else
			{
				return addCodeChunk(getArithmeticResultType(a, b), TEXT("(%s + %s)"), getParameterCode(a).c_str(), getParameterCode(b).c_str());
			}
		}

		virtual int32 sub(int32 a, int32 b) override
		{
			if (a == INDEX_NONE || b == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			if (getParameterConstantExpression(a) && getParameterConstantExpression(b))
			{
				return addConstantExpression(new MaterialConstantExpressionFoldedMath(getParameterConstantExpression(a), getParameterConstantExpression(b), FMO_Sub), getArithmeticResultType(a, b), TEXT("(%s - %s)"), getParameterCode(a).c_str(), getParameterCode(b).c_str());
			}
			else
			{
				return addCodeChunk(getArithmeticResultType(a, b), TEXT("(%s - %s)"), getParameterCode(a).c_str(), getParameterCode(b).c_str());
			}
		}

		virtual int32 mul(int32 a, int32 b) override
		{
			if (a == INDEX_NONE || b == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			if (getParameterConstantExpression(a) && getParameterConstantExpression(b))
			{
				return addConstantExpression(new MaterialConstantExpressionFoldedMath(getParameterConstantExpression(a), getParameterConstantExpression(b), FMO_Mul), getArithmeticResultType(a, b), TEXT("(%s * %s)"), getParameterCode(a).c_str(), getParameterCode(b).c_str());
			}
			else
			{
				return addCodeChunk(getArithmeticResultType(a, b), TEXT("(%s * %s)"), getParameterCode(a).c_str(), getParameterCode(b).c_str());
			}
		}

		virtual int32 div(int32 a, int32 b) override
		{
			if (a == INDEX_NONE || b == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			if (getParameterConstantExpression(a) && getParameterConstantExpression(b))
			{
				return addConstantExpression(new MaterialConstantExpressionFoldedMath(getParameterConstantExpression(a), getParameterConstantExpression(b), FMO_Div), getArithmeticResultType(a, b), TEXT("(%s / %s)"), getParameterCode(a).c_str(), getParameterCode(b).c_str());
			}
			else
			{
				return addCodeChunk(getArithmeticResultType(a, b), TEXT("(%s / %s)"), getParameterCode(a).c_str(), getParameterCode(b).c_str());
			}
		}

		wstring coerceParameter(int32 index, EMaterialValueType destType)
		{
			BOOST_ASSERT(index >= 0 && index < mCurrentScopeChuncks->size());
			const ShaderCodeChunk& codeChunk = (*mCurrentScopeChuncks)[index];
			if (codeChunk.mType == destType)
			{
				return getParameterCode(index);
			}
			else
			{
				if ((codeChunk.mType & destType) && (codeChunk.mType & MCT_Float))
				{
					switch (destType)
					{
					case MCT_Float1:
						return printf(TEXT("MaterialFloat(%s)"), getParameterCode(index).c_str());
					case MCT_Float2:
						return printf(TEXT("MaterialFloat2(%s, %s)"), getParameterCode(index).c_str(), getParameterCode(index).c_str());
					case MCT_Float3:
						return printf(TEXT("MaterialFloat3(%s, %s, %s)"), getParameterCode(index).c_str(), getParameterCode(index).c_str(), getParameterCode(index).c_str());
					case MCT_Float4:
						return printf(TEXT("MaterialFloat4(%s, %s, %s, %s)"), getParameterCode(index).c_str(), getParameterCode(index).c_str(), getParameterCode(index).c_str(), getParameterCode(index).c_str());
					default:
						return printf(TEXT("%s"), getParameterCode(index).c_str());
					}
				}
				else
				{
					errorf(TEXT("Coercion failed: %s: %s ->%s"), codeChunk.mDefinition.c_str(), describeType(codeChunk.mType), describeType(destType));
					return TEXT("");
				}
			}
		}
		
		virtual int32 dot(int32 a, int32 b) override
		{
			if (a == INDEX_NONE || b == INDEX_NONE)
			{
				return INDEX_NONE;
			}
			MaterialConstantExpression* expressionA = getParameterConstantExpression(a);
			MaterialConstantExpression* expressionB = getParameterConstantExpression(b);
			EMaterialValueType typeA = getParameterType(a);
			EMaterialValueType typeB = getParameterType(b);
			if (expressionA && expressionB)
			{
				if (typeA == MCT_Float && typeB == MCT_Float)
				{
					return addConstantExpression(new MaterialConstantExpressionFoldedMath(expressionA, expressionB, FMO_Mul), MCT_Float, TEXT("mul(%s, %s)"), getParameterCode(a).c_str(), getParameterCode(b).c_str());

				}
				else
				{
					if (typeA == typeB)
					{
						return addConstantExpression(new MaterialConstantExpressionFoldedMath(expressionA, expressionB, FMO_Dot, typeA), MCT_Float, TEXT("dot(%s, %s)"), getParameterCode(a).c_str(), getParameterCode(b).c_str());
					}
					else
					{
						if (typeA == MCT_Float || (typeB != MCT_Float && getNumComponents(typeA) > getNumComponents(typeB)))
						{
							return addConstantExpression(new MaterialConstantExpressionFoldedMath(expressionA, expressionB, FMO_Dot, typeB), MCT_Float, TEXT("dot(%s, %s)"), coerceParameter(a, typeB).c_str(), getParameterCode(b).c_str());
						}
						else
						{
							return addConstantExpression(new MaterialConstantExpressionFoldedMath(expressionA, expressionB, FMO_Dot, typeA), MCT_Float, TEXT("dot(%s, %s)"), getParameterCode(a).c_str(), coerceParameter(b, typeA).c_str());
						}
					}
				}
			}
			else
			{
				if (typeA == MCT_Float || (typeB != MCT_Float && getNumComponents(typeA) > getNumComponents(typeB)))
				{
					return addCodeChunk(MCT_Float, TEXT("dot(%s, %s)"), coerceParameter(a, typeB).c_str(), getParameterCode(b).c_str());
				}
				else
				{
					return addCodeChunk(MCT_Float, TEXT("dot(%s, %s)"), getParameterCode(a).c_str(), coerceParameter(b, typeA).c_str());
				}
			}
		}

		virtual int32 cross(int32 a, int32 b) override
		{
			if (a == INDEX_NONE || b == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			return addCodeChunk(MCT_Float3, TEXT("cross(%s, %s)"), coerceParameter(a, MCT_Float3).c_str(), coerceParameter(b, MCT_Float3).c_str());
		}

		virtual int32 power(int32 a, int32 b) override
		{
			if (a == INDEX_NONE || b == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			return addCodeChunk(getParameterType(a), TEXT("ClampedPow(%s, %s)"), getParameterCode(a).c_str(), coerceParameter(b, MCT_Float).c_str());
		}

		virtual int32 logarithm2(int32 x) override
		{
			if (x == INDEX_NONE)
			{
				return INDEX_NONE;
			}
			if (getParameterConstantExpression(x))
			{
				return addConstantExpression(new MaterialConstantExpressionLogarithm2(getParameterConstantExpression(x)), getParameterType(x), TEXT("log2(%s)"), getParameterCode(x).c_str());
			}
			return addCodeChunk(getParameterType(x), TEXT("log2(%s)"), getParameterCode(x).c_str());
		}

		virtual int32 squareRoot(int32 x) override
		{
			if (x == INDEX_NONE)
			{
				return INDEX_NONE;
			}
			if (getParameterConstantExpression(x))
			{
				return addConstantExpression(new MaterialConstantExpressionSquareRoot(getParameterConstantExpression(x)), getParameterType(x), TEXT("sqrt(%s)"), getParameterCode(x).c_str());
			}
			return addCodeChunk(getParameterType(x), TEXT("sqrt(%s)"), getParameterCode(x).c_str());
		}

		virtual int32 length(int32 x) override
		{
			if (x == INDEX_NONE)
			{
				return INDEX_NONE;
			}
			if (getParameterConstantExpression(x))
			{
				return addConstantExpression(new MaterialConstantExpressionLength(getParameterConstantExpression(x), getParameterType(x)), MCT_Float, TEXT("length(%s)"), getParameterCode(x).c_str());
			}
			return addCodeChunk(MCT_Float, TEXT("length(%s)"), getParameterCode(x).c_str());
		}

		virtual int32 lerp(int32 x, int32 y, int32 a) override
		{
			if (x == INDEX_NONE || y == INDEX_NONE || a == INDEX_NONE)
			{
				return INDEX_NONE;
			}
			MaterialConstantExpression* expressionX = getParameterConstantExpression(x);
			MaterialConstantExpression* expressionY = getParameterConstantExpression(y);
			MaterialConstantExpression* expressionA = getParameterConstantExpression(a);
			bool bExpressinEqual = false;
			if (x == y)
			{
				bExpressinEqual = true;
			}
			else if (expressionX && expressionY)
			{
				if (expressionX->isConstant() && expressionY->isConstant() && (*mCurrentScopeChuncks)[x].mType == (*mCurrentScopeChuncks)[y].mType)
				{
					LinearColor valueX, valueY;
					MaterialRenderContext dummyContext(nullptr, *mMaterial, nullptr);
					expressionX->getNumberValue(dummyContext, valueX);
					expressionY->getNumberValue(dummyContext, valueY);
					if (valueX == valueY)
					{
						bExpressinEqual = true;
					}
				}
			}
			if (bExpressinEqual)
			{
				return x;
			}
			EMaterialValueType resultType = getArithmeticResultType(x, y);
			EMaterialValueType alphaType = resultType == (*mCurrentScopeChuncks)[a].mType ? resultType : MCT_Float;
			if (alphaType == MCT_Float1 && expressionA && expressionA->isConstant())
			{
				LinearColor value;
				MaterialRenderContext dummyContext(nullptr, *mMaterial, nullptr);
				expressionA->getNumberValue(dummyContext, value);
				if (value.R == 0.0f)
				{
					return x;

				}
				else if (value.R == 1.0f)
				{
					return y;
				}
			}
			return addCodeChunk(resultType, TEXT("lerp(%s, %s, %s)"), coerceParameter(x, resultType).c_str(), coerceParameter(y, resultType).c_str(), coerceParameter(a, alphaType).c_str());
		}

		virtual int32 min(int32 a, int32 b) override
		{
			if (a == INDEX_NONE || b == INDEX_NONE)
			{
				return INDEX_NONE;
			}
			if (getParameterConstantExpression(a) && getParameterConstantExpression(b))
			{
				return addConstantExpression(new MaterialConstantExpressionMin(getParameterConstantExpression(a), getParameterConstantExpression(b)), getParameterType(a), TEXT("min(%s, %s)"), getParameterCode(a).c_str(), coerceParameter(b, getParameterType(a)).c_str());
			}
			else
			{
				return addCodeChunk(getParameterType(a), TEXT("min(%s, %s)"), getParameterCode(a).c_str(), coerceParameter(b, getParameterType(a)).c_str());
			}
		}


		virtual int32 max(int32 a, int32 b) override
		{
			if (a == INDEX_NONE || b == INDEX_NONE)
			{
				return INDEX_NONE;
			}
			if (getParameterConstantExpression(a) && getParameterConstantExpression(b))
			{
				return addConstantExpression(new MaterialConstantExpressionMax(getParameterConstantExpression(a), getParameterConstantExpression(b)), getParameterType(a), TEXT("max(%s, %s)"), getParameterCode(a).c_str(), coerceParameter(b, getParameterType(a)).c_str());
			}
			else
			{
				return addCodeChunk(getParameterType(a), TEXT("max(%s, %s)"), getParameterCode(a).c_str(), coerceParameter(b, getParameterType(a)).c_str());
			}
		}

		virtual int32 clamp(int32 x, int32 a, int32 b) override {
			if (x == INDEX_NONE || a == INDEX_NONE || b == INDEX_NONE)
			{
				return INDEX_NONE;
			}
			if (getParameterConstantExpression(x) && getParameterConstantExpression(a) && getParameterConstantExpression(b))
			{
				return addConstantExpression(new MaterialConstantExpressionClamp(getParameterConstantExpression(x), getParameterConstantExpression(a), getParameterConstantExpression(b)), getParameterType(x), TEXT("clamp(%s, %s, %s)"), getParameterCode(x).c_str(), coerceParameter(a, getParameterType(x)).c_str(), coerceParameter(b, getParameterType(x)).c_str());
			}
			else
			{
				addCodeChunk(getParameterType(x), TEXT("(clamp(%s, %s, %s)"), getParameterCode(x).c_str(), coerceParameter(a, getParameterType(x)).c_str(), coerceParameter(b, getParameterType(x)).c_str());
			}
		}

		virtual int32 saturate(int32 x) override
		{
			if (x == INDEX_NONE)
			{
				return INDEX_NONE;
			}
			if (getParameterConstantExpression(x))
			{
				return addConstantExpression(new MaterialConstantExpressionSaturate(getParameterConstantExpression(x)), getParameterType(x), TEXT("saturate(%s)"), getParameterCode(x).c_str());
			}
			else
			{
				addCodeChunk(getParameterType(x), TEXT("saturate(%s)"), getParameterCode(x).c_str());
			}
		}
		virtual int32 appendVector(int32 a, int32 b) override
		{
			if (a == INDEX_NONE || b == INDEX_NONE)
			{
				return INDEX_NONE;
			}
			int32 numResultComponents = getNumComponents(getParameterType(a)) + getNumComponents(getParameterType(b));
			EMaterialValueType resultType = getVectorType(numResultComponents);
			if (getParameterConstantExpression(a) && getParameterConstantExpression(b))
			{
				return addConstantExpression(new MaterialConstantExpressionAppendVector(getParameterConstantExpression(a), getParameterConstantExpression(b), getNumComponents(getParameterType(a))), resultType, TEXT("MaterialFloat%u(%s, %s)"), numResultComponents, getParameterCode(a).c_str(), getParameterCode(b).c_str());
			}
			else
			{
				return addInlinedCodeChunk(resultType, TEXT("MaterialFloat%u(%s, %s)"), numResultComponents, getParameterCode(a).c_str(), getParameterCode(b).c_str());
			}
		}

		virtual int32 callExpression(MaterialExpressionKey expressionKey, MaterialCompiler* compiler) override
		{
			if (expressionKey.mExpression && !expressionKey.mExpression->containsInputLoop() && !expressionKey.mExpression->isResultMaterialAttributes(expressionKey.mOuputIndex))
			{
				expressionKey.mMaterialAttributeID = Guid(0, 0, 0, 0);
			}
			BOOST_ASSERT(mShaderFrequency < SF_NumFrequencies);
			auto& currentFunctionStack = mFunctionStacks[mShaderFrequency];
			auto& existingCodeIt = currentFunctionStack.last().mExpressionCodeMap.find(expressionKey);
			if (existingCodeIt != currentFunctionStack.last().mExpressionCodeMap.end())
			{
				return existingCodeIt->second;
			}
			else
			{
				if (currentFunctionStack.last().mExpressionStack.find(expressionKey) != INDEX_NONE)
				{
					return error(TEXT("Reentrant expression"));
				}
				currentFunctionStack.last().mExpressionStack.add(expressionKey);
				const int32 functionDepth = currentFunctionStack.size();
				int32 result = expressionKey.mExpression->compile(compiler, expressionKey.mOuputIndex);
				MaterialExpressionKey popedExpressionKey = currentFunctionStack.last().mExpressionStack.pop();
				BOOST_ASSERT(popedExpressionKey == expressionKey);
				BOOST_ASSERT(functionDepth == currentFunctionStack.size());
				currentFunctionStack.last().mExpressionCodeMap.emplace(expressionKey, result);
				return result;
			}

			

		}

		virtual const Guid getMaterialAttribute() override
		{
			return mMaterialAttributesStack.last();
		}

		virtual Guid popMaterialAttribute() override
		{
			return mMaterialAttributesStack.pop(false);
		}

		virtual int32 constant(float x) override
		{
			return addConstantExpression(new MaterialConstantExpressionConstant(LinearColor(x, x, x, x), MCT_Float), MCT_Float, TEXT("%0.8f"), x);
		}
		virtual int32 constant2(float x, float y) override
		{
			return addConstantExpression(new MaterialConstantExpressionConstant(LinearColor(x, y, 0, 0), MCT_Float2), MCT_Float2, TEXT("MaterialFloat2(%0.8f, %0.8f)"), x, y);
		}
		virtual int32 constant3(float x, float y, float z) override
		{
			return addConstantExpression(new MaterialConstantExpressionConstant(LinearColor(x, y, z, 0), MCT_Float3), MCT_Float3, TEXT("MaterialFloat3(%0.8f, %0.8f, %0.8f)"), x, y, z);
		}
		virtual int32 constant4(float x, float y, float z, float w) override
		{
			return addConstantExpression(new MaterialConstantExpressionConstant(LinearColor(x, y, z, w), MCT_Float4), MCT_Float4, TEXT("MaterialFloat4(%0.8f, %0.8f, %0.8f, %0.8f)"), x, y, z, w);
		}

		virtual int32 textureCoordinate(uint32 coordinateIndex, bool unMirrorU, bool unMirrorV) override
		{
			const int32 maxNumCoordinates = (mFeatureLevel == ERHIFeatureLevel::ES2) ? 3 : 8;
			if (coordinateIndex >= maxNumCoordinates)
			{
				return errorf(TEXT("Only %u texture coordinate sets can be used by this feature level, currently using %u"), maxNumCoordinates, coordinateIndex + 1);
			}
			if (mShaderFrequency == SF_Vertex)
			{
				mNumUserVertexTexCoords = Math::max(coordinateIndex + 1, mNumUserVertexTexCoords);

			}
			else
			{
				mNumUserTexCoords = Math::max(coordinateIndex + 1, mNumUserTexCoords);
			}
			wstring sampleCode;
			if (unMirrorU && unMirrorV)
			{
				sampleCode = TEXT("UnMirrorUV(parameters.TexCoords[%u].xy, parameters)");
			}
			else if (unMirrorU)
			{
				sampleCode = TEXT("UnMirrorU(parameters.TexCoords[%u].xy, parameters)");
			}
			else if (unMirrorV)
			{
				sampleCode = TEXT("UnMirrorV(parameters.TexCoords[%u].xy, parameters)");
			}
			else
			{
				sampleCode = TEXT("parameters.TexCoords[%u].xy");
			}
			return addInlinedCodeChunk(MCT_Float2, sampleCode.c_str(), coordinateIndex);
		}

		virtual int32 scalarParameter(wstring parameterName, float defaultValue)
		{
			return addConstantExpression(new MaterialConstantExpressionScalarParameter(parameterName, defaultValue), MCT_Float, TEXT(""));
		}

		virtual int32 textureSample(int32 textureIndex, int32 coordinateIndex, enum EMaterialSamplerType samplerType, int32 mipValue0Index  = INDEX_NONE, int32 mipValue1Index = INDEX_NONE, ETextureMipValueMode mipValueMode = TMVM_None, ESamplerSourceMode samplerSource = SSM_FromTextureAsset, int32 textureReferenceIndex = INDEX_NONE) override
		{
			if (textureIndex == INDEX_NONE || coordinateIndex == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			if (mFeatureLevel == ERHIFeatureLevel::ES2 && mShaderFrequency == SF_Vertex)
			{
				if (mipValueMode != TMVM_MipLevel)
				{
					errorf(TEXT("Sampling from vertex textures requires an absolute mip level on feature level es2!"));
					return INDEX_NONE;
				}
			}
			else if (mShaderFrequency != SF_Pixel && errorUnlessFeatureLevelSupported(ERHIFeatureLevel::ES3_1) == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			EMaterialValueType textureType = getParameterType(textureIndex);

			if (textureType != MCT_Texture2D && textureType != MCT_TextureCube)
			{
				errorf(TEXT("Sampling unknown texture type: %s"), describeType(textureType));
				return INDEX_NONE;
			}
			if (mShaderFrequency != SF_Pixel && mipValueMode == TMVM_MipBias)
			{
				errorf(TEXT("MipBias is only supported in the pixel shader"));
				return INDEX_NONE;
			}

			wstring mipValue0Code = TEXT("0.0f");
			wstring mipValue1Code = TEXT("0.0f");

			if (mipValue0Index != INDEX_NONE && (mipValueMode == TMVM_MipBias || mipValueMode == TMVM_MipLevel))
			{
				mipValue0Code = coerceParameter(mipValue0Index, MCT_Float1);
			}
			if (mShaderFrequency != SF_Pixel)
			{
				mipValueMode = TMVM_MipLevel;
			}

			wstring samplerStateCode;
			if (samplerSource == SSM_FromTextureAsset)
			{
				samplerStateCode = TEXT("%sSampler");
			}
			else if (samplerSource == SSM_Wrap_WorldGroupSettings)
			{
				samplerStateCode = TEXT("GetMaterialSharedSampler(%sSampler, Material.Wrap_WroldGroupSettings)");
			}
			else if (samplerSource == SSM_Clamp_WorldGroupSettings)
			{
				samplerStateCode = TEXT("GetMaterialSharedSampler(%sSampler, Material.Clamp_WorldGroupSettings)");
			}

			wstring sampleCode = (textureType == MCT_TextureCube)
				? TEXT("TextureCubeSample") : TEXT("Texture2DSample");

			EMaterialValueType uvsType = (textureType == MCT_TextureCube) ? MCT_Float3 : MCT_Float2;

			if (mipValueMode == TMVM_None)
			{
				sampleCode += wstring(TEXT("(%s,")) + samplerStateCode + TEXT(", %s)");
			}
			else if (mipValueMode == TMVM_MipLevel)
			{
				if (errorUnlessFeatureLevelSupported(ERHIFeatureLevel::ES3_1) == INDEX_NONE)
				{
					errorf(TEXT("Sampling for a specific mip-level is not supported for es2"));
					return INDEX_NONE;
				}
				sampleCode += TEXT("Level(%s, %sSampler, %s, %s)");
			}
			else if (mipValueMode == TMVM_MipBias)
			{
				sampleCode += TEXT("Bias(%s, %sSampler, %s, %s)");
			}
			else if (mipValueMode == TMVM_Derivative)
			{
				if (mipValue0Index == INDEX_NONE)
				{
					return errorf(TEXT("Missing DDX(UVs) parameter"));
				}
				else if (mipValue1Index == INDEX_NONE)
				{
					return errorf(TEXT("Missing DDY(UVs) parameter"));
				}
				sampleCode += TEXT("Grad(%s, %sSampler, %s, %s, %s)");

				mipValue0Code = coerceParameter(mipValue0Index, uvsType);
				mipValue1Code = coerceParameter(mipValue1Index, uvsType);
			}
			else
			{
				BOOST_ASSERT(false);
			}

			switch (samplerType)
			{
			case Air::SamplerType_Color:
				sampleCode = printf(TEXT("processMaterialColorTextureLookup(%s)"), sampleCode.c_str());
				break;

			case Air::SamplerType_Grayscale:
				sampleCode = printf(TEXT("processMaterialLinearGreyscaleTextureLookup((%s).r).rrrr"), sampleCode.c_str());
				break;
			case Air::SamplerType_Normal:
				sampleCode = printf(TEXT("unpackNormalMap(%s)"), sampleCode.c_str());
				break;
			case Air::SamplerType_Masks:
				break;
			case Air::SamplerType_DistanceFieldFont:
			case Air::SamplerType_Alpha:
				sampleCode = printf(TEXT("(%s).rrrr"), sampleCode.c_str());
				break;
			case Air::SamplerType_LinearColor:
				sampleCode = printf(TEXT("processMaterialLinearColorTextureLookup(%s)"), sampleCode.c_str());
				break;
			case Air::SamplerType_LinearGrayscale:
				sampleCode = printf(TEXT("processMaterialLinearGreyscaleTextureLookup((%s).r).rrrr"), sampleCode.c_str());
				break;
			}

			wstring textureName = (textureType == MCT_TextureCube) ? coerceParameter(textureIndex, MCT_TextureCube) : coerceParameter(textureIndex, MCT_Texture2D);

			wstring uvs = coerceParameter(coordinateIndex, uvsType);

			int32 samplingCodeIndex = addCodeChunk(MCT_Float4, sampleCode.c_str(), textureName.c_str(), textureName.c_str(), uvs.c_str(), mipValue0Code.c_str(), mipValue1Code.c_str());

			return samplingCodeIndex;
		}

		virtual int32 textureParameter(wstring parameterName, std::shared_ptr<RTexture> defaultValue, int32& textureReferenceIndex, ESamplerSourceMode samplerSource = SSM_FromTextureAsset)
		{
			if (mShaderFrequency != SF_Pixel && errorUnlessFeatureLevelSupported(ERHIFeatureLevel::SM4) == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			EMaterialValueType shaderType = defaultValue->getMaterialType();
			textureReferenceIndex = mMaterial->getReferencedTextures().find(defaultValue);
			BOOST_ASSERT(textureReferenceIndex != INDEX_NONE);
			return addConstantExpression(new MaterialConstantExpressionTextureParameter(parameterName, textureReferenceIndex, samplerSource), shaderType, TEXT(""));
		}

		virtual int32 texture(std::shared_ptr<RTexture> inTexture, int32& textureReferenceIndex, ESamplerSourceMode samplerSource = SSM_FromTextureAsset, ETextureMipValueMode mipValueMode = TMVM_None) override
		{
			if (mFeatureLevel == ERHIFeatureLevel::ES2 && mShaderFrequency == SF_Vertex)
			{
				if (mipValueMode != TMVM_MipLevel)
				{
					errorf(TEXT("Sampling from vertex textures requires an absolute mip level on feature level es2"));
					return INDEX_NONE;
				}
			}
			else if (mShaderFrequency != SF_Pixel && errorUnlessFeatureLevelSupported(ERHIFeatureLevel::ES3_1) == INDEX_NONE)
			{
				return INDEX_NONE;
			}

			EMaterialValueType shaderType = inTexture->getMaterialType();
			textureReferenceIndex = mMaterial->getReferencedTextures().find(inTexture);
			BOOST_ASSERT(textureReferenceIndex != INDEX_NONE);
			return addConstantExpression(new MaterialConstantExpressionTexture(textureReferenceIndex, samplerSource), shaderType, TEXT(""));
		}

		wstring getMaterialShaderCode();
	};

}