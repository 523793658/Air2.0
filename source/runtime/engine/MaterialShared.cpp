#include "Classes/Engine/EngineType.h"
#include "MeshMaterialShaderType.h"
#include "MaterialShared.h"
#include "VertexFactory.h"
#include "TextureResource.h"
#include "SceneManagement.h"
#include "Texture.h"
#include "HAL/PlatformTime.h"
#include "RenderUtils.h"
#include "HAL/PlatformProperties.h"
#include "Classes/Materials/MaterialInstance.h"
#include "Classes/Materials/Material.h"
#include "RenderingThread.h"
#include "ShaderCompiler.h"
#include "Serialization/MemoryWriter.h"
#include "Containers/StringConv.h"
#include "Classes/Materials/HLSLMaterialTranslator.h"
#include "Classes/Materials/XMLMaterialTranslator.h"
#include "Misc/App.h"
#include "shader.h"
namespace Air
{
	int32 GCreateShadersOnLoad = 0;

	wstring MaterialQualityLevelNames[] =
	{
		TEXT("Low"),
		TEXT("High"),
		TEXT("Medium"),
		TEXT("Num")
	};


	void getMaterialQualityLevelName(EMaterialQualityLevel::Type inQualityLevel, wstring & outName)
	{
		BOOST_ASSERT(inQualityLevel < ARRAY_COUNT(MaterialQualityLevelNames));
		outName = MaterialQualityLevelNames[(uint32)inQualityLevel];
	}

	TLinkedList<MaterialConstantExpressionType*>*& MaterialConstantExpressionType::getTypeList()
	{
		static TLinkedList<MaterialConstantExpressionType*>* mTypeList = nullptr;
		return mTypeList;
	}

	TMap<wstring, MaterialConstantExpressionType*>& MaterialConstantExpressionType::getTypeMap()
	{
		static TMap<wstring, MaterialConstantExpressionType*> mTypeMap;
		TLinkedList<MaterialConstantExpressionType*>* typeListLink = getTypeList();
		while (typeListLink)
		{
			TLinkedList<MaterialConstantExpressionType*>* nextLink = typeListLink->next();
			MaterialConstantExpressionType* type = **typeListLink;
			mTypeMap.emplace(type->mName, type);
			typeListLink->unLink();
			delete typeListLink;
			typeListLink = nextLink;
		}
		return mTypeMap;
	}

	MaterialConstantExpressionType::MaterialConstantExpressionType(const TCHAR* inName, SerializationConstructorType inSerializationConstructor)
		:mName(inName)
		,mSerializationConstructor(inSerializationConstructor)
	{
		(new TLinkedList<MaterialConstantExpressionType*>(this))->linkHead(getTypeList());
	}

	Archive& operator << (Archive& ar, MaterialConstantExpression*& ref)
	{
		if (ar.isSaving())
		{
			wstring typeName(ref->getType()->mName);
			ar << typeName;
		}
		else if (ar.isLoading())
		{
			wstring typeName;
			ar << typeName;
			MaterialConstantExpressionType* type = MaterialConstantExpressionType::getTypeMap().findRef(typeName);
			ref = (*type->mSerializationConstructor)();
		}
		ref->serialize(ar);
		return ar;
	}
	Archive& operator << (Archive& ar, MaterialConstantExpressionTexture*& ref)
	{
		ar << (MaterialConstantExpression*&)ref;
		return ar;
	}

	MaterialShaderMap* FMaterial::getRenderingThreadShaderMap() const
	{
		BOOST_ASSERT(isInParallelRenderingThread());
		return mRenderingThreadShaderMap;
	}

	EMaterialTessellationMode FMaterial::getTessellationMode() const
	{
		return MTM_NOTessellation;
	}



	bool FMaterial::needsSceneTextures() const
	{
		return mMaterialCompilationOutput.bNeedsSceneTextures;
	}

	void FMaterial::getShaderMapId(EShaderPlatform platform, MaterialShaderMapId& outShaderMapId)const
	{
		if (bLoadedCookedShaderMapId)
		{
			outShaderMapId = mCookedShaderMapId;
		}
		else
		{
			TArray<ShaderType*> shaderTypes;
			TArray<VertexFactoryType*> VFTypes;
			TArray<const ShaderPipelineType*> shaderPipelineTypes;
			getDependentShaderAndVFTypes(platform, shaderTypes, shaderPipelineTypes, VFTypes);
			outShaderMapId.mUsage = getShaderMapUsage();
			outShaderMapId.mBaseMaterialId = getMaterialId();
			outShaderMapId.mQualityLevel = getQualityLevelForShaderMapId();
			outShaderMapId.mFeatureLevel = getFeatureLevel();
			outShaderMapId.setShaderDependencies(shaderTypes, shaderPipelineTypes, VFTypes);
			getReferencedTexturesHash(platform, outShaderMapId.mTextureReferencesHash);
		}
	}

	void FMaterial::getReferencedTexturesHash(EShaderPlatform platform, SHAHash& outHash) const
	{
		SHA1 hashState;
		const TArray<std::shared_ptr<RTexture>>& referencedTextures = getReferencedTextures();
		for (int32 textureIndex = 0; textureIndex < referencedTextures.size(); textureIndex++)
		{
			wstring textureName;
			if (referencedTextures[textureIndex])
			{
				textureName = referencedTextures[textureIndex]->getName();
			}
			hashState.updateWithString(textureName.c_str(), textureName.length());
		}
		hashState.finalize();
		hashState.getHash(&outHash.mHash[0]);
	}



	void FMaterial::registerInlineShaderMap()
	{
		if (mGameThreadShaderMap)
		{
			mGameThreadShaderMap->registerSerializedShaders();
		}
	}

	void FMaterial::setInlineShaderMap(MaterialShaderMap* inMaterialShaderMap)
	{
		BOOST_ASSERT(isInGameThread() || isInAsyncLoadingThread());
		mGameThreadShaderMap = inMaterialShaderMap;
		bContainsInlineShaders = true;
		bLoadedCookedShaderMapId = true;
		mCookedShaderMapId = inMaterialShaderMap->getShaderMapId();
	}

	void MaterialShaderMap::registerSerializedShaders()
	{
		BOOST_ASSERT(isInGameThread());
		TShaderMap<MaterialShaderType>::registerSerializedShaders();
		for (MeshMaterialShaderMap* meshShaderMap : mOrderredMeshShaderMaps)
		{
			if (meshShaderMap)
			{
				meshShaderMap->registerSerializedShaders();
			}
		}
		for (int32 VFIndex = 0; VFIndex < mOrderredMeshShaderMaps.size(); VFIndex++)
		{
			if (mOrderredMeshShaderMaps[VFIndex] && mOrderredMeshShaderMaps[VFIndex]->isEmpty())
			{
				mOrderredMeshShaderMaps[VFIndex] = nullptr;
			}
		}

		for (int32 index = mMeshShaderMaps.size() - 1; index >= 0; index--)
		{
			if (mMeshShaderMaps[index].isEmpty())
			{
				mMeshShaderMaps.removeAt(index);
			}
		}
	}

	static inline bool shouldCacheMaterialShader(const MaterialShaderType* shaderType, EShaderPlatform platform, const FMaterial* material)
	{
		return shaderType->shouldCache(platform, material) && material->shouldCache(platform, shaderType, nullptr);
	}


	TMap<TRefCountPtr<MaterialShaderMap>, TArray<FMaterial*>> MaterialShaderMap::mShaderMapsBeingCompiled;

	bool MaterialShaderMap::isComplete(const FMaterial* material, bool bSilent)
	{
		BOOST_ASSERT(mNumRef > 0);
		auto it = MaterialShaderMap::mShaderMapsBeingCompiled.find(this);
		if (it != mShaderMapsBeingCompiled.end())
		{
			BOOST_ASSERT(!bCompilationFinalized);
			return false;
		}

		for (TLinkedList<VertexFactoryType*>::TIterator vertexFactoryTypeIT(VertexFactoryType::getTypeList()); vertexFactoryTypeIT; vertexFactoryTypeIT.next())
		{
			VertexFactoryType* vertexFactoryType = *vertexFactoryTypeIT;
			if (vertexFactoryType->isUsedWithMaterial())
			{
				const MeshMaterialShaderMap* meshShaderMap = getMeshShaderMap(vertexFactoryType);
				if (!MeshMaterialShaderMap::isComplete(meshShaderMap, mPlatform, material, vertexFactoryType, bSilent))
				{
					if (!meshShaderMap && !bSilent)
					{

					}
					return false;
				}
			}
		}
		for (TLinkedList<ShaderType*>::TIterator shaderTypeIt(ShaderType::getTypeList()); shaderTypeIt; shaderTypeIt.next())
		{
			MaterialShaderType* shaderType = shaderTypeIt->getMaterialShaderType();
			if (shaderType && !isMaterialShaderComplete(material, shaderType, nullptr, bSilent))
			{
				return false;
			}
		}
		const bool bHasTessellation = material->getTessellationMode() != MTM_NOTessellation;
		for (TLinkedList<ShaderPipelineType*>::TIterator shaderPipelineIt(ShaderPipelineType::getTypeList()); shaderPipelineIt; shaderPipelineIt.next())
		{
			const ShaderPipelineType* pipeline = *shaderPipelineIt;
			if (pipeline->isMaterialTypePipeline() && pipeline->hasTessellation() == bHasTessellation)
			{
				auto& stageTypes = pipeline->getStages();
				int32 numShouldCache = 0;
				for (int32 index = 0; index < stageTypes.size(); ++index)
				{
					auto* shaderType = stageTypes[index]->getMaterialShaderType();
					if (shouldCacheMaterialShader(shaderType, mPlatform, material))
					{
						++numShouldCache;
					}
					else
					{
						break;
					}
				}
				if (numShouldCache == stageTypes.size())
				{
					for (int32 index = 0; index < stageTypes.size(); ++index)
					{
						auto* shaderType = stageTypes[index]->getMaterialShaderType();
						if (!isMaterialShaderComplete(material, shaderType, pipeline, bSilent))
						{
							return false;
						}
					}
				}
			}
		}
		return true;
	}


	void MaterialShaderMap::AddRef()
	{
		BOOST_ASSERT(!bDeletedThroughDeferredCleanup);
		++mNumRef;
	}

	void MaterialShaderMap::Register(EShaderPlatform inShaderPlatform)
	{
		if (GCreateShadersOnLoad && mPlatform == inShaderPlatform)
		{
			for (auto& keyValue : getShaders())
			{
				Shader* shader = keyValue.second;
				if (shader)
				{
					shader->beginInitializeResources();
				}
			}
			for (int32 index = 0; index < mMeshShaderMaps.size(); index++)
			{
				for (auto& it : mMeshShaderMaps[index].getShaders())
				{
					Shader* shader = it.second;
					if (shader)
					{
						shader->beginInitializeResources();
					}
				}
			}
		}
		GIdToMaterialShaderMap[mPlatform].emplace(mShaderMapId, this);
		bRegistered = true;
	}

