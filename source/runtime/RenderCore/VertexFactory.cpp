#include "Serialization/MemoryWriter.h"
#include "HAL/PlatformProperties.h"
#include "VertexFactory.h"
namespace Air
{
	bool VertexFactoryType::bInitializedSerializationHistory = false;

	uint32 VertexFactoryType::mNextHashIndex = 0;

	VertexFactoryType::VertexFactoryType(
		const TCHAR* inName,
		const TCHAR* inShaderFilename,
		bool bInUseWithMaterials,
		bool bInSupportsStaticLighting,
		bool bInSupportsDynamicLightings,
		bool bInSupportsPrecisePrevWorldPos,
		bool bInSupportsPositionOnly,
		bool bInSupportsCachingMeshDrawCommands,
		bool bInSupportsPrimitiveIdStream,
		ConstructParametersType inConstructParameters,
		ShouldCacheType inShouldCache,
		ModifyCompilationEnvironmentType inModifyCompilationEnvironment,
		ValidateCompiledResultType inValidateCompiledResult,
		SupportsTessellationShaderType inSupportsTessellationShaders)
		:mName(inName),
		mShaderFilename(inShaderFilename),
		mTypeName(inName),
		bUseWithMaterials(bInUseWithMaterials),
		bSupportsStaticLighting(bInSupportsStaticLighting),
		bSupportsDynamicLighting(bInSupportsDynamicLightings),
		bSupportsPrecisePrevWorldPos(bInSupportsPrecisePrevWorldPos),
		bSupportsPositionOnly(bInSupportsPositionOnly),
		bSupportsCachingMeshDrawCommands(bInSupportsCachingMeshDrawCommands),
		bSupportsPrimitveIdStream(bInSupportsPrimitiveIdStream),
		mConstructParameters(inConstructParameters),
		mShouldCacheRef(inShouldCache),
		mModifyCompilationEnvironmentRef(inModifyCompilationEnvironment),
		mValidateCompiledResultRef(inValidateCompiledResult),
		mSupportsTessellationShadersRef(inSupportsTessellationShaders),
		mGlobalListLink(this)
	{
		for (int32 platform = 0; platform < SP_NumPlatforms; platform++)
		{
			bCachedShaderParametersMetadataDeclarations[platform] = false;
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
				generateReferencedConstantBuffers(type->mShaderFilename, type->mName, shaderFileToConstantBufferVariables, type->mReferencedShaderParametersMetadatasCache);
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

	bool operator <<(Archive& ar, VertexFactoryParameterRef& ref)
	{
		bool bShaderHasOutdatedParameters = false;

		ar << ref.mVertexFactoryType;

		uint8 shaderFrequencyByte = ref.mShaderFrequency;

		ar << shaderFrequencyByte;
		if (ar.isLoading())
		{
			ref.mShaderFrequency = (EShaderFrequency)shaderFrequencyByte;
		}
		uint8 shaderPlatformByte = ref.mShaderPlatform;
		ar << shaderPlatformByte;
		if (ar.mArIsLoading)
		{
			ref.mShaderPlatform = (EShaderPlatform)shaderPlatformByte;
		}
		SHAHash VFHash;

		ar << ShaderResource::filterShaderSourceHashForSerialization(ar, VFHash);

		if (ar.isLoading())
		{
			delete ref.mParameters;
			if (ref.mVertexFactoryType)
			{
				ref.mParameters = ref.mVertexFactoryType->createShaderParameters(ref.mShaderFrequency);
			}
			else
			{
				bShaderHasOutdatedParameters = true;
				ref.mParameters = nullptr;
			}
		}

		int64 skipOffset = ar.tell();
		{
			ar << skipOffset;
		}

		if (ref.mParameters)
		{
			ref.mParameters->serialize(ar);
		}
		else if (ar.isLoading())
		{
			ar.seek(skipOffset);
		}

		if (ar.isSaving())
		{
			int64 endOffset = ar.tell();
			ar.seek(skipOffset);
			ar << endOffset;
			ar.seek(endOffset);
		}

		return bShaderHasOutdatedParameters;
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

	const SHAHash& VertexFactoryType::getSourceHash(EShaderPlatform shaderPlatform) const
	{
		return getShaderFileHash(getShaderFilename(), shaderPlatform);
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


	VertexFactoryParameterRef::VertexFactoryParameterRef(VertexFactoryType* inVertexFactoryType, const ShaderParameterMap& parameterMap, EShaderFrequency inShaderFrequency, EShaderPlatform inShaderPlatform)
		:mParameters(nullptr)
		,mVertexFactoryType(inVertexFactoryType)
		,mShaderFrequency(inShaderFrequency)
	{
		mParameters = mVertexFactoryType->createShaderParameters(inShaderFrequency);
		mVFHash = getShaderFileHash(mVertexFactoryType->getShaderFilename(), inShaderPlatform);
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