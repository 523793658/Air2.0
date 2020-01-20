#include "Shader.h"
namespace Air
{
	struct ShaderParameterStructBindingContext
	{
		const Shader* mShader;

		ShaderParameterBindings* mBindings;

		const ShaderParameterMap* mParametersMap;

		TMap<wstring, wstring> mShaderGlobalScopeBindings;

		wstring mRenderTargetBindingSlotCppName;

		bool bUseRootShaderParameters;

		void bind(
			const ShaderParametersMetadata& structMetaData,
			const TCHAR* memberPrefix,
			uint32 generalByteOffset)
		{
			const TArray<ShaderParametersMetadata::Member>& structMembers = structMetaData.getMembers();

			for (const ShaderParametersMetadata::Member& member : structMembers)
			{
				EConstantBufferBaseType baseType = member.getBaseType();

				wstring cppName = String::printf(TEXT("%s::%s"), structMetaData.getStructTypeName(), member.getName());

				if (baseType == CBMT_RENDER_TARGET_BINDING_SLOTS)
				{
					if (!mRenderTargetBindingSlotCppName.empty())
					{
						AIR_LOG(LogShaders, Fatal, TEXT("Render target binding slots collision: %s & %s"), mRenderTargetBindingSlotCppName.c_str(), cppName.c_str());
					}
					mRenderTargetBindingSlotCppName = cppName;
					continue;
				}

				wstring shaderBindingName = String::printf(TEXT("%s%s"), memberPrefix, member.getName());

				uint16 byteOffset = uint16(generalByteOffset + member.getOffset());
				BOOST_ASSERT(uint32(byteOffset) == generalByteOffset + member.getOffset());

				const uint32 arraySize = member.getNumElements();
				const bool bIsArray = arraySize > 0;
				const bool bIsRHIResource = (
					baseType == CBMT_TEXTURE ||
					baseType == CBMT_SRV ||
					baseType == CBMT_UAV ||
					baseType == CBMT_SAMPLER
					);
				const bool bIsRDGResource = isRDGResourceReferenceShaderParameterType(baseType) && baseType != CBMT_RDG_BUFFER;
				const bool bIsVariableNativeType = (
					baseType == CBMT_BOOL ||
					baseType == CBMT_INT32 ||
					baseType == CBMT_UINT32 ||
					baseType == CBMT_FLOAT32
					);

				if (baseType == CBMT_INCLUDED_STRUCT)
				{
					BOOST_ASSERT(!bIsArray);
					bind(*member.getStructMetadata(),
						memberPrefix,
						byteOffset);
					continue;
				}
				else if (baseType == CBMT_NESTED_STRUCT && bIsArray)
				{
					const ShaderParametersMetadata* childStruct = member.getStructMetadata();
					uint32 structSize = childStruct->getSize();
					for (uint32 arrayElementId = 0; arrayElementId < (bIsArray ? arraySize : 1u); arrayElementId++)
					{
						wstring newPrefix = String::printf(TEXT("%s%s_%d_"), memberPrefix, member.getName(), arrayElementId);
						bind(*childStruct, newPrefix.c_str(), byteOffset + arrayElementId * structSize);
					}
					continue;
				}
				else if (baseType == CBMT_NESTED_STRUCT && !bIsArray)
				{
					wstring newPrefix = String::printf(TEXT("%s%s_"), memberPrefix, member.getName());
					bind(*member.getStructMetadata(), newPrefix.c_str(), byteOffset);
					continue;
				}
				else if (baseType == CBMT_REFERENCED_STRUCT)
				{
					BOOST_ASSERT(!bIsArray);
					shaderBindingName = member.getStructMetadata()->getShaderVariableName();
				}
				else if (baseType == CBMT_RDG_BUFFER)
				{
					BOOST_ASSERT(!bIsArray);
					if (mParametersMap->containsParameterAllocation(shaderBindingName.c_str()))
					{
						AIR_LOG(logShaders, Fatal, TEXT("%s can't bind shader parameter %s as buffer. Use buffer SRV for reading in shader."), cppName.c_str(), shaderBindingName.c_str());
					}
					continue;
				}
				else if (bUseRootShaderParameters && bIsVariableNativeType)
				{
					continue;
				}
				const bool bIsResourceArray = bIsArray && (bIsRHIResource || bIsRDGResource);
				for (uint32 arrayElementId = 0; arrayElementId < (bIsResourceArray ? arraySize : 1u); arrayElementId++)
				{
					wstring elementShaderBindingName;
					if (bIsResourceArray)
					{
						if (0)
						{
							elementShaderBindingName = String::printf(TEXT("%s_[%d]"), shaderBindingName.c_str(), arrayElementId);
						}
						else
						{
							elementShaderBindingName = String::printf(TEXT("%s_%d"), shaderBindingName.c_str(), arrayElementId);
						}
					}
					else
					{
						elementShaderBindingName = shaderBindingName;
					}
					if (mShaderGlobalScopeBindings.contains(elementShaderBindingName))
					{
						AIR_LOG(LogShaders, Fatal, TEXT("%s can't bind shader parameter %s, because it has already be bound by %s."), cppName.c_str(), elementShaderBindingName.c_str(), mShaderGlobalScopeBindings.find(shaderBindingName)->second.c_str());
					}

					uint16 bufferIndex, baseIndex, boundSize;
					if (!mParametersMap->findParameterAllocation(elementShaderBindingName.c_str(), bufferIndex, baseIndex, boundSize))
					{
						continue;;
					}
					mShaderGlobalScopeBindings.emplace(elementShaderBindingName, cppName);

					if (bIsVariableNativeType)
					{
						BOOST_ASSERT(arrayElementId == 0);
						uint32 byteSize = member.getMemberSize();

						ShaderParameterBindings::Parameter parameter;
						parameter.mBufferIndex = bufferIndex;
						parameter.mBaseIndex = baseIndex;
						parameter.mByteOffset = byteOffset;
						parameter.mByteSize = byteSize;

						if (uint32(boundSize) > byteSize)
						{
							AIR_LOG(LogShader, Fatal, TEXT("The size required to bind shader %s's (Permutation Id %d) struct %s parameter %s is %i bytes, smaller than %s's %i bytes."), mShader->getType()->getName(), mShader->getPermutationId(), structMetaData.getStructTypeName(), elementShaderBindingName.c_str(), boundSize, cppName.c_str(), byteSize);
						}
						mBindings->mParameters.add(parameter);
					}
					else if (baseType == CBMT_REFERENCED_STRUCT)
					{
						BOOST_ASSERT(!bIsArray);
						ShaderParameterBindings::ParameterStructReference parameter;
						parameter.mBufferIndex = bufferIndex;
						parameter.mByteOffset = byteOffset;

						mBindings->mParameterReferences.add(parameter);
					}
					else if (bIsRHIResource || bIsRDGResource)
					{
						ShaderParameterBindings::ResourceParameter parameter;
						parameter.mBaseIndex = baseIndex;
						parameter.mByteOffset = byteOffset + arrayElementId * SHADER_PARAMETER_POINTER_ALIGNMENT;
						BOOST_ASSERT(boundSize == 1);

						if (baseType == CBMT_TEXTURE)
						{
							mBindings->mTextures.add(parameter);
						}
						else if (baseType == CBMT_SRV)
						{
							mBindings->mSRVs.add(parameter);
						}
						else if (baseType == CBMT_UAV)
						{
							mBindings->mUAVs.add(parameter);
						}
						else if (baseType == CBMT_SAMPLER)
						{
							mBindings->mSamplers.add(parameter);
						}
						else if (baseType == CBMT_RDG_TEXTURE)
						{
							mBindings->mGraphTextures.add(parameter);
						}
						else if (baseType == CBMT_RDG_TEXTURE_SRV || baseType == CBMT_RDG_BUFFER_SRV)
						{
							mBindings->mGraphSRVs.add(parameter);
						}
						else
						{
							mBindings->mGraphUAVs.add(parameter);
						}
					}
					else
					{
						BOOST_ASSERT(false);
					}
				}
			}
		}
	};