	MaterialShaderMap* MaterialShaderMap::findId(const MaterialShaderMapId& shaderMapId, EShaderPlatform platform)
	{
		BOOST_ASSERT(shaderMapId.mBaseMaterialId != Guid());
		return GIdToMaterialShaderMap[platform].findRef(shaderMapId);
	}

	

	

	Guid MaterialResource::getMaterialId() const
	{
		return mMaterial->mStateId;
	}

	int32 MaterialResource::compilePropertyAndSetMaterialProperty(EMaterialProperty prop, class MaterialCompiler* compiler, EShaderFrequency overrideShaderFrequency /* = SF_NumFrequencies */, bool bUsePreviousFrameTime /* = false */) const
	{
#if WITH_EDITOR
		compiler->setMaterialProperty(prop, overrideShaderFrequency, bUsePreviousFrameTime);
		EShaderFrequency shaderFrequency = compiler->getCurrentShaderFrequency();
		int32 selectionColorIndex = INDEX_NONE;
		if (shaderFrequency == SF_Pixel)
		{
			selectionColorIndex = compiler->componentMask(compiler->vectorParameter(TEXT("SelectionColor"), LinearColor::Black), 1, 1, 1, 0);

		}
		MaterialInterface* materialInterface = mMaterialInstance ? dynamic_cast<MaterialInterface*>(mMaterialInstance.get()) : dynamic_cast<MaterialInterface*>(mMaterial.get());
		int32 ret = INDEX_NONE;
		switch (prop)
		{
		case Air::MP_BaseColor:
			ret = compiler->mul(materialInterface->compileProperty(compiler, MP_BaseColor, MFCF_ForceCast), compiler->sub(compiler->constant(1.0f), selectionColorIndex));
			break;
		default:
			ret = materialInterface->compileProperty(compiler, prop);
			break;
		}
		return compiler->forceCast(ret, MaterialAttributeDefinationMap::getValueType(prop));
#else
		BOOST_ASSERT(false);
		return INDEX_NONE;
#endif
	}

	Shader* MaterialShaderMap::processCompilationResultsForSingleJob(class ShaderCompileJob* singleJob, const ShaderPipelineType* shaderPipelineType, const SHAHash& materialShaderMapHash)
	{
		BOOST_ASSERT(singleJob);
		const ShaderCompileJob& currentJob = *singleJob;
		BOOST_ASSERT(currentJob.mId == mCompilingId);
		Shader* shader = nullptr;
		if (currentJob.mVFType)
		{
			VertexFactoryType* vertexFactoryType = currentJob.mVFType;
			BOOST_ASSERT(vertexFactoryType->isUsedWithMaterial());
			MeshMaterialShaderMap* meshShaderMap = nullptr;
			int32 meshShaderMapIndex = INDEX_NONE;
			for (int32 shaderMapIndex = 0; shaderMapIndex < mMeshShaderMaps.size(); shaderMapIndex++)
			{
				if (mMeshShaderMaps[shaderMapIndex].getVertexFactoryType() == vertexFactoryType)
				{
					meshShaderMap = &mMeshShaderMaps[shaderMapIndex];
					meshShaderMapIndex = shaderMapIndex;
					break;
				}
			}
			BOOST_ASSERT(meshShaderMap);
			MeshMaterialShaderType* meshMaterialShaderType = currentJob.mShaderType->getMeshMaterialShaderType();
			BOOST_ASSERT(meshMaterialShaderType);
			shader = meshMaterialShaderType->finishCompileShader(mMaterialCompilationOutput.mConstantExpressionSet, materialShaderMapHash, currentJob, shaderPipelineType, mFriendlyName);
			BOOST_ASSERT(shader);
			if (!shaderPipelineType)
			{
				BOOST_ASSERT(!meshShaderMap->hasShader(meshMaterialShaderType));
				meshShaderMap->addShader(meshMaterialShaderType, shader);
			}
		}
		else
		{
			MaterialShaderType* materialShaderType = currentJob.mShaderType->getMaterialShaderType();
			BOOST_ASSERT(materialShaderType);
			shader = materialShaderType->finishCompileShader(mMaterialCompilationOutput.mConstantExpressionSet, materialShaderMapHash, currentJob, shaderPipelineType, mFriendlyName);
			BOOST_ASSERT(shader);
			if (!shaderPipelineType)
			{
				BOOST_ASSERT(!hasShader(materialShaderType));
				addShader(materialShaderType, shader);
			}
		}
		return shader;
	}

	void MaterialShaderMapId::getMaterialHash(SHAHash& outHash) const
	{
		SHA1 hashState;
		hashState.update((const uint8*)&mUsage, sizeof(mUsage));

		hashState.update((const uint8*)&mBaseMaterialId, sizeof(mBaseMaterialId));
		wstring qualityLevelString;
		getMaterialQualityLevelName(mQualityLevel, qualityLevelString);
		hashState.updateWithString(qualityLevelString.c_str(), qualityLevelString.length());
		hashState.update((const uint8*)&mFeatureLevel, sizeof(mFeatureLevel));

		mParameterSet.updateHash(hashState);

		for (int32 functionIndex = 0; functionIndex < mReferencedFunctions.size(); functionIndex++)
		{
			hashState.update((const uint8*)&mReferencedFunctions[functionIndex], sizeof(mReferencedFunctions[functionIndex]));
		}

		for (int32 collectionIndex = 0; collectionIndex < mReferencedParameterCollections.size(); collectionIndex++)
		{
			hashState.update((const uint8*)&mReferencedParameterCollections[collectionIndex], sizeof(mReferencedParameterCollections[collectionIndex]));
		}

		hashState.update((const uint8*)&mTextureReferencesHash, sizeof(mTextureReferencesHash));
		hashState.update((const uint8*)&mBasePropertyOverridesHash, sizeof(mBasePropertyOverridesHash));
		hashState.finalize();
		hashState.getHash(&outHash.mHash[0]);
	}

	void MaterialShaderMapId::setShaderDependencies(const TArray<ShaderType*>& shaderTypes, const TArray<const ShaderPipelineType*>& shaderPipelineTypes, const TArray<VertexFactoryType*>& VFType)
	{
		if (!PlatformProperties::requiresCookedData())
		{
			for (int32 shaderTypeIndex = 0; shaderTypeIndex < shaderTypes.size(); shaderTypeIndex++)
			{
				ShaderTypeDependency dependency;
				dependency.mShaderType = shaderTypes[shaderTypeIndex];
				dependency.mSourceHash = shaderTypes[shaderTypeIndex]->getSourceHash();
				mShaderTypeDependencies.add(dependency);
			}
			for (int32 VFTypeIndex = 0; VFTypeIndex < VFType.size(); VFTypeIndex++)
			{
				VertexFactoryTypeDependency dependency;
				dependency.mVertexFactoryType = VFType[VFTypeIndex];
				dependency.mVFSourceHash = VFType[VFTypeIndex]->getSourceHash();
				mVertexFactoryTypeDependencies.add(dependency);
			}

			for (int32 typeIndex = 0; typeIndex < shaderPipelineTypes.size(); typeIndex++)
			{
				const ShaderPipelineType* pipeline = shaderPipelineTypes[typeIndex];
				ShaderPipelineTypeDependency dependency;
				dependency.mShaderPipelineType = pipeline;
				dependency.mStagesSourceHash = pipeline->getSourceHash();
				mShaderPipelineTypeDependencies.add(dependency);
			}
		}
	}


	TMap<MaterialShaderMapId, MaterialShaderMap*> MaterialShaderMap::GIdToMaterialShaderMap[SP_NumPlatforms];

	TArray<MaterialShaderMap*> MaterialShaderMap::mAllMaterialShaderMaps;

	MaterialShaderMap::MaterialShaderMap()
		:mPlatform(SP_NumPlatforms),
		mCompilingId(1),
		mNumRef(0),
		bDeletedThroughDeferredCleanup(false),
		bRegistered(false),
		bCompilationFinalized(true),
		bCompiledSuccessfully(true),
		bIsPersistent(true)
	{
		BOOST_ASSERT(isInGameThread());
		mAllMaterialShaderMaps.add(this);
	}

	MaterialShaderMap::~MaterialShaderMap()
	{
		BOOST_ASSERT(isInGameThread());
		BOOST_ASSERT(bDeletedThroughDeferredCleanup);
		BOOST_ASSERT(!bRegistered);
		mAllMaterialShaderMaps.removeSingleSwap(this);
	}


	void MaterialShaderMap::Release()
	{
		BOOST_ASSERT(mNumRef > 0);
		if (--mNumRef == 0)
		{
			if (bRegistered)
			{
				GIdToMaterialShaderMap[mPlatform].erase(mShaderMapId);
				bRegistered = false;
			}
			beginCleanup(this);
		}
	}

	MaterialRenderProxy::MaterialRenderProxy()
		:bSelected(false)
		,bHovered(false)
	{

	}

	MaterialRenderProxy::MaterialRenderProxy(bool bInSelected, bool bInHovered)
		:bSelected(bInSelected)
		,bHovered(bInHovered)
	{

	}

	MaterialRenderProxy::~MaterialRenderProxy()
	{
		if (isInitialized())
		{
			BOOST_ASSERT(isInRenderingThread());
			releaseResource();
		}
	}


	void ConstantExpressionSet::serialize(Archive& ar)
	{
		ar << mConstantVectorExpressions;
		ar << mConstantScalarExpressions;
		ar << mConstant2DTextureExpression;
		ar << mConstantCubeTextureExpressions;

		ar << mParameterCollections;

		ar << mPerFrameConstantScalarExpressions;
		ar << mPerFrameConstantVectorExpressions;
		ar << mPerFramePrevConstantScalarExpressions;
		ar << mPerFramePrevConstantVectorExpressions;
		if (ar.isLoading())
		{
			createBufferStruct();
		}
	}

	static ShaderConstantBufferParameter* constructMaterialCosntantBufferParameter()
	{
		return nullptr;
	}

	static wstring MaterialLayout(TEXT("Material"));

