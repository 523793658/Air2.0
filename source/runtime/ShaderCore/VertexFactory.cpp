#include "Serialization/MemoryWriter.h"
#include "HAL/PlatformProperties.h"
#include "VertexFactory.h"
namespace Air
{
	bool VertexFactoryType::bInitializedSerializationHistory = false;

	uint32 VertexFactoryType::mNextHashIndex = 0;

	VertexFactoryType::VertexFactoryType(const TCHAR* inName, const TCHAR* inShaderFilename, bool bInUseWithMaterials, bool bInSupportsStaticLighting, bool bInSupportsDynamicLightings, bool bInSupportsPrecisePrevWorldPos, bool bInSupportsPositionOnly, ConstructParametersType inConstructParameters, ShouldCacheType inShouldCache, ModifyCompilationEnvironmentType inModifyCompilationEnvironment, supportsTessellationShaderType inSupportsTessellationShaders)
		:mName(inName),
		mShaderFilename(inShaderFilename),
		mTypeName(inName),
		bUseWithMaterials(bInUseWithMaterials),
		bSupportsStaticLighting(bInSupportsStaticLighting),
		bSupportsDynamicLighting(bInSupportsDynamicLightings),
		bSupportsPrecisePrevWorldPos(bInSupportsPrecisePrevWorldPos),
		bSupportsPositionOnly(bInSupportsPositionOnly),
		mConstructParameters(inConstructParameters),
		mShouldCacheRef(inShouldCache),
		mModifyCompilationEnvironmentRef(inModifyCompilationEnvironment),
		mSupportsTessellationShadersRef(inSupportsTessellationShaders),
		mGlobalListLink(this)
	{
		for (int32 platform = 0; platform < SP_NumPlatforms; platform++)
		{
			bCachedConstantBufferStructDeclarations[platform] = false;
		}

		BOOST_ASSERT(!bInitializedSerializationHistory);
		mGlobalListLink.linkHead(getTypeList());
		mHashIndex = mNextHashIndex++;
	}

	VertexFactoryType::~VertexFactoryType()
	{
		mGlobalListLink.unLink();
	}

	void VertexFactoryType::initialize(const TMap<wstring, TArray<const TCHAR*>>& shaderFileToConstantBufferVariables)
	{
		if (!PlatformProperties::requiresCookedData())
		{
			for (TLinkedList<VertexFactoryType*>::TIterator it(VertexFactoryType::getTypeList()); it; it.next())
			{
				VertexFactoryType* type = *it;
				generateReferencedConstantBuffers(type->mShaderFilename, type->mName, shaderFileToConstantBufferVariables, type->mReferencedConstantBufferStructsCache);
				for (int32 frequency = 0; frequency < SF_NumFrequencies; frequency++)
				{
					VertexFactoryShaderParameters* parameters = type->createShaderParameters((EShaderFrequency)frequency);
					if (parameters)
					{
						TArray<uint8> tempData;
						MemoryWriter ar(tempData, true);
						ShaderSaveArchive saveArchive(ar, type->mSerializationHistory[frequency]);
						delete parameters;
					}
				}
			}
		}
		bInitializedSerializationHistory = true;
	}

	const SHAHash& VertexFactoryParameterRef::getHash() const
	{
		return mVFHash;
	}

	TLinkedList<VertexFactoryType*>*& VertexFactoryType::getTypeList()
	{
		static TLinkedList<VertexFactoryType*>* typeList = nullptr;
		return typeList;
	}

	Archive& operator << (Archive& ar, VertexFactoryType*& typeRef)
	{
		wstring typeName = typeRef ? typeRef->getName() : L"";
		ar << typeName;
		return ar;
	}

	const SHAHash& VertexFactoryType::getSourceHash() const
	{
		return getShaderFileHash(getShaderFilename());
	}

	VertexFactoryType* VertexFactoryType::getVFByName(const wstring& VFName)
	{
		for (TLinkedList<VertexFactoryType*>::TIterator it(getTypeList()); it; it.next())
		{
			wstring currentVFName = it->getName();
			if (currentVFName == VFName)
			{
				return *it;
			}
		}
		return nullptr;
	}


	void VertexFactory::set(RHICommandList& RHICmdList) const
	{
		BOOST_ASSERT(isInitialized());
		for (int32 streamIndex = 0; streamIndex < mStreams.size(); streamIndex++)
		{
			const VertexStream& stream = mStreams[streamIndex];
			if (!stream.bSetByVertexFactoryInSetMesh)
			{
				BOOST_ASSERT(stream.mVertexBuffer->isInitialized());
				RHICmdList.setStreamSource(streamIndex, stream.mVertexBuffer->mVertexBufferRHI, stream.mStride, stream.mOffset);
			}
		}
	}

	VertexFactoryParameterRef::VertexFactoryParameterRef(VertexFactoryType* inVertexFactoryType, const ShaderParameterMap& parameterMap, EShaderFrequency inShaderFrequency)
		:mParameters(nullptr)
		,mVertexFactoryType(inVertexFactoryType)
		,mShaderFrequency(inShaderFrequency)
	{
		mParameters = mVertexFactoryType->createShaderParameters(inShaderFrequency);
		mVFHash = getShaderFileHash(mVertexFactoryType->getShaderFilename());
		if (mParameters)
		{
			mParameters->bind(parameterMap);
		}
	}

	VertexElement VertexFactory::accessPositionStreamComponent(const VertexStreamComponent& component, uint8 attributeIndex)
	{
		VertexStream vertexStream;
		vertexStream.mVertexBuffer = component.mVertexBuffer;
		vertexStream.mStride = component.mStride;
		vertexStream.mOffset = 0;
		vertexStream.bUseInstanceIndex = component.bUseInstanceIndex;
		vertexStream.bSetByVertexFactoryInSetMesh = component.bSetByVertexFactoryInSetMesh;
		return VertexElement(mPositionStream.addUnique(vertexStream), component.mOffset, component.mType, attributeIndex, vertexStream.mStride, component.bUseInstanceIndex);
	}

	VertexElement VertexFactory::accessStreamComponent(const VertexStreamComponent& component, uint8 attributeIndex)
	{
		VertexStream vertexStream;
		vertexStream.mVertexBuffer = component.mVertexBuffer;
		vertexStream.mStride = component.mStride;
		vertexStream.mOffset = 0;
		vertexStream.bUseInstanceIndex = component.bUseInstanceIndex;
		vertexStream.bSetByVertexFactoryInSetMesh = component.bSetByVertexFactoryInSetMesh;
		return VertexElement(mStreams.addUnique(vertexStream), component.mOffset, component.mType, attributeIndex, vertexStream.mStride, component.bUseInstanceIndex);
	}

	void VertexFactory::initDeclaration(VertexDeclarationElementList& elements)
	{
		mDeclaration = RHICreateVertexDeclaration(elements);
	}

	void VertexFactory::initPositionDeclaration(const VertexDeclarationElementList& elements)
	{
		mPositionDeclaration = RHICreateVertexDeclaration(elements);
	}
}