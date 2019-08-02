#include "Classes/Materials/Material.h"
#include "SimpleReflection.h"
#include "MaterialShared.h"
#include "Classes/Materials/MaterialInstance.h"
#include "Misc/App.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "boost/algorithm/algorithm.hpp"
#include "AirEngine.h"
#include "boost/algorithm/string.hpp"
#include "rapidxml/src/rapidxml.hpp"
#include "Misc/FileHelper.h"
#include "Texture.h"
#include "HAL/PlatformProperties.h"
#include "MaterialCompiler.h"
#include "Classes/Materials/MaterialExpression.h"
#include "Classes/Materials/MaterialExpressionMaterialFunctionCall.h"
namespace Air
{

	static std::shared_ptr<RMaterial> GDefaultMaterials[MD_MAX] = { nullptr, nullptr, nullptr, nullptr,nullptr };

	static const TCHAR* GDefaultMaterialFilepath[MD_MAX] =
	{
		TEXT("assets/materials/defaultMaterial.mtl"),
		TEXT("assets/materials/defaultDeferredDecalMaterial.mtl"),
		TEXT("assets/materials/defaulLightFunctionMaterial.mtl"),
		TEXT("assets/materials/defaultPostProcessMaterial.mtl"),
		TEXT("assets/materials/defaultMaterial.mtl")
	};



	void processSerializedInlineShaderMaps(MaterialInterface* owner, TArray<MaterialResource>& loadedResources, MaterialResource* (&outMaterialResourcesLoaded)[EMaterialQualityLevel::Num][ERHIFeatureLevel::Num])
	{
		RMaterial* ownerMaterial = dynamic_cast<RMaterial*>(owner);
		MaterialInstance* ownerMaterialInstance = dynamic_cast<MaterialInstance*>(owner);
		for (MaterialResource& resource : loadedResources)
		{
			resource.registerInlineShaderMap();
		}
		if (GDiscardUnusedQualityLevels)
		{
			static const int32 qualityScores[EMaterialQualityLevel::Num + 1] = { 0, 3, 1, 10 };
			int32 desiredQL = (int32)getCachedScalabilityCVars().mMaterialQualityLevel;
			BOOST_ASSERT(desiredQL < EMaterialQualityLevel::Num);
			const int32 desiredScore = qualityScores[desiredQL];
			for (int32 resourceIndex = 0; resourceIndex < loadedResources.size(); resourceIndex++)
			{
				MaterialResource& loadedResource = loadedResources[resourceIndex];
				MaterialShaderMap* loadedShaderMap = loadedResource.getGameThreadShaderMap();
				if (loadedShaderMap && loadedShaderMap->getShaderPlatform() == GMaxRHIShaderPlatform)
				{
					EMaterialQualityLevel::Type loadedQualityLevel = loadedShaderMap->getShaderMapId().mQualityLevel;
					loadedQualityLevel = loadedQualityLevel == EMaterialQualityLevel::Num ? EMaterialQualityLevel::High : loadedQualityLevel;
					ERHIFeatureLevel::Type loadedFeatureLevel = loadedShaderMap->getShaderMapId().mFeatureLevel;

					int32 currentQL = (int32)EMaterialQualityLevel::Num;
					for (int32 i = 0; i < EMaterialQualityLevel::Num; i++)
					{
						MaterialResource* materialResource = outMaterialResourcesLoaded[i][loadedFeatureLevel];
						if (materialResource != nullptr && materialResource->getGameThreadShaderMap() != nullptr)
						{
							int32 foundQL = materialResource->getGameThreadShaderMap()->getShaderMapId().mQualityLevel;
							currentQL = foundQL == EMaterialQualityLevel::Num ? EMaterialQualityLevel::High : foundQL;
						}
					}
					const int32 currentScore = Math::abs(qualityScores[currentQL] - desiredScore);

					const int32 potentialScore = Math::abs(qualityScores[loadedQualityLevel] - desiredScore);
					if (potentialScore < currentScore)
					{
						for (int32 qualityLevelIndex = 0; qualityLevelIndex < EMaterialQualityLevel::Num; qualityLevelIndex++)
						{
							if (!outMaterialResourcesLoaded[qualityLevelIndex][loadedFeatureLevel])
							{
								outMaterialResourcesLoaded[qualityLevelIndex][loadedFeatureLevel] = ownerMaterialInstance ? ownerMaterialInstance->allocatePermutationResource() : ownerMaterial->allocateResource();
							}
							outMaterialResourcesLoaded[qualityLevelIndex][loadedFeatureLevel]->releaseShaderMap();
							outMaterialResourcesLoaded[qualityLevelIndex][loadedFeatureLevel]->setInlineShaderMap(loadedShaderMap);
						}
					}
				}
			}
		}
		else
		{
			for (int32 passIndex = 0; passIndex < 2; passIndex++)
			{
				for (int32 resourceIndex = 0; resourceIndex < loadedResources.size(); resourceIndex++)
				{
					MaterialResource& loadedResource = loadedResources[resourceIndex];
					MaterialShaderMap* loadedShaderMap = loadedResource.getGameThreadShaderMap();
					if (loadedShaderMap && loadedShaderMap->getShaderPlatform() == GMaxRHIShaderPlatform)
					{
						EMaterialQualityLevel::Type loadedQualityLevel = loadedShaderMap->getShaderMapId().mQualityLevel;
						ERHIFeatureLevel::Type loadedFeatureLevel = loadedShaderMap->getShaderMapId().mFeatureLevel;
						for (int32 qualityLevelIndex = 0; qualityLevelIndex < EMaterialQualityLevel::Num; qualityLevelIndex++)
						{
							if ((passIndex == 0 && loadedQualityLevel == EMaterialQualityLevel::Num) || (passIndex == 1 && qualityLevelIndex == loadedQualityLevel))
							{
								if (!outMaterialResourcesLoaded[qualityLevelIndex][loadedFeatureLevel])
								{
									outMaterialResourcesLoaded[qualityLevelIndex][loadedFeatureLevel] = ownerMaterialInstance ? ownerMaterialInstance->allocatePermutationResource() : ownerMaterial->allocateResource();
								}
								outMaterialResourcesLoaded[qualityLevelIndex][loadedFeatureLevel]->setInlineShaderMap(loadedShaderMap);
							}
						}
					}
				}

			}
		}
	}

