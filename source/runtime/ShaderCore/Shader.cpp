#include "VertexFactory.h"
#include "HAL/PlatformProperties.h"
#include "Containers/StringConv.h"
#include "ShaderCore.h"
#include "ShaderParameters.h"
#include "Serialization/MemoryWriter.h"
#include "Shader.h"
#include "Containers/Set.h"
#include "Misc/Compression.h"
namespace Air
{
	static const ECompressionFlags ShaderCompressionFlag = ECompressionFlags::COMPRESS_ZLIB;

	TMap<ShaderResourceId, ShaderResource*> ShaderResource::mShaderResourceIdMap;


	static void safeAssignHash(RHIShader* inShader, const SHAHash& hash)
	{
		if (inShader)
		{
			inShader->setHash(hash);
		}
	}


	ShaderResource* ShaderResource::findShaderResourceById(const ShaderResourceId& id)
	{
		BOOST_ASSERT(isInGameThread());
		auto it = mShaderResourceIdMap.find(id);
		if (it != mShaderResourceIdMap.end())
		{
			return it->second;
		}
		else
		{
			return nullptr;
		}
	}

	ShaderResourceId ShaderResource::getId() const
	{
		ShaderResourceId shaderId;
		shaderId.mTarget = mTarget;
		shaderId.mOutputHash = mOutputHash;
		shaderId.mSpecificShaderTypeName = mSpecificType ? mSpecificType->getName() : nullptr;
		return shaderId;
	}

	

	

	void ShaderType::addToShaderIdMap(ShaderId id, Shader* shader)
	{
		BOOST_ASSERT(isInGameThread());
		mShaderIdMap.emplace(id, shader);
	}

	const SHAHash& ShaderType::getSourceHash() const
	{
		return getShaderFileHash(getShaderFilename());
	}

	Shader* ShaderType::findShaderById(const ShaderId& id)
	{
		BOOST_ASSERT(isInGameThread());
		auto it = mShaderIdMap.find(id);
		if (it == mShaderIdMap.end())
		{
			return nullptr;
		}
		else
		{
			return it->second;
		}
	}

	void ShaderType::initialize(const TMap<wstring, TArray<const TCHAR*>>& shaderFileToConstantBufferVariables)
	{
		if (!PlatformProperties::requiresCookedData())
		{
			for (TLinkedList<ShaderType*>::TIterator it(ShaderType::getTypeList()); it; it.next())
			{
				ShaderType* type = *it;
				generateReferencedConstantBuffers(type->mSourceFilename, type->mName, shaderFileToConstantBufferVariables, type->mReferencedConstantBufferStructsCache);
				{
					Shader* tempShader = type->constructForDeserialization();
					BOOST_ASSERT(tempShader != nullptr);
					tempShader->mType = type;
					TArray<uint8> tempData;
					MemoryWriter ar(tempData, true);
					ShaderSaveArchive saveArchive(ar, type->mSerializationHistory);
					tempShader->serializeBase(saveArchive, false);
					delete tempShader;
				}
			}
		}
		bInitialiezedSerializationHistory = true;
	}


	Shader::Shader()
		:mSerializedResource(nullptr),
		mShaderPipeline(nullptr),
		mVFType(nullptr),
		mType(nullptr),
		mNumRefs(0),
		mSetParametersId(0),
		mCanary(ShaderMagic_Initialized)
	{
		mTarget.mFrequency = 0;
		mTarget.mPlatform = GShaderPlatformForFeatureLevel[GMaxRHIFeatureLevel];
	}