	void ConstantExpressionSet::createBufferStruct()
	{
		TArray<ConstantBufferStruct::Member> members;
		uint32 nextMemberOffset = 0;
		if (mConstantVectorExpressions.size())
		{
			new (members)ConstantBufferStruct::Member(TEXT("VectorExpressions"), TEXT(""), nextMemberOffset, CBMT_FLOAT32, EShaderPrecisionModifier::Half, 1, 4, mConstantVectorExpressions.size(), nullptr);
			const uint32 vectorArraySize = mConstantVectorExpressions.size() * sizeof(float4);
			nextMemberOffset += vectorArraySize;
		}
		if (mConstantScalarExpressions.size())
		{
			new(members)ConstantBufferStruct::Member(TEXT("ScalarExpressions"), TEXT(""), nextMemberOffset, CBMT_FLOAT32, EShaderPrecisionModifier::Half, 1, 4, (mConstantScalarExpressions.size() + 3) / 4, nullptr);
			const uint32 scalarArraySize = (mConstantScalarExpressions.size() + 3) / 4 * sizeof(float4);
			nextMemberOffset += scalarArraySize;
		}

		static wstring texture2DNames[128];
		static wstring texture2DSamplerNames[128];
		static wstring textureCubeNames[128];
		static wstring textureCubeSamplerNames[128];
		static bool bInitializedTextureNames = false;
		if (!bInitializedTextureNames)
		{
			bInitializedTextureNames = true;
			for (int32 i = 0; i < 128; ++i)
			{
				texture2DNames[i] = printf(TEXT("Texture2D_%d"), i);
				texture2DSamplerNames[i] = printf(TEXT("Texture2D_%dSampler"), i);
				textureCubeNames[i] = printf(TEXT("TextureCube_%d"), i);
				textureCubeSamplerNames[i] = printf(TEXT("TextureCube_%dSampler"), i);
			}
		}
		BOOST_ASSERT(mConstant2DTextureExpression.size() <= 128);
		BOOST_ASSERT(mConstantCubeTextureExpressions.size() < 128);
		for (int32 i = 0; i < mConstant2DTextureExpression.size(); i++)
		{
			BOOST_ASSERT((nextMemberOffset & 0x7) == 0);
			new(members)ConstantBufferStruct::Member(texture2DNames[i].c_str(), TEXT("Texture2D"), nextMemberOffset, CBMT_TEXTURE, EShaderPrecisionModifier::Float, 1, 1, 1, nullptr);
			nextMemberOffset += 8;
			new (members)ConstantBufferStruct::Member(texture2DSamplerNames[i].c_str(), TEXT("SamplerState"), nextMemberOffset, CBMT_SAMPLER, EShaderPrecisionModifier::Float, 1, 1, 1, nullptr);
			nextMemberOffset += 8;
		}

		for (int32 i = 0; i < mConstantCubeTextureExpressions.size(); ++i)
		{
			BOOST_ASSERT((nextMemberOffset & 0x7) == 0);
			new(members)ConstantBufferStruct::Member(textureCubeNames[i].c_str(), TEXT("TextureCube"), nextMemberOffset, CBMT_TEXTURE, EShaderPrecisionModifier::Float, 1, 1, 1, nullptr);
			nextMemberOffset += 8;
			new (members)ConstantBufferStruct::Member(textureCubeSamplerNames[i].c_str(), TEXT("SamplerState"), nextMemberOffset, CBMT_SAMPLER, EShaderPrecisionModifier::Float, 1, 1, 1, nullptr);
			nextMemberOffset += 8;
		}
		new (members)ConstantBufferStruct::Member(TEXT("Wrap_WorldGroupSettings"), TEXT("SamplerState"), nextMemberOffset, CBMT_SAMPLER, EShaderPrecisionModifier::Float, 1, 1, 1, nullptr);
		nextMemberOffset += 8;
		new (members)ConstantBufferStruct::Member(TEXT("Clamp_WorldGroupSettings"), TEXT("SamplerState"), nextMemberOffset, CBMT_SAMPLER, EShaderPrecisionModifier::Float, 1, 1, 1, nullptr);
		nextMemberOffset += 8;
		const uint32 structSize = align(nextMemberOffset, CONSTANT_BUFFER_STRUCT_ALIGNMENT);
		mConstantBufferStruct.emplace(MaterialLayout.c_str(), TEXT("MaterialConstants"), TEXT("Material"), constructMaterialCosntantBufferParameter, structSize, members, false);
	}

	bool ConstantExpressionSet::isEmpty() const
	{
		return mConstantVectorExpressions.size() == 0
			&& mConstantScalarExpressions.size() == 0
			&& mConstant2DTextureExpression.size() == 0
			&& mConstantCubeTextureExpressions.size() == 0
			&& mPerFrameConstantVectorExpressions.size() == 0
			&& mPerFrameConstantScalarExpressions.size() == 0
			&& mPerFramePrevConstantScalarExpressions.size() == 0
			&& mPerFramePrevConstantVectorExpressions.size() == 0
			&& mParameterCollections.size() == 0;
	}

	bool ConstantExpressionSet::operator ==(const ConstantExpressionSet& other) const
	{
		if (mConstantVectorExpressions.size() != other.mConstantVectorExpressions.size()
			|| mConstantScalarExpressions.size() != other.mConstantScalarExpressions.size()
			|| mConstant2DTextureExpression.size() != other.mConstant2DTextureExpression.size()
			|| mConstantCubeTextureExpressions.size() != other.mConstantCubeTextureExpressions.size()
			|| mPerFrameConstantScalarExpressions.size() != other.mPerFrameConstantScalarExpressions.size()
			|| mPerFrameConstantVectorExpressions.size() != other.mPerFrameConstantVectorExpressions.size()
			|| mPerFrameConstantScalarExpressions.size() != other.mPerFrameConstantScalarExpressions.size()
			|| mPerFramePrevConstantScalarExpressions.size() != other.mPerFramePrevConstantScalarExpressions.size()
			|| mPerFramePrevConstantVectorExpressions.size() != other.mPerFramePrevConstantScalarExpressions.size()
			|| mParameterCollections.size() != other.mParameterCollections.size()
			)
		{
			return false;
		}

		for (int32 i = 0; i < mConstantVectorExpressions.size(); i++)
		{
			if (!mConstantVectorExpressions[i]->isIdentical(other.mConstantVectorExpressions[i]))
			{
				return false;
			}
		}

		for (int i = 0 ; i < mConstantScalarExpressions.size(); i++)
		{
			if (!mConstantScalarExpressions[i]->isIdentical(other.mConstantScalarExpressions[i]))
			{
				return false;
			}
		}
		for (int32 i = 0; i < mConstant2DTextureExpression.size(); i++)
		{
			if (!mConstant2DTextureExpression[i]->isIdentical(other.mConstant2DTextureExpression[i]));
			{
				return false;
			}
		}
		for (int32 i = 0; i < mConstantCubeTextureExpressions.size(); i++)
		{
			if (!mConstantCubeTextureExpressions[i]->isIdentical(other.mConstantCubeTextureExpressions[i]))
			{
				return false;
			}
		}


		for (int32 i = 0; i < mPerFrameConstantScalarExpressions.size(); i++)
		{
			if (!mPerFrameConstantScalarExpressions[i]->isIdentical(other.mPerFrameConstantScalarExpressions[i]))
			{
				return false;
			}
		}

		for (int32 i = 0; i < mPerFrameConstantVectorExpressions.size(); i++)
		{
			if (!mPerFrameConstantVectorExpressions[i]->isIdentical(other.mPerFrameConstantVectorExpressions[i]))
			{
				return false;
			}
		}

		for (int32 i = 0; i < mPerFramePrevConstantScalarExpressions.size(); i++)
		{
			if (!mPerFramePrevConstantScalarExpressions[i]->isIdentical(other.mPerFramePrevConstantScalarExpressions[i]))
			{
				return false;
			}
		}

		for (int32 i = 0; i < mPerFramePrevConstantVectorExpressions.size(); i++)
		{
			if (!mPerFramePrevConstantVectorExpressions[i]->isIdentical(other.mPerFramePrevConstantVectorExpressions[i]))
			{
				return false;
			}
		}

		for (int32 i = 0; i < mParameterCollections.size(); i++)
		{
			if (mParameterCollections[i] != other.mParameterCollections[i])
			{
				return false;
			}
		}
		return true;
	}

	

	void MaterialConstantExpressionTexture::getTextureValue(const MaterialRenderContext& context, const FMaterial& material, std::shared_ptr<const RTexture>& outValue, ESamplerSourceMode& outSamplerSource) const
	{
		BOOST_ASSERT(isInParallelRenderingThread());
		outSamplerSource = mSamplerSource;
		if (mTransientOverrideValue_RenderThread)
		{
			outValue = mTransientOverrideValue_RenderThread;
		}
		else
		{
			outValue = getIndexedTexture(material, mTextureIndex);
		}
	}


	wstring ConstantExpressionSet::getSummaryString() const
	{
		return printf(TEXT("(%u vectors, %u scalars, %u 2d tex, %u cube tex, %u scalars/frame, %u vectors/frame, %u collections)"), mConstantVectorExpressions.size(),
			mConstantScalarExpressions.size(),
			mConstant2DTextureExpression.size(),
			mConstantCubeTextureExpressions.size(),
			mPerFrameConstantScalarExpressions.size(),
			mPerFrameConstantVectorExpressions.size(),
			mParameterCollections.size());
	}
	const ConstantBufferStruct& ConstantExpressionSet::getConstantBufferStruct() const
	{
		return mConstantBufferStruct.getValue();
	}