	void MaterialInterface::assertDefaultMaterialsPostLoaded()
	{
		for (int32 domain = 0; domain < MD_MAX; ++domain)
		{
			BOOST_ASSERT(GDefaultMaterials[domain] != nullptr);
		}
	}

	class DefaultMaterialInstance : public MaterialRenderProxy
	{
	public:
		DefaultMaterialInstance(RMaterial* inMaterial, bool bInSeleted, bool bInHovered) :
			MaterialRenderProxy(bInSeleted, bInHovered),
			mMaterial(inMaterial)
		{

		}

		virtual const class FMaterial* getMaterial(ERHIFeatureLevel::Type inFeatureLevel) const
		{
			const MaterialResource* materialResource = mMaterial->getMaterialResource(inFeatureLevel);
			if (materialResource && materialResource->getRenderingThreadShaderMap())
			{
				BOOST_ASSERT(materialResource->getRenderingThreadShaderMap()->isCompilationFinalized());
				BOOST_ASSERT(materialResource->getRenderingThreadShaderMap()->compiledSuccessfully());
				return materialResource;
			}
			BOOST_ASSERT(!mMaterial->isDefaultMaterial());
			return getFallbackRenderProxy().getMaterial(inFeatureLevel);
		}

		virtual bool getVectorValue(const wstring parameterName, LinearColor* outValue, const MaterialRenderContext& context) const override
		{
			const MaterialResource* materialResource = mMaterial->getMaterialResource(context.mMaterial.getFeatureLevel());
			if (materialResource && materialResource->getRenderingThreadShaderMap())
			{
				if (parameterName == TEXT("SelectionColor"))
				{
					*outValue = LinearColor::Black;
					return true;
				}
				return false;
			}
			else
			{
				return getFallbackRenderProxy().getVectorValue(parameterName, outValue, context);
			}
		}

		virtual bool getScalarValue(const wstring parameterName, float* outValue, const MaterialRenderContext& context) const override
		{
			const MaterialResource* materialResource = mMaterial->getMaterialResource(context.mMaterial.getFeatureLevel());
			if (materialResource && materialResource->getRenderingThreadShaderMap())
			{
				static wstring namesubsurfaceProfile(TEXT("__SubsurfaceProfile"));
				if (parameterName == namesubsurfaceProfile)
				{
					return true;
				}
				return false;
			}
			else
			{
				return getFallbackRenderProxy().getScalarValue(parameterName, outValue, context);
			}
		}

