#include "HLSLMaterialTranslator.h"
#include "Containers/LazyPrintf.h"
#include "Classes/Materials/MaterialExpressionCustomOutput.h"
#include "Classes/Materials/Material.h"
#include "Containers/Set.h"
#include "class.h"
namespace Air
{
	wstring getDefinitions(TArray<ShaderCodeChunk>& codeChunks, int32 startChunk, int32 endChunk)
	{
		wstring definitions;
		for (int32 chunkIndex = startChunk; chunkIndex < endChunk; chunkIndex++)
		{
			const ShaderCodeChunk& codeChunk = codeChunks[chunkIndex];
			if (!codeChunk.mConstantExpression && !codeChunk.bInline)
			{
				definitions += codeChunk.mDefinition;
			}
		}
		return definitions;
	}

	void getFixedParameterCode(int32 startChunk, int32 endChunk, int32 resultIndex, TArray<ShaderCodeChunk>& codeChunks, wstring& outDefinitions, wstring & outValue)
	{
		if (resultIndex != INDEX_NONE)
		{
			BOOST_ASSERT(resultIndex >= 0 && resultIndex < codeChunks.size());
			BOOST_ASSERT(!codeChunks[resultIndex].mConstantExpression || codeChunks[resultIndex].mConstantExpression->isConstant());
			if (codeChunks[resultIndex].mConstantExpression && codeChunks[resultIndex].mConstantExpression->isConstant())
			{
				const ShaderCodeChunk& resultChunk = codeChunks[resultIndex];
				outValue = resultChunk.mDefinition;
			}
			else
			{
				const ShaderCodeChunk& resultChunk = codeChunks[resultIndex];
				BOOST_ASSERT(resultChunk.bInline || resultChunk.mSymbolName.length() > 0);
				outDefinitions = getDefinitions(codeChunks, startChunk, endChunk);
				outValue = resultChunk.bInline ? resultChunk.mDefinition : resultChunk.mSymbolName;
			}
		}
		else
		{
			outValue = TEXT("0");
		}
	}

	void getFixedParameterCode(int32 resultIndex, TArray<ShaderCodeChunk>& codeChunks, wstring& outDefinitions, wstring & outValue)
	{
		getFixedParameterCode(0, codeChunks.size(), resultIndex, codeChunks, outDefinitions, outValue);
	}

