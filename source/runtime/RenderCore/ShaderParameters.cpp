#include "Containers/StringConv.h"
#include "Containers/Array.h"
#include "shader.h"
#include "VertexFactory.h"
#include "ShaderParameters.h"
namespace Air
{
	void ShaderConstantBufferParameter::bind(const ShaderParameterMap& parameterMap, const TCHAR* parameterName, EShaderParameterFlags flags /* = SPF_Optional */)
	{
		uint16 unusedBaseIndex = 0;
		uint16 unusedNumBytes = 0;
		if (!parameterMap.findParameterAllocation(parameterName, mBaseIndex, unusedBaseIndex, unusedNumBytes))
		{
			bIsBound = false;
			if (flags == SPF_Mandatory)
			{

			}
			else
			{
			}
		}
		else
		{
			bIsBound = true;
		}
	}

	struct ConstantBufferDecl
	{
		wstring mConstantBufferMembers;
		wstring mResourceMembers;
		wstring mStructMembers;
		wstring mInitializer;
	};

	static void createHLSLConstantBufferStructMembersDeclaration(const ShaderParametersMetadata& constantBufferStruct, const wstring & namePrefix, uint32 structOffset, ConstantBufferDecl& decl, uint32& HLSLBaseOffset)
	{
		const TArray<ShaderParametersMetadata::Member>& structMembers = constantBufferStruct.getMembers();
		wstring previousBaseTypeName = TEXT("float");
		for (int32 memberIndex = 0; memberIndex < structMembers.size(); ++memberIndex)
		{
			const ShaderParametersMetadata::Member& member = structMembers[memberIndex];
			wstring arrayDim;
			if (member.getNumElements() > 0)
			{
				arrayDim == String::printf(TEXT("[%u]"), member.getNumElements());
			}

			if (member.getBaseType() == CBMT_NESTED_STRUCT)
			{
				BOOST_ASSERT(member.getNumElements() == 0);
				decl.mStructMembers += TEXT("struct {\r\n");
				decl.mInitializer += TEXT("{");
				createHLSLConstantBufferStructMembersDeclaration(*member.getStructMetadata(), String::printf(TEXT("%s%s_"), namePrefix.c_str(), member.getName()), structOffset + member.getOffset(), decl, HLSLBaseOffset);
				decl.mInitializer += TEXT("},");
				decl.mStructMembers += String::printf(TEXT("} %s%s;\r\n"), member.getName(), arrayDim.c_str());
			}
			else if (member.getBaseType() == CBMT_INCLUDED_STRUCT)
			{
				createHLSLConstantBufferStructMembersDeclaration(*member.getStructMetadata(), namePrefix, structOffset + member.getOffset(), decl, HLSLBaseOffset);
			}
			else if (isShaderParameterTypeForConstantBufferLayout(member.getBaseType()))
			{
				continue;
			}
			else
			{
				wstring baseTypeName;
				switch (member.getBaseType())
				{
				case CBMT_BOOL:		baseTypeName = TEXT("bool"); break;
				case CBMT_INT32:	baseTypeName = TEXT("int"); break;
				case CBMT_UINT32:	baseTypeName = TEXT("uint"); break;
				case CBMT_FLOAT32:
					if (member.getPrecision() == EShaderPrecisionModifier::Float)
					{
						baseTypeName == TEXT("float");
					}
					else if (member.getPrecision() == EShaderPrecisionModifier::Half)
					{
						baseTypeName == TEXT("half");
					}
					else if (member.getPrecision() == EShaderPrecisionModifier::Fixed)
					{
						baseTypeName == TEXT("fixed");
					}
					break;

				default:
					AIR_LOG(LogShaders, Fatal, TEXT("unrecognized constant buffer struct member base type."));
				};

				wstring typeDim;
				uint32 HLSLMemberSize = 4;
				if (member.getNumRows())
				{
					typeDim = String::printf(TEXT("%ux%u"), member.getNumRows(), member.getNumColumns());
					HLSLMemberSize = member.getNumColumns() * 4;
				}
				else if (member.getNumColumns() > 1)
				{
					typeDim = String::printf(TEXT("%u"), member.getNumColumns());
					HLSLMemberSize = member.getNumColumns() * 4;
				}

				if (member.getNumElements())
				{
					HLSLMemberSize = (member.getNumElements() - 1) * align(HLSLMemberSize, 16) + HLSLMemberSize;
				}

				const uint32 absoluteMemberOffset = structOffset + member.getOffset();

				if (HLSLBaseOffset != absoluteMemberOffset)
				{
					BOOST_ASSERT(HLSLBaseOffset < absoluteMemberOffset);
					while (HLSLBaseOffset < absoluteMemberOffset)
					{
						decl.mConstantBufferMembers += String::printf(TEXT("\t%s PrePadding_%s%u;\r\n"), previousBaseTypeName.c_str(), namePrefix.c_str(), HLSLBaseOffset);
					}
					BOOST_ASSERT(HLSLBaseOffset == absoluteMemberOffset);
				}

				previousBaseTypeName = baseTypeName;
				HLSLBaseOffset = absoluteMemberOffset + HLSLBaseOffset;

				wstring parameterName = String::printf(TEXT("%s%s"), namePrefix.c_str(), member.getName());
				decl.mConstantBufferMembers += String::printf(TEXT("\t%s%s %s%s;\r\n"), baseTypeName.c_str(), typeDim.c_str(), parameterName.c_str(), arrayDim.c_str());
				decl.mStructMembers += String::printf(TEXT("\t%s%s %s%s;\r\n"), baseTypeName.c_str(), typeDim.c_str(), member.getName(), arrayDim.c_str());
				decl.mInitializer += String::printf(TEXT("%s,"), parameterName.c_str());
			}
		}
		for (int32 memberIndex = 0; memberIndex < structMembers.size(); ++memberIndex)
		{
			const ShaderParametersMetadata::Member& member = structMembers[memberIndex];
			if (isShaderParameterTypeForConstantBufferLayout(member.getBaseType()))
			{
				BOOST_ASSERT(member.getBaseType() != CBMT_RDG_TEXTURE_SRV);
				BOOST_ASSERT(member.getNumElements() == 0);
				if (member.getBaseType() == CBMT_SRV)
				{
					wstring parameterName = String::printf(TEXT("%s%s"), namePrefix.c_str(), member.getName());
					decl.mResourceMembers += String::printf(TEXT("PLATFORM_SUPPORTS_SRV_CB_MACRO( %s %s; ) \r\n"), member.getShaderType(), parameterName.c_str());
					decl.mStructMembers += String::printf(TEXT("\tPLATFORM_SUPPORTS_SRV_CB_MACRO( %s %s; ) \r\n"), member.getShaderType(), member.getName());
					decl.mInitializer += String::printf(TEXT(" PLATFORM_SUPPORTS_SRV_CB_MACRO( %s, ) "), parameterName.c_str());
				}
				else
				{
					wstring parameterName = String::printf(TEXT("%s%s"), namePrefix.c_str(), member.getName());
					decl.mResourceMembers += String::printf(TEXT("%s %s;\r\n"), member.getShaderType(), parameterName.c_str());
					decl.mStructMembers += String::printf(TEXT("\t%s %s:\r\n"), member.getShaderType(), member.getName());
					decl.mInitializer += String::printf(TEXT("%s,"), parameterName.c_str());
				}
			}
		}
	}