		virtual bool getTextureValue(const wstring parameterName, std::shared_ptr<const RTexture>& outValue, const MaterialRenderContext& context) const override
		{
			const MaterialResource* materialResource = mMaterial->getMaterialResource(context.mMaterial.getFeatureLevel());
			if (materialResource && materialResource->getRenderingThreadShaderMap())
			{
				return false;
			}
			else
			{
				return getFallbackRenderProxy().getTextureValue(parameterName, outValue, context);
			}
		}

	private:


		MaterialRenderProxy & getFallbackRenderProxy() const
		{
			return *(RMaterial::getDefaultMaterial(mMaterial->mMaterialDomain)->getRenderProxy(isSelected(), isHovered()));
		}

		RMaterial* mMaterial;

	};

	void RMaterial::postLoad()
	{
		ParentType::postLoad();
		processSerializedInlineShaderMaps(this, mLoadedMaterialResources, mMaterialResources);
		mLoadedMaterialResources.empty();

		if (!isDefaultMaterial())
		{
			assertDefaultMaterialsPostLoaded();
		}

		//for(int32 expressionIndex = 0 ; expressionIndex < mexpre)

		if (mShadingMode == MSM_MAX)
		{
			mShadingMode = MSM_DefaultLit;
		}
		if (!mStateId.isValid())
		{
			PlatformMisc::createGuid(mStateId);
		}

		propagateDataToMaterialProxy();

		{
			if (App::canEverRender())
			{
				rebuildExpressionTextureReferences();

				for (int32 textureIndex = 0, numTextures = mExpressionTextureReferences.size(); textureIndex < numTextures; ++textureIndex)
				{
					std::shared_ptr<RTexture> texture = mExpressionTextureReferences[textureIndex];
					if (texture)
					{
						texture->conditionalPostLoad();
					}
				}
				cacheResourceShadersForRendering(false);
			}
		}
		if (GIsEditor && !isTemplate())
		{

		}
	}

	void MaterialInterface::initDefaultMaterials()
	{
		static bool bInitialized = false;
		if (!bInitialized)
		{
			BOOST_ASSERT(isInGameThread());
			for (int32 domain = 0; domain < MD_MAX; ++domain)
			{
				if (GDefaultMaterials[domain] == nullptr)
				{
					GDefaultMaterials[domain] = loadObjectAsync<RMaterial>(GDefaultMaterialFilepath[domain]);
					flushAsyncLoading();
				}
			}
			bInitialized = true;
		}
	}

	void MaterialInterface::assertDefaultMaterialsExist()
	{
		for (int32 domain = 0; domain < MD_MAX; ++domain)
		{
			BOOST_ASSERT(GDefaultMaterials[domain] != nullptr);
		}
	}


	bool RMaterial::isDefaultMaterial() const
	{
		bool bDefault = false;
		for (int32 domain = MD_Surface; !bDefault && domain < MD_MAX; ++domain)
		{
			bDefault = (GDefaultMaterials[domain].get() == this);
		}
		return bDefault;
	}

	bool RMaterial::isMasked() const
	{
		return getBlendMode() == EBlendMode::BLEND_Masked;
	}

	bool RMaterial::isTwoSided() const
	{
		return mTwoSided != 0;
	}


	int32 MaterialResource::getMaterialDomain()const
	{
		return mMaterial->mMaterialDomain;
	}

	MaterialResource* RMaterial::getMaterialResource(ERHIFeatureLevel::Type inFeatureLvel, EMaterialQualityLevel::Type qualityLevel /* = EMaterialQualityLevel::Num */)
	{
		if (qualityLevel == EMaterialQualityLevel::Num)
		{
			qualityLevel = EMaterialQualityLevel::High;
		}
		return mMaterialResources[qualityLevel][inFeatureLvel];
	}

	const MaterialResource* RMaterial::getMaterialResource(ERHIFeatureLevel::Type inFeatureLvel, EMaterialQualityLevel::Type qualityLevel /* = EMaterialQualityLevel::Num */) const
	{
		if (qualityLevel = EMaterialQualityLevel::Num)
		{
			qualityLevel = EMaterialQualityLevel::High;
		}
		return mMaterialResources[qualityLevel][inFeatureLvel];
	}