	bool HLSLMaterialTranslator::translate()
	{
		bSuccess = true;
		mMaterial->mCompileErrors.empty();
		mMaterial->mErrorExpressions.empty();

		int32 normalCodeChunkEnd = -1;
		int32 chunk[CompiledMP_Max];

		memset(chunk, -1, sizeof(chunk));
		const EShaderFrequency normalShaderFrequency = MaterialAttributeDefinationMap::getShaderFrequency(MP_Normal);
		{
			BOOST_ASSERT(mSharedPropertyCodeChunks[normalShaderFrequency].size() == 0);
			chunk[MP_Normal] = mMaterial->compilePropertyAndSetMaterialProperty(MP_Normal, this);
			normalCodeChunkEnd = mSharedPropertyCodeChunks[normalShaderFrequency].size();
		}
		chunk[MP_BaseColor] = mMaterial->compilePropertyAndSetMaterialProperty(MP_BaseColor, this);
		chunk[MP_Metallic] = mMaterial->compilePropertyAndSetMaterialProperty(MP_Metallic, this);
		chunk[MP_Specular] = mMaterial->compilePropertyAndSetMaterialProperty(MP_Specular, this);
		chunk[MP_Roughness] = mMaterial->compilePropertyAndSetMaterialProperty(MP_Roughness, this);
		EMaterialShadingModel materialShadingModel = mMaterial->getShadingModel();
		const EMaterialDomain domain = (const EMaterialDomain)mMaterial->getMaterialDomain();

		const uint32 saveNumUserTexCoords = mNumUserTexCoords;
		if (mMaterial->getBlendMode() == BLEND_Modulate && materialShadingModel != MSM_Unlit)
		{
			errorf(TEXT("Dynamically lit translucency is not supported for blend_modulate materials"));

		}
		if (mMaterial->isLightFunction() && mMaterial->getBlendMode() != BLEND_Opaque)
		{
			errorf(TEXT("Light function materials must be opaque"));
		}
		if (mMaterial->isLightFunction() && materialShadingModel != MSM_Unlit)
		{
			errorf(TEXT("Light function materials must use unlit"));
		}

		if (domain == MD_PostProcess && materialShadingModel != MSM_Unlit)
		{
			errorf(TEXT("Post process materials must use unlit"));
		}
		mResourcesString = TEXT("");
		{
			TArray<RMaterialExpressionCustomOutput*> customOutputExpressions;
			mMaterial->gatherCustomOutputExpressions(customOutputExpressions);
			TSet<RClass*> seenCustomOutputExpressionsClasses;
			for (RMaterialExpressionCustomOutput* customOutput : customOutputExpressions)
			{
				if (seenCustomOutputExpressionsClasses.contains(customOutput->getClass()))
				{
					errorf(TEXT("The material can contain only onc %s node"), customOutput->getDescription().c_str());
				}
				else
				{
					seenCustomOutputExpressionsClasses.add(customOutput->getClass());
					int32 numOutputs = customOutput->getNumOutputs();
					mResourcesString += printf(TEXT("#define NUM_MATERIAL_OUTPUTS_%s %d\r\n"), boost::to_upper_copy(customOutput->getFunctionName()).c_str(), numOutputs);
					if (numOutputs > 0)
					{
						for (int32 index = 0; index < numOutputs; index++)
						{
							{
								mFunctionStacks[SF_Pixel].empty();
								mFunctionStacks[SF_Pixel].add(MaterialFunctionCompileState(nullptr));
							}
							mMaterialProperty = MP_Max;
							mShaderFrequency = SF_Pixel;
							TArray<ShaderCodeChunk> customExpressionChunks;
							mCurrentScopeChuncks = &customExpressionChunks;
							customOutput->compile(this, index);
						}
					}
				}
			}
		}
		for (int32 expressionIndex = 0; expressionIndex < mCustomExpressionImplementations.size(); expressionIndex++)
		{
			mResourcesString += mCustomExpressionImplementations[expressionIndex] + TEXT("\r\n\r\n");
		}
		{
			for (int32 index = 0, num = mMaterialCompilationOutput.mConstantExpressionSet.mPerFrameConstantScalarExpressions.size(); index < num; ++index)
			{
				mResourcesString += printf(TEXT("float Air_Material_PerFrameScalarExpression%u;"), index) + TEXT("\r\n\r\n");
			}

			for (int32 index = 0, num = mMaterialCompilationOutput.mConstantExpressionSet.mPerFrameConstantVectorExpressions.size(); index < num; ++index)
			{
				mResourcesString += printf(TEXT("float4 Air_Material_PerFrameVectorExpression%u;"), index) + TEXT("\r\n\r\n");
			}
			for (int32 index = 0, num = mMaterialCompilationOutput.mConstantExpressionSet.mPerFramePrevConstantScalarExpressions.size(); index < num; ++index)
			{
				mResourcesString += printf(TEXT("float Air_Material_PerFramePrevScalarExpression%u;"), index) + TEXT("\r\n\r\n");
			}
			for (int32 index = 0, num = mMaterialCompilationOutput.mConstantExpressionSet.mPerFramePrevConstantVectorExpressions.size(); index < num; ++index)
			{
				mResourcesString += printf(TEXT("float PerFramePrevConstantVectorExpression%u;"), index) + TEXT("\r\n\r\n");
			}
		}
		{
			getFixedParameterCode(0, normalCodeChunkEnd, chunk[MP_Normal], mSharedPropertyCodeChunks[normalShaderFrequency], mTranslatedCodeChunkDefinitions[MP_Normal], mTranslatedCodeChunks[MP_Normal]);
			if (mTranslatedCodeChunkDefinitions[MP_Normal].size() == 0)
			{
				mTranslatedCodeChunkDefinitions[MP_Normal] = getDefinitions(mSharedPropertyCodeChunks[normalShaderFrequency], 0, normalCodeChunkEnd);
			}

		}
		for (uint32 propertyId = 0; propertyId < MP_Max; ++propertyId)
		{
			if (propertyId == MP_Normal)
			{
				continue;
			}
			const EShaderFrequency propertyShaderFrequency = MaterialAttributeDefinationMap::getShaderFrequency((EMaterialProperty)propertyId);
			int32 startChunk = 0;
			if (propertyShaderFrequency == normalShaderFrequency && mSharedPixelProperties[propertyId])
			{
				startChunk = normalCodeChunkEnd;
			}
			getFixedParameterCode(startChunk, mSharedPropertyCodeChunks[propertyShaderFrequency].size(), chunk[propertyId], mSharedPropertyCodeChunks[propertyShaderFrequency], mTranslatedCodeChunkDefinitions[propertyId], mTranslatedCodeChunks[propertyId]);

		}
		for (uint32 propertyId = MP_Max; propertyId < CompiledMP_Max; ++propertyId)
		{
			switch (propertyId)
			{
			case CompiledMP_EmissiveColorCS:
				if (bCompileForComputeShader)
				{
					getFixedParameterCode(chunk[propertyId], mSharedPropertyCodeChunks[SF_Compute], mTranslatedCodeChunkDefinitions[propertyId], mTranslatedCodeChunks[propertyId]);
				}
				break;
			default:
				BOOST_ASSERT(false);
				break;
			}
		}
		for (int32 expressionIndex = 0; expressionIndex < mCustomExpressionImplementations.size(); expressionIndex++)
		{
			mResourcesString += mCustomOutputImplementations[expressionIndex] + TEXT("\r\n\r\n");
		}

		loadShaderSourceFileChecked(TEXT("MaterialTemplate"), mMaterialTemplate);
		const int32 lineIndex = mMaterialTemplate.find(TEXT("#line"));
		BOOST_ASSERT(lineIndex != INDEX_NONE);
		mMaterialTemplateLineNumber = INDEX_NONE;
		int32 startPosition = lineIndex + 1;
		do
		{
			mMaterialTemplateLineNumber++;
			startPosition = StringUtil::find(mMaterialTemplate, TEXT("\r"), ESearchCase::CaseSensitive, ESearchDir::FromEnd, startPosition - 1);

		} while (startPosition != INDEX_NONE);
		BOOST_ASSERT(mMaterialTemplateLineNumber != INDEX_NONE);
		mMaterialTemplateLineNumber += 3;
		mMaterialCompilationOutput.mConstantExpressionSet.setParameterCollections(mParameterCollections);
		mMaterialCompilationOutput.mConstantExpressionSet.createBufferStruct();


		return bSuccess;
	}

