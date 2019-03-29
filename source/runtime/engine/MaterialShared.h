#pragma once
#include "CoreType.h"
#include "EngineMininal.h"
#include "RenderResource.h"
#include "Classes/Engine/EngineType.h"
#include "Misc/Optional.h"
#include "ConstantBuffer.h"
#include "Shader.h"
#include "SceneTypes.h"
#include "Misc/Guid.h"
#include "Containers/IndirectArray.h"
#include "StaticParameterSet.h"
#include "VertexFactory.h"
namespace Air
{
	class Shader;
	class MaterialShaderType;
	class MaterialConstantExpressionTexture;
	class RMaterial;
	class MaterialCompiler;
	class MaterialInterface;
	class MaterialInstance;

	enum ECompiledMaterialProperty
	{
		CompiledMP_EmissiveColorCS = MP_Max,
		CompiledMP_Max

	};

	inline bool isSubsurfaceShadingMode(EMaterialShadingModel shadingMode)
	{
		return shadingMode == MSM_Subsurface || shadingMode == MSM_PreintegratedSkin || shadingMode == MSM_SubsurfaceProfile || shadingMode == MSM_TwoSidedFoliage || shadingMode == MSM_Cloth;
	}

	class MaterialConstantExpressionType
	{
	public:
		typedef class MaterialConstantExpression* (*SerializationConstructorType)();

		static TLinkedList<MaterialConstantExpressionType*>*& getTypeList();

		static TMap<wstring, MaterialConstantExpressionType*>& getTypeMap();

		MaterialConstantExpressionType(const TCHAR* inName, SerializationConstructorType inSerializationConstructor);

		friend Archive& operator << (Archive& ar, class MaterialConstantExpression*& ref);
		friend Archive& operator << (Archive& ar, MaterialConstantExpressionTexture*& ref);

		const TCHAR* getName() const { return mName; }
	private:
		const TCHAR* mName;
		SerializationConstructorType mSerializationConstructor;
	};
	class MeshMaterialShaderMap;
	class ShaderCommonCompileJob;
	class SceneView;
	class RTexture;
	class MaterialRenderProxy;