	RMaterial::RMaterial(const ObjectInitializer& objectInterface /* = ObjectInitializer::get() */)
		:ParentType(objectInterface)
	{
		initMaterialPropertyTable();
	}

	void RMaterial::initMaterialPropertyTable()
	{
		mMaterialPropertyTable[MP_BaseColor] = &mBaseColor;
		mMaterialPropertyTable[MP_Normal] = &mNormal;
		mMaterialPropertyTable[MP_Metallic] = &mMetallic;
		mMaterialPropertyTable[MP_Specular] = &mSpecular;
		mMaterialPropertyTable[MP_Roughness] = &mRoughness;
		for (int32 index = 0; index < 8; index++)
		{
			mMaterialPropertyTable[MP_CustomizedUVs0 + index] = &mCustomizedUVs[index];
		}
		mMaterialPropertyTable[MP_AmbientOcclusion] = &mAmbientOcclusion;
		mMaterialPropertyTable[MP_EmissiveColor] = &mEmissiveColor;
	}

	MaterialRenderProxy* RMaterial::getRenderProxy(bool selected, bool bHovered /* = false */) const
	{
		BOOST_ASSERT(!(selected || bHovered) || GIsEditor);
		return mDefaultMaterialInstances[selected ? 1 : (bHovered ? 2 : 0)];
	}

	std::shared_ptr<RMaterial> RMaterial::getMaterial()
	{
		return std::dynamic_pointer_cast<RMaterial>(this->shared_from_this());
	}

	std::shared_ptr<const RMaterial> RMaterial::getMaterial() const
	{
		return std::dynamic_pointer_cast<const RMaterial>(this->shared_from_this());
	}


	void RMaterial::postInitProperties()
	{
		ParentType::postInitProperties();
		if (!hasAnyFlags(RF_ClassDefaultObject))
		{
			mDefaultMaterialInstances[0] = new DefaultMaterialInstance(this, false, false);
			if (GIsEditor)
			{
				mDefaultMaterialInstances[1] = new DefaultMaterialInstance(this, true, false);
				mDefaultMaterialInstances[2] = new DefaultMaterialInstance(this, false, true);
			}
		}
		PlatformMisc::createGuid(mStateId);
		updateResourceAllocations();
	}

	void RMaterial::updateResourceAllocations()
	{
		if (App::canEverRender())
		{
			for (int32 featureLevelIndex = 0; featureLevelIndex < ERHIFeatureLevel::Num; featureLevelIndex++)
			{
				TArray<bool, TInlineAllocator<EMaterialQualityLevel::Num>> qualityLevelsUsed;
				EShaderPlatform shaderPlatform = GShaderPlatformForFeatureLevel[featureLevelIndex];
				getQualityLevelUsage(qualityLevelsUsed, shaderPlatform);
				for (int32 qualityLevelIndex = 0; qualityLevelIndex < EMaterialQualityLevel::Num; qualityLevelIndex++)
				{
					MaterialResource*& currentResource = mMaterialResources[qualityLevelIndex][featureLevelIndex];
					if (!currentResource)
					{
						currentResource = allocateResource();
					}
					const bool bHasQualityLevelUsage = qualityLevelsUsed[qualityLevelIndex];
					currentResource->setMaterial(this, (EMaterialQualityLevel::Type)qualityLevelIndex, bHasQualityLevelUsage, (ERHIFeatureLevel::Type)featureLevelIndex);
				}
			}
		}
	}

	void RMaterial::getQualityLevelUsage(TArray<bool, TInlineAllocator<EMaterialQualityLevel::Num>>& qualityLevelsUsed, EShaderPlatform shaderPlatform)
	{
		qualityLevelsUsed.addZeroed(EMaterialQualityLevel::Num);
		qualityLevelsUsed[EMaterialQualityLevel::High] = true;
	}

	void RMaterial::flushResourceShaderMaps()
	{
		PlatformMisc::createGuid(mStateId);
		if (App::canEverRender())
		{
			MaterialInterface::IterateOverActiveFeatureLevels([&](ERHIFeatureLevel::Type inFeatureLevel)
			{
				for (int32 qualityLevelIndex = 0; qualityLevelIndex < EMaterialQualityLevel::Num; qualityLevelIndex++)
				{
					MaterialResource* currentResource = mMaterialResources[qualityLevelIndex][inFeatureLevel];
					currentResource->releaseShaderMap();
				}
			});
		}
	}