	Shader::Shader(const CompiledShaderInitializerType& initializer)
		:mSerializedResource(nullptr),
		mMaterialShaderMapHash(initializer.mMaterialShaderMapHash),
		mShaderPipeline(initializer.mShaderPipeline),
		mVFType(initializer.mVertexFactoryType),
		mType(initializer.mType),
		mTarget(initializer.mTarget),
		mNumRefs(0),
		mSetParametersId(0),
		mCanary(ShaderMagic_Initialized)
	{
		mOutputHash = initializer.mOutputHash;
		BOOST_ASSERT(mOutputHash != SHAHash());
		BOOST_ASSERT(mType);
		mSourceHash = mType->getSourceHash();
		if (mVFType)
		{
			mVFSourceHash = mVFType->getSourceHash();
		}
		for (TLinkedList<ConstantBufferStruct*>::TIterator structIt(ConstantBufferStruct::getStructList()); structIt; structIt.next())
		{
			if (initializer.mParameterMap.containsParameterAllocation(structIt->getShaderVariableName()))
			{
				mConstantBufferParameterStructs.push_back(*structIt);
				mConstantBufferParameters.push_back(structIt->constructTypeParameter());
				ShaderConstantBufferParameter* parameter = mConstantBufferParameters.back();
				parameter->bind(initializer.mParameterMap, structIt->getShaderVariableName(), SPF_Mandatory);
			}
		}
		setResource(initializer.mResource);
		Register();
	}

	Shader::~Shader()
	{
		BOOST_ASSERT(mCanary == ShaderMagic_Uninitialized || mCanary == ShaderMagic_CleaningUp || mCanary == ShaderMagic_Initialized);
		BOOST_ASSERT(mNumRefs == 0);
		mCanary = 0;
		for (int32 structIndex = 0; structIndex < mConstantBufferParameters.size(); structIndex++)
		{
			delete mConstantBufferParameters[structIndex];
		}
	}

	void Shader::setResource(ShaderResource* inResource)
	{
		BOOST_ASSERT(inResource && inResource->mTarget == mTarget);
		mResource = inResource;
	}


	void Shader::finishCleanup()
	{
		delete this;
	}

	void Shader::Register()
	{
		ShaderId shaderId = getId();
		BOOST_ASSERT(shaderId.mMaterialShaderMapHash != SHAHash());
		BOOST_ASSERT(shaderId.mSourceHash != SHAHash());
		BOOST_ASSERT(mResource);
		mType->addToShaderIdMap(shaderId, this);
	}

	ShaderId Shader::getId() const
	{
		ShaderId shaderId(mType->getSerializationHistory());
		shaderId.mMaterialShaderMapHash = mMaterialShaderMapHash;
		shaderId.mShaderPipeline = mShaderPipeline;
		shaderId.mVertexFactoryType = mVFType;
		shaderId.mVFSourceHash = mVFSourceHash;
		shaderId.mVFSerializationHistory = mVFType ? mVFType->getSerializationHistory((EShaderFrequency)getTarget().mFrequency) : nullptr;
		shaderId.mShaderType = mType;
		shaderId.mSourceHash = mSourceHash;
		shaderId.mTarget = mTarget;
		return shaderId;
	}

	void Shader::AddRef()
	{
		BOOST_ASSERT(mCanary != ShaderMagic_CleaningUp);
		++mNumRefs;
		if (mNumRefs == 0)
		{
		}
	}
	void Shader::Release()
	{
		if (--mNumRefs == 0)
		{
			deRegister();
			mCanary = ShaderMagic_CleaningUp;
			beginCleanup(this);
		}
	}
	void Shader::deRegister()
	{
		mType->removeFromShaderIdMap(getId());
	}



	void Shader::registerSerializedResource()
	{
		if (mSerializedResource)
		{
			ShaderResource* existingResource = ShaderResource::findShaderResourceById(mSerializedResource->getId());
			if (existingResource)
			{
				delete mSerializedResource;
				mSerializedResource = existingResource;
			}
			else
			{
				mSerializedResource->Register();
			}
			setResource(mSerializedResource);
		}
	}

	bool Shader::serializeBase(Archive& ar, bool bShadersInline)
	{
		serialize(ar);
		ar << mOutputHash << mMaterialShaderMapHash << mShaderPipeline << mVFType;
		ar << mVFSourceHash;
		ar << mType;
		ar << mSourceHash;
		ar << mTarget;
		int32 numConstantParamters = mConstantBufferParameters.size();
		ar << numConstantParamters;
		for (int32 structIndex = 0; structIndex < numConstantParamters; structIndex++)
		{
			wstring structName(mConstantBufferParameterStructs[structIndex]->getStructTypeName());
			ar << structName;
			ar << *mConstantBufferParameters[structIndex];
		}
		if (bShadersInline)
		{
			mResource->serialize(ar);
		}
		else
		{

		}
		return false;
	}
	