	ConstantBufferRHIRef ConstantExpressionSet::createConstantBuffer(const MaterialRenderContext& materialRenderContext, RHICommandList* commandListIfLocalMode, struct LocalConstantBuffer* outLocalConstantBuffer) const
	{
		BOOST_ASSERT(mConstantBufferStruct);
		BOOST_ASSERT(isInParallelRenderingThread());
		ConstantBufferRHIRef constantBuffer;
		if (mConstantBufferStruct->getSize() > 0)
		{
			MemMark mark(MemStack::get());
			void* const tempBuffer = MemStack::get().pushBytes(mConstantBufferStruct->getSize(), CONSTANT_BUFFER_STRUCT_ALIGNMENT);
			LinearColor* tempVectorBuffer = (LinearColor*)tempBuffer;
			for (int32 vectorIndex = 0; vectorIndex < mConstantVectorExpressions.size(); ++vectorIndex)
			{
				tempVectorBuffer[vectorIndex] = LinearColor(0, 0, 0, 0);
				mConstantVectorExpressions[vectorIndex]->getNumberValue(materialRenderContext, tempVectorBuffer[vectorIndex]);
			}
			float* temScalarBuffer = (float*)(tempVectorBuffer + mConstantVectorExpressions.size());
			for (int32 scalarIndex = 0; scalarIndex < mConstantScalarExpressions.size(); ++scalarIndex)
			{
				LinearColor vectorValue(0, 0, 0, 0);
				mConstantScalarExpressions[scalarIndex]->getNumberValue(materialRenderContext, vectorValue);
				temScalarBuffer[scalarIndex] = vectorValue.R;
			}

			void** resourceTable = (void**)((uint8*)tempBuffer + mConstantBufferStruct->getLayout().mResourceOffset);
			BOOST_ASSERT(((UPTRINT)resourceTable & 0x7) == 0);
			BOOST_ASSERT(mConstantBufferStruct->getLayout().mResource.size() == mConstant2DTextureExpression.size() * 2 + mConstantCubeTextureExpressions.size() * 2 + 2);

			for (int32 expressionIndex = 0; expressionIndex < mConstant2DTextureExpression.size(); expressionIndex++)
			{
				std::shared_ptr<const RTexture> value;
				ESamplerSourceMode sourceMode;
				mConstant2DTextureExpression[expressionIndex]->getTextureValue(materialRenderContext, materialRenderContext.mMaterial, value, sourceMode);
				if (value && value->mResource)
				{
					*resourceTable++ = value->mTextureReference.mTextureReferenceRHI;
					SamplerStateRHIRef* samplerSource = &value->mResource->mSamplerStateRHI;
					if (sourceMode == SSM_Wrap_WorldGroupSettings)
					{
						samplerSource = &Wrap_WorldGroupSettings->mSamplerStateRHI;
					}
					else if(sourceMode == SSM_Clamp_WorldGroupSettings)
					{
						samplerSource = &Clamp_WorldGroupSettings->mSamplerStateRHI;
					}
					*resourceTable++ = *samplerSource;
				}
				else
				{
					*resourceTable++ = GWhiteTexture->mTextureRHI;
					*resourceTable++ = GWhiteTexture->mSamplerStateRHI;
				}
			}
			for (int32 expressionIndex = 0; expressionIndex < mConstantCubeTextureExpressions.size(); expressionIndex++)
			{
				std::shared_ptr<const RTexture> value;
				ESamplerSourceMode sourceMode;
				mConstantCubeTextureExpressions[expressionIndex]->getTextureValue(materialRenderContext, materialRenderContext.mMaterial, value, sourceMode);
				if (value && value->mResource)
				{
					BOOST_ASSERT(value->mTextureReference.mTextureReferenceRHI);
					*resourceTable++ = value->mTextureReference.mTextureReferenceRHI;
					SamplerStateRHIRef* samplerSource = &value->mResource->mSamplerStateRHI;
					if (sourceMode == SSM_Wrap_WorldGroupSettings)
					{
						samplerSource = &Wrap_WorldGroupSettings->mSamplerStateRHI;
					}
					else if (sourceMode == SSM_Clamp_WorldGroupSettings)
					{
						samplerSource = &Clamp_WorldGroupSettings->mSamplerStateRHI;
					}
					*resourceTable++ = *samplerSource;
				}
				else
				{
					*resourceTable++ = GWhiteTexture->mTextureRHI;
					*resourceTable++ = GWhiteTexture->mSamplerStateRHI;
				}
			}
			*resourceTable++ = Wrap_WorldGroupSettings->mSamplerStateRHI;
			*resourceTable++ = Clamp_WorldGroupSettings->mSamplerStateRHI;
			if (commandListIfLocalMode)
			{
				BOOST_ASSERT(outLocalConstantBuffer);
				*outLocalConstantBuffer = commandListIfLocalMode->buildLocalConstantBuffer(tempBuffer, mConstantBufferStruct->getSize(), mConstantBufferStruct->getLayout());
				BOOST_ASSERT(outLocalConstantBuffer->isValid());
			}
			else
			{
				constantBuffer = RHICreateConstantBuffer(tempBuffer, mConstantBufferStruct->getLayout(), ConstantBuffer_MultiFrame);
				BOOST_ASSERT(!outLocalConstantBuffer->isValid());
			}
		}
		return constantBuffer;
	}

	void MaterialConstantExpressionTexture::serialize(Archive& ar)
	{
		int32 samplerSourceInt = (int32)mSamplerSource;
		ar << mTextureIndex << samplerSourceInt;
		mSamplerSource = (ESamplerSourceMode)samplerSourceInt;
	}

	void MaterialCompilationOutput::serialize(Archive& ar)
	{
		mConstantExpressionSet.serialize(ar);
		ar << bRequiresSceneColorCopy;
		ar << bNeedsSceneTextures;
		ar << bUsesEyeAdaptation;
		ar << bModifiesMeshPosition;
		ar << bUsesWorldPostionOffset;
		ar << bNeedsGBuffer;
		ar << bUsesGlobalDistanceField;
		ar << bUsesPixelDepthOffset;
		ar << bUsesSceneDepthLookup;
	}
	bool FMaterial::shouldCache(EShaderPlatform platform, const ShaderType* shaderType, const VertexFactoryType* vertexFactoryType) const
	{
		return true;
	}

	void FMaterial::releaseShaderMap()
	{
		if (mGameThreadShaderMap)
		{
			mGameThreadShaderMap = nullptr;
			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
				releaseShaderMap,
				FMaterial*, material, this,
				{
					material->setRenderingThreadShaderMap(nullptr);
				}
			);
		}
	}

	void FMaterial::setRenderingThreadShaderMap(MaterialShaderMap* inMaterialShaderMap)
	{
		BOOST_ASSERT(isInRenderingThread());
		mRenderingThreadShaderMap = inMaterialShaderMap;
	}

	void FMaterial::getDependentShaderAndVFTypes(EShaderPlatform platform, TArray<ShaderType*>& outShaderType, TArray<const ShaderPipelineType*>& outShaderPipelineTypes, TArray<VertexFactoryType*>& outVFTypes) const
	{
		const bool bHashTessellation = getTessellationMode() != MTM_NOTessellation;
		for (TLinkedList<VertexFactoryType*>::TIterator vertexFactoryTypeIt(VertexFactoryType::getTypeList()); vertexFactoryTypeIt; vertexFactoryTypeIt.next())
		{
			VertexFactoryType* vertexFactoryType = *vertexFactoryTypeIt;
			BOOST_ASSERT(vertexFactoryType);
			if (vertexFactoryType->isUsedWithMaterial())
			{
				bool bAddedTypeFromThisVF = false;
				for (TLinkedList<ShaderType*>::TIterator shaderTypeIt(ShaderType::getTypeList()); shaderTypeIt; shaderTypeIt.next())
				{
					MeshMaterialShaderType* shaderType = shaderTypeIt->getMeshMaterialShaderType();
					if (shaderType&& shaderType->shouldCache(platform, this, vertexFactoryType) && shouldCache(platform, shaderType, vertexFactoryType) && vertexFactoryType->shouldCache(platform, this, shaderType))
					{
						bAddedTypeFromThisVF = true;
						outShaderType.addUnique(shaderType);
					}
				}
				for (TLinkedList<ShaderPipelineType*>::TIterator pipelineTypeIt(ShaderPipelineType::getTypeList()); pipelineTypeIt; pipelineTypeIt.next())
				{
					auto* pipelineType = *pipelineTypeIt;
					if (pipelineType->isMeshMaterialTypePipeline() && pipelineType->hasTessellation() == bHashTessellation)
					{
						int32 numShouldCache = 0;
						auto& shaderStages = pipelineType->getStages();
						for (const ShaderType* type : shaderStages)
						{
							const MeshMaterialShaderType* shaderType = type->getMeshMaterialShaderType();
							if (shaderType->shouldCache(platform, this, vertexFactoryType) && shouldCache(platform, shaderType, vertexFactoryType) && vertexFactoryType->shouldCache(platform, this, shaderType))
							{
								++numShouldCache;
							}
						}
						if (numShouldCache == shaderStages.size())
						{
							bAddedTypeFromThisVF = true;
							outShaderPipelineTypes.addUnique(pipelineType);
							for (const ShaderType* type : shaderStages)
							{
								outShaderType.addUnique((ShaderType*)type);
							}
						}
					}
				}
				if (bAddedTypeFromThisVF)
				{
					outVFTypes.add(vertexFactoryType);
				}
			}
		}
		for (TLinkedList<ShaderType*>::TIterator shaderTypeIt(ShaderType::getTypeList()); shaderTypeIt; shaderTypeIt.next())
		{
			MaterialShaderType* shaderType = shaderTypeIt->getMaterialShaderType();
			if (shaderType && shaderType->shouldCache(platform, this) && shouldCache(platform, shaderType, nullptr))
			{
				outShaderType.add(shaderType);
			}
		}

		for (TLinkedList<ShaderPipelineType*>::TIterator pipelineTypeIt(ShaderPipelineType::getTypeList()); pipelineTypeIt; pipelineTypeIt.next())
		{
			auto* pipelineType = *pipelineTypeIt;
			if (pipelineType->isMaterialTypePipeline() && pipelineType->hasTessellation() == bHashTessellation)
			{
				int32 numShouldCache = 0;
				auto& shaderStages = pipelineType->getStages();
				for (const ShaderType* type : shaderStages)
				{
					const MaterialShaderType* shaderType = type->getMaterialShaderType();
					if (shaderType && shaderType->shouldCache(platform, this) && shouldCache(platform, shaderType, nullptr))
					{
						++numShouldCache;
					}
				}
				if (numShouldCache == shaderStages.size())
				{
					for (const ShaderType* type : shaderStages)
					{
						const MaterialShaderType* shaderType = type->getMaterialShaderType();
						outShaderPipelineTypes.add(pipelineType);
						outShaderType.add((ShaderType*)type);
					}
				}
			}
		}
		outShaderType.sort(CompareShaderTypes());

		outVFTypes.sort(CompareVertexFactoryTypes());

		outShaderPipelineTypes.sort(CompareShaderPipelineType());
	}

	bool FMaterial::beginCompileShaderMap(const MaterialShaderMapId& shaderMapId, EShaderPlatform platform, TRefCountPtr<class MaterialShaderMap>& outShaderMap, bool bApplyCompletedShaderMapForRendering)
	{
		bool bSuccess = false;
		TRefCountPtr<MaterialShaderMap> newShaderMap = new MaterialShaderMap();
		MaterialCompilationOutput newComilationOutput;
		XMLMaterialTranslator materialTranslator(this, newComilationOutput, shaderMapId.mParameterSet, platform, mQualityLevel, shaderMapId.mFeatureLevel);

		bSuccess = materialTranslator.translate();

		if (bSuccess)
		{
			TRefCountPtr<ShaderCompilerEnvironment> materialEnvironment = new ShaderCompilerEnvironment();
			materialTranslator.getMaterialEnvironment(platform, *materialEnvironment);

			const wstring materialShaderCode =  materialTranslator.getMaterialShaderCode();
			const bool bSynchronousCompile = requiresSynchronousCompilation() || !GShaderCompilingManager->allowAsynchronousShaderCompiling() || isDefaultMaterial();
			materialEnvironment->mIncludeFileNameToContentsMap.emplace(TEXT("Material.hlsl"), stringToArray<ANSICHAR>(materialShaderCode.c_str(), materialShaderCode.length() + 1));
			newShaderMap->compile(this, shaderMapId, materialEnvironment, newComilationOutput, platform, bSynchronousCompile, bApplyCompletedShaderMapForRendering);
			if (bSynchronousCompile)
			{
				outShaderMap = newShaderMap->compiledSuccessfully() ? newShaderMap : nullptr;
			}
			else
			{
				mOutstandingCompileShaderMapIds.addUnique(newShaderMap->getCompilingId());
				outShaderMap = nullptr;
			}
		}
		return bSuccess;
	}

	bool FMaterial::cacheShaders(EShaderPlatform platform, bool bApplyCompletedShaderMapForRendering)
	{
		MaterialShaderMapId noStaticParametersId;
		getShaderMapId(platform, noStaticParametersId);
		return cacheShaders(noStaticParametersId, platform, bApplyCompletedShaderMapForRendering);
	}

	bool FMaterial::cacheShaders(const MaterialShaderMapId& shaderMapId, EShaderPlatform platform, bool bApplyCompletedShaderMapForRendering)
	{
		bool bSucceeded = false;
		BOOST_ASSERT(shaderMapId.mBaseMaterialId.isValid());
		if (bContainsInlineShaders)
		{
			MaterialShaderMap* existingShaderMap = nullptr;
			if (mGameThreadShaderMap)
			{
				existingShaderMap = MaterialShaderMap::findId(mGameThreadShaderMap->getShaderMapId(), platform);


			}

			if (existingShaderMap)
			{
				mGameThreadShaderMap = existingShaderMap;
			}
			else if (mGameThreadShaderMap)
			{
				mGameThreadShaderMap->Register(platform);
			}
		}
		else
		{
			mGameThreadShaderMap = MaterialShaderMap::findId(shaderMapId, platform);
			if ((!mGameThreadShaderMap || !mGameThreadShaderMap->isComplete(this, true)) && !PlatformProperties::requiresCookedData())
			{
				//MaterialShaderMap::loadFromDerivedDataCache(this, shaderMapId, platform, mGameThreadShaderMap);
			}
		}
		const bool bLogShaderMapFailInfo = false;
		bool bAssumeShaderMapIsComplete = false;

		if (mGameThreadShaderMap && mGameThreadShaderMap->tryToAddToExistingCompilationTask(this))
		{
			mOutstandingCompileShaderMapIds.addUnique(mGameThreadShaderMap->getCompilingId());
			mGameThreadShaderMap = nullptr;
			bSucceeded = true;
		}
		else if (!mGameThreadShaderMap || !(bAssumeShaderMapIsComplete || mGameThreadShaderMap->isComplete(this, !bLogShaderMapFailInfo)))
		{
			if (bContainsInlineShaders || PlatformProperties::requiresCookedData())
			{
				mGameThreadShaderMap = nullptr;
			}
			else
			{
				bSucceeded = beginCompileShaderMap(shaderMapId, platform, mGameThreadShaderMap, bApplyCompletedShaderMapForRendering);
				if (!bSucceeded)
				{
					mGameThreadShaderMap = nullptr;
					if (isDefaultMaterial())
					{
						for (int32 errorIndex = 0; errorIndex, mCompileErrors.size(); errorIndex++)
						{
							AIR_LOG(logMateiral, Warning, TEXT("%s"), mCompileErrors[errorIndex].c_str());
						}
					}
				}
			}
		}
		else
		{
			bSucceeded = true;
		}
		mRenderingThreadShaderMap = mGameThreadShaderMap;
		return bSucceeded;
	}

	Shader* FMaterial::getShader(MeshMaterialShaderType* shaderType, VertexFactoryType* vertexFactoryType) const
	{
		const MeshMaterialShaderMap* meshShaderMap = mRenderingThreadShaderMap->getMeshShaderMap(vertexFactoryType);
		Shader* shader = meshShaderMap ? meshShaderMap->getShader(shaderType) : nullptr;
		if (!shader)
		{
			auto shaderPlatform = GShaderPlatformForFeatureLevel[getFeatureLevel()];
			bool bMaterialShouldCache = shouldCache(shaderPlatform, shaderType, vertexFactoryType);
			bool bVFShouldCache = vertexFactoryType->shouldCache(shaderPlatform, this, shaderType);
			bool bShaderShouldCache = shaderType->shouldCache(shaderPlatform, this, vertexFactoryType);
		}
		return shader;
	}

	MaterialRenderContext::MaterialRenderContext(const MaterialRenderProxy* inMaterialProxy, const FMaterial& inMaterial, const SceneView* inView)
		:mMaterialRenderProxy(inMaterialProxy)
		,mMaterial(inMaterial)
		,mTime(0.0f)
		,mRealTime(0.0f)
	{
		bShowSelection = false;
		
	}

	void MaterialRenderProxy::evaluateConstantExpressions(ConstantExpressionCache& outConstantExpressionCache, const MaterialRenderContext& context, class RHICommandList* commandListIfLocalMode /* = nullptr */) const
	{
		BOOST_ASSERT(isInParallelRenderingThread());
		const ConstantExpressionSet& constantExpressionSet = context.mMaterial.getRenderingThreadShaderMap()->getConstantExpressionSet();
		outConstantExpressionCache.mCachedConstantExpressionShaderMap = context.mMaterial.getRenderingThreadShaderMap();
		outConstantExpressionCache.mConstantBuffer = constantExpressionSet.createConstantBuffer(context, commandListIfLocalMode, &outConstantExpressionCache.mLocalConstantBuffer);
		outConstantExpressionCache.mParameterCollections = constantExpressionSet.mParameterCollections;
		outConstantExpressionCache.bUpToData = true;
	}

	const MeshMaterialShaderMap* MaterialShaderMap::getMeshShaderMap(VertexFactoryType* vertexFactoryType) const
	{
		BOOST_ASSERT(bCompilationFinalized);
		const MeshMaterialShaderMap* meshShaderMap = mOrderredMeshShaderMaps[vertexFactoryType->getId()];
		BOOST_ASSERT(!meshShaderMap || meshShaderMap->getVertexFactoryType() == vertexFactoryType);
		return meshShaderMap;
	}

	EBlendMode MaterialResource::getBlendMode() const
	{
		return mMaterialInstance ? mMaterialInstance->getBlendMode() : mMaterial->getBlendMode();
	}

	bool MaterialResource::isDefaultMaterial() const
	{
		return mMaterial->isDefaultMaterial();
	}

	float MaterialResource::getOpacityMaskClipValue() const
	{
		return mMaterialInstance ? mMaterialInstance->getOpacityMaskClipValue() : mMaterial->getOpacityMaskClipValue();
	}

	EMaterialShadingModel MaterialResource::getShadingModel() const
	{
		return mMaterialInstance ? mMaterialInstance->getShadingModel() : mMaterial->getShadingModel();
	}

	bool MaterialResource::isWireFrame() const
	{
		return mMaterial->bWireFrame;
	}

	bool MaterialResource::isTwoSided() const
	{
		return mMaterial->isTwoSided();
	}

	bool MaterialResource::isDeferredDecal() const
	{
		return mMaterial->mMaterialDomain == MD_DeferredDecal;
	}

	const TArray<std::shared_ptr<RTexture>>& MaterialResource::getReferencedTextures() const
	{
		return mMaterial ? mMaterial->mExpressionTextureReferences : RMaterial::getDefaultMaterial(MD_Surface)->mExpressionTextureReferences;
	}

	bool MaterialResource::isMasked() const
	{
		return mMaterialInstance ? mMaterialInstance->isMasked() : mMaterial->isMasked();
	}

	static inline bool shouldCacheMeshShader(const MeshMaterialShaderType* shaderType, EShaderPlatform platform, const FMaterial* material, VertexFactoryType* inVertexFactoryType)
	{
		return shaderType->shouldCache(platform, material, inVertexFactoryType) && material->shouldCache(platform, shaderType, inVertexFactoryType) && inVertexFactoryType->shouldCache(platform, material, shaderType);
	}

	bool MeshMaterialShaderMap::isComplete(const MeshMaterialShaderMap* meshShaderMap, EShaderPlatform platform, const FMaterial* material, VertexFactoryType* inVertexFactoryType, bool bSilent)
	{
		for (TLinkedList<ShaderType*>::TIterator shaderTypeIt(ShaderType::getTypeList()); shaderTypeIt; shaderTypeIt.next())
		{
			MeshMaterialShaderType* shaderType = shaderTypeIt->getMeshMaterialShaderType();
			if (shaderType && !isMeshShaderComplete(meshShaderMap, platform, material, shaderType, nullptr, inVertexFactoryType, bSilent))
			{
				return false;
			}
		}

		const bool bHasTessellation = material->getTessellationMode() != MTM_NOTessellation;
		for (TLinkedList<ShaderPipelineType*>::TIterator shaderPipelineIt(ShaderPipelineType::getTypeList()); shaderPipelineIt; shaderPipelineIt.next())
		{
			const ShaderPipelineType* shaderPipelineType = *shaderPipelineIt;
			if (shaderPipelineType->isMeshMaterialTypePipeline() && shaderPipelineType->hasTessellation() == bHasTessellation)
			{
				auto& stages = shaderPipelineType->getStages();
				int32 numShouldCache = 0;
				for (int32 index = 0; index < stages.size(); ++index)
				{
					auto* shaderType = stages[index]->getMeshMaterialShaderType();
					if (shouldCacheMeshShader(shaderType, platform, material, inVertexFactoryType))
					{
						++numShouldCache;
					}
					else
					{
						break;
					}
				}
				if (numShouldCache == stages.size())
				{
					for (int32 index = 0; index < stages.size(); ++index)
					{
						auto* shaderType = stages[index]->getMeshMaterialShaderType();
						if (shaderType && !isMeshShaderComplete(meshShaderMap, platform, material, shaderType, shaderPipelineType, inVertexFactoryType, bSilent))
						{
							return false;
						}
					}
				}
			}
		}
		return false;
	}

	inline bool MeshMaterialShaderMap::isMeshShaderComplete(const MeshMaterialShaderMap* meshShaderMap, EShaderPlatform platform, const FMaterial* material, const MeshMaterialShaderType* shaderType, const ShaderPipelineType* pipeline, VertexFactoryType* inVertexFactoryType, bool bSilent)
	{
		if (shouldCacheMeshShader(shaderType, platform, material, inVertexFactoryType) && (!meshShaderMap || (pipeline && !meshShaderMap->hasShaderPipeline(pipeline)) || (!pipeline && !meshShaderMap->hasShader((ShaderType*)shaderType))))
		{
			if (!bSilent)
			{
				if (pipeline)
				{

				}
				else
				{

				}
			}
			return false;
		}
		return true;
	}

	bool MaterialResource::isLightFunction() const
	{
		return mMaterial->mMaterialDomain == MD_LightFunction;
	}

	bool MaterialShaderMap::isMaterialShaderComplete(const FMaterial* material, const MaterialShaderType* shaderType, const ShaderPipelineType* pipelineType, bool bSilent)
	{
		if (shouldCacheMaterialShader(shaderType, mPlatform, material) && ((pipelineType && !hasShaderPipeline(pipelineType)) || (!pipelineType && !hasShader((ShaderType*)shaderType))))
		{
			if (!bSilent)
			{
				if (pipelineType)
				{

				}
				else
				{

				}
			}
			return false;
		}
		return true;
	}

	bool MaterialShaderMap::tryToAddToExistingCompilationTask(FMaterial* material)
	{
		BOOST_ASSERT(mNumRef > 0);
		auto correspondingMaterials = MaterialShaderMap::mShaderMapsBeingCompiled.find(this);
		if (correspondingMaterials == MaterialShaderMap::mShaderMapsBeingCompiled.end())
		{
			correspondingMaterials->second.addUnique(material);
			return true;
		}
		return false;
	}

	uint32 MaterialShaderMap::mNextCompilingId = 2;

	void MaterialShaderMap::compile(FMaterial* material, const MaterialShaderMapId& shaderMapId, TRefCountPtr<ShaderCompilerEnvironment> materialEnvironment, const MaterialCompilationOutput& inMaterialCompilationOutput, EShaderPlatform platform, bool bSynchronousCompile, bool bApplyCompletedShaderMapForRendering)
	{
		if (PlatformProperties::requiresCookedData())
		{

		}
		else
		{
			BOOST_ASSERT(!material->bContainsInlineShaders);
			BOOST_ASSERT(mNumRef > 0);

			auto correspondingMaterials = mShaderMapsBeingCompiled.find(this);
			if (correspondingMaterials != mShaderMapsBeingCompiled.end())
			{
				BOOST_ASSERT(!bSynchronousCompile);
				correspondingMaterials->second.addUnique(material);
			}
			else
			{
				mCompilingId = mNextCompilingId;
				BOOST_ASSERT(mNextCompilingId < UINT_MAX);
				mNextCompilingId++;
				TArray<FMaterial*> newCorrespondingMaterials;
				newCorrespondingMaterials.add(material);
				mShaderMapsBeingCompiled.emplace(this, newCorrespondingMaterials);
				material->setupMaterialEnvironment(platform, inMaterialCompilationOutput.mConstantExpressionSet, *materialEnvironment);

				mFriendlyName = material->getFriendlyName();
				mMaterialCompilationOutput = inMaterialCompilationOutput;
				mShaderMapId = shaderMapId;
				mPlatform = platform;
				bIsPersistent = material->isPersistent();

				const wstring materialUsage = TEXT("");
			/*	for (int32 staticSwitchIndex = 0; staticSwitchIndex < shaderMapId.mParameterSet.mStaticSwitchParameters.size(); staticSwitchIndex++)
				{
					const StaticSwitchParameter& staticSwitchParameter = shaderMapId.mParameterSet.mStaticSwitchParameters[staticSwitchIndex];

				}*/

				uint32 numShaders = 0; 
				uint32 numVertexFactories = 0;

				TArray<ShaderCommonCompileJob*> newJobs;
				for (TLinkedList<VertexFactoryType*>::TIterator vertexFactoryTypeIt(VertexFactoryType::getTypeList()); vertexFactoryTypeIt; vertexFactoryTypeIt.next())
				{
					VertexFactoryType* vertexFactoryType = *vertexFactoryTypeIt;
					BOOST_ASSERT(vertexFactoryType);
					if (vertexFactoryType->isUsedWithMaterial())
					{
						MeshMaterialShaderMap* meshShaderMap = nullptr;
						int32 meshShaderMapIndex = INDEX_NONE;
						for (int32 shaderMapIndex = 0; shaderMapIndex < mMeshShaderMaps.size(); shaderMapIndex++)
						{
							if (mMeshShaderMaps[shaderMapIndex].getVertexFactoryType() == vertexFactoryType)
							{
								meshShaderMap = &mMeshShaderMaps[shaderMapIndex];
								meshShaderMapIndex = shaderMapIndex;
								break;
							}
						}
						if (meshShaderMap == nullptr)
						{
							meshShaderMapIndex = mMeshShaderMaps.size();
							meshShaderMap = new(mMeshShaderMaps) MeshMaterialShaderMap(vertexFactoryType);

						}
						const uint32 meshShaders = meshShaderMap->beginCompile(mCompilingId, shaderMapId, material, materialEnvironment, platform, newJobs);
						numShaders += meshShaders;
						if (meshShaders > 0)
						{
							numVertexFactories++;
						}
					}
				}
				TMap<ShaderType*, ShaderCompileJob*> sharedShaderJobs;
				for (TLinkedList<ShaderType*>::TIterator shaderTypeIt(ShaderType::getTypeList()); shaderTypeIt; shaderTypeIt.next())
				{
					MaterialShaderType* shaderType = shaderTypeIt->getMaterialShaderType();
					if (shaderType && shouldCacheMaterialShader(shaderType, platform, material))
					{
						BOOST_ASSERT(shaderMapId.containsShaderType(shaderType));
						TArray<wstring> shaderErrors;
						if (!hasShader(shaderType))
						{
							auto* job = shaderType->beginCompileShader(mCompilingId,
								material,
								materialEnvironment,
								nullptr,
								platform,
								newJobs);
							BOOST_ASSERT(sharedShaderJobs.find(shaderType) == sharedShaderJobs.end());
							sharedShaderJobs.emplace(shaderType, job);
						}
						numShaders++;
					}
				}
				const bool bHasTessellation = material->getTessellationMode() != MTM_NOTessellation;
				for (TLinkedList<ShaderPipelineType*>::TIterator shaderPipelineIt(ShaderPipelineType::getTypeList()); shaderPipelineIt; shaderPipelineIt.next())
				{
					const ShaderPipelineType* pipeline = *shaderPipelineIt;
					if (pipeline->isMaterialTypePipeline() && pipeline->hasTessellation() == bHasTessellation)
					{
						auto& stageTypes = pipeline->getStages();
						TArray<MaterialShaderType*> shaderStagesToCompile;
						for (int32 index = 0; index < stageTypes.size(); ++index)
						{
							MaterialShaderType* shaderType = (MaterialShaderType*)(stageTypes[index]->getMaterialShaderType());
							if (shaderType && shouldCacheMaterialShader(shaderType, platform, material))
							{
								BOOST_ASSERT(shaderMapId.containsShaderType(shaderType));
								shaderStagesToCompile.add(shaderType);
							}
							else
							{
								break;
							}
						}
						if (shaderStagesToCompile.size() == stageTypes.size())
						{
							BOOST_ASSERT(shaderMapId.containsShaderPipelineType(pipeline));
							if (pipeline->shoudlOptimizeUnusedOutputs())
							{
								numShaders += shaderStagesToCompile.size();
								MaterialShaderType::beginCompileShaderPipeline(mCompilingId, platform, material, materialEnvironment, pipeline, shaderStagesToCompile, newJobs);

							}
							else
							{
								for (const ShaderType* shaderType : stageTypes)
								{
									auto jobIt = sharedShaderJobs.find(const_cast<ShaderType*>(shaderType));
									ShaderCompileJob** job = &jobIt->second;
									BOOST_ASSERT(job);
									auto* singleJob = (*job)->getSingleShaderJob();
									auto& pipelinesToShare = singleJob->mSharingPipelines.findOrAdd(nullptr);
									BOOST_ASSERT(!pipelinesToShare.contains(pipeline));
									pipelinesToShare.add(pipeline);
								}
							}
						}
					}
				}
				if (correspondingMaterials == mShaderMapsBeingCompiled.end())
				{

				}
				Register(platform);
				bCompilationFinalized = false;
				bCompiledSuccessfully = false;
				const bool bRecreateComponentRenderStateOnCompletion = material->isPersistent();
				GShaderCompilingManager->addJobs(newJobs, bApplyCompletedShaderMapForRendering && !bSynchronousCompile, bSynchronousCompile || !material->isPersistent(), bRecreateComponentRenderStateOnCompletion);
				if (bSynchronousCompile)
				{
					TArray<int32> currentShaderMapId;
					currentShaderMapId.add(mCompilingId);
					GShaderCompilingManager->finishCompilation(mFriendlyName.c_str(), currentShaderMapId);
				}
			}
		}
	}

	bool MaterialShaderMapId::containsShaderType(const ShaderType* shaderType) const
	{
		for (int32 typeIndex = 0; typeIndex < mShaderTypeDependencies.size(); ++typeIndex)
		{
			if (mShaderTypeDependencies[typeIndex].mShaderType == shaderType)
			{
				return true;
			}
		}
		return false;
	}

	bool MaterialShaderMapId::containsShaderPipelineType(const ShaderPipelineType* shaderPipelineType) const
	{
		for (int32 typeIndex = 0; typeIndex < mShaderPipelineTypeDependencies.size(); ++typeIndex)
		{
			if (mShaderPipelineTypeDependencies[typeIndex].mShaderPipelineType == shaderPipelineType)
			{
				return true;
			}
		}
		return false;
	}

	bool MaterialShaderMapId::containsVertexFactoryType(const VertexFactoryType* vfType) const
	{
		for (int32 typeIndex = 0; typeIndex < mVertexFactoryTypeDependencies.size(); typeIndex++)
		{
			if (mVertexFactoryTypeDependencies[typeIndex].mVertexFactoryType == vfType)
			{
				return true;
			}
		}
		return false;
	}

	wstring MaterialResource::getFriendlyName() const { return mMaterial->getName(); }

	bool MaterialResource::isSpecialEngineMaterial() const { return mMaterial->bUsedAsSpecialEngineMaterial; }


	void MaterialResource::compilePropertyAndSetMaterialProperty(wstring** chunk, class MaterialCompiler* compiler, EShaderFrequency overrideShaderFrequency /* = SF_NumFrequencies */, bool bUsePreviousFrameTime /* = false */) const
	{
		MaterialInterface* materialInterface = mMaterialInstance ? static_cast<MaterialInterface*>(mMaterialInstance.get()) : static_cast<MaterialInterface*>(mMaterial.get());

		materialInterface->compileProperty(compiler, chunk);
	}

	void MaterialResource::setMaterial(class RMaterial* inMaterial, EMaterialQualityLevel::Type inQualityLevel, bool bInQualityLevelHasDifferentNodes, ERHIFeatureLevel::Type inFeatureLevel, std::shared_ptr<MaterialInstance> instance)
	{
		mMaterial = std::dynamic_pointer_cast<RMaterial>(inMaterial->shared_from_this());
		mMaterialInstance = instance;
		setQualityLevelProperties(inQualityLevel, bInQualityLevelHasDifferentNodes, inFeatureLevel);
	}

	void FMaterial::setupMaterialEnvironment(EShaderPlatform platform, const ConstantExpressionSet& inConstantExpressionSet, ShaderCompilerEnvironment& outEnvironment) const
	{
		ShaderConstantBufferParameter::modifyCompilationEnvironment(TEXT("Material"), inConstantExpressionSet.getConstantBufferStruct(), platform, outEnvironment);

		switch (getShadingModel())
		{
		case MSM_Unlit: outEnvironment.setDefine(TEXT("MATERIAL_SHADINGMODEL_UNLIT"), TEXT("1")); break;
		case MSM_DefaultLit: outEnvironment.setDefine(TEXT("MATERIAL_SHADINGMODEL_DEFAULT_LIT"), TEXT("1")); break;
		default:
			outEnvironment.setDefine(TEXT("MATERIAL_SHADINGMODEL_DEFAULT_LIT"), TEXT("1"));
			break;
		}

		switch (getBlendMode())
		{
		case BLEND_Opaque:
		case BLEND_Masked:
		{
			outEnvironment.setDefine(TEXT("MATERIALBLENDING_SOLID"), TEXT("1"));
			break;
		}
		case BLEND_AlphaComposite:
		{
			outEnvironment.setDefine(TEXT("MATERIALBLENDING_ALPHACOMPOSITE"), TEXT("1"));
		}
		case BLEND_Translucent: outEnvironment.setDefine(TEXT("MATERIALBLENDING_TRANSLUCENT"), TEXT("1")); break;
		case BLEND_Additive: outEnvironment.setDefine(TEXT("MATERIALBLENDING_ADDITIVE"), TEXT("1")); break;
		case BLEND_Modulate: outEnvironment.setDefine(TEXT("MATERIALBLENDING_MODULATE"), TEXT("1"));
		default:
			outEnvironment.setDefine(TEXT("MATERIALBLENDING_SOLID"), TEXT("1"));
			break;
		}

		outEnvironment.setDefine(TEXT("MATERIAL_TANGENTSPACENORMAL"), true);
	}

	bool MaterialShaderMapId::operator == (const MaterialShaderMapId& referenceSet) const
	{
		if (mUsage != referenceSet.mUsage)
		{
			return false;
		}

		if (mBaseMaterialId != referenceSet.mBaseMaterialId ||
			mQualityLevel != referenceSet.mQualityLevel ||
			mFeatureLevel != referenceSet.mFeatureLevel)
		{
			return false;
		}

		if (mParameterSet != referenceSet.mParameterSet ||
			mReferencedFunctions.size() != referenceSet.mReferencedFunctions.size() ||
			mReferencedParameterCollections.size() != referenceSet.mReferencedParameterCollections.size() ||
			mShaderTypeDependencies.size() != referenceSet.mShaderTypeDependencies.size() ||
			mShaderPipelineTypeDependencies.size() != referenceSet.mShaderPipelineTypeDependencies.size() ||
			mVertexFactoryTypeDependencies.size() != referenceSet.mVertexFactoryTypeDependencies.size())
		{
			return false;
		}

		for (int32 refFunctionIndex = 0; refFunctionIndex < referenceSet.mReferencedFunctions.size(); refFunctionIndex++)
		{
			const Guid& referenceGuid = referenceSet.mReferencedFunctions[refFunctionIndex];
			if (mReferencedFunctions[refFunctionIndex] != referenceGuid)
			{
				return false;
			}
		}

		for (int32 index = 0; index < referenceSet.mReferencedParameterCollections.size(); index++)
		{
			const Guid& referenceGuid = referenceSet.mReferencedParameterCollections[index];
			if (mReferencedParameterCollections[index] != referenceGuid)
			{
				return false;
			}
		}

		for (int32 index = 0; index < mShaderTypeDependencies.size(); index++)
		{
			const ShaderTypeDependency& shaderTypedependency = mShaderTypeDependencies[index];
			if (shaderTypedependency.mShaderType != referenceSet.mShaderTypeDependencies[index].mShaderType || shaderTypedependency.mSourceHash != referenceSet.mShaderTypeDependencies[index].mSourceHash)
			{
				return false;
			}
		}

		for (int32 shaderPipelineIndex = 0; shaderPipelineIndex < mShaderPipelineTypeDependencies.size(); shaderPipelineIndex++)
		{
			const ShaderPipelineTypeDependency& shaderPipelineTypeDependency = mShaderPipelineTypeDependencies[shaderPipelineIndex];
			if (shaderPipelineTypeDependency.mShaderPipelineType != referenceSet.mShaderPipelineTypeDependencies[shaderPipelineIndex].mShaderPipelineType || shaderPipelineTypeDependency.mStagesSourceHash != referenceSet.mShaderPipelineTypeDependencies[shaderPipelineIndex].mStagesSourceHash)
			{
				return false;
			}

		}

		for (int32 vfIndex = 0; vfIndex < mVertexFactoryTypeDependencies.size(); vfIndex++)
		{
			const VertexFactoryTypeDependency& vfdependency = mVertexFactoryTypeDependencies[vfIndex];
			if (vfdependency.mVertexFactoryType != referenceSet.mVertexFactoryTypeDependencies[vfIndex].mVertexFactoryType || vfdependency.mVFSourceHash != referenceSet.mVertexFactoryTypeDependencies[vfIndex].mVFSourceHash)
			{
				return false;
			}
		}
		if (mTextureReferencesHash != referenceSet.mTextureReferencesHash)
		{
			return false;
		}

		if (mBasePropertyOverridesHash != referenceSet.mBasePropertyOverridesHash)
		{
			return false;
		}
		return true;
	}


	bool MaterialShaderMap::processCompilationResults(const TArray<ShaderCommonCompileJob*>& inCompilationResults, int32 & resultIndex, float& timeBudget, TMap<const VertexFactoryType*, TArray<const ShaderPipelineType*>>& shaderPipelines)
	{
		BOOST_ASSERT(resultIndex < inCompilationResults.size());
		double startTime = PlatformTime::seconds();
		SHAHash materialShaderMapHash;
		mShaderMapId.getMaterialHash(materialShaderMapHash);
		do
		{
			ShaderCompileJob* singleJob = inCompilationResults[resultIndex]->getSingleShaderJob();
			if (singleJob)
			{
				processCompilationResultsForSingleJob(singleJob, nullptr, materialShaderMapHash);
				for (auto& pair : singleJob->mSharingPipelines)
				{
					auto& sharedPipelinesPerVf = shaderPipelines.findOrAdd(singleJob->mVFType);
					for (auto* pipeline : pair.second)
					{
						sharedPipelinesPerVf.addUnique(pipeline);
					}
				}
			}
			else
			{
				auto* pipelineJob = inCompilationResults[resultIndex]->getShaderPipelineJob();
				BOOST_ASSERT(pipelineJob);
				const ShaderPipelineCompileJob& currentJob = *pipelineJob;
				BOOST_ASSERT(currentJob.mId == mCompilingId);
				TArray<Shader*> shaderStages;
				VertexFactoryType* vertexFactoryType = currentJob.mStageJobs[0]->getSingleShaderJob()->mVFType;
				for (int32 index = 0; index < currentJob.mStageJobs.size(); ++index)
				{
					singleJob = currentJob.mStageJobs[index]->getSingleShaderJob();
					Shader* shader = processCompilationResultsForSingleJob(singleJob, pipelineJob->mShaderPipeline, materialShaderMapHash);
					shaderStages.add(shader);
					BOOST_ASSERT(vertexFactoryType == currentJob.mStageJobs[index]->getSingleShaderJob()->mVFType);
				}
				ShaderPipeline* shaderPipeline = new ShaderPipeline(pipelineJob->mShaderPipeline, shaderStages);
				if (shaderPipeline)
				{
					if (vertexFactoryType)
					{
						BOOST_ASSERT(vertexFactoryType->isUsedWithMaterial());
						MeshMaterialShaderMap* meshShaderMap = nullptr;
						int32 meshShaderMapIndex = INDEX_NONE;
						for (int32 shaderMapIndex = 0; shaderMapIndex < mMeshShaderMaps.size(); shaderMapIndex++)
						{
							if (mMeshShaderMaps[shaderMapIndex].getVertexFactoryType() == vertexFactoryType)
							{
								meshShaderMap = &mMeshShaderMaps[shaderMapIndex];
								meshShaderMapIndex = shaderMapIndex;
								break;
							}
						}
						BOOST_ASSERT(meshShaderMap);
						BOOST_ASSERT(!meshShaderMap->hasShaderPipeline(shaderPipeline->mPipelineType));
						meshShaderMap->addShaderPipeline(pipelineJob->mShaderPipeline, shaderPipeline);
					}
					else
					{
						BOOST_ASSERT(!hasShaderPipeline(shaderPipeline->mPipelineType));
						addShaderPipeline(pipelineJob->mShaderPipeline, shaderPipeline);
					}
				}
			}
			resultIndex++;
			double newStartTime = PlatformTime::seconds();
			timeBudget -= newStartTime - startTime;

		} while ((timeBudget > 0.0f) && (resultIndex < inCompilationResults.size()));
		if (resultIndex == inCompilationResults.size())
		{
			{
				for (int32 shaderMapIndex = 0; shaderMapIndex < mMeshShaderMaps.size(); shaderMapIndex++)
				{
					auto* meshShaderMap = &mMeshShaderMaps[shaderMapIndex];
					auto * vertexFactory = meshShaderMap->getVertexFactoryType();
					auto foundSharedPipelines = shaderPipelines.find(vertexFactory);
					if (vertexFactory && foundSharedPipelines != shaderPipelines.end())
					{
						for (const ShaderPipelineType* shaderPipelineType : foundSharedPipelines->second)
						{
							if (shaderPipelineType->isMeshMaterialTypePipeline() && !meshShaderMap->hasShaderPipeline(shaderPipelineType))
							{
								auto& stageTypes = shaderPipelineType->getStages();
								TArray<Shader*> shaderStages;
								for (int32 index = 0; index < stageTypes.size(); ++index)
								{
									MeshMaterialShaderType* shaderType = ((MeshMaterialShaderType*)(stageTypes[index]))->getMeshMaterialShaderType();
									Shader* shader = meshShaderMap->getShader(shaderType);
									BOOST_ASSERT(shader);
									shaderStages.add(shader);
								}
								BOOST_ASSERT(stageTypes.size() == shaderStages.size());
								ShaderPipeline* shaderPipeline = new ShaderPipeline(shaderPipelineType, shaderStages);
								meshShaderMap->addShaderPipeline(shaderPipelineType, shaderPipeline);
							}
						}
					}
				}
				auto foundSharedPipelines = shaderPipelines.find(nullptr);
				if (foundSharedPipelines != shaderPipelines.end())
				{
					for (const ShaderPipelineType* shaderPipelineType : foundSharedPipelines->second)
					{
						if (shaderPipelineType->isMaterialTypePipeline() && !hasShaderPipeline(shaderPipelineType))
						{
							auto& stageTypes = shaderPipelineType->getStages();
							TArray<Shader*> shaderStages;
							for (int32 index = 0; index < stageTypes.size(); ++index)
							{
								MaterialShaderType* shaderType = ((MaterialShaderType*)(stageTypes[index]))->getMaterialShaderType();
								Shader* shader = getShader(shaderType);
								BOOST_ASSERT(shader);
								shaderStages.add(shader);
							}
							BOOST_ASSERT(stageTypes.size() == shaderStages.size());
							ShaderPipeline* shaderPipeline = new ShaderPipeline(shaderPipelineType, shaderStages);
							addShaderPipeline(shaderPipelineType, shaderPipeline);
						}
					}
				}
			}
			for (int32 shaderMapIndex = mMeshShaderMaps.size() - 1; shaderMapIndex >= 0; shaderMapIndex--)
			{
				if (mMeshShaderMaps[shaderMapIndex].getNumShaders() == 0 && mMeshShaderMaps[shaderMapIndex].getNumShaderPipelines() == 0)
				{
					mMeshShaderMaps.removeAt(shaderMapIndex);
				}
			}
			initOrderedMeshShaderMaps();
			bCompilationFinalized = true;
			return true;
		}
		return false;
	}

	MaterialAttributeDefination::MaterialAttributeDefination(const Guid& inGuid, const wstring& inDisplayName, EMaterialProperty inProperty, EMaterialValueType inValueType, const float4 & inDefaultValue, EShaderFrequency inShaderFrequency, int32 inTexCoordIndex /* = INDEX_NONE */, bool bInIsHidden /* = false */, MaterialAttributeBlendFunction inBlendFunction /* = nullptr */)
		:mAttributeID(inGuid)
		, mDisplayName(inDisplayName)
		, mProperty(inProperty)
		, mValueType(inValueType)
		, mDefaultValue(inDefaultValue)
		, mShaderFrequency(inShaderFrequency)
		, mTexCoordIndex(inTexCoordIndex)
		, bIsHidden(bInIsHidden)
		, mBlendFunction(inBlendFunction)
	{
		BOOST_ASSERT(mValueType & MCT_Float && mValueType != MCT_Float);
	}

	int32 MaterialAttributeDefination::compileDefaultValue(MaterialCompiler* compiler)
	{
		int32 ret;
		if (mTexCoordIndex == INDEX_NONE)
		{
			switch (mValueType)
			{
			case Air::MCT_Float1: ret = compiler->constant(mDefaultValue.x);
				break;
			case Air::MCT_Float2: ret = compiler->constant2(mDefaultValue.x, mDefaultValue.y);
				break;
			case Air::MCT_Float3: ret = compiler->constant3(mDefaultValue.x, mDefaultValue.y, mDefaultValue.z);
				break;
			default:
				ret = compiler->constant4(mDefaultValue.x, mDefaultValue.y, mDefaultValue.z, mDefaultValue.w);
				break;
			}
		}
		else
		{
			ret = compiler->textureCoordinate(mTexCoordIndex, false, false);
		}
		return ret;
	}

	MaterialAttributeDefination* MaterialAttributeDefinationMap::find(EMaterialProperty property)
	{
		auto it = mAttributeMap.find(property);
		if (it != mAttributeMap.end())
		{
			return &it->second;
		}
		return nullptr;
	}

	MaterialAttributeDefination* MaterialAttributeDefinationMap::find(const Guid& attributeId)
	{
		for (auto& attribute : mCustomAttributes)
		{
			if (attribute.mAttributeID == attributeId)
			{
				return &attribute;
			}
		}

		for (auto& attribute : mAttributeMap)
		{
			if (attribute.second.mAttributeID == attributeId)
			{
				return &attribute.second;
			}
		}
		return find(MP_Max);
	}

	void MaterialAttributeDefinationMap::initializeAttributeMap()
	{
		BOOST_ASSERT(!bIsInitialized);
		bIsInitialized = true;
		const bool bHideAttribute = true;
		mAttributeMap.reserve(MP_Max + 1);
		add(Guid::newGuid(), TEXT("BaseColor"), MP_BaseColor, MCT_Float3, float4(0, 0, 0, 0), SF_Pixel);
		add(Guid::newGuid(), TEXT("Normal"), MP_Normal, MCT_Float3, float4(0, 0, 1, 0), SF_Pixel);
		add(Guid::newGuid(), TEXT("Metallic"), MP_Metallic, MCT_Float1, float4(0, 0, 0, 0), SF_Pixel);
		add(Guid::newGuid(), TEXT("Specular"), MP_Specular, MCT_Float1, float4(0.5f, 0, 0, 0), SF_Pixel);
		add(Guid::newGuid(), TEXT("Roughness"), MP_Roughness, MCT_Float1, float4(0.5f, 0, 0, 0), SF_Pixel);
		add(Guid::newGuid(), TEXT("CustomizedUV0"), MP_CustomizedUVs0, MCT_Float2, float4(0, 0, 0, 0), SF_Vertex, 0);
		add(Guid::newGuid(), TEXT("CustomizedUV1"), MP_CustomizedUVs1, MCT_Float2, float4(0, 0, 0, 0), SF_Vertex, 1);
		add(Guid::newGuid(), TEXT("CustomizedUV2"), MP_CustomizedUVs2, MCT_Float2, float4(0, 0, 0, 0), SF_Vertex, 2);
		add(Guid::newGuid(), TEXT("CustomizedUV3"), MP_CustomizedUVs3, MCT_Float2, float4(0, 0, 0, 0), SF_Vertex, 3);
		add(Guid::newGuid(), TEXT("CustomizedUV4"), MP_CustomizedUVs4, MCT_Float2, float4(0, 0, 0, 0), SF_Vertex, 4);
		add(Guid::newGuid(), TEXT("CustomizedUV5"), MP_CustomizedUVs5, MCT_Float2, float4(0, 0, 0, 0), SF_Vertex, 5);
		add(Guid::newGuid(), TEXT("CustomizedUV6"), MP_CustomizedUVs6, MCT_Float2, float4(0, 0, 0, 0), SF_Vertex, 6);
		add(Guid::newGuid(), TEXT("CustomizedUV7"), MP_CustomizedUVs7, MCT_Float2, float4(0, 0, 0, 0), SF_Vertex, 7);
		add(Guid::newGuid(), TEXT("AmbientOcclusion"), MP_AmbientOcclusion, MCT_Float1, float4(1, 0, 0, 0), SF_Pixel);
		add(Guid::newGuid(), TEXT("EmissiveColor"), MP_EmissiveColor, MCT_Float3, float4(0, 0, 0, 0), SF_Pixel);
		add(Guid::newGuid(), TEXT("Missing"), MP_Max, MCT_Float1, float4(0, 0, 0, 0), SF_Pixel, INDEX_NONE, bHideAttribute);

	}

	void MaterialAttributeDefinationMap::add(const Guid& attributeId, const wstring& displayName, EMaterialProperty inProperty, EMaterialValueType valueType, const float4& defalutValue, EShaderFrequency shaderFrequency, int32 texCoordIndex /* = INDEX_NONE */, bool bIsHidden /* = false */, MaterialAttributeBlendFunction blendFunction /* = nullptr */)
	{
		BOOST_ASSERT(mAttributeMap.find(inProperty) == mAttributeMap.end());
		mAttributeMap.emplace(inProperty, MaterialAttributeDefination(attributeId, displayName, inProperty, valueType, defalutValue, shaderFrequency, texCoordIndex, bIsHidden));
	}
	MaterialCustomOutputAttributeDefination::MaterialCustomOutputAttributeDefination(const Guid& inGuid, const wstring& inDisplayName, const wstring& inFunctionName, EMaterialProperty inProperty, EMaterialValueType inValueType, const float4 & inDefaultValue, EShaderFrequency inShaderFrequency, int32 inTexCoordIndex /* = INDEX_NONE */, bool bInIsHidden /* = false */, MaterialAttributeBlendFunction inBlendFunction /* = nullptr */)
		:MaterialAttributeDefination(inGuid, inDisplayName, inProperty, inValueType, inDefaultValue, inShaderFrequency, inTexCoordIndex, bInIsHidden, inBlendFunction),
		mFunctionName(inFunctionName)
	{

	}

	FMaterial* MaterialRenderProxy::getMaterialNoFallback(ERHIFeatureLevel::Type inFeatureLevel) const
	{
		return nullptr;
	}

	void MaterialRenderProxy::invalidateConstantExpressionCache()
	{
		BOOST_ASSERT(isInRenderingThread());
		for (int32 i = 0; i < ERHIFeatureLevel::Num; i++)
		{
			mConstantExpressionCache[i].bUpToData = false;
			mConstantExpressionCache[i].mConstantBuffer.safeRelease();
			mConstantExpressionCache[i].mCachedConstantExpressionShaderMap = nullptr;
		}
	}

	void MaterialRenderProxy::cacheConstantExpressions_GameThread()
	{
		if (App::canEverRender())
		{
			MaterialRenderProxy* renderProxy = this;
			ENQUEUE_UNIQUE_RENDER_COMMAND(CacheConstantExpresionsCommand)([renderProxy](RHICommandListImmediate& RHICmdList) {
				renderProxy->cacheConstantExpressions();
			});
		}
	}

	void MaterialRenderProxy::cacheConstantExpressions()
	{
		initResource();
		BOOST_ASSERT(RMaterial::getDefaultMaterial(MD_Surface));
		TArray<MaterialResource*> resourceToCache;

		MaterialInterface::IterateOverActiveFeatureLevels([&](ERHIFeatureLevel::Type inFeatureLevel)
		{
			const FMaterial* materialNoFallBack = getMaterialNoFallback(inFeatureLevel);
			if (materialNoFallBack && materialNoFallBack->getRenderingThreadShaderMap())
			{
				const FMaterial* material = getMaterial(inFeatureLevel);
				bool bIsFallbackMaterial = (material != materialNoFallBack);
				if (!bIsFallbackMaterial)
				{
					MaterialRenderContext materialRenderContext(this, *material, nullptr);
					materialRenderContext.bShowSelection = GIsEditor;
					evaluateConstantExpressions(mConstantExpressionCache[(int32)inFeatureLevel], materialRenderContext);
				}
				else
				{
					invalidateConstantExpressionCache();
					return;
				}
			}
			else
			{
				invalidateConstantExpressionCache();
				return;
			}
		});
	}

	MaterialAttributeDefinationMap MaterialAttributeDefinationMap::GMaterialPropertyAttributesMap;

	int32 MaterialCompiler::errorf(const TCHAR* format, ...)
	{
		TCHAR errorText[2048];
		GET_VARARGS(errorText, ARRAY_COUNT(errorText), ARRAY_COUNT(errorText) - 1, format, format);
		return error(errorText);
	}

	void doMaterialAttributeReorder(class ExpressionInput* input, int32 version)
	{}

	static bool serializeExpressionInput(Archive& ar, ExpressionInput& input)
	{
		return true;
	}
	
	bool ExpressionInput::serialize(Archive& ar)
	{
		return serializeExpressionInput(ar, *this);
	}
}