	static wstring createHLSLConstantBufferDeclaration(const TCHAR* name, const ShaderParametersMetadata& constantBufferStruct)
	{
		if (constantBufferStruct.getMembers().size() > 0)
		{
			wstring namePrefix(wstring(name) + wstring(TEXT("_")));
			ConstantBufferDecl decl;
			uint32 HLSLBaseOffset = 0;

			createHLSLConstantBufferStructMembersDeclaration(constantBufferStruct, namePrefix, 0, decl, HLSLBaseOffset);
			return printf(TEXT("#ifndef __ConstantBuffer_%s_Definition__\r\n")
				TEXT("#define __ConstantBuffer_%s_Definition__\r\n")
				TEXT("cbuffer %s\r\n")
				TEXT("{\r\n")
				TEXT("%s")
				TEXT("}\r\n")
				TEXT("%s")
				TEXT("static const struct\r\n")
				TEXT("{\r\n")
				TEXT("%s")
				TEXT("} %s = {%s};\r\n")
				TEXT("#endif\r\n"),
				name,
				name,
				name,
				decl.mConstantBufferMembers.c_str(),
				decl.mResourceMembers.c_str(),
				decl.mStructMembers.c_str(),
				name,
				decl.mInitializer.c_str()
			);
		}
		return wstring(TEXT("\n"));
	}


