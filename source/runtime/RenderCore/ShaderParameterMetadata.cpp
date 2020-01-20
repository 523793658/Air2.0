#include "ShaderParameterMetadata.h"
#include "ConstantBuffer.h"
namespace Air
{
	class ConstantBufferMemberAndOffset
	{
	public:
		ConstantBufferMemberAndOffset(const ShaderParametersMetadata& inContainingStruct, const ShaderParametersMetadata::Member& inMember, int32 inStructOffset)
			:mContainingStruct(inContainingStruct)
			,mMember(inMember)
			,mStructOffset(inStructOffset)
		{

		}
		const ShaderParametersMetadata& mContainingStruct;
		const ShaderParametersMetadata::Member& mMember;
		int32 mStructOffset;
	};


	ShaderParametersMetadata::ShaderParametersMetadata(EUseCase useCase, const wstring& inLayoutName, const TCHAR* inStructTypeName, const TCHAR* inShaderVariableName, uint32 inSize, const TArray<Member>& inMembers)
		:mStructTypeName(inStructTypeName)
		,mShaderVariableName(inShaderVariableName)
		,mSize(inSize)
		,mUseCase(useCase)
		,mLayout(inLayoutName)
		,mMembers(inMembers)
		,mGlobalListLink(this)
		,bLayoutInitialized(false)
	{
		BOOST_ASSERT(mStructTypeName);
		if (useCase == EUseCase::ShaderParameterStruct)
		{
			BOOST_ASSERT(mShaderVariableName == nullptr);
		}
		else
		{
			BOOST_ASSERT(mShaderVariableName);
		}

		if (mUseCase == EUseCase::GlobalShaderParameterStruct)
		{
			mGlobalListLink.linkHead(getStructList());
			wstring structTypeName(mStructTypeName);
			getNameStructMap().emplace(mStructTypeName, this);
		}
		else
		{
			initializeLayout();
		}
	}

	void ShaderParametersMetadata::initializeLayout()
	{
		BOOST_ASSERT(!bLayoutInitialized);
		mLayout.mConstantBufferSize = mSize;

		TArray<ConstantBufferMemberAndOffset> memberStack;
		memberStack.reserve(mMembers.size());

		for (int32 memberIndex = 0; memberIndex < mMembers.size(); memberIndex++)
		{
			memberStack.push(ConstantBufferMemberAndOffset(*this, mMembers[memberIndex], 0));
		}

		bool bAllowGraphResources = mUseCase == EUseCase::ShaderParameterStruct;

		bool bAllowCosntantBufferReferences = mUseCase == EUseCase::ShaderParameterStruct;

		bool bAllowResourceArrays = mUseCase == EUseCase::ShaderParameterStruct;

		bool bAllowStructureInlining = mUseCase == EUseCase::ShaderParameterStruct || mUseCase == EUseCase::GlobalShaderParameterStruct;

		for (int32 i = 0; i < memberStack.size(); ++i)
		{
			const ShaderParametersMetadata& currentSturct = memberStack[i].mContainingStruct;
			const Member& currentMember = memberStack[i].mMember;

			EConstantBufferBaseType baseType = currentMember.getBaseType();
			const uint32 arraySize = currentMember.getNumElements();
			const ShaderParametersMetadata* childStruct = currentMember.getStructMetadata();
			const bool bIsArray = arraySize > 0;
			const bool bIsRHIResource = (
				baseType == CBMT_TEXTURE ||
				baseType == CBMT_SRV ||
				baseType == CBMT_SAMPLER
				);
			const bool bIsRDGResource = isRDGResourceReferenceShaderParameterType(baseType);
			const bool bIsVariableNativeType = (
				baseType == CBMT_BOOL ||
				baseType == CBMT_INT32 ||
				baseType == CBMT_UINT32 ||
				baseType == CBMT_FLOAT32);

			if (isShaderParameterTypeForConstantBufferLayout(baseType))
			{
				for (uint32 arrayElementId = 0; arrayElementId < (bIsArray ? arraySize : 1u); arrayElementId++)
				{
					const uint32 absoluteMemberOffset = currentMember.getOffset() + memberStack[i].mStructOffset + arrayElementId * SHADER_PARAMETER_POINTER_ALIGNMENT;
					BOOST_ASSERT(absoluteMemberOffset < (1u << (sizeof(RHIConstantBufferLayout::ResourceParameter::mMemberOffset) * 8)));
					mLayout.mResources.add(RHIConstantBufferLayout::ResourceParameter{ uint16(absoluteMemberOffset), baseType });
				}
			}

			if (childStruct && baseType != CBMT_REFERENCED_STRUCT)
			{
				for (uint32 arrayElementId = 0; arrayElementId < (bIsArray ? arraySize : 1u); arrayElementId++)
				{
					int32 absoluteStructOffset = currentMember.getOffset() + memberStack[i].mStructOffset + arrayElementId * childStruct->getSize();
					for (int32 structMemberIndex = 0; structMemberIndex < childStruct->mMembers.size(); structMemberIndex++)
					{
						const Member& structMember = childStruct->mMembers[structMemberIndex];
						memberStack.insert(ConstantBufferMemberAndOffset(*childStruct, structMember, absoluteStructOffset), i + 1 + structMemberIndex);
					}
				}
			}
		}

		mLayout.mResources.sort([](
			const RHIConstantBufferLayout::ResourceParameter& a,
			const RHIConstantBufferLayout::ResourceParameter& b)
			{
				return a.mMemberOffset < b.mMemberOffset;
			});
		mLayout.computeHash();
		bLayoutInitialized = true;
	}