	TLinkedList<ShaderType*>*& ShaderType::getTypeList()
	{
		static TLinkedList<ShaderType*>* GShaderTypeList = nullptr;
		return GShaderTypeList;
	}

	Archive& operator << (Archive& ar, ShaderType*& type)
	{
		if (ar.isSaving())
		{
			ar << ((type != nullptr) ? type->mName : TEXT(""));
		}
		return ar;
	}

	void ShaderType::flushShaderFileCache(const TMap<wstring, TArray<const TCHAR*>>& shaderFileToConstantBufferVariables)
	{
		mReferencedConstantBufferStructsCache.clear();
		generateReferencedConstantBuffers(mSourceFilename, mName, shaderFileToConstantBufferVariables, mReferencedConstantBufferStructsCache);
		for (int32 platform = 0; platform < SP_NumPlatforms; platform++)
		{
			bCachedConstantBufferStructDeclarations[platform] = false;
		}
	}

	TMap<wstring, ShaderType*>& ShaderType::getNameToTypeMap()
	{
		static TMap<wstring, ShaderType*>* GShaderNameToTypeMap = nullptr;
		if (!GShaderNameToTypeMap)
		{
			GShaderNameToTypeMap = new TMap<wstring, ShaderType*>();
		}
		return *GShaderNameToTypeMap;
	}

	void shaderMapAppendKeyString(EShaderPlatform platform, wstring& keyString)
	{
	}

	TLinkedList<ShaderPipelineType*>*& ShaderPipelineType::getTypeList()
	{
		static TLinkedList<ShaderPipelineType*>* GShaderPipelineList = nullptr;
		return GShaderPipelineList;
	}


	const SHAHash& ShaderPipelineType::getSourceHash()const
	{
		TArray<wstring> filenames;
		for (const ShaderType* shaderType : mStages)
		{
			filenames.add(shaderType->getShaderFilename());
		}
		return getShaderFilesHash(filenames);
	}

	bool ShaderPipelineType::bInitialized = false;

	void ShaderPipelineType::initialize()
	{
		BOOST_ASSERT(!bInitialized);
		TSet<wstring> mUsedNames;
		for (TLinkedList<ShaderPipelineType*>::TIterator it(ShaderPipelineType::getTypeList()); it; it.next())
		{
			const auto* pipelineType = *it;
			for (int32 index = 0; index < SF_NumFrequencies; ++index)
			{
				BOOST_ASSERT(!pipelineType->mAllStages[index] || pipelineType->mAllStages[index]->getFrequency() == (EShaderFrequency)index);
			}
			auto& stages = pipelineType->getStages();
			const GlobalShaderType* globalType = stages[0]->getGlobalShaderType();
			const MeshMaterialShaderType* meshType = stages[0]->getMeshMaterialShaderType();
			const MaterialShaderType* materialType = stages[0]->getMaterialShaderType();
			for (uint32 index = 1; index < stages.size(); ++index)
			{
				if (globalType)
				{
					BOOST_ASSERT(stages[index]->getGlobalShaderType());
				}
				else if (meshType)
				{
					BOOST_ASSERT(stages[index]->getMeshMaterialShaderType());
				}
				else if (materialType)
				{
					BOOST_ASSERT(stages[index]->getMaterialShaderType());
				}
			}
			wstring pipelineName = pipelineType->getName();
			BOOST_ASSERT(!mUsedNames.contains(pipelineName));
			mUsedNames.add(pipelineName);
		}
		bInitialized = true;
	}


	Archive& operator << (Archive& ar, const ShaderPipelineType*& typeRef)
	{
		wstring typeName = typeRef ? typeRef->mName : L"";
		ar << typeName;
		return ar;
	}