	void HLSLMaterialTranslator::setMaterialProperty(EMaterialProperty inProperty, EShaderFrequency overrideShaderFrequency /* = SF_NumFrequencies */, bool bUsePreviousframeTime /* = false */)
	{
		mMaterialProperty = inProperty;
		setBaseMaterialAttribute(MaterialAttributeDefinationMap::getID(inProperty));
		if (overrideShaderFrequency != SF_NumFrequencies)
		{
			mShaderFrequency = overrideShaderFrequency;
		}
		else
		{
			mShaderFrequency = MaterialAttributeDefinationMap::getShaderFrequency(inProperty);
		}
		bCompilingPreviousFrame = bUsePreviousframeTime;
		mCurrentScopeChuncks = &mSharedPropertyCodeChunks[mShaderFrequency];

	}

	wstring HLSLMaterialTranslator::getMaterialShaderCode()
	{
		LazyPrintf lazyPrintf(mMaterialTemplate.c_str());
		lazyPrintf.pushParam(printf(TEXT("%u"), mNumUserVertexTexCoords).c_str());
		lazyPrintf.pushParam(printf(TEXT("%u"), mNumUserTexCoords).c_str());
		wstring pixelMembersDeclaration;
		wstring normalAssignment;
		wstring pixelMembersSetupAndAssignments;
		getSharedInputsMaterialCode(pixelMembersDeclaration, normalAssignment, pixelMembersSetupAndAssignments);
		lazyPrintf.pushParam(pixelMembersDeclaration.c_str());
		lazyPrintf.pushParam(mResourcesString.c_str());
		lazyPrintf.pushParam(printf(TEXT("return %.5f"), mMaterial->getOpacityMaskClipValue()).c_str());

		lazyPrintf.pushParam(mTranslatedCodeChunkDefinitions[MP_Normal].c_str());
		lazyPrintf.pushParam(normalAssignment.c_str());
		lazyPrintf.pushParam(pixelMembersSetupAndAssignments.c_str());
		lazyPrintf.pushParam(printf(TEXT("%u"), mMaterialTemplateLineNumber).c_str());
		return lazyPrintf.getResultString();
		
	}

	void HLSLMaterialTranslator::getSharedInputsMaterialCode(wstring& pixelMembersDeclaration, wstring & normalAssignment, wstring& pixelMembersInitializationEplilog)
	{
		int32 lastProperty = -1;
		wstring pixelInputInitializerValues;
		wstring normalInitializerValue;
		for (int32 propertyIndex = 0; propertyIndex < MP_Max; ++propertyIndex)
		{
			if (!mSharedPixelProperties[propertyIndex])
			{
				continue;
			}
			const EMaterialProperty prop = (EMaterialProperty)propertyIndex;
			BOOST_ASSERT(MaterialAttributeDefinationMap::getShaderFrequency(prop) == SF_Pixel);
			const wstring propertyName = MaterialAttributeDefinationMap::getDisplayName(prop);
			BOOST_ASSERT(propertyName.length() > 0);
			const EMaterialValueType type = MaterialAttributeDefinationMap::getValueType(prop);
			if (prop == MP_Normal)
			{
				normalInitializerValue = printf(TEXT("\tpixelMaterialInputs.%s = %s;\n"), propertyName.c_str(), mTranslatedCodeChunks[prop].c_str());

			}
			else
			{
				if (mTranslatedCodeChunkDefinitions[prop].length() > 0)
				{
					if (lastProperty >= 0)
					{
						BOOST_ASSERT(mTranslatedCodeChunkDefinitions[prop].length() == mTranslatedCodeChunkDefinitions[lastProperty].length());
					}
					lastProperty = prop;
				}
				pixelInputInitializerValues += printf(TEXT("\tpixelMaterialInputs.%s = %s;\n"), propertyName.c_str(), mTranslatedCodeChunks[prop].c_str());
			}
			pixelMembersDeclaration += printf(TEXT("\t%s %s;\n"), HLSLTypeString(type), propertyName.c_str());

		}
		normalAssignment = normalInitializerValue;
		if (lastProperty != -1)
		{
			pixelMembersInitializationEplilog += mTranslatedCodeChunkDefinitions[lastProperty] + TEXT("\n");
		}
		pixelMembersInitializationEplilog += pixelInputInitializerValues;
	}
}