	void ShaderParameterBindings::bindForLegacyShaderParameters(const Shader* shader, const ShaderParameterMap& parameterMaps, const ShaderParametersMetadata& structMetaData, bool bShouldBindEverything /* = false */)
	{
		BOOST_ASSERT(structMetaData.getSize() < (1 << (sizeof(uint16) * 8)));
		BOOST_ASSERT(this == &shader->mBindings);

		switch (shader->getType()->getFrequency())
		{
		case SF_Vertex:
		case SF_Hull:
		case SF_Domain:
		case SF_Pixel:
		case SF_Geometry:
		case SF_Compute:
			break;
		default:
			BOOST_ASSERT(false);
			break;
		}

		ShaderParameterStructBindingContext bindingContext;
		bindingContext.mShader = shader;
		bindingContext.mBindings = this;
		bindingContext.mParametersMap = &parameterMaps;
		bindingContext.bUseRootShaderParameters = false;
		bindingContext.bind(
			structMetaData, TEXT(""), 0);

		mRootParameterBufferIndex = kInvalidBufferIndex;

		TArray<wstring> allParameterNames;
		parameterMaps.getAllParameterNames(allParameterNames);
		if (bShouldBindEverything && bindingContext.mShaderGlobalScopeBindings.size() != allParameterNames.size())
		{
			wstring errorString = String::printf(
				TEXT("Shader %s has unbound parameters not represented in the parameter struct:"), shader->getType()->getName());

			for (const wstring& globalParameterName : allParameterNames)
			{
				if (!bindingContext.mShaderGlobalScopeBindings.contains(globalParameterName))
				{
					errorString += String::printf(TEXT("\n %s"), globalParameterName.c_str());
				}
			}
			AIR_LOG(LogShader, Fatal, TEXT("%s"), errorString.c_str());
		}
	}
}