	ShaderPipeline::ShaderPipeline(const ShaderPipelineType* inPipelineType, const TArray<Shader*>& inStages)
		:mPipelineType(inPipelineType),
		mVertexShader(nullptr),
		mHullShader(nullptr),
		mDomainShader(nullptr),
		mGeometryShader(nullptr),
		mPixelShader(nullptr)
	{
		BOOST_ASSERT(inPipelineType);
		for (Shader* shader : inStages)
		{
			if (shader)
			{
				switch (shader->getType()->getFrequency())
				{
				case SF_Vertex:
					BOOST_ASSERT(!mVertexShader);
					mVertexShader = shader;
					break;
				case SF_Pixel:
					BOOST_ASSERT(!mPixelShader);
					mPixelShader = shader;
					break;
				case SF_Hull:
					BOOST_ASSERT(!mHullShader);
					mHullShader = shader;
					break;
				case SF_Domain:
					BOOST_ASSERT(!mDomainShader);
					mDomainShader = shader;
					break;
				case SF_Geometry:
					BOOST_ASSERT(!mGeometryShader);
					mGeometryShader = shader;
					break;
				default:
					BOOST_ASSERT(false);
					break;
				}
			}
		}
		validate();
	}

	ShaderPipeline::ShaderPipeline(const ShaderPipelineType* inPipelineType, const TArray<TRefCountPtr<Shader>>& inStages)
	{
		BOOST_ASSERT(inPipelineType);
		for (Shader* shader : inStages)
		{
			if (shader)
			{
				switch (shader->getType()->getFrequency())
				{
				case SF_Vertex:
					BOOST_ASSERT(!mVertexShader);
					mVertexShader = shader;
					break;
				case SF_Pixel:
					BOOST_ASSERT(!mPixelShader);
					mPixelShader = shader;
					break;
				case SF_Hull:
					BOOST_ASSERT(!mHullShader);
					mHullShader = shader;
					break;
				case SF_Domain:
					BOOST_ASSERT(!mDomainShader);
					mDomainShader = shader;
					break;
				case SF_Geometry:
					BOOST_ASSERT(!mGeometryShader);
					mGeometryShader = shader;
					break;
				default:
					BOOST_ASSERT(false);
					break;
				}
			}
		}
	}


	void ShaderPipeline::validate()
	{
		for (const ShaderType* stage : mPipelineType->getStages())
		{
			switch (stage->getFrequency())
			{
			case SF_Vertex:
				BOOST_ASSERT(mVertexShader && mVertexShader->getType() == stage);
				break;
			case SF_Pixel:
				BOOST_ASSERT(mPixelShader && mPixelShader->getType() == stage);
				break;
			case SF_Hull:
				BOOST_ASSERT(mHullShader && mHullShader->getType() == stage);
				break;
			case SF_Domain:
				BOOST_ASSERT(mDomainShader && mDomainShader->getType() == stage);
				break;
			case SF_Geometry:
				BOOST_ASSERT(mGeometryShader && mGeometryShader->getType() == stage);
				break;
			default:
				break;
			}
		}
	}

	ShaderPipeline::~ShaderPipeline()
	{
		mVertexShader = nullptr;
		mHullShader = nullptr;
		mDomainShader = nullptr;
		mGeometryShader = nullptr;
		mPixelShader = nullptr;
	}

	ShaderId::ShaderId(const SHAHash& inMaterialShaderMapHash, const ShaderPipelineType * inShaderPipeline, VertexFactoryType* inVertexFactoryType, ShaderType* inShaderType, ShaderTarget inTarget)
		:mMaterialShaderMapHash(inMaterialShaderMapHash)
		,mShaderPipeline(inShaderPipeline)
		,mShaderType(inShaderType)
		,mSourceHash(inShaderType->getSourceHash())
		,mSerializationHistory(&inShaderType->getSerializationHistory())
		,mTarget(inTarget)
	{
		if (inVertexFactoryType)
		{
			mVFSerializationHistory = inVertexFactoryType->getSerializationHistory((EShaderFrequency)inTarget.mFrequency);
			mVertexFactoryType = inVertexFactoryType;
			mVFSourceHash = inVertexFactoryType->getSourceHash();
		}
		else
		{
			mVFSerializationHistory = nullptr;
			mVertexFactoryType = nullptr;
		}
	}