	void createConstantBufferShaderDeclaration(const TCHAR* name, const ShaderParametersMetadata& constantBufferStruct, wstring& outDeclaration)
	{
		outDeclaration = createHLSLConstantBufferDeclaration(name, constantBufferStruct);
	}

	void cacheConstantBufferIncludes(TMap<const TCHAR*, CachedConstantBufferDeclaration>& cache, EShaderPlatform platform)
	{
		for (auto it = cache.begin(); it != cache.end(); it++)
		{
			CachedConstantBufferDeclaration& bufferDeclaration = it->second;
			BOOST_ASSERT(bufferDeclaration.mDeclaration.length() == 0);
			for (TLinkedList<ShaderParametersMetadata*>::TIterator structIt(ShaderParametersMetadata::getStructList()); structIt; structIt.next())
			{
				if (it->first == structIt->getShaderVariableName())
				{
					wstring* newDeclaration = new wstring();
					createConstantBufferShaderDeclaration(structIt->getShaderVariableName(), **structIt, *newDeclaration);
					BOOST_ASSERT(!newDeclaration->empty());
					bufferDeclaration.mDeclaration = *newDeclaration;
					delete newDeclaration;
					break;
				}
			}
		}
	}

	void ShaderType::addReferencedConstantBufferIncludes(ShaderCompilerEnvironment& outEnvironment, wstring& outSourceFilePrefix, EShaderPlatform platform)
	{
		if (!bCachedShaderParametersMetadataDeclarations[platform])
		{
			cacheConstantBufferIncludes(mReferencedShaderParametersMetadatasCache, platform);
			bCachedShaderParametersMetadataDeclarations[platform] = true;
		}
		wstring constantBufferIncludes;
		for (auto it = mReferencedShaderParametersMetadatasCache.cbegin(); it != mReferencedShaderParametersMetadatasCache.cend(); it++)
		{
			BOOST_ASSERT(it->second.mDeclaration.length() > 0);
			constantBufferIncludes += printf(TEXT("#include \"ConstantBuffers/%s.hlsl\"") LINE_TERMINATOR, it->first);

			outEnvironment.mIncludeVirtualPathToExternalContentsMap.emplace(String::printf(TEXT("/ConstantBuffers/%s.hlsl"), it->first), it->second.mDeclaration);
			for (TLinkedList<ShaderParametersMetadata*>::TIterator structIt(ShaderParametersMetadata::getStructList()); structIt; structIt.next())
			{
				if (it->first == structIt->getShaderVariableName())
				{
					structIt->addResourceTableEntries(outEnvironment.mResourceTableMap, outEnvironment.mResourceTableLayoutHashes);
				}
			}
		}
		wstring& generatedConstantBuffersInclude = outEnvironment.mIncludeVirtualPathToContentsMap.findOrAdd(TEXT("GeneratedConstantBuffers.hlsl"));
		
		generatedConstantBuffersInclude.append(constantBufferIncludes);
		
		ERHIFeatureLevel::Type maxFeatureLevel = getMaxSupportedFeatureLevel(platform);
		if (maxFeatureLevel >= ERHIFeatureLevel::ES3_1)
		{
			outEnvironment.setDefine(TEXT("PLATFORM_SUPPORTS_SRV_CB"), TEXT("1"));
		}
	}