	TMap<wstring, ShaderParametersMetadata*>& ShaderParametersMetadata::getNameStructMap()
	{
		static TMap<wstring, ShaderParametersMetadata*> GlobalNameStructMap;
		return GlobalNameStructMap;
	}

	void ShaderParametersMetadata::getNestedStructs(TArray<const ShaderParametersMetadata*>& outNestedStructs) const
	{
		for (int32 i = 0; i < mMembers.size(); i++)
		{
			const Member& currentMember = mMembers[i];
			const ShaderParametersMetadata* memberStruct = currentMember.getStructMetadata();
			if (memberStruct)
			{
				outNestedStructs.add(memberStruct);
				memberStruct->getNestedStructs(outNestedStructs);
			}
		}
	}

	void ShaderParametersMetadata::initializeAllGlobalStructs()
	{
		for (TLinkedList<ShaderParametersMetadata*>::TIterator structIt(ShaderParametersMetadata::getStructList()); structIt; structIt.next())
		{
			structIt->initializeLayout();
		}
	}

	static TLinkedList<ShaderParametersMetadata*>* GConstantStructList = nullptr;

	TLinkedList<ShaderParametersMetadata*>*& ShaderParametersMetadata::getStructList()
	{
		return GConstantStructList;
	}

	void ShaderParametersMetadata::addResourceTableEntriesRecursive(const TCHAR* constantBufferName, const TCHAR* prefix, uint16& resourceIndex, TMap<wstring, ResourceTableEntry>& resourceTableMap) const
	{
		for (int32 memberIndex = 0; memberIndex < mMembers.size(); ++memberIndex)
		{
			const Member& member = mMembers[memberIndex];
			uint32 numElements = member.getNumElements();

			if (isShaderParameterTypeForConstantBufferLayout(member.getBaseType()))
			{
				ResourceTableEntry& entry = resourceTableMap.findOrAdd(String::printf(TEXT("%s%s"), prefix, member.getName()));
				if (entry.mConstantBufferName.empty())
				{
					entry.mConstantBufferName = constantBufferName;
					entry.mType = member.getBaseType();
					entry.mResourceIndex = resourceIndex;
				}
			}
			else if (member.getBaseType() == CBMT_NESTED_STRUCT && numElements == 0)
			{
				BOOST_ASSERT(member.getStructMetadata());
				wstring memberPrefix = String::printf(TEXT("%s%s_"), prefix, member.getName());
				member.getStructMetadata()->addResourceTableEntriesRecursive(constantBufferName, memberPrefix.c_str(), resourceIndex, resourceTableMap);
			}
			else if (member.getBaseType() == CBMT_NESTED_STRUCT && numElements > 0)
			{
				for (uint32 arrayElementId = 0; arrayElementId < numElements; arrayElementId++)
				{
					BOOST_ASSERT(member.getStructMetadata());
					wstring memberPrefix = String::printf(TEXT("%s%s_%u_"), prefix, member.getName(), arrayElementId);
					member.getStructMetadata()->addResourceTableEntriesRecursive(constantBufferName, memberPrefix.c_str(), resourceIndex, resourceTableMap);
				}
			}
			else if (member.getBaseType() == CBMT_INCLUDED_STRUCT)
			{
				BOOST_ASSERT(member.getStructMetadata());
				BOOST_ASSERT(numElements == 0);
				member.getStructMetadata()->addResourceTableEntriesRecursive(constantBufferName, prefix, resourceIndex, resourceTableMap);
			}
		}
	}

	void ShaderParametersMetadata::addResourceTableEntries(TMap<wstring, ResourceTableEntry>& resourceTableMap, TMap<wstring, uint32>& resourceTableLayoutHashes) const
	{
		uint16 resourceIndex = 0;
		wstring prefix = String::printf(TEXT("%s_"), mShaderVariableName);
		addResourceTableEntriesRecursive(mShaderVariableName, prefix.c_str(), resourceIndex, resourceTableMap);
		resourceTableLayoutHashes.emplace(mShaderVariableName, getLayout().getHash());
	}

	ShaderParametersMetadata* findConstantBufferStructByName(const TCHAR* structName)
	{
		wstring findByName(structName);
		ShaderParametersMetadata* foundStruct = ShaderParametersMetadata::getNameStructMap().findRef(findByName);
		return foundStruct;
	}

	ShaderParametersMetadata* findConstantBufferStructByName(wstring structName)
	{
		return ShaderParametersMetadata::getNameStructMap().findRef(structName);
	}
}