	SelfContainedShaderId::SelfContainedShaderId()
		:mTarget(ShaderTarget(SF_NumFrequencies, SP_NumPlatforms))
	{}
	SelfContainedShaderId::SelfContainedShaderId(const ShaderId & inShaderId)
	{
		mMaterialShaderMapHash = inShaderId.mMaterialShaderMapHash;
		mVertexFactoryTypeName = inShaderId.mVertexFactoryType ? inShaderId.mVertexFactoryType->getName() : TEXT("");
		mShaderPipelineName = inShaderId.mShaderPipeline ? inShaderId.mShaderPipeline->getName() : TEXT("");
		mVFSourceHash = inShaderId.mVFSourceHash;
		mVFSerializationHistory = inShaderId.mVFSerializationHistory ? *inShaderId.mVFSerializationHistory : SerializationHistory();
		mShaderTypeName = inShaderId.mShaderType->getName();
		mSourceHash = inShaderId.mSourceHash;
		mSerializationHistory = *inShaderId.mSerializationHistory;
		mTarget = inShaderId.mTarget;
	}

	bool SelfContainedShaderId::isValid()
	{
		auto map = ShaderType::getNameToTypeMap();
		auto it = map.find(mShaderTypeName);
		if (it != map.end() && mSourceHash == it->second->getSourceHash() && mSerializationHistory == it->second->getSerializationHistory())
		{
			VertexFactoryType* VFTypePtr = VertexFactoryType::getVFByName(mVertexFactoryTypeName);
			if (mVertexFactoryTypeName == TEXT("") || (VFTypePtr && mVFSourceHash == VFTypePtr->getSourceHash() && mVFSerializationHistory == *VFTypePtr->getSerializationHistory((EShaderFrequency)mTarget.mFrequency)))
			{
				return true;
			}
		}
		return false;
	}

	Archive& operator << (Archive& ar, class SelfContainedShaderId& ref)
	{
		ar << ref.mMaterialShaderMapHash
			<< ref.mVertexFactoryTypeName
			<< ref.mShaderPipelineName
			<< ref.mVFSourceHash
			<< ref.mVFSerializationHistory
			<< ref.mShaderTypeName
			<< ref.mVFSourceHash
			<< ref.mSerializationHistory
			<< ref.mTarget;
		return ar;
	}

	ShaderResource::ShaderResource()
		:mSpecificType(nullptr)
		,mNumInstructions(0)
		,mNumTextureSamplers(0)
		,mNumRefs(0)
		,mCanary(Shader::ShaderMagic_Initialized)
	{

	}
	ShaderResource::ShaderResource(const ShaderCompilerOutput& output, ShaderType* inSpecificType)
		: mSpecificType(inSpecificType)
		, mNumInstructions(output.mNumInstructions)
		, mNumTextureSamplers(output.mNumTextureSamplers)
		, mNumRefs(0)
		, mCanary(Shader::ShaderMagic_Initialized)
	{
		mTarget = output.mTarget;
		compressCode(output.mShaderCode.getReadAccess());
		BOOST_ASSERT(mCode.size() > 0);
		mOutputHash = output.mOutputHash;
		BOOST_ASSERT(mOutputHash != SHAHash());
		{
			BOOST_ASSERT(isInGameThread());
			mShaderResourceIdMap.emplace(getId(), this);
		}
	}

	ShaderResource::~ShaderResource()
	{
		BOOST_ASSERT(mCanary == Shader::ShaderMagic_Uninitialized || mCanary == Shader::ShaderMagic_CleaningUp || mCanary == Shader::ShaderMagic_Initialized);
		BOOST_ASSERT(mNumRefs == 0);
		mCanary = 0;
	}

	void ShaderResource::uncompressCode(TArray<uint8>& uncompressedCode) const
	{
		if (mCode.size() != mUncompressedCodeSize && RHISupportsShaderCompression((EShaderPlatform)mTarget.mPlatform))
		{
			uncompressedCode.setNum(mUncompressedCodeSize);
			auto bSucceed = Compression::uncompressMemory(ShaderCompressionFlag, uncompressedCode.getData(), mUncompressedCodeSize, mCode.getData(), mCode.size());
			BOOST_ASSERT(bSucceed);
		}
		else
		{
			uncompressedCode = mCode;
		}
		
	}