	void RMaterial::recacheConstantExpressions() const
	{

	}

	void RMaterial::cacheResourceShadersForRendering(bool bRegenerateId)
	{
		if (bRegenerateId)
		{
			flushResourceShaderMaps();
		}

		updateResourceAllocations();

		if (App::canEverRender())
		{
			uint32 featureLevelsToCompile = getFeatureLevelsToCompileForRendering();
			EMaterialQualityLevel::Type activeQualityLevel = getCachedScalabilityCVars().mMaterialQualityLevel;
			TArray<MaterialResource*> resourceTCache;
			while (featureLevelsToCompile != 0)
			{
				ERHIFeatureLevel::Type featureLevel = (ERHIFeatureLevel::Type)BitSet::getAndClearNextBit(featureLevelsToCompile);
				EShaderPlatform shaderPlatform = GShaderPlatformForFeatureLevel[featureLevel];
				resourceTCache.reset();
				MaterialResource* materialResource = mMaterialResources[activeQualityLevel][featureLevel];
				BOOST_ASSERT(materialResource);
				resourceTCache.add(materialResource);
				cacheShadersForResource(shaderPlatform, resourceTCache, true);
			}
			recacheConstantExpressions();
		}
	}

	void RMaterial::cacheShadersForResource(EShaderPlatform shaderPlatform, const TArray<MaterialResource*>& resourceToCache, bool bApplyCompletedShaderMapForRendering)
	{
		rebuildExpressionTextureReferences();
		for (int32 resourceIndex = 0; resourceIndex < resourceToCache.size(); resourceIndex++)
		{
			MaterialResource* currentResource = resourceToCache[resourceIndex];
			const bool bSuccess = currentResource->cacheShaders(shaderPlatform, bApplyCompletedShaderMapForRendering);
			if (!bSuccess)
			{
				if (isDefaultMaterial())
				{

				}
				const TArray<wstring>& compileErrors = currentResource->getCompileErrors();
				for (int32 errorIndex = 0; errorIndex < compileErrors.size(); errorIndex++)
				{
					AIR_LOG(LogMaterial, Warning, TEXT(" %s"), compileErrors[errorIndex].c_str());
				}
			}
		}
	}

	struct ChunkData
	{
		EMaterialProperty mProperty;
		wstring mValue;
		ChunkData(EMaterialProperty inProperty, wstring& inValue)
			:mProperty(inProperty),
			mValue(inValue)
		{

		}
	};

	struct MaterialData
	{
		wstring mName;
		wstring mShaderString;
		TArray<ChunkData> mChunksData;
		bool bLoaded{ false };
		void addChunk(EMaterialProperty prop, wstring value)
		{
			mChunksData.add(ChunkData(prop, value));
		}
	};

	class MaterialLoadingDesc : public ResLoadingDesc
	{
	private:
		struct MaterialDesc
		{
			std::wstring mResPath;



			std::shared_ptr<MaterialData> mMaterialData;
			std::shared_ptr<std::shared_ptr<RMaterial>> mResource;
		};

		class MaterialReader
		{
		public:
			virtual void readMaterialData(wstring& str, std::shared_ptr<MaterialData> &outData, MaterialDesc& materialDesc) = 0;
		};

		class MaterialReaderXML : public MaterialReader
		{
		public:
			virtual void readMaterialData(wstring& str, std::shared_ptr<MaterialData> &outData, MaterialDesc& materialDesc) override
			{
				using namespace rapidxml;
				xml_document<TCHAR> doc;
				/*char* buffer = (char*)Memory::malloc(archive->totalSize() + 1);
				archive->serialize(buffer, archive->totalSize());*/
				doc.parse<0>(&str[0]);
				auto *root = doc.first_node(TEXT("Material"));
				auto* materialExpressionsNode = root->first_node(TEXT("MaterialExpressions"));
				if (materialExpressionsNode)
				{
					auto* expressionNode = materialExpressionsNode->first_node(TEXT("MaterialExpression"));
					while (expressionNode)
					{
						wstring className = expressionNode->first_attribute(TEXT("class"))->value();
						std::shared_ptr<RMaterialExpression> materialExpression = newObject<RMaterialExpression>((*materialDesc.mResource).get(), className, TEXT(""));
						materialExpression->serialize(expressionNode);
						expressionNode = expressionNode->next_sibling(TEXT("MaterialExpression"));
					}
				}

				auto* materialInputsNode = root->first_node(TEXT("MaterialInputs"));
				RMaterial& material = **materialDesc.mResource;
				if (materialInputsNode)
				{
					auto* materialInputNode = materialInputsNode->first_node(TEXT("MaterialInput"));
					while (materialInputNode)
					{
						EMaterialProperty prop = stringToMaterialProperty(materialInputNode->first_attribute(TEXT("EMaterialProperty"))->value());
						ExpressionInput* input = material.mMaterialPropertyTable[prop];
						XMLNode* expressionInputNode = materialInputNode->first_node(TEXT("ExpressionInput"));
						input->serialize(expressionInputNode);
						materialInputNode = materialInputNode->next_sibling(TEXT("MaterialInput"));
					}
				}
				(*materialDesc.mResource)->_connect();

				outData->bLoaded = true;
			}
		};