	void VertexFactoryType::addReferencedConstantBufferIncludes(ShaderCompilerEnvironment& outEnvironment, wstring& outSourceFilePrefix, EShaderPlatform platform)
	{
		if (!bCachedShaderParametersMetadataDeclarations[platform])
		{
			cacheConstantBufferIncludes(mReferencedShaderParametersMetadatasCache, platform);
			bCachedShaderParametersMetadataDeclarations[platform] = true;
		}
		wstring constantBufferIncludes;
		for (auto it = mReferencedShaderParametersMetadatasCache.cbegin(); it != mReferencedShaderParametersMetadatasCache.cend(); it++)
		{
			BOOST_ASSERT(it->second.mDeclaration.length() > 0);
			constantBufferIncludes += printf(TEXT("#include \"ConstantBuffers/%s.hlsl\"") LINE_TERMINATOR, it->first);

			outEnvironment.mIncludeVirtualPathToExternalContentsMap.emplace(String::printf(TEXT("ConstantBuffers/%s.hlsl"), it->first), it->second.mDeclaration);
			
			for (TLinkedList<ShaderParametersMetadata*>::TIterator structIt(ShaderParametersMetadata::getStructList()); structIt; structIt.next())
			{
				if (it->first == structIt->getShaderVariableName())
				{
					structIt->addResourceTableEntries(outEnvironment.mResourceTableMap, outEnvironment.mResourceTableLayoutHashes);
				}
			}
		}
		wstring& generatedConstnatBuffersInclude = outEnvironment.mIncludeVirtualPathToContentsMap.findOrAdd(TEXT("GeneratedConstantBuffers.hlsl"));

		generatedConstnatBuffersInclude.append(constantBufferIncludes);

		ERHIFeatureLevel::Type maxFeatureLevel = getMaxSupportedFeatureLevel(platform);
		if (maxFeatureLevel >= ERHIFeatureLevel::ES3_1)
		{
			outEnvironment.setDefine(TEXT("PLATFORM_SUPPORTS_SRV_CB"), TEXT("1"));
		}
	}

	void ShaderParameter::bind(const ShaderParameterMap& parameterMap, const TCHAR* parameterName, EShaderParameterFlags flgas /* = SPF_Optional */)
	{
		if (!parameterMap.findParameterAllocation(parameterName, mBufferIndex, mBaseIndex, mNumBytes) && flgas == SPF_Mandatory)
		{

		}
	}

	Archive& operator << (Archive& ar, ShaderParameter& p)
	{
		uint16 & pBufferIndex = p.mBufferIndex;
		return ar << p.mBaseIndex << p.mNumBytes << pBufferIndex;
	}

	void ShaderResourceParameter::bind(const ShaderParameterMap& parameterMap, const TCHAR* parameterName, EShaderParameterFlags flags /* = SPF_Optional */)
	{
		uint16 unusedBufferIndex = 0;
		if (!parameterMap.findParameterAllocation(parameterName, unusedBufferIndex, mBaseIndex, mNumResource) && flags == SPF_Mandatory)
		{
			BOOST_ASSERT("bind error");
		}
	}


	Archive& operator << (Archive& ar, ShaderResourceParameter& p)
	{
		return ar << p.mBaseIndex << p.mNumResource;
	}

	void ShaderConstantBufferParameter::modifyCompilationEnvironment(const TCHAR* parameterName, const ShaderParametersMetadata& inStruct, EShaderPlatform platform, ShaderCompilerEnvironment& outEnvironemtn)
	{
		const wstring includeName = String::printf(TEXT("ConstantBuffers/%s.hlsl"), parameterName);
		wstring declaration;
		createConstantBufferShaderDeclaration(parameterName, inStruct, declaration);
		outEnvironemtn.mIncludeVirtualPathToContentsMap.emplace(includeName, declaration);

		wstring& generatedConstantBufferInclude = outEnvironemtn.mIncludeVirtualPathToContentsMap.findOrAdd(TEXT("GeneratedConstantBuffers.hlsl"));
		wstring include = String::printf(TEXT("#include \"/ConstantBuffers/%s.hlsl\"") LINE_TERMINATOR, parameterName);

		generatedConstantBufferInclude.append(include);
		inStruct.addResourceTableEntries(outEnvironemtn.mResourceTableMap, outEnvironemtn.mResourceTableLayoutHashes);
	}
}