	void ShaderResource::compressCode(const TArray<uint8> & uncompressedCode)
	{
		mUncompressedCodeSize = uncompressedCode.size();
		mCode = uncompressedCode;
		if (RHISupportsShaderCompression((EShaderPlatform)mTarget.mPlatform))
		{
		}
	}

	void ShaderResource::Register()
	{
		BOOST_ASSERT(isInGameThread());
		mShaderResourceIdMap.emplace(getId(), this);
	}
	void ShaderResource::serialize(Archive& ar)
	{
		ar << mSpecificType;
		ar << mTarget;
		ar << mCode;
		ar << mOutputHash;
		ar << mNumInstructions;
		ar << mNumTextureSamplers;
		ar << mUncompressedCodeSize;
	}

	void ShaderResource::AddRef()
	{
		BOOST_ASSERT(isInGameThread());
		BOOST_ASSERT(mCanary != Shader::ShaderMagic_CleaningUp);
		++mNumRefs;
	}

	void ShaderResource::Release()
	{
		BOOST_ASSERT(isInGameThread());
		BOOST_ASSERT(mNumRefs != 0);
		if (--mNumRefs == 0)
		{
			mShaderResourceIdMap.erase(getId());
			beginReleaseResource(this);
			mCanary = Shader::ShaderMagic_CleaningUp;
			beginCleanup(this);
		}
	}

	void ShaderResource::initializeShaderRHI()
	{
		if (!isInitialized())
		{
			initResourceFromPossiblyParallelRendering();
		}
		BOOST_ASSERT(isInitialized());
	}

	void ShaderResource::initRHI()
	{
		BOOST_ASSERT(mCode.size() > 0);
		TArray<uint8> uncompressedCode;
		uncompressCode(uncompressedCode);
		if (!arePlatformsCompatible(GMaxRHIShaderPlatform, (EShaderPlatform)mTarget.mPlatform))
		{
			if (PlatformProperties::requiresCookedData())
			{

			}
			return;
		}
		if (mTarget.mFrequency == SF_Vertex)
		{
			mVertexShader = RHICreateVertexShader(uncompressedCode);
			safeAssignHash(mVertexShader, mOutputHash);
		}
		else if (mTarget.mFrequency == SF_Domain)
		{
			mDomainShader = RHICreateDomainShader(uncompressedCode);
			safeAssignHash(mDomainShader, mOutputHash);
		}
		else if (mTarget.mFrequency == SF_Hull)
		{
			mHullShader = RHICreateHullShader(uncompressedCode);
			safeAssignHash(mHullShader, mOutputHash);
		}
		else if (mTarget.mFrequency == SF_Geometry)
		{
			if (mSpecificType)
			{
				StreamOutElementList elementList;
				TArray<uint32> streamStrides;
				int32 rasterizedStream = -1;
				mSpecificType->getStreamOutElements(elementList, streamStrides, rasterizedStream);
				mGeometryShader = RHICreateGeometryShaderWithStreamOutput(uncompressedCode, elementList, streamStrides.size(), streamStrides.data(), rasterizedStream);
			}
			else
			{
				mGeometryShader = RHICreateGeometryShader(uncompressedCode);
				safeAssignHash(mGeometryShader, mOutputHash);
			}
			
		}
		else if (mTarget.mFrequency == SF_Pixel)
		{
			mPixelShader = RHICreatePixelShader(uncompressedCode);
			safeAssignHash(mPixelShader, mOutputHash);
		}
		else if (mTarget.mFrequency == SF_Compute)
		{
			mComputeShader = RHICreateComputeShader(uncompressedCode);
			safeAssignHash(mComputeShader, mOutputHash);
		}
	}