	namespace EMaterialShaderMapUsage
	{
		enum Type
		{
			Default,
		};
	}

#define DECLARE_MATERIALCONSTANTEXPRESSION_TYPE(Name)	\
	public:	\
	static MaterialConstantExpressionType mStaticType;	\
	static MaterialConstantExpression* serializationConstructor() {return new Name();}\
	virtual MaterialConstantExpressionType* getType() const{return &mStaticType;}

#define IMPLEMENT_MATERIALCONSTANTEXPRESSION_TYPE(Name)	\
	MaterialConstantExpressionType Name::mStaticType(TEXT(#Name), &Name::serializationConstructor);

	extern void getMaterialQualityLevelName(EMaterialQualityLevel::Type inQualityLevel, wstring & outName);

	class MaterialConstantExpression : public RefCountedObject
	{
	public:
		virtual ~MaterialConstantExpression() {}
		virtual MaterialConstantExpressionType* getType() const = 0;
		virtual void serialize(Archive& ar) = 0;
		virtual void getNumberValue(const struct MaterialRenderContext& context, LinearColor& outValue)const {}
		virtual class MaterialConstantExpressionTexture* getTextureConstantExpression() { return nullptr; }

		virtual bool isConstant()const { return false; }
		virtual bool isChangingPerFrame()const { return false; }
		virtual bool isIdentical(const MaterialConstantExpression* otherExpression) const { return false;
		}
		friend Archive & operator <<(Archive& ar, class MaterialConstantExpression*& ref);
	};




	

	class ConstantExpressionSet : public RefCountedObject
	{
	public:
		ConstantExpressionSet() {}
		ENGINE_API void serialize(Archive& ar);
		bool isEmpty() const;
		bool operator == (const ConstantExpressionSet & referenceSet) const; wstring getSummaryString() const;
		ENGINE_API const ConstantBufferStruct& getConstantBufferStruct() const;
		void createBufferStruct();
		ENGINE_API ConstantBufferRHIRef createConstantBuffer(const MaterialRenderContext& materialRenderContext, RHICommandList* commandListIfLocalMode, struct LocalConstantBuffer* outLocalConstantBuffer) const;
		void setParameterCollections(const TArray<class MaterialParameterCollection*>& collections);
	protected:
		TArray<TRefCountPtr<MaterialConstantExpression>> mConstantVectorExpressions;
		TArray<TRefCountPtr<MaterialConstantExpression>> mConstantScalarExpressions;
		TArray<TRefCountPtr<MaterialConstantExpressionTexture>>
			mConstant2DTextureExpression;
		TArray<TRefCountPtr<MaterialConstantExpressionTexture>>
			mConstantCubeTextureExpressions;
		TArray<TRefCountPtr<MaterialConstantExpression>>
			mPerFrameConstantScalarExpressions;
		TArray<TRefCountPtr<MaterialConstantExpression>>
			mPerFrameConstantVectorExpressions;
		TArray<TRefCountPtr<MaterialConstantExpression>>
			mPerFramePrevConstantScalarExpressions;
		TArray<TRefCountPtr<MaterialConstantExpression>>
			mPerFramePrevConstantVectorExpressions;

		TArray<Guid> mParameterCollections;

		TOptional<ConstantBufferStruct> mConstantBufferStruct;

		friend class FMaterial;
		friend class MaterialShader;
		friend class MaterialRenderProxy;
		friend class MaterialShaderMap;
		friend class DebugConstantExpressionSet;
		friend class HLSLMaterialTranslator;
		friend class XMLMaterialTranslator;
	};

	class MaterialCompilationOutput
	{
	public:
		MaterialCompilationOutput() {};

		ENGINE_API void serialize(Archive& ar);

		ConstantExpressionSet mConstantExpressionSet;

		bool bRequiresSceneColorCopy{ false };
		bool bNeedsSceneTextures{ false };
		bool bUsesEyeAdaptation{ false };
		bool bModifiesMeshPosition{ false };
		bool bUsesWorldPostionOffset{ false };
		bool bNeedsGBuffer{ false };
		bool bUsesGlobalDistanceField{ false };
		bool bUsesPixelDepthOffset{ false };
		bool bUsesSceneDepthLookup{ false };
	};

	class MaterialShaderMapId
	{
	public:
		Guid mBaseMaterialId;
		EMaterialQualityLevel::Type mQualityLevel;
		ERHIFeatureLevel::Type mFeatureLevel;
		EMaterialShaderMapUsage::Type mUsage;

		StaticParameterSet mParameterSet;

		TArray<Guid> mReferencedFunctions;

		TArray<Guid> mReferencedParameterCollections;

		TArray<ShaderTypeDependency>	mShaderTypeDependencies;

		TArray<ShaderPipelineTypeDependency> mShaderPipelineTypeDependencies;

		TArray<VertexFactoryTypeDependency> mVertexFactoryTypeDependencies;

		SHAHash mTextureReferencesHash;

		SHAHash mBasePropertyOverridesHash;

		bool containsShaderType(const ShaderType* shaderType) const;

		bool containsShaderPipelineType(const ShaderPipelineType* shaderPipelineType) const;

		bool containsVertexFactoryType(const VertexFactoryType* vfType) const;

		MaterialShaderMapId()
			:mBaseMaterialId(0, 0, 0, 0),
			mQualityLevel(EMaterialQualityLevel::High),
			mFeatureLevel(ERHIFeatureLevel::SM5),
			mUsage(EMaterialShaderMapUsage::Default)
		{}

		~MaterialShaderMapId()
		{}

		bool operator == (const MaterialShaderMapId& referenceSet) const;
		bool operator !=(const MaterialShaderMapId& referenceSet) const
		{
			return !(*this == referenceSet);
		}

		friend uint32 getTypeHash(const MaterialShaderMapId& ref)
		{
			return ref.mBaseMaterialId.A;
		}

		ENGINE_API void setShaderDependencies(const TArray<ShaderType*>& shaderTypes, const TArray<const ShaderPipelineType*>& shaderPipelineType, const TArray<VertexFactoryType*>& VFType);

		void appendKeyString(wstring& keyString) const;

		void getMaterialHash(SHAHash& outHash) const;

		void serialize(Archive& ar);
	};

	class MaterialShaderMap : public TShaderMap<MaterialShaderType>, public DeferredCleanupInterface
	{
	public:
		MaterialShaderMap();
		~MaterialShaderMap();


		bool isValidForRendering() const
		{
			return bCompilationFinalized && bCompiledSuccessfully && !bDeletedThroughDeferredCleanup;
		}

		const ConstantExpressionSet& getConstantExpressionSet() const { return mMaterialCompilationOutput.mConstantExpressionSet; }

		ENGINE_API const MeshMaterialShaderMap* getMeshShaderMap(VertexFactoryType* vertexFactoryType) const;

		bool isCompilationFinalized() const { return bCompilationFinalized; }

		bool compiledSuccessfully() const { return bCompiledSuccessfully; }

		virtual void registerSerializedShaders() override;

		EShaderPlatform getShaderPlatform() const { return mPlatform; }

		bool processCompilationResults(const TArray<ShaderCommonCompileJob*>& inCompilationResults, int32 & resultIndex, float& timeBudget, TMap<const VertexFactoryType*, TArray<const ShaderPipelineType*>>& shaderPipelines);

		Shader* processCompilationResultsForSingleJob(class ShaderCompileJob* singleJob, const ShaderPipelineType* shaderPipelineType, const SHAHash& materialShaderMapHash);

		void initOrderedMeshShaderMaps();

		void saveToDerivedDataCache();

		const MaterialShaderMapId& getShaderMapId() const { return mShaderMapId; }

		void compile(FMaterial* material, const MaterialShaderMapId& shaderMapId, TRefCountPtr<ShaderCompilerEnvironment> materialEnvironment, const MaterialCompilationOutput& inMateiralCompilationOutput, EShaderPlatform platform, bool bSynchronousCompile, bool bApplyCompletedShaderMapForRendering);

		virtual void finishCleanup()
		{
			bDeletedThroughDeferredCleanup = true;
			delete this;
		}

		void serialize(Archive& ar, bool bInlineShaderResources = true);

		ENGINE_API void AddRef();
		ENGINE_API void Release();

		void Register(EShaderPlatform inShaderPlatform);

		static MaterialShaderMap* findId(const MaterialShaderMapId& shaderMapId, EShaderPlatform platform);

		static void loadFromDerivedDataCache(const FMaterial* material, const MaterialShaderMapId& shaderMapId, EShaderPlatform platform, TRefCountPtr<MaterialShaderMap>& inOutShaderMap);

		bool tryToAddToExistingCompilationTask(FMaterial* material);

		bool isComplete(const FMaterial* material, bool bSilent);

		uint32 getCompilingId()const { return mCompilingId; }

		bool isMateiralShaderComplete(const FMaterial* mateiral, const MaterialShaderType* shaderType, const ShaderPipelineType* pipelineType, bool bSilent);

		const wstring& getFriendlyName() const { return mFriendlyName; }
	private:
		static TMap<MaterialShaderMapId, MaterialShaderMap*> GIdToMaterialShaderMap[SP_NumPlatforms];

		static TArray<MaterialShaderMap*> mAllMaterialShaderMaps;

		uint32 bCompilationFinalized : 1;
		uint32 bCompiledSuccessfully : 1;
		uint32 bRegistered : 1;
		uint32 bIsPersistent : 1;
		bool bDeletedThroughDeferredCleanup;

		MaterialCompilationOutput mMaterialCompilationOutput;

		TArray<MeshMaterialShaderMap*> mOrderredMeshShaderMaps;

		TindirectArray<MeshMaterialShaderMap> mMeshShaderMaps;

		EShaderPlatform mPlatform;

		MaterialShaderMapId mShaderMapId;

		uint32 mCompilingId;

		wstring mFriendlyName;

		static uint32 mNextCompilingId;

		mutable int32 mNumRef;

		static TMap<TRefCountPtr<MaterialShaderMap>, TArray<FMaterial*>> mShaderMapsBeingCompiled;

		friend class ShaderCompilingManager;
	};

	class FMaterial
	{
	public:
		FMaterial()
			:mRenderingThreadShaderMap(nullptr),
			Id_DEPRECATED(0, 0, 0, 0),
			mQualityLevel(EMaterialQualityLevel::High),
			bHasQualityLevelUsage(false),
			mFeatureLevel(ERHIFeatureLevel::SM5),
			bContainsInlineShaders(false),
			bLoadedCookedShaderMapId(false)
		{}
			
		
		virtual enum EBlendMode getBlendMode() const = 0;

		virtual enum EMaterialShadingModel getShadingModel() const = 0;

		virtual int32 getMaterialDomain() const = 0;

		virtual bool isMasked() const = 0;

		virtual bool isTwoSided() const = 0;

		virtual bool isWireFrame() const = 0;

		virtual void notifyCompilationFinished() {}

		virtual bool isLightFunction() const = 0;

		virtual wstring getFriendlyName() const = 0;

		virtual bool isPersistent() const = 0;

		virtual bool isSpecialEngineMaterial() const = 0;

		virtual bool requiresSynchronousCompilation()const { return false; }

		virtual const TArray<RTexture*>& getReferencedTextures() const = 0;

		virtual float getOpacityMaskClipValue() const = 0;

		void removeOutstandingCompileId(int32 const oldOutstandingCompileShaderMapId)
		{
			mOutstandingCompileShaderMapIds.remove(oldOutstandingCompileShaderMapId);
		}

		EMaterialQualityLevel::Type getQualityLevel() const
		{
			return mQualityLevel;
		}

		void setupMaterialEnvironment(EShaderPlatform platform, const ConstantExpressionSet& inConstantExpressionSet, ShaderCompilerEnvironment& outEnvironment)const;

		void registerInlineShaderMap();

		void releaseShaderMap();

		void setInlineShaderMap(MaterialShaderMap* inMaterialShaderMap);

		void getReferencedTexturesHash(EShaderPlatform platform, SHAHash& outHash) const;

		bool beginCompileShaderMap(const MaterialShaderMapId& shaderMapId, EShaderPlatform platform, TRefCountPtr<class MaterialShaderMap>& outShaderMap, bool bApplyCompletedShaderMapForRendering);

		void setQualityLevelProperties(EMaterialQualityLevel::Type inQualityLevel, bool bInHasQualityLevelUsage, ERHIFeatureLevel::Type inFeatureLevel)
		{
			mQualityLevel = inQualityLevel;
			bHasQualityLevelUsage = bInHasQualityLevelUsage;
			mFeatureLevel = inFeatureLevel;
		}

		virtual void gatherCustomOutputExpressions(TArray<class RMaterialExpressionCustomOutput*>& outCustomOutputs) const {}

		virtual bool isDefaultMaterial() const { return false; }

		virtual bool isDeferredDecal() const = 0;

		ENGINE_API virtual bool shouldCache(EShaderPlatform platform, const ShaderType* shaderType, const VertexFactoryType* vertexFactoryType) const;

		ENGINE_API virtual enum EMaterialTessellationMode getTessellationMode() const;

		ENGINE_API bool needsSceneTextures() const;

		ENGINE_API MaterialShaderMap* getRenderingThreadShaderMap() const;

		ENGINE_API void getDependentShaderAndVFTypes(EShaderPlatform platform, TArray<ShaderType*>& outShaderType, TArray<const ShaderPipelineType*>& outShaderPipelineTypes, TArray<VertexFactoryType*>& outVFTypes) const;

		virtual EMaterialShaderMapUsage::Type getShaderMapUsage() const { return EMaterialShaderMapUsage::Default; }

		virtual Guid getMaterialId() const = 0;

		EMaterialQualityLevel::Type getQualityLevelForShaderMapId() const
		{
			return bHasQualityLevelUsage ? mQualityLevel : EMaterialQualityLevel::Num;
		}

		virtual bool getAllowDevelopmentShaderCompile() const { return true; }

		ERHIFeatureLevel::Type getFeatureLevel() const { return mFeatureLevel; }

		template<typename ShaderType>
		ShaderType* getShader(VertexFactoryType* vertexFactoryType) const
		{
			return (ShaderType*)getShader(&ShaderType::mStaticType, vertexFactoryType);
		}

		ENGINE_API Shader* getShader(class MeshMaterialShaderType* ShaderType, VertexFactoryType* vertexFactoryType) const;

		class MaterialShaderMap* getGameThreadShaderMap() const
		{
			BOOST_ASSERT(isInGameThread() || isInAsyncLoadingThread());
			return mGameThreadShaderMap;
		}

		void setGameThreadShaderMap(MaterialShaderMap* inMaterialShaderMap)
		{
			BOOST_ASSERT(isInGameThread() || isInAsyncLoadingThread());
			mGameThreadShaderMap = inMaterialShaderMap;
		}

		ENGINE_API void setRenderingThreadShaderMap(MaterialShaderMap* inMaterialShaderMap);

		ENGINE_API bool cacheShaders(EShaderPlatform platform, bool bApplyCompletedShaderMapForRendering);

		ENGINE_API bool cacheShaders(const MaterialShaderMapId& shaderMapId, EShaderPlatform platform, bool bApplyCompletedShaderMapForRendering);

		const TArray<wstring>& getCompileErrors() const { return mCompileErrors; }

		ENGINE_API virtual void getShaderMapId(EShaderPlatform platform, MaterialShaderMapId& outShaderMapId) const;

		virtual void compilePropertyAndSetMaterialProperty(wstring** chunk, class MaterialCompiler* compiler, EShaderFrequency overrideShaderFrequency = SF_NumFrequencies, bool bUsePreviousFrameTime = false) const = 0;

		virtual int32 compilePropertyAndSetMaterialProperty(EMaterialProperty prop, class MaterialCompiler* compiler, EShaderFrequency overrideShaderFrequency = SF_NumFrequencies, bool bUsePreviousFrameTime = false) const = 0;

	private:
		MaterialShaderMap* mRenderingThreadShaderMap;

		TRefCountPtr<MaterialShaderMap>	mGameThreadShaderMap;

		ERHIFeatureLevel::Type mFeatureLevel;

		MaterialCompilationOutput mMaterialCompilationOutput;

		uint32 bContainsInlineShaders : 1;

		uint32 bLoadedCookedShaderMapId : 1;

		MaterialShaderMapId mCookedShaderMapId;

		Guid Id_DEPRECATED;


		EMaterialQualityLevel::Type mQualityLevel;

		TArray<wstring> mCompileErrors;
		TArray<int32, TInlineAllocator<1>> mOutstandingCompileShaderMapIds;

		TArray<class RMaterialExpression*> mErrorExpressions;

		bool bHasQualityLevelUsage;

		friend class MaterialShaderMap;
		friend class ShaderCompilingManager;
		friend class HLSLMaterialTranslator;
		friend class XMLMaterialTranslator;
	};

	class MaterialResource : public FMaterial
	{
	public:
		virtual enum EBlendMode getBlendMode() const override;

		virtual enum EMaterialShadingModel getShadingModel() const override;

		virtual int32 getMaterialDomain() const override;

		virtual bool isMasked() const override;

		virtual bool isTwoSided() const override;

		virtual bool isWireFrame() const override;

		virtual bool isPersistent() const override { return true; }

		virtual bool isDeferredDecal() const override;

		ENGINE_API virtual bool isDefaultMaterial() const override;

		ENGINE_API virtual bool isLightFunction() const override;

		virtual const TArray<RTexture*>& getReferencedTextures() const override;

		wstring getFriendlyName() const override;

		virtual float getOpacityMaskClipValue() const override;

		virtual Guid getMaterialId() const;

		void setMaterial(RMaterial* inMaterial, EMaterialQualityLevel::Type inQualityLevel, bool bInQualityLevelHasDifferentNodes, ERHIFeatureLevel::Type inFeatureLevel, MaterialInstance* instance = nullptr)
		{
			mMaterial = inMaterial;
			mMaterialInstance = instance;
			setQualityLevelProperties(inQualityLevel, bInQualityLevelHasDifferentNodes, inFeatureLevel);
		}

		ENGINE_API virtual bool isSpecialEngineMaterial() const override;


		virtual void compilePropertyAndSetMaterialProperty(wstring** chunk, class MaterialCompiler* compiler, EShaderFrequency overrideShaderFrequency = SF_NumFrequencies, bool bUsePreviousFrameTime = false) const override;

		virtual int32 compilePropertyAndSetMaterialProperty(EMaterialProperty prop, class MaterialCompiler* compiler, EShaderFrequency overrideShaderFrequency = SF_NumFrequencies, bool bUsePreviousFrameTime = false) const override;


	protected:
		RMaterial* mMaterial;
		MaterialInstance* mMaterialInstance;
	};

	struct ENGINE_API MaterialRenderContext
	{
		const MaterialRenderProxy* mMaterialRenderProxy;
		const FMaterial& mMaterial;

		float mTime;
		float mRealTime;
		bool bShowSelection;
		MaterialRenderContext(const MaterialRenderProxy* inMaterialProxy, const FMaterial& inMaterial, const SceneView* inView);
	};


	class MaterialConstantExpressionTexture : public MaterialConstantExpression
	{
	public:

		MaterialConstantExpressionTexture();
		MaterialConstantExpressionTexture(int32 inTextureIndex, ESamplerSourceMode inSamplerSource);
		virtual void serialize(Archive& ar);
		virtual void getTextureValue(const MaterialRenderContext& context, const FMaterial& material, const RTexture* & outValue, ESamplerSourceMode& outSamplerSource) const;

	protected:
		int32 mTextureIndex;
		ESamplerSourceMode mSamplerSource;
		RTexture* mTransientOverrideValue_GameThread;
		RTexture* mTransientOverrideValue_RenderThread;

	};

	inline bool isTranslucentBlendMode(enum EBlendMode blendMode)
	{
		return blendMode != BLEND_Opaque && blendMode != BLEND_Masked;
	}



	struct ConstantExpressionCache 
	{
		ConstantBufferRHIRef mConstantBuffer;
		LocalConstantBuffer mLocalConstantBuffer;
		TArray<Guid> mParameterCollections;
		bool bUpToData;
		const MaterialShaderMap* mCachedConstantExpressionShaderMap;
		ConstantExpressionCache()
			:bUpToData(false),
			mCachedConstantExpressionShaderMap(nullptr)
		{}

		~ConstantExpressionCache()
		{
			mConstantBuffer.safeRelease();
		}
	};

	class MaterialRenderProxy : public RenderResource
	{
	public:
		ENGINE_API MaterialRenderProxy();

		ENGINE_API MaterialRenderProxy(bool bInSelected, bool bInHovered);

		ENGINE_API virtual ~MaterialRenderProxy();

		mutable ConstantExpressionCache mConstantExpressionCache[ERHIFeatureLevel::Num];

		virtual const class FMaterial* getMaterial(ERHIFeatureLevel::Type inFeatureLevel) const = 0;

		void ENGINE_API evaluateConstantExpressions(ConstantExpressionCache& outConstantExpressionCache, const MaterialRenderContext& context, class RHICommandList* commandListIfLocalMode = nullptr) const;

		void ENGINE_API cacheConstantExpressions_GameThread();

		void ENGINE_API cacheConstantExpressions();

		virtual FMaterial* getMaterialNoFallback(ERHIFeatureLevel::Type inFeatureLevel) const;

		void setReferencedInDrawList() const
		{
#if !(BUILD_SHIPPING || BUILD_TEST)
			bIsStaticDrawListReferenced = 1;
#endif
		}

		void setUnreferencedInDrawList() const
		{
#if !(BUILD_SHIPPING || BUILD_TEST)
			bIsStaticDrawListReferenced = 0;
#endif
		}

		bool isSelected() const { return bSelected; }
		bool isHovered() const { return bHovered; }

		virtual bool getVectorValue(const wstring parameterName, LinearColor* outValue, const MaterialRenderContext& context) const = 0;

		virtual bool getScalarValue(const wstring parameterName, float* outValue, const MaterialRenderContext& context) const = 0;

		virtual bool getTextureValue(const wstring parameterName, const RTexture** outValue, const MaterialRenderContext& context) const = 0;

		void ENGINE_API invalidateConstantExpressionCache();

	private:
#if !(BUILD_SHIPPING || BUILD_TEST)
		mutable int32 mDeletedFlag : 1;
		mutable int32 bIsStaticDrawListReferenced : 1;
#endif

		bool bSelected : 1;
		bool bHovered : 1;
	};

	

	

	class MeshMaterialShaderMap : public TShaderMap<MeshMaterialShaderType>
	{
	public:
		MeshMaterialShaderMap(VertexFactoryType* inVFType)
			:mVertexFactoryType(inVFType)
		{}
		uint32 beginCompile(
			uint32 shaderMapId,
			const MaterialShaderMapId& inShaderMapId,
			const FMaterial* material,
			ShaderCompilerEnvironment* materialEnvironment,
			EShaderPlatform platform,
			TArray<ShaderCommonCompileJob*>& newJobs);

		static bool isComplete(
			const MeshMaterialShaderMap* meshShaderMap,
			EShaderPlatform platform,
			const FMaterial* material,
			VertexFactoryType* inVertexFactoryType,
			bool bSilent);

		void loadMissingShadersFromMemory(
			const SHAHash& materialShaderMapHash,
			const FMaterial* material,
			EShaderPlatform platform);

		void flushShaderByShaderType(ShaderType* shaderType);

		void flushShaderByShaderPipelineType(const ShaderPipelineType* shaderPipelineType);

		inline VertexFactoryType* getVertexFactoryType() const {
			return mVertexFactoryType;
		}

	private:
		VertexFactoryType* mVertexFactoryType;

		static bool isMeshShaderComplete(const MeshMaterialShaderMap* meshShaderMap, EShaderPlatform platform, const FMaterial* mateiral, const MeshMaterialShaderType* shaderType, const ShaderPipelineType* pipeline, VertexFactoryType* inVertexFactoryType, bool bSilent);

	};

	enum EMaterialValueType
	{
		MCT_Float1		= 1,
		MCT_Float2		= 2,
		MCT_Float3		= 4,
		MCT_Float4		= 8,

		MCT_Float		=8|4|2|1,
		MCT_Texture2D	=16,
		MCT_TextureCube =32,
		MCT_Texture		=16|32,
		MCT_StaticBool	=64,
		MCT_Unknown		=128,
		MCT_MaterialAttributes	= 256
	};

	typedef int32(*MaterialAttributeBlendFunction)(MaterialCompiler* compiler, int32 A, int32 B, int32 Alpha);

	class MaterialAttributeDefination
	{
	public:
		MaterialAttributeDefination(const Guid& inGuid, const wstring& inDisplayName, EMaterialProperty inProperty, EMaterialValueType inValueType, const float4 & inDefaultValue, EShaderFrequency inShaderFrequency, int32 inTexCoordIndex = INDEX_NONE, bool bInIsHidden = false, MaterialAttributeBlendFunction inBlendFunction = nullptr);

		int32 compileDefaultValue(MaterialCompiler* compiler);

		bool operator== (const MaterialAttributeDefination& other) const
		{
			return (mAttributeID == other.mAttributeID);
		}

		Guid				mAttributeID;
		wstring				mDisplayName;
		EMaterialProperty	mProperty;
		EMaterialValueType	mValueType;
		float4				mDefaultValue;
		EShaderFrequency	mShaderFrequency;
		int32				mTexCoordIndex;

		MaterialAttributeBlendFunction mBlendFunction;
		bool				bIsHidden;
	};

	class MaterialCustomOutputAttributeDefination : public MaterialAttributeDefination
	{
	public:
		MaterialCustomOutputAttributeDefination(const Guid& inGuid, const wstring& inDisplayName, const wstring& inFunctionName, EMaterialProperty inProperty, EMaterialValueType inValueType, const float4 & inDefaultValue, EShaderFrequency inShaderFrequency, int32 inTexCoordIndex = INDEX_NONE, bool bInIsHidden = false, MaterialAttributeBlendFunction inBlendFunction = nullptr);

		bool operator == (const MaterialCustomOutputAttributeDefination& other) const
		{
			return (mAttributeID == other.mAttributeID);
		}


		wstring	mFunctionName;
	};

	
	class MaterialAttributeDefinationMap
	{
	public:
		MaterialAttributeDefinationMap()
			:mAttributeDDCString(TEXT(""))
			, bIsInitialized(false)
		{
			mAttributeMap.reserve(MP_Max);
			initializeAttributeMap();
		}


		ENGINE_API static EShaderFrequency getShaderFrequency(EMaterialProperty property)
		{
			MaterialAttributeDefination* attribute = GMaterialPropertyAttributesMap.find(property);
			return attribute->mShaderFrequency;
		}

		ENGINE_API static wstring getDisplayName(EMaterialProperty prop)
		{
			MaterialAttributeDefination* attribute = GMaterialPropertyAttributesMap.find(prop);
			return attribute->mDisplayName;
		}

		ENGINE_API static wstring getDisplayName(const Guid& attributeId)
		{
			MaterialAttributeDefination* attribute = GMaterialPropertyAttributesMap.find(attributeId);
			return attribute->mDisplayName;
		}

		ENGINE_API static EMaterialValueType getValueType(EMaterialProperty prop)
		{
			MaterialAttributeDefination* attribute = GMaterialPropertyAttributesMap.find(prop);
			return attribute->mValueType;
		}

		ENGINE_API static EMaterialValueType getValueType(const Guid& attributeID)
		{
			MaterialAttributeDefination* attribute = GMaterialPropertyAttributesMap.find(attributeID);
			return attribute->mValueType;
		}

		ENGINE_API static Guid getID(EMaterialProperty prop)
		{
			MaterialAttributeDefination* attribute = GMaterialPropertyAttributesMap.find(prop);
			return attribute->mAttributeID;
		}

		ENGINE_API static int32 compileDefaultExpression(MaterialCompiler* compiler, EMaterialProperty prop)
		{
			MaterialAttributeDefination* attribute = GMaterialPropertyAttributesMap.find(prop);
			return attribute->compileDefaultValue(compiler);
		}

		ENGINE_API static int32 compileDefaultExpression(MaterialCompiler* compiler,const  Guid& prop)
		{
			MaterialAttributeDefination* attribute = GMaterialPropertyAttributesMap.find(prop);
			return attribute->compileDefaultValue(compiler);
		}

		ENGINE_API static EMaterialProperty getProperty(const Guid& attributeID)
		{
			if (MaterialAttributeDefination* attribute = GMaterialPropertyAttributesMap.find(attributeID))
			{
				return attribute->mProperty;
			}
			return MP_Max;
		}
	private:
		ENGINE_API static MaterialAttributeDefinationMap GMaterialPropertyAttributesMap;

		void initializeAttributeMap();

		void add(const Guid& attributeId, const wstring& displayName, EMaterialProperty property, EMaterialValueType valueType, const float4& defalutValue, EShaderFrequency shaderFrequency, int32 texCoordIndex = INDEX_NONE, bool bIsHidden = false, MaterialAttributeBlendFunction blendFunction = nullptr);

		MaterialAttributeDefination* find(const Guid& attributeId);
		MaterialAttributeDefination* find(EMaterialProperty property);

		TMap<EMaterialProperty, MaterialAttributeDefination>	mAttributeMap;
		TArray<MaterialCustomOutputAttributeDefination>	mCustomAttributes;
		wstring	mAttributeDDCString;
		bool bIsInitialized;
	};

	class RMaterialExpression;
	class MaterialExpressionKey
	{
	public:
		RMaterialExpression * mExpression;
		int32 mOuputIndex;
		Guid mMaterialAttributeID;
		bool bCompilingPreviousFrameKey;
		MaterialExpressionKey(RMaterialExpression* inExpression, int32 inOutputIndex)
			:mExpression(inExpression)
			,mOuputIndex(inOutputIndex)
			,mMaterialAttributeID(Guid(0, 0, 0, 0))
			,bCompilingPreviousFrameKey(false)
		{}

		MaterialExpressionKey(RMaterialExpression* inExpression, int32 inOutputIndex, const Guid& inMaterialAttributeId, bool bInCompilingPreviousFrameKey)
			:mExpression(inExpression)
			,mOuputIndex(inOutputIndex)
			,mMaterialAttributeID(inMaterialAttributeId)
			,bCompilingPreviousFrameKey(bInCompilingPreviousFrameKey)
		{}

		friend bool operator == (const MaterialExpressionKey& x, const MaterialExpressionKey& y)
		{
			return x.mExpression == y.mExpression && x.mOuputIndex == y.mOuputIndex && x.mMaterialAttributeID == y.mMaterialAttributeID && x.bCompilingPreviousFrameKey == y.bCompilingPreviousFrameKey;
		}

		friend uint32 getTypeHash(const MaterialExpressionKey& expressionKey)
		{
			return pointerHash(expressionKey.mExpression);
		}

	};

	class MaterialFunctionCompileState
	{
	public:
		class RMaterialExpressionMaterialFunctionCall* mFunctionCall;
		TArray<MaterialExpressionKey> mExpressionStack;

		TMap<MaterialExpressionKey, int32> mExpressionCodeMap;
		explicit MaterialFunctionCompileState(RMaterialExpressionMaterialFunctionCall* inFunction)
			:mFunctionCall(inFunction)
		{}
	};

#if WITH_EDITORONLY_DATA
	ENGINE_API void doMaterialAttributeReorder(class ExpressionInput* input, int32 version);
#endif
}