		class MaterialReaderBinary : public MaterialReader
		{
		public:
			virtual void readMaterialData(wstring& str, std::shared_ptr<MaterialData> &outData, MaterialDesc& materialDesc) override
			{

			}
		};


		static MaterialReaderXML xmlReader;
		static MaterialReaderBinary binaryReader;
	public:
		explicit MaterialLoadingDesc(std::wstring const & resPath)
		{
			mMaterialDesc.mResPath = resPath;
			mMaterialDesc.mMaterialData = MakeSharedPtr<MaterialData>();
			mMaterialDesc.mResource = MakeSharedPtr<std::shared_ptr<RMaterial>>();
			mResourceClass = RMaterial::StaticClass();
		}

		void setResource(std::shared_ptr<Object> ptr) override
		{
			*mMaterialDesc.mResource = std::static_pointer_cast<RMaterial>(ptr);
		}

		uint64_t type() const override
		{
			static uint64_t const mType = boost::hash_value("MaterialLoadingDesc");
			return mType;
		}

		bool stateLess() const override
		{
			return false;
		}

		void subThreadStage() override
		{
			std::lock_guard<std::mutex> lock(mMainThreadStageMutex);
			if (*mMaterialDesc.mResource && mMaterialDesc.mMaterialData->bLoaded)
			{
				return;
			}
			//Archive* archive = IFileManager::get().createFileReader(mMaterialDesc.mResPath.c_str());
			wstring str;
			bool r = FileHelper::loadFileToString(str, mMaterialDesc.mResPath.c_str());
			BOOST_ASSERT(r);
			if (boost::ends_with(mMaterialDesc.mResPath, TEXT(".mtl")))
			{
				xmlReader.readMaterialData(str, mMaterialDesc.mMaterialData, mMaterialDesc);
			}
			else
			{
				binaryReader.readMaterialData(str, mMaterialDesc.mMaterialData, mMaterialDesc);
			}


		}

		bool hasSubThreadStage()const override
		{
			return true;
		}

		void copyDataFrom(ResLoadingDesc const & rhs)
		{
			BOOST_ASSERT(this->type() == rhs.type());
			MaterialLoadingDesc const & mld = static_cast<MaterialLoadingDesc const&>(rhs);
			mMaterialDesc.mResPath = mld.mMaterialDesc.mResPath;
			mMaterialDesc.mMaterialData = mld.mMaterialDesc.mMaterialData;
			mMaterialDesc.mResource = mld.mMaterialDesc.mResource;
		}

		std::shared_ptr<void> cloneResourceFrom(std::shared_ptr<void> const & resource) override
		{
			return resource;
		}

		bool match(ResLoadingDesc const & rhs) const override
		{
			if (this->type() == rhs.type())
			{
				MaterialLoadingDesc const & tld = static_cast<MaterialLoadingDesc const &>(rhs);
				return mMaterialDesc.mResPath == tld.mMaterialDesc.mResPath;
			}
			return false;
		}

		std::shared_ptr<Object> resource() const override
		{
			return *mMaterialDesc.mResource;
		}

		void mainThreadStage() override
		{
			(*mMaterialDesc.mResource)->mLocalData = mMaterialDesc.mMaterialData;
			std::lock_guard<std::mutex> lock(mMainThreadStageMutex);
			(*mMaterialDesc.mResource)->conditionalPostLoad();
		}

		void mainThreadStageNoLock()
		{

		}