	bool ShaderResource::arePlatformsCompatible(EShaderPlatform currentPlatform, EShaderPlatform targetPlatform)
	{
		bool bFeatureLevelCompatible = currentPlatform == targetPlatform;
		if (!bFeatureLevelCompatible && isPCPlatform(currentPlatform) && isPCPlatform(targetPlatform))
		{
			if (currentPlatform == SP_OPENGL_SM4_MAC || targetPlatform == SP_OPENGL_SM4_MAC)
			{

			}
			else
			{
				bFeatureLevelCompatible = getMaxSupportedFeatureLevel(currentPlatform) >= getMaxSupportedFeatureLevel(targetPlatform);
			}
			bool const bIsTargetD3D = targetPlatform == SP_PCD3D_SM4 || targetPlatform == SP_PCD3D_SM5 || targetPlatform == SP_PCD3D_ES2 || targetPlatform == SP_PCD3D_ES3_1;
			bool const bIsCurrentPlatformD3D = currentPlatform == SP_PCD3D_SM4 || currentPlatform == SP_PCD3D_SM5 || currentPlatform == SP_PCD3D_ES2 || currentPlatform == SP_PCD3D_ES3_1;

			bool const bIsCurrentMetal = isMetalPlatform(currentPlatform);
			bool const bIsTargetMetal = isMetalPlatform(targetPlatform);

			bool const bIsCurrentOpenGL = isOpenGLPlatform(currentPlatform);
			bool const bIsTargetOpenGL = isOpenGLPlatform(targetPlatform);

			bFeatureLevelCompatible = bFeatureLevelCompatible && (bIsCurrentPlatformD3D == bIsTargetD3D && bIsCurrentMetal == bIsTargetMetal && bIsCurrentOpenGL == bIsTargetOpenGL);
		}
		return bFeatureLevelCompatible;
	}

	void ShaderResource::releaseRHI()
	{
		mVertexShader.safeRelease();
		mHullShader.safeRelease();
		mDomainShader.safeRelease();
		mGeometryShader.safeRelease();
		mPixelShader.safeRelease();
		mComputeShader.safeRelease();
	}
	void ShaderResource::finishCleanup()
	{
		delete this;
	}

	ShaderResource* ShaderResource::findOrCreateShaderResource(const ShaderCompilerOutput& output, class ShaderType* specificType)
	{
		const ShaderResourceId resourceId(output, specificType ? specificType->getName() : nullptr);
		ShaderResource* resource = findShaderResourceById(resourceId);
		if (!resource)
		{
			resource = new ShaderResource(output, specificType);
		}
		else
		{
			BOOST_ASSERT(resource->mCanary == Shader::ShaderMagic_Initialized);
		}
		return resource;
	}

	void ShaderResource::getAllShaderResourceId(TArray<ShaderResourceId>& ids)
	{
		BOOST_ASSERT(isInGameThread());
		for (auto& it : mShaderResourceIdMap)
		{
			ids.push_back(it.first);
		}
	}

	bool ShaderType::bInitialiezedSerializationHistory = false;

	ShaderType::ShaderType(EShaderTypeForDynamicCast inShaderTypeForDynamicCast, const TCHAR* inName, const TCHAR* inSourceFilename, const TCHAR* inFunctionName, uint32 inFrequency, ConstructSerializedType inConstructSerializedRef, GetStreamOutElementsType inGetStreamOutElementsRef)
		:mShaderTypeForDynamicCast(inShaderTypeForDynamicCast)
		,mName(inName)
		,mTypeName(inName)
		,mSourceFilename(inSourceFilename)
		,mFunctionName(inFunctionName)
		,mFrequency(inFrequency)
		,mConstructSerializedRef(inConstructSerializedRef)
		,mGetStreamOutElementsRef(inGetStreamOutElementsRef)
		,mGlobalListLink(this)
	{
		for (int32 platform = 0; platform < SP_NumPlatforms; platform++)
		{
			bCachedConstantBufferStructDeclarations[platform] = false;
		}
		BOOST_ASSERT(!bInitialiezedSerializationHistory);
		BOOST_ASSERT(CString::strlen(inName) < 1024);
		mGlobalListLink.linkHead(getTypeList());
		getNameToTypeMap().emplace(mTypeName, this);
		static uint32 nextHashIndex = 0; 
		mHashIndex = nextHashIndex++;

	}

	ShaderType::~ShaderType()
	{
		mGlobalListLink.unLink();
		getNameToTypeMap().erase(mTypeName);
	}
}