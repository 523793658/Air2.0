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

	static void createHLSLConstantBufferStructMembersDeclaration(ConstantBufferDecl& decl, const ConstantBufferStruct& constantBufferStruct, const wstring & namePrefix, bool bExplicitPadding)
	{
		uint32 HLSLBaseOffset = 0;
		bool bFoundResourceMember = false;
		int32 memberIndex = 0;
		const TArray<ConstantBufferStruct::Member>& structMembers = constantBufferStruct.getMembers();
		decl.mInitializer += TEXT("{");
		int32 openingBraceLocPlusOne = decl.mInitializer.length();
		for (; memberIndex < structMembers.size(); ++memberIndex)
		{
			const ConstantBufferStruct::Member& member = structMembers[memberIndex];
			if (isConstantBufferResourceType(member.getBaseType()))
			{
				bFoundResourceMember = true;
				break;
			}
			wstring arrayDim;
			if (member.getNumElements() > 0)
			{
				arrayDim = printf(TEXT("[%u]"), member.getNumElements());
			}
			if (member.getBaseType() == CBMT_STRUCT)
			{
				decl.mStructMembers += TEXT("struct {\r\n");
				decl.mInitializer += TEXT(",");
				createHLSLConstantBufferStructMembersDeclaration(decl, *member.getStruct(), printf(TEXT("%s%s_"), namePrefix.c_str(), member.getName()), bExplicitPadding);
				decl.mStructMembers += printf(TEXT("} %s%s;\r\n"), member.getName(), arrayDim.c_str());
				HLSLBaseOffset += member.getStruct()->getSize() * member.getNumElements();
			}
			else
			{
				wstring baseTypeName;
				switch (member.getBaseType())
				{
				case CBMT_BOOL:
					baseTypeName = TEXT("bool"); break;
				case CBMT_INT32:
					baseTypeName = TEXT("int"); break;
				case CBMT_UINT32:
					baseTypeName = TEXT("uint"); break;
				case CBMT_FLOAT32:
					if (member.getPrecision() == EShaderPrecisionModifier::Float)
					{
						baseTypeName = TEXT("float");
					}
					else if (member.getPrecision() == EShaderPrecisionModifier::Fixed)
					{
						baseTypeName = TEXT("fixed");
					}
					else if (member.getPrecision() == EShaderPrecisionModifier::Half)
					{
						baseTypeName = TEXT("half");
					}
					break;
					default:
					break;
				};
				wstring typeDim;
				uint32 HLSLMemberSize = 4;
				if (member.getNumRows() > 1)
				{
					typeDim = printf(TEXT("%ux%u"), member.getNumRows(), member.getNumColumns());
					HLSLMemberSize = (member.getNumRows() - 1) * 16 + member.getNumColumns() * 4;
				}
				else if (member.getNumColumns() > 1)
				{
					typeDim = printf(TEXT("%u"), member.getNumColumns());
					HLSLMemberSize = member.getNumColumns() * 4;
				}
				if (member.getNumElements() > 0)
				{
					HLSLMemberSize = (member.getNumElements() - 1) * align(HLSLMemberSize, 16) + HLSLMemberSize;
				}
				if (HLSLBaseOffset != member.getOffset())
				{
					BOOST_ASSERT(HLSLBaseOffset < member.getOffset());
					while (HLSLBaseOffset < member.getOffset())
					{
						if (bExplicitPadding)
						{
							decl.mConstantBufferMembers += printf(TEXT("\t float1 _%sPrePadding%u; \r\n"), namePrefix.c_str(), HLSLBaseOffset);
						}
						HLSLBaseOffset += 4;
					}
					BOOST_ASSERT(HLSLBaseOffset == member.getOffset());
				}

				HLSLBaseOffset = member.getOffset() + HLSLMemberSize;
				wstring paramterName = printf(TEXT("%s%s"), namePrefix.c_str(), member.getName());
				decl.mConstantBufferMembers += printf(TEXT("\t%s%s %s%s;\r\n"), baseTypeName.c_str(), typeDim.c_str(), paramterName.c_str(), arrayDim.c_str());
				decl.mStructMembers += printf(TEXT("\t%s%s %s%s;\r\n"), baseTypeName.c_str(), typeDim.c_str(), member.getName(), arrayDim.c_str());
				decl.mInitializer += printf(TEXT(",%s"), paramterName.c_str());

			}
		}
		for (; memberIndex < structMembers.size(); ++memberIndex)
		{
			const ConstantBufferStruct::Member& member = structMembers[memberIndex];
			if (!isConstantBufferResourceType(member.getBaseType()))
			{
				BOOST_ASSERT(!bFoundResourceMember);
				continue;
			}
			bFoundResourceMember = true;
			wstring parameterName = printf(TEXT("%s%s"), namePrefix.c_str(), member.getName());
			decl.mResourceMembers += printf(TEXT("%s %s;\r\n"), member.getShaderType(), parameterName.c_str());
			decl.mStructMembers += printf(TEXT("\t%s %s;\r\n"), member.getShaderType(), member.getName());
			decl.mInitializer += printf(TEXT(",%s"), parameterName.c_str());
		}
		decl.mInitializer += TEXT("}");
		if (decl.mInitializer[openingBraceLocPlusOne] == TEXT(','))
		{
			decl.mInitializer[openingBraceLocPlusOne] = TEXT(' ');
		}
	}


	static wstring createHLSLConstantBufferDeclaration(const TCHAR* name, const ConstantBufferStruct& constantBufferStruct, bool BExplicitPadding)
	{
		if (constantBufferStruct.getMembers().size() > 0)
		{
			wstring namePrefix(wstring(name) + wstring(TEXT("_")));
			ConstantBufferDecl decl;
			createHLSLConstantBufferStructMembersDeclaration(decl, constantBufferStruct, namePrefix, BExplicitPadding);
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
				TEXT("} %s = %s;\r\n")
				TEXT("#endif\r\n"),
				name,
				name,
				name,
				decl.mConstantBufferMembers.c_str(),
				decl.mResourceMembers.c_str(),
				decl.mStructMembers.c_str(),
				name,
				decl.mInitializer.c_str(),
				name
			);
		}
		return wstring(TEXT("\n"));
	}


	wstring createConstantBufferShaderDeclaration(const TCHAR* name, const ConstantBufferStruct& constantBufferStruct, EShaderPlatform platform)
	{
		switch (platform)
		{
		case SP_OPENGL_ES3_1_ANDROID:
		case SP_OPENGL_ES31_EXT:
		case SP_OPENGL_SM4:
		case SP_OPENGL_SM4_MAC:
		case SP_OPENGL_SM5:
		case SP_SWITCH:
			return createHLSLConstantBufferDeclaration(name, constantBufferStruct, false);
		case SP_PCD3D_SM5:
		default:
			return createHLSLConstantBufferDeclaration(name, constantBufferStruct, true);
		}
	}

	void cacheConstantBufferIncludes(TMap<const TCHAR*, CachedConstantBufferDeclaration>& cache, EShaderPlatform platform)
	{
		for (auto it = cache.begin(); it != cache.end(); it++)
		{
			CachedConstantBufferDeclaration& bufferDeclaration = it->second;
			BOOST_ASSERT(bufferDeclaration.mDeclaration[platform].length() == 0);
			for (TLinkedList<ConstantBufferStruct*>::TIterator structIt(ConstantBufferStruct::getStructList()); structIt; structIt.next())
			{
				if (it->first == structIt->getShaderVariableName())
				{
					bufferDeclaration.mDeclaration[platform] = createConstantBufferShaderDeclaration(structIt->getShaderVariableName(), **structIt, platform);
					break;
				}
			}
		}
	}

	void ShaderType::addReferencedConstantBufferIncludes(ShaderCompilerEnvironment& outEnvironment, wstring& outSourceFilePrefix, EShaderPlatform platform)
	{
		if (!bCachedConstantBufferStructDeclarations[platform])
		{
			cacheConstantBufferIncludes(mReferencedConstantBufferStructsCache, platform);
			bCachedConstantBufferStructDeclarations[platform] = true;
		}
		wstring constantBufferIncludes;
		for (auto it = mReferencedConstantBufferStructsCache.cbegin(); it != mReferencedConstantBufferStructsCache.cend(); it++)
		{
			BOOST_ASSERT(it->second.mDeclaration[platform].length() > 0);
			constantBufferIncludes += printf(TEXT("#include \"ConstantBuffers/%s.hlsl\"") LINE_TERMINATOR, it->first);
			wstring declaration = it->second.mDeclaration[platform];
			outEnvironment.mIncludeFileNameToContentsMap.emplace(printf(TEXT("ConstantBuffers/%s.hlsl"), it->first), stringToArray<ANSICHAR>(declaration.c_str(), declaration.length() + 1));
			for (TLinkedList<ConstantBufferStruct*>::TIterator structIt(ConstantBufferStruct::getStructList()); structIt; structIt.next())
			{
				if (it->first == structIt->getShaderVariableName())
				{
					structIt->addResourceTableEntries(outEnvironment.mResourceTableMap, outEnvironment.mResourceTableLayoutHashes);
				}
			}
		}
		TArray<ANSICHAR>& generatedConstantBuffersInclude = outEnvironment.mIncludeFileNameToContentsMap.findOrAdd(TEXT("GeneratedConstantBuffers.hlsl"));
		if (generatedConstantBuffersInclude.size() > 0)
		{
			generatedConstantBuffersInclude.removeAt(generatedConstantBuffersInclude.size() - 1);
		}
		TArray<ANSICHAR>&& a = stringToArray<ANSICHAR>(constantBufferIncludes.c_str(), constantBufferIncludes.length() + 1);
		generatedConstantBuffersInclude.append(a.data(), a.size());
	}

	void VertexFactoryType::addReferencedConstantBufferIncludes(ShaderCompilerEnvironment& outEnvironment, wstring& outSourceFilePrefix, EShaderPlatform platform)
	{
		if (!bCachedConstantBufferStructDeclarations[platform])
		{
			cacheConstantBufferIncludes(mReferencedConstantBufferStructsCache, platform);
			bCachedConstantBufferStructDeclarations[platform] = true;
		}
		wstring constantBufferIncludes;
		for (auto it = mReferencedConstantBufferStructsCache.cbegin(); it != mReferencedConstantBufferStructsCache.cend(); it++)
		{
			BOOST_ASSERT(it->second.mDeclaration[platform].length() > 0);
			constantBufferIncludes += printf(TEXT("#include \"ConstantBuffers/%s.hlsl\"") LINE_TERMINATOR, it->first);
			wstring declaration = it->second.mDeclaration[platform];
			outEnvironment.mIncludeFileNameToContentsMap.emplace(printf(TEXT("ConstantBuffers/%s.hlsl"), it->first), stringToArray<ANSICHAR>(declaration.c_str(), declaration.length() + 1));
			for (TLinkedList<ConstantBufferStruct*>::TIterator structIt(ConstantBufferStruct::getStructList()); structIt; structIt.next())
			{
				if (it->first == structIt->getShaderVariableName())
				{
					structIt->addResourceTableEntries(outEnvironment.mResourceTableMap, outEnvironment.mResourceTableLayoutHashes);
				}
			}
		}
		TArray<ANSICHAR>& generatedConstnatBuffersInclude = outEnvironment.mIncludeFileNameToContentsMap.findOrAdd(TEXT("GeneratedConstantBuffers.hlsl"));
		if (generatedConstnatBuffersInclude.size() > 0)
		{
			generatedConstnatBuffersInclude.removeAt(generatedConstnatBuffersInclude.size() - 1);
		}

		TArray<ANSICHAR>&& aa = stringToArray<ANSICHAR>(constantBufferIncludes.c_str(), constantBufferIncludes.length() + 1);
		generatedConstnatBuffersInclude.append(aa.data(), aa
		.size());
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

	void ShaderConstantBufferParameter::modifyCompilationEnvironment(const TCHAR* parameterName, const ConstantBufferStruct& inStruct, EShaderPlatform platform, ShaderCompilerEnvironment& outEnvironemtn)
	{
		const wstring includeName = printf(TEXT("ConstantBuffers/%s.hlsl"), parameterName);
		wstring declaration = createConstantBufferShaderDeclaration(parameterName, inStruct, platform);
		outEnvironemtn.mIncludeFileNameToContentsMap.emplace(includeName, stringToArray<ANSICHAR>(declaration.c_str(), declaration.length() + 1));
		TArray<ANSICHAR>& generatedConstantBuffersInclude = outEnvironemtn.mIncludeFileNameToContentsMap.findOrAdd(TEXT("GeneratedConstantBuffers.hlsl"));
		wstring include = printf(TEXT("#include \"ConstantBuffers/%s.hlsl\"")LINE_TERMINATOR, parameterName);
		if (generatedConstantBuffersInclude.size() > 0)
		{
			generatedConstantBuffersInclude.removeAt(generatedConstantBuffersInclude.size() - 1);

		}
		generatedConstantBuffersInclude.append(stringToArray<ANSICHAR>(include.c_str(), include.length() + 1));
		inStruct.addResourceTableEntries(outEnvironemtn.mResourceTableMap, outEnvironemtn.mResourceTableLayoutHashes);
	}
}