		virtual wstring name() const
		{
			return mMaterialDesc.mMaterialData->mName;
		}

	private:
		MaterialDesc mMaterialDesc;
		std::mutex mMainThreadStageMutex;
	};

	MaterialLoadingDesc::MaterialReaderXML MaterialLoadingDesc::xmlReader;

	MaterialLoadingDesc::MaterialReaderBinary MaterialLoadingDesc::binaryReader;


	std::shared_ptr<ResLoadingDesc> RMaterial::createLoadingDesc(wstring const & path)
	{
		return MakeSharedPtr<MaterialLoadingDesc>(path);
	}

	MaterialResource* RMaterial::allocateResource()
	{
		return new MaterialResource();
	}

	void RMaterial::propagateDataToMaterialProxy()
	{
		for (int32 i = 0; i < ARRAY_COUNT(mDefaultMaterialInstances); i++)
		{
			if (mDefaultMaterialInstances[i])
			{
				updateMaterialRenderProxy(*mDefaultMaterialInstances[i]);
			}
		}
	}

	void RMaterial::rebuildExpressionTextureReferences()
	{
		if (PlatformProperties::hasEditorOnlyData())
		{
			rebuildMaterialFunctionInfo();
			rebuildMaterialParameterCollectionInfo();
		}

		mExpressionTextureReferences.empty();
		appendReferencedTextures(mExpressionTextureReferences);
	}

	void MaterialInterface::postLoadDefaultMaterials()
	{
		static bool bPostLoaeded = false;
		if (!bPostLoaeded)
		{
			BOOST_ASSERT(isInGameThread());
			bPostLoaeded = true;
			for (int32 domain = 0; domain < MD_MAX; ++domain)
			{
				if (!GDefaultMaterials[domain])
				{
					bPostLoaeded = false;
					return;
				}
				RMaterial* material = GDefaultMaterials[domain].get();
				BOOST_ASSERT(material);
				material->conditionalPostLoad();
				if (material->hasAnyFlags(RF_NeedPostLoad))
				{
					bPostLoaeded = false;
				}
			}
		}
	}

	EMaterialShadingModel RMaterial::getShadingModel() const
	{
		switch (mMaterialDomain)
		{
		case MD_Surface:
			return mShadingMode;
			/*case MD_DeferredDecal:
				return MSM_DefaultLit;
			case MD_PostProcess:
			case MD_LightFunction:
			case MD_UI:
				return MSM_Unlit;*/
		default:
			BOOST_ASSERT(false);
			return MSM_Unlit;
		}
	}

	EBlendMode RMaterial::getBlendMode() const
	{
		if (EBlendMode(mBlendMode) == BLEND_Masked)
		{
			return BLEND_Masked;
		}
		else
		{
			mBlendMode;
		}
	}

	void RMaterial::appendReferencedTextures(TArray<std::shared_ptr<RTexture>>& inOutTextures) const
	{
		for (int32 expressionIndex = 0; expressionIndex < mExpressions.size(); expressionIndex++)
		{
			std::shared_ptr<RMaterialExpression> expression = mExpressions[expressionIndex];
			RMaterialExpressionMaterialFunctionCall* materialFunctionNode = dynamic_cast<RMaterialExpressionMaterialFunctionCall*>(expression.get());
			if (materialFunctionNode && materialFunctionNode->mMaterialFunction)
			{
				TArray<RMaterialFunction*> functions;
				functions.add(materialFunctionNode->mMaterialFunction);
				materialFunctionNode->mMaterialFunction->getDependenctFunction(functions);

				for (int32 functionIndex = 0; functionIndex < functions.size(); functionIndex++)
				{
					RMaterialFunction* currentFunction = functions[functionIndex];
					currentFunction->appendReferencedTextures(inOutTextures);
				}
			}
			else if (expression)
			{
				std::shared_ptr<RTexture> referencedTexture = expression->getReferencedTexture();
				if (referencedTexture)
				{
					inOutTextures.addUnique(referencedTexture);
				}
			}
		}
	}

	void RMaterial::compileProperty(MaterialCompiler* compiler, wstring** properties, uint32 forceCastFlags /* = 0 */)
	{
		std::shared_ptr<MaterialData> data = std::static_pointer_cast<MaterialData>(mLocalData);
		for (auto& it : data->mChunksData)
		{
			properties[it.mProperty] = &it.mValue;
		}
		compiler->setShaderString(data->mShaderString);
	}


	void RMaterial::rebuildMaterialFunctionInfo()
	{
		
	}

	void RMaterial::rebuildMaterialParameterCollectionInfo()
	{
	}




	std::shared_ptr<RMaterial>& RMaterial::getDefaultMaterial(EMaterialDomain domain)
	{
		initDefaultMaterials();
		BOOST_ASSERT(domain >= MD_Surface && domain < MD_MAX);
		BOOST_ASSERT(GDefaultMaterials[domain] != nullptr);
		return GDefaultMaterials[domain];
	}

	void RMaterial::cacheExpressionTextureReferences()
	{
		if (mExpressionTextureReferences.size() <= 0)
		{
			rebuildExpressionTextureReferences();
		}
	}

	float RMaterial::getOpacityMaskClipValue() const
	{
		return mOpacityMaskClipValue;
	}

	bool RMaterial::isDitheredLODTransition() const
	{
		return mDitheredLODTransition;
	}

	bool RMaterial::isPropertyActive(EMaterialProperty inProperty) const
	{
		if (mMaterialDomain == MD_PostProcess)
		{

		}
		else if (mMaterialDomain == MD_LightFunction)
		{

		}
		const bool bIsTranslucentBlendMode = isTranslucentBlendMode((EBlendMode)mBlendMode);
		const bool bIsNonDirectionalTranslucencyLightingMode = false;
		bool active = true;
		switch (inProperty)
		{
		case Air::MP_BaseColor:
			active = mShadingMode != MSM_Unlit;
			break;
		case Air::MP_Metallic:
		case Air::MP_Specular:
		case Air::MP_Roughness:
			active = mShadingMode != MSM_Unlit && !bIsTranslucentBlendMode;
			break;
		default:
			active = true;
			break;
		}
		return active;
	}

#if WITH_EDITOR
	int32 RMaterial::compilePropertyEx(class MaterialCompiler* compiler, const Guid& attributeId)
	{
		const EMaterialProperty prop = MaterialAttributeDefinationMap::getProperty(attributeId);
		if (bUseMaterialAttributes)
		{
			return mMaterialAttributes.compileWithDefault(compiler, attributeId);
		}
		switch (prop)
		{
		case MP_Metallic: return mMetallic.compileWithDefault(compiler, prop);
		case MP_Specular: return mSpecular.compileWithDefault(compiler, prop);
		case MP_Roughness: return mRoughness.compileWithDefault(compiler, prop);
		case MP_BaseColor: return mBaseColor.compileWithDefault(compiler, prop);
		case MP_Normal: return mNormal.compileWithDefault(compiler, prop);
		case MP_AmbientOcclusion: return mAmbientOcclusion.compileWithDefault(compiler, prop);
		case MP_EmissiveColor: return mEmissiveColor.compileWithDefault(compiler, prop);
		default:
			if (prop >= MP_CustomizedUVs0 && prop <= MP_CustomizedUVs7)
			{
				const int32 textureCoordinateIndex = prop - MP_CustomizedUVs0;
				if (mCustomizedUVs[textureCoordinateIndex].mExpression && textureCoordinateIndex < mNumCustomizedUVs)
				{
					return mCustomizedUVs[textureCoordinateIndex].compileWithDefault(compiler, prop);
				}
				else
				{
					return compiler->textureCoordinate(textureCoordinateIndex, false, false);
				}
			}
			break;
		}
		BOOST_ASSERT(false);
		return INDEX_NONE;
	}

	void RMaterial::_connect()
	{
		for (int propertyIndex = 0; propertyIndex < MP_Max; propertyIndex++)
		{
			if (isPropertyActive((EMaterialProperty)propertyIndex))
			{
				auto* expression = findExpression(mMaterialPropertyTable[propertyIndex]->mExpressionId);
				if (expression)
				{
					mMaterialPropertyTable[propertyIndex]->connect(expression);
				}
			}
		}
	}

	RMaterialExpression* RMaterial::findExpression(uint32 id)
	{
		for (int32 i = 0; i < mExpressions.size(); i++)
		{
			if (mExpressions[i]->mID == id)
			{
				return mExpressions[i].get();
			}
		}
		return nullptr;
	}

#endif


	DECLARE_SIMPLER_REFLECTION(RMaterial);
}