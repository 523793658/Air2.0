#pragma once
#include "ShaderCoreConfig.h"
#include "RenderingThread.h"
#include "ShaderCore.h"
#include "Misc/Crc.h"
#include "RenderResource.h"
#include "Containers/Array.h"
#include "Serialization/Archive.h"
#include "Containers/Map.h"
#include "Serialization/ArchiveProxy.h"
#include "ShaderParameters.h"
#include "ShaderPermutation.h"
#include "Misc/ScopeLock.h"
#include "Containers/HashTable.h"
#include "Serialization/MemoryImage.h"
namespace Air
{
	extern RENDER_CORE_API SHAHash GGlobalShaderMapHash;

	class VertexFactoryType;
	class ShaderType;
	class Shader;
	class ShaderConstantBufferParameter;
	class VertexFactoryParameterRef;


	template<typename MetaShaderType>
	struct TShaderTypePermutation
	{
		MetaShaderType* const mType;
		const int32 mPermutationId;
		TShaderTypePermutation(MetaShaderType* inType, int32 inPermutationId)
			:mType(inType)
			,mPermutationId(inPermutationId)
		{}

		FORCEINLINE bool operator == (const TShaderTypePermutation& other) const
		{
			return mType == other.mType && mPermutationId == other.mPermutationId;
		}

		FORCEINLINE bool operator != (const TShaderTypePermutation& other) const
		{
			return !(*this == other);
		}
	};

	using ShaderPermutation = TShaderTypePermutation<ShaderType>;

	template<typename MetaShaderType>
	FORCEINLINE uint32 getTypeHash(const TShaderTypePermutation<MetaShaderType>& var)
	{
		size_t hashValue = getTypeHash(var.mType);
		boost::hash_combine(hashValue, var.mPermutationId);
		return hashValue;
	}

	class ShaderParameterInfo
	{
	public:
		uint16 mBaseIndex;
		uint16 mSize;

		ShaderParameterInfo() {}

		ShaderParameterInfo(uint16 inBaseIndex, uint16 inSize)
		{
			mBaseIndex = inBaseIndex;
			mSize = inSize;
			BOOST_ASSERT(mBaseIndex == inBaseIndex && mSize == inSize);
		}

		friend Archive& operator << (Archive& ar, ShaderParameterInfo& info)
		{
			ar << info.mBaseIndex;
			ar << info.mSize;
			return ar;
		}

		inline bool operator == (const ShaderParameterInfo& rhs) const
		{
			return mBaseIndex == rhs.mBaseIndex && mSize == rhs.mSize;
		}
	};

	class ShaderLooseParameterBufferInfo
	{
	public:
		uint16 mBufferIndex;
		uint16 mBufferSize;
		TArray<ShaderParameterInfo> mParameters;

		ShaderLooseParameterBufferInfo() {}

		ShaderLooseParameterBufferInfo(uint16 inBufferIndex, uint16 inBufferSize)
		{
			mBufferIndex = inBufferIndex;
			mBufferSize = inBufferSize;
			BOOST_ASSERT(mBufferIndex == inBufferIndex);
		}

		friend Archive& operator << (Archive& ar, ShaderLooseParameterBufferInfo& info)
		{
			ar << info.mBufferIndex;
			ar << info.mBufferSize;
			ar << info.mParameters;
			return ar;
		}

		inline bool operator == (const ShaderLooseParameterBufferInfo& rhs) const
		{
			return mBufferIndex == rhs.mBufferIndex
				&& mBufferSize == rhs.mBufferSize
				&& mParameters == rhs.mParameters;
		}
	};

	class ShaderParameterMapInfo
	{
	public:
		TArray<ShaderParameterInfo> mConstantBuffers;
		TArray<ShaderParameterInfo> mTextureSamplers;
		TArray<ShaderParameterInfo> mSRVs;

		TArray<ShaderLooseParameterBufferInfo> mLooseParameterBuffers;

		friend Archive& operator << (Archive& ar, ShaderParameterMapInfo& info)
		{
			ar << info.mConstantBuffers;
			ar << info.mTextureSamplers;
			ar << info.mSRVs;
			ar << info.mLooseParameterBuffers;
			return ar;
		}

		inline bool operator == (const ShaderParameterMapInfo& rhs) const
		{
			return mConstantBuffers == rhs.mConstantBuffers
				&& mTextureSamplers == rhs.mTextureSamplers
				&& mSRVs == rhs.mSRVs
				&& mLooseParameterBuffers == rhs.mLooseParameterBuffers;
		}
	};


#define INTERNAL_DECLARE_SHADER_TYPE_COMMON(ShaderClass, ShaderMetaTypeShortcut, RequiredAPI) \
	public: \
	using ShaderMetaType = ShaderMetaTypeShortcut##ShaderType; \
	using ShaderMapType = ShaderMetaTypeShortcut##ShaderMap; \
	\
	static RequiredAPI ShaderMetaType mStaticType; \
	\
	SHADER_DECLARE_VTABLE(ShaderClass)
	


#define DECLARE_EXPORTED_SHADER_TYPE(ShaderClass, ShaderMetaTypeShortcut, RequiredAPI, ...)  \
	INTERNAL_DECLARE_SHADER_TYPE_COMMON(ShaderClass, ShaderMetaTypeShortcut, RequiredAPI); \
	public:

#define DECLARE_SHADER_TYPE(ShaderClass, ShaderMetaTypeShortcut, ...)	\
	DECLARE_EXPORTED_SHADER_TYPE(ShaderClass, ShaderMetaTypeShortcut,, ##_VA_ARGS__)

#define SHADER_TYPE_VTABLE(ShaderClass) \
	ShaderClass::constructSerializedInstance, \
	ShaderClass::constructCompiledInstance, \
	ShaderClass::modifyCompilationEnvironmentImpl, \
	ShaderClass::shouldCompilePermutationImpl, \
	ShaderClass::validateCompiledResult
	


#define IMPLEMENT_SHADER_TYPE(TemplatePrefix, ShaderClass, SourceFilename, FunctionName, Frequency) \
	TemplatePrefix	\
	ShaderClass::ShaderMetaType	ShaderClass::mStaticType(\
		TEXT(#ShaderClass),	\
		SourceFilename,	\
		FunctionName,	\
		Frequency,	\
		ShaderClass::PermutationDomain::mPermutationCount, \
		SHADER_TYPE_VTABLE(ShaderClass), \
		sizeof(ShaderClass), \
		ShaderClass::getRootParametersMetadata() \
	);

#define IMPLEMENT_SHADER_TYPE2(ShaderClass, Frequency)	\
	template<>	\
	ShaderClass::ShaderMetaType ShaderClass::mStaticType(	\
	TEXT(#ShaderClass),	\
	ShaderClass::getSourceFilename(),	\
	ShaderClass::getFunctionName(),	\
	Frequency,	\
	1, \
	ShaderClass::constructSerializedInstance,	\
	ShaderClass::constructCompiledInstance,	\
	ShaderClass::modifyCompilationEnvironment,	\
	ShaderClass::shouldCompilePermutation,	\
	ShaderClass::validateCompiledResult, \
	ShaderClass::getStreamOutElements	\
		);

#define IMPLEMENT_SHADER_TYPE2_WITH_TEMPLATE_PREFIX(TemplatePrefix, ShaderClass, Frequency) \
	TemplatePrefix \
	ShaderClass::ShaderMetaType ShaderClass::mStaticType(\
		TEXT(#ShaderClass),\
		ShaderClass::getSourceFilename(),\
		ShaderClass::getFunctionName(),\
		Frequency,\
		1,\
		ShaderClass::constructSerializedInstance,\
		ShaderClass::constructCompiledInstance,\
		ShaderClass::modifyCompilationEnvironment,\
		ShaderClass::shouldCompilePermutation,\
		ShaderClass::validateCompiledResult,\
		ShaderClass::getStreamOutElements\
	);

	class ShaderResourceId
	{
	public:
		ShaderResourceId() {}
		ShaderResourceId(const ShaderTarget& inTarget, const SHAHash& inOutputHash, const TCHAR* inSpecificShaderTypeName, int32 inSpecificPermutationId)
			:
			mOutputHash(inOutputHash),
			mTarget(inTarget),
			mSpecificShaderTypeName(inSpecificShaderTypeName),
			mSpecificPermutationId(inSpecificPermutationId)
		{
			BOOST_ASSERT(!(mSpecificShaderTypeName == nullptr && inSpecificPermutationId != 0));
		}

		friend inline uint32 getTypeHash(const ShaderResourceId& id)
		{
			return Crc::memCrc_DEPRECATED((const void*) &id.mOutputHash, sizeof(id.mOutputHash));
		}

		friend bool operator == (const ShaderResourceId& x, const ShaderResourceId& y)
		{
			return x.mTarget == y.mTarget && x.mOutputHash == y.mOutputHash
				&& ((x.mSpecificShaderTypeName == nullptr  && y.mSpecificShaderTypeName == nullptr) || (CString::strcmp(x.mSpecificShaderTypeName, y.mSpecificShaderTypeName) == 0));
		}

		friend bool operator !=(const ShaderResourceId& x, const ShaderResourceId& y)
		{
			return !(x == y);
		}

		friend Archive & operator << (Archive& ar, ShaderResourceId& id)
		{
			ar << id.mTarget << id.mOutputHash;
			id.mSpecificShaderTypeStorge = id.mSpecificShaderTypeName ? id.mSpecificShaderTypeName : TEXT("");
			ar << id.mSpecificShaderTypeStorge;
			return ar;
		}



		ShaderTarget mTarget;
		SHAHash mOutputHash;
		const TCHAR* mSpecificShaderTypeName;
		wstring mSpecificShaderTypeStorge;

		int32 mSpecificPermutationId;
	};


	class RENDER_CORE_API ShaderMapResource : public RenderResource, public DeferredCleanupInterface
	{
	public:
		static bool arePlatformsCompatible(EShaderPlatform currentPlatform, EShaderPlatform targetPlatform);

		EShaderPlatform getPlatform() const { return mPlatform; }

		void AddRef();

		void Release();

		inline int32 GetNumRefs() const { return mNumRef; }

		virtual void releaseRHI();

		inline int32 getNumShaders() const
		{
			return mNumRHIShaders;
		}

		inline bool hasShader(int32 shaderIndex) const
		{
			return mRHIShaders[shaderIndex].load(std::memory_order_acquire) != nullptr;
		}

		inline RHIShader* getShader(int32 shaderIndex)
		{
			RHIShader* shader = mRHIShaders[shaderIndex].load(std::memory_order_acquire);
			if (shader == nullptr)
			{
				ScopeLock scopeLock(&mRHIShadersCreationGuard);
				shader = mRHIShaders[shaderIndex].load(std::memory_order_relaxed);
				if (shader == nullptr)
				{
					shader = createShader(shaderIndex);
					mRHIShaders[shaderIndex].store(shader, std::memory_order_release);
				}
			}
			return shader;
		}

		void beginCreateAllShaders();

		virtual uint32 getSizeBytes() const = 0;

	protected:
		explicit ShaderMapResource(EShaderPlatform inPlatform, int32 numShaders);

		virtual ~ShaderMapResource();

		uint32 getAllocatedSize() const
		{
			uint32 size = mNumRHIShaders * sizeof(std::atomic<RHIShader*>);
			return size;
		}

		RHIShader* createShader(int32 shaderIndex);

		virtual TRefCountPtr<RHIShader> createRHIShader(int32 shaderIndex) = 0;

		virtual bool tryRelease() { return true; }

		void releaseShaders();

	private:
		CriticalSection mRHIShadersCreationGuard;

		std::unique_ptr<std::atomic<RHIShader*>[]> mRHIShaders;

		int32 mNumRHIShaders;

		EShaderPlatform mPlatform;

		int32 mNumRef;
	};

	class RENDER_CORE_API ShaderMapResource_InlineCode : public ShaderMapResource
	{
	public:
		ShaderMapResource_InlineCode(EShaderPlatform InPlatform, ShaderMapResourceCode* InCode)
			: ShaderMapResource(InPlatform, InCode->mShaderEntries.size())
			, Code(InCode)
		{}

		// FShaderMapResource interface
		virtual TRefCountPtr<RHIShader> createRHIShader(int32 ShaderIndex) override;
		virtual uint32 getSizeBytes() const override { return sizeof(*this) + getAllocatedSize(); }

		TRefCountPtr<ShaderMapResourceCode> Code;
	};

	class ShaderMapResourceCode : public ThreadSafeRefCountedObject
	{
	public:
		struct ShaderEntry
		{
			TArray<uint8> mCode;
			int32 mUncompressedSize;
			EShaderFrequency mFrequency;

			friend Archive& operator << (Archive& ar, ShaderEntry& entry)
			{
				uint8 freq = entry.mFrequency;
				ar << entry.mCode << entry.mUncompressedSize << freq;
				entry.mFrequency = (EShaderFrequency)freq;
				return ar;
			}
		};

		RENDER_CORE_API ~ShaderMapResourceCode();

		RENDER_CORE_API void finalize();

		RENDER_CORE_API void serialize(Archive& ar, bool bLoadedByCookedMaterial);

		RENDER_CORE_API uint32 getSizeBytes() const;

		RENDER_CORE_API void addShaderCompilerOutput(const ShaderCompilerOutput& output);

		int32 findShaderIndex(const SHAHash& inHash) const;

		SHAHash mResourceHash;
		TArray<SHAHash> mShaderHashes;
		TArray<ShaderEntry> mShaderEntries;
#if WITH_EDITORONLY_DATA
		TArray<TArray<uint8>> mPlatformDebugData;
		TArray<SHAHash> mPlatformDebugDataHashes;
#endif // WITH_EDITORONLY_DATA
	};

	class RENDER_CORE_API ShaderMapPointerTable
	{
	public:
		template<typename T>
		int32 AddIndexedPointer(void* ptr);

		template<typename T>
		void* GetIndexedPointer(uint32 i) const;

		TArray<ShaderType*> mShaderTypes;
		TArray<VertexFactoryType*> mVFType;
	};

	template<>
	int32 ShaderMapPointerTable::AddIndexedPointer<ShaderType>(void* ptr)
	{
		mShaderTypes.push_back((ShaderType*)ptr);
		return mShaderTypes.size() - 1;
	}

	template<>
	int32 ShaderMapPointerTable::AddIndexedPointer<VertexFactoryType>(void* ptr)
	{
		mVFType.push_back((VertexFactoryType*)ptr);
		return mVFType.size() - 1;
	}
	
	template<>
	void* ShaderMapPointerTable::GetIndexedPointer<ShaderType>(uint32 i) const
	{
		return mShaderTypes[i];
	}

	template<>
	void* ShaderMapPointerTable::GetIndexedPointer<VertexFactoryType>(uint32 i) const
	{
		return mVFType[i];
	}
	class ShaderPipeline;


	
	class RENDER_CORE_API ShaderMapContent
	{
	public:
		struct ProjectShaderPipelineToKey
		{
			inline bool operator()(const HashedName& name, const ShaderPipeline* inShaderPipeline){ return inShaderPipeline->mTypeName == name; }
		};


		EShaderPlatform getShaderPlatform() const { return mPlatform; }
		void validate(const ShaderMapBase& InShaderMap);

		/** Finds the shader with the given type.  Asserts on failure. */
		template<typename ShaderType>
		ShaderType* GetShader(int32 PermutationId = 0) const
		{
			Shader* shader = getShader(&ShaderType::mStaticType, PermutationId);
			checkf(Shader != nullptr, TEXT("Failed to find shader type %s in Platform %s"), ShaderType::mStaticType.GetName(), *legacyShaderPlatformToShaderFormat(mPlatform).ToString());
			return static_cast<ShaderType*>(Shader);
		}

		/** Finds the shader with the given type.  Asserts on failure. */
		template<typename ShaderType>
		ShaderType* getShader(const typename ShaderType::FPermutationDomain& PermutationVector) const
		{
			return getShader<ShaderType>(PermutationVector.toDimensionValueId());
		}

		/** Finds the shader with the given type.  May return NULL. */
		Shader* getShader(ShaderType* ShaderType, int32 PermutationId = 0) const
		{
			return getShader(ShaderType->getHashedName(), PermutationId);
		}

		/** Finds the shader with the given type name.  May return NULL. */
		Shader* getShader(const HashedName& TypeName, int32 PermutationId = 0) const;

		/** Finds the shader with the given type. */
		bool hasShader(const HashedName& TypeName, int32 PermutationId) const
		{
			const Shader* shader = getShader(TypeName, PermutationId);
			return shader != nullptr;
		}

		bool hasShader(const ShaderType* Type, int32 PermutationId) const
		{
			return hasShader(Type->getHashedName(), PermutationId);
		}

		inline TArrayView<Shader* const> getShaders() const
		{
			return mShaders;
		}

		inline TArrayView<ShaderPipeline* const> getShaderPipelines() const
		{
			return mShaderPipelines;
		}

		void addShader(const HashedName& TypeName, int32 PermutationId, Shader* shader);

		Shader* findOrAddShader(const HashedName& TypeName, int32 PermutationId, Shader* Shader);

		void addShaderPipeline(ShaderPipeline* Pipeline);

		ShaderPipeline* findOrAddShaderPipeline(ShaderPipeline* Pipeline);

		/**
		 * Removes the shader of the given type from the shader map
		 * @param Type Shader type to remove the entry for
		 */
		void removeShaderTypePermutaion(const HashedName& TypeName, int32 PermutationId);

		inline void removeShaderTypePermutaion(const ShaderType* Type, int32 PermutationId)
		{
			removeShaderTypePermutaion(Type->getHashedName(), PermutationId);
		}

		void removeShaderPipelineType(const ShaderPipelineType* shaderPipelineType);

		/** Builds a list of the shaders in a shader map. */
		void getShaderList(const ShaderMapBase& InShaderMap, const SHAHash& InMaterialShaderMapHash, TMap<ShaderId, TShaderRef<Shader>>& OutShaders) const;

		/** Builds a list of the shaders in a shader map. Key is FShaderType::TypeName */
		void getShaderList(const ShaderMapBase& InShaderMap, TMap<HashedName, TShaderRef<Shader>>& OutShaders) const;

		/** Builds a list of the shader pipelines in a shader map. */
		void getShaderPipelineList(const ShaderMapBase& InShaderMap, TArray<ShaderPipelineRef>& OutShaderPipelines, ShaderPipeline::EFilter Filter) const;

#if WITH_EDITOR
		uint32 getMaxTextureSamplersShaderMap(const ShaderMapBase& InShaderMap) const;

		void getOutdatedTypes(const ShaderMapBase& InShaderMap, TArray<const ShaderType*>& OutdatedShaderTypes, TArray<const ShaderPipelineType*>& OutdatedShaderPipelineTypes, TArray<const VertexFactoryType*>& OutdatedFactoryTypes) const;

		void saveShaderStableKeys(const ShaderMapBase& InShaderMap, EShaderPlatform TargetShaderPlatform, const struct FStableShaderKeyAndValue& SaveKeyVal);
#endif // WITH_EDITOR

		/** @return true if the map is empty */
		inline bool isEmpty() const
		{
			return mShaders.size() == 0;
		}

		/** @return The number of shaders in the map. */
		uint32 getNumShaders() const;

		/** @return The number of shader pipelines in the map. */
		inline uint32 getNumShaderPipelines() const
		{
			return mShaderPipelines.size();
		}

		/** clears out all shaders and deletes shader pipelines held in the map */
		void empty();

		inline ShaderPipeline* getShaderPipeline(const HashedName& PipelineTypeName) const
		{
			const int32 Index = std::binary_search(mShaderPipelines.begin(), mShaderPipelines.end(), PipelineTypeName, ProjectShaderPipelineToKey());
			return (Index != INDEX_NONE) ? mShaderPipelines[Index] : nullptr;
		}

		inline ShaderPipeline* getShaderPipeline(const ShaderPipelineType* PipelineType) const
		{
			return getShaderPipeline(PipelineType->getHashedName());
		}

		inline bool hasShaderPipeline(const HashedName& PipelineTypeName) const { return getShaderPipeline(PipelineTypeName) != nullptr; }
		inline bool hasShaderPipeline(const ShaderPipelineType* PipelineType) const { return (getShaderPipeline(PipelineType) != nullptr); }

		uint32 getMaxNumInstructionsForShader(const ShaderMapBase& InShaderMap, ShaderType* ShaderType) const;

		void finalize(const ShaderMapResourceCode* Code);

		void UpdateHash(SHA1& Hasher) const;

	protected:

		void EmptyShaderPipelines();


		using HashTypeDefault = THashTable<DefaultAllocator>;
		HashTypeDefault mShaderHash;
		TArray<void*> mShaderTypes;
		TArray<int32> mShaderPermutations;
		TArray<Shader*> mShaders;
		TArray<ShaderPipeline*> mShaderPipelines;

		EShaderPlatform mPlatform;
	};


	class RENDER_CORE_API ShaderMapBase
	{
	public:
		virtual ~ShaderMapBase();

		ShaderMapResourceCode* getResourceCode();

		inline ShaderMapResource* getResource() const { return mResource; }

		inline ShaderMapResource* getResourceChecked() const { BOOST_ASSERT(mResource); return mResource; }

		inline const ShaderMapPointerTable* getPointerTable() const { BOOST_ASSERT(mPointerTable); return mPointerTable; }

		inline const ShaderMapContent* getContent() const { return mContent; }

		inline ShaderMapContent* getMutableContent()
		{
			return mContent;
		}

		inline EShaderPlatform getShaderPlatform() const { return mContent ? mContent->getShaderPlatform() : SP_NumPlatforms; }

		void assignContent(ShaderMapContent* inContent);
		void finalizeContent();
		bool serialize(Archive& ar, bool bInlineShaderResource, bool bLoadedByCookedMaterial, bool bInlineShaderCode = false);

	protected:
		explicit ShaderMapBase();

		void destroyContent();

		virtual ShaderMapPointerTable* createPointerTable() const = 0;

	private:
		TRefCountPtr<ShaderMapResource> mResource;
		TRefCountPtr<ShaderMapResourceCode> mCode;
		ShaderMapPointerTable* mPointerTable;
		ShaderMapContent* mContent;
	};
	class ShaderType;
	template<typename ShaderType, typename PointerTableType>
	class TShaderRefBase
	{
	public:
		TShaderRefBase() : mShaderContent(nullptr), mShaderMap(nullptr) {}
		TShaderRefBase(ShaderType* InShader, const ShaderMapBase& InShaderMap) : mShaderContent(InShader), mShaderMap(&InShaderMap) {}
		TShaderRefBase(const TShaderRefBase&) = default;

		template<typename OtherShaderType, typename OtherPointerTableType>
		TShaderRefBase(const TShaderRefBase<OtherShaderType, OtherPointerTableType>&Rhs) : mShaderContent(Rhs.GetShader()), mShaderMap(Rhs.GetShaderMap()) {}

		TShaderRefBase& operator=(const TShaderRefBase&) = default;

		template<typename OtherShaderType, typename OtherPointerTableType>
		TShaderRefBase& operator=(const TShaderRefBase<OtherShaderType, OtherPointerTableType>&Rhs)
		{
			mShaderContent = Rhs.GetShader();
			mShaderMap = Rhs.GetShaderMap();
			return *this;
		}

		template<typename OtherShaderType, typename OtherPointerTableType>
		static TShaderRefBase<ShaderType, PointerTableType> cast(const TShaderRefBase<OtherShaderType, OtherPointerTableType>&Rhs)
		{
			return TShaderRefBase<ShaderType, PointerTableType>(static_cast<ShaderType*>(Rhs.GetShader()), Rhs.GetShaderMapChecked());
		}

		template<typename OtherShaderType, typename OtherPointerTableType>
		static TShaderRefBase<ShaderType, PointerTableType> reinterpretCast(const TShaderRefBase<OtherShaderType, OtherPointerTableType>&Rhs)
		{
			return TShaderRefBase<ShaderType, PointerTableType>(reinterpret_cast<ShaderType*>(Rhs.GetShader()), Rhs.GetShaderMapChecked());
		}

		inline bool isValid() const { return mShaderContent != nullptr; }
		inline bool isNull() const { return mShaderContent == nullptr; }

		inline void reset() { mShaderContent = nullptr; mShaderMap = nullptr; }

		inline ShaderType* getShader() const { return mShaderContent; }
		inline const ShaderMapBase* getShaderMap() const { return mShaderMap; }
		inline const ShaderMapBase& getShaderMapChecked() const { check(mShaderMap); return *mShaderMap; }
		inline ShaderType* getType() const { return mShaderContent->getType(getPointerTable()); }
		inline VertexFactoryType* getVertexFactoryType() const { return mShaderContent->getVertexFactoryType(GetPointerTable()); }
		inline ShaderMapResource& getResourceChecked() const { ShaderMapResource* Resource = getResource(); check(Resource); return *Resource; }
		const PointerTableType& getPointerTable() const;
		ShaderMapResource* getResource() const;

		inline ShaderType* operator->() const { return mShaderContent; }

		inline RHIShader* getRHIShaderBase(EShaderFrequency Frequency) const
		{
			RHIShader* RHIShader = nullptr;
			if (mShaderContent)
			{
				BOOST_ASSERT(mShaderContent->getFrequency() == Frequency);
				RHIShader = getResourceChecked().getShader(mShaderContent->getResourceIndex());
				BOOST_ASSERT(RHIShader->getFrequency() == Frequency);
			}
			return RHIShader;
		}

		/** @return the shader's vertex shader */
		inline RHIVertexShader* getVertexShader() const
		{
			return static_cast<RHIVertexShader*>(getRHIShaderBase(SF_Vertex));
		}
		/** @return the shader's pixel shader */
		inline RHIPixelShader* getPixelShader() const
		{
			return static_cast<RHIPixelShader*>(getRHIShaderBase(SF_Pixel));
		}
		/** @return the shader's hull shader */
		inline RHIHullShader* getHullShader() const
		{
			return static_cast<RHIHullShader*>(getRHIShaderBase(SF_Hull));
		}
		/** @return the shader's domain shader */
		inline RHIDomainShader* getDomainShader() const
		{
			return static_cast<RHIDomainShader*>(getRHIShaderBase(SF_Domain));
		}
		/** @return the shader's geometry shader */
		inline RHIGeometryShader* getGeometryShader() const
		{
			return static_cast<RHIGeometryShader*>(getRHIShaderBase(SF_Geometry));
		}
		/** @return the shader's compute shader */
		inline RHIComputeShader* getComputeShader() const
		{
			return static_cast<RHIComputeShader*>(getRHIShaderBase(SF_Compute));
		}

#if RHI_RAYTRACING
		/*inline RHIRayTracingShader* GetRayTracingShader() const
		{
			FRHIRayTracingShader* RHIShader = nullptr;
			if (ShaderContent)
			{
				const EShaderFrequency Frequency = ShaderContent->GetFrequency();
				checkSlow(Frequency == SF_RayGen
					|| Frequency == SF_RayMiss
					|| Frequency == SF_RayHitGroup
					|| Frequency == SF_RayCallable);
				RHIShader = static_cast<FRHIRayTracingShader*>(GetResourceChecked().GetShader(ShaderContent->GetResourceIndex()));
				checkSlow(RHIShader->GetFrequency() == Frequency);
			}
			return RHIShader;
		}

		inline uint32 GetRayTracingMaterialLibraryIndex() const
		{
			checkSlow(ShaderContent);
			checkSlow(ShaderContent->GetFrequency() == SF_RayHitGroup);
			return GetResourceChecked().GetRayTracingMaterialLibraryIndex(ShaderContent->GetResourceIndex());
		}*/
#endif // RHI_RAYTRACING


	private:
		ShaderType* mShaderContent;
		const ShaderMapBase* mShaderMap;
	};


	template<typename ShaderType>
	using TShaderRef = TShaderRefBase<ShaderType, ShaderMapPointerTable>;

	class ShaderResource : public RenderResource, public DeferredCleanupInterface
	{
		friend class Shader;

	public:
		RENDER_CORE_API ShaderResource();

		ShaderResource(const ShaderCompilerOutput& output, ShaderType* inSpecificType);

		~ShaderResource();
		RENDER_CORE_API void serialize(Archive& ar);

		RENDER_CORE_API void AddRef();
		RENDER_CORE_API void Release();
		RENDER_CORE_API void Register();


		FORCEINLINE RHIVertexShader* getVertexShader()
		{
			BOOST_ASSERT(mTarget.mFrequency == SF_Vertex);
			if (!isInitialized())
			{
				initializeShaderRHI();
			}
			return mVertexShader;
		}

		FORCEINLINE RHIPixelShader* getPixelShader()
		{
			BOOST_ASSERT(mTarget.mFrequency == SF_Pixel);
			if (!isInitialized())
			{
				initializeShaderRHI();
			}
			return mPixelShader;
		}

		FORCEINLINE RHIHullShader* getHullShader()
		{
			BOOST_ASSERT(mTarget.mFrequency == SF_Hull);
			if (!isInitialized())
			{
				initializeShaderRHI();
			}
			return mHullShader;
		}

		FORCEINLINE RHIDomainShader* getDomainShader()
		{
			BOOST_ASSERT(mTarget.mFrequency == SF_Domain);
			if (!isInitialized())
			{
				initializeShaderRHI();
			}
			return mDomainShader;
		}

		FORCEINLINE RHIGeometryShader* getGeometryShader()
		{
			BOOST_ASSERT(mTarget.mFrequency == SF_Geometry);
			if (!isInitialized())
			{
				initializeShaderRHI();
			}
			return mGeometryShader;
		}

		FORCEINLINE RHIComputeShader* getComputeShader()
		{
			BOOST_ASSERT(mTarget.mFrequency == SF_Compute);
			if (!isInitialized())
			{
				initializeShaderRHI();
			}
			return mComputeShader;
		}

		RENDER_CORE_API ShaderResourceId getId() const;

		RENDER_CORE_API void initializeShaderRHI();

		uint32 getSizeBytes() const
		{
			return mCode.getAllocatedSize() + sizeof(ShaderResource);
		}

		virtual void initRHI();
		virtual void releaseRHI();

		RENDER_CORE_API static ShaderResource* findShaderResourceById(const ShaderResourceId& id);

		RENDER_CORE_API static ShaderResource* findOrCreateShaderResource(const ShaderCompilerOutput& output, class ShaderType* specificType, int32 specificPermutationId);

		RENDER_CORE_API static void getAllShaderResourceId(TArray<ShaderResourceId>& ids);

		RENDER_CORE_API static bool arePlatformsCompatible(EShaderPlatform currentPlatform, EShaderPlatform targetPlatform);

		RENDER_CORE_API static SHAHash& filterShaderSourceHashForSerialization(const Archive& ar, SHAHash& hashToSerialize);

	public:
		ShaderParameterMapInfo mParameterMapInfo;

	private:
		void uncompressCode(TArray<uint8>& uncompressedCode) const;
		void compressCode(const TArray<uint8> & uncompressedCode);


		ShaderTarget mTarget;

		VertexShaderRHIRef mVertexShader;
		PixelShaderRHIRef mPixelShader;
		HullShaderRHIRef mHullShader;
		DomainShaderRHIRef mDomainShader;
		GeometryShaderRHIRef mGeometryShader;
		ComputeShaderRHIRef mComputeShader;

		TArray<uint8> mCode;
		uint32 mUncompressedCodeSize = 0;

		SHAHash mOutputHash;
		class ShaderType* mSpecificType;

		uint32 mNumInstructions;

		uint32 mNumTextureSamplers;

		mutable uint32 mNumRefs;
		uint32 mCanary;

		static TMap<ShaderResourceId, ShaderResource*> mShaderResourceIdMap;

		static CriticalSection mShaderResourceIdMapCritical;


	};

	class ShaderType;
	class MeshMaterialShaderType;
	class MaterialShaderType;
	class GlobalShaderType;

	class SerializationHistory
	{
	public:
		TArray<uint32> mTokenBits;
		int32 mNumTokens;
		TArray<uint32> mFullLengths;
		SerializationHistory() :
			mNumTokens(0)
		{}

	public:
		void addValue(uint32 inValue)
		{
			const int32 uIntIndex = mNumTokens / 8;
			if (uIntIndex >= mTokenBits.size())
			{
				mTokenBits.addZeroed();
			}
			uint8 token = inValue;
			if (inValue > 7)
			{
				token = 0;
				mFullLengths.push_back(inValue);
			}

			const uint32 shift = (mNumTokens % 8) * 4;
			mTokenBits[uIntIndex] = mTokenBits[uIntIndex] | (token << shift);
			mNumTokens++;
		}
		uint8 getToken(int32 index) const
		{
			BOOST_ASSERT(index < mNumTokens);
			const int32 uIntIndex = index / 8;
			BOOST_ASSERT(uIntIndex < mTokenBits.size());
			const uint32 shift = (index % 8) * 4;
			const uint8 token = (mTokenBits[uIntIndex] >> shift) & 0xF;
			return token;
		}

		void appendKeyString(wstring& keyString) const
		{
			keyString += mNumTokens;
			keyString += bytesToHex((uint8*)mTokenBits.getData(), mTokenBits.size() * mTokenBits.getTypeSize());
			keyString += bytesToHex((uint8*)mFullLengths.getData(), mFullLengths.size() * mFullLengths.getTypeSize());
		}

		inline bool operator == (const SerializationHistory& other) const
		{
			return mTokenBits == other.mTokenBits && mNumTokens == other.mNumTokens && mFullLengths == other.mFullLengths;
		}

		friend Archive& operator << (Archive& ar, class SerializationHistory& ref)
		{
			ar << ref.mTokenBits << ref.mNumTokens << ref.mFullLengths;
			return ar;
		}
	};
	class ShaderPipelineType;

	class ShaderId
	{
	public:
		SHAHash mMaterialShaderMapHash;
		const ShaderPipelineType* mShaderPipeline;
		VertexFactoryType* mVertexFactoryType;
		SHAHash VFSourceHash;
		const SerializationHistory* mVFSerializationHistory;

		ShaderType* mShaderType;
		SHAHash mSourceHash;
		const SerializationHistory* mSerializationHistory;

		ShaderTarget mTarget;

		int32 mPermutationId;

		ShaderId(const SerializationHistory& inSerializationHistory) :
			mSerializationHistory(&inSerializationHistory)
		{}

		RENDER_CORE_API ShaderId(const SHAHash& inMaterialShaderMapHash, const ShaderPipelineType * inShaderPipeline, VertexFactoryType* inVertexFactoryType, ShaderType* inShaderType, int32 inPermutationId, ShaderTarget inTarget);

		friend inline uint32 getTypeHash(const ShaderId& id)
		{
			return Crc::memCrc_DEPRECATED((const void*)&id.mMaterialShaderMapHash, sizeof(id.mMaterialShaderMapHash));
		}

		size_t hash_value()
		{
			return Crc::memCrc_DEPRECATED((const void*)&mMaterialShaderMapHash, sizeof(mMaterialShaderMapHash));
		}
		

		bool operator == (const ShaderId& rhs) const
		{
			return mMaterialShaderMapHash == rhs.mMaterialShaderMapHash && mShaderPipeline == rhs.mShaderPipeline
				&& mVertexFactoryType == rhs.mVertexFactoryType
				&& VFSourceHash == rhs.VFSourceHash
				&& ((mVFSerializationHistory == nullptr && rhs.mVFSerializationHistory == nullptr) || (mVFSerializationHistory != nullptr && rhs.mVFSerializationHistory != nullptr && *mVFSerializationHistory == *rhs.mVFSerializationHistory))
				&& mShaderType == rhs.mShaderType
				&& mSourceHash == rhs.mSourceHash
				&& mSerializationHistory == rhs.mSerializationHistory
				&& mTarget == rhs.mTarget;
		}

		friend bool operator != (const ShaderId & x, const ShaderId &y)
		{
			return !(x == y);
		}

	public:
		SHAHash mVFSourceHash;
	};

	const int32 kUniquePermutationId = 0;

	class RENDER_CORE_API ShaderParameterBindings
	{
	public:
		static constexpr uint16 kInvalidBufferIndex = 0xFFFF;

		struct Parameter
		{
			uint16 mBufferIndex;
			uint16 mBaseIndex;
			uint16 mByteOffset;
			uint16 mByteSize;

			friend Archive& operator <<(Archive& ar, Parameter& parameterBindingData)
			{
				ar << parameterBindingData.mBufferIndex << parameterBindingData.mBaseIndex << parameterBindingData.mByteOffset << parameterBindingData.mByteSize;
				return ar;
			}
		};

		struct ResourceParameter
		{
			uint16 mBaseIndex;
			uint16 mByteOffset;

			friend Archive& operator << (Archive& ar, ResourceParameter& parameterBindingData)
			{
				ar << parameterBindingData.mBaseIndex << parameterBindingData.mByteOffset;
				return ar;
			}
		};

		struct ParameterStructReference
		{
			uint16 mBufferIndex;
			uint16 mByteOffset;

			friend Archive& operator <<(Archive& ar, ParameterStructReference& parameterBindingData)
			{
				ar << parameterBindingData.mBufferIndex << parameterBindingData.mByteOffset;
				return ar;
			}
		};

		TArray<Parameter> mParameters;
		TArray<ResourceParameter> mTextures;
		TArray<ResourceParameter> mSRVs;
		TArray<ResourceParameter> mUAVs;
		TArray<ResourceParameter> mSamplers;
		TArray<ResourceParameter> mGraphTextures;
		TArray<ResourceParameter> mGraphSRVs;
		TArray<ResourceParameter> mGraphUAVs;
		TArray<ParameterStructReference> mParameterReferences;

		uint16 mRootParameterBufferIndex = ShaderParameterBindings::kInvalidBufferIndex;

		friend Archive& operator << (Archive& ar, ShaderParameterBindings& parametersBindingData)
		{
			ar << parametersBindingData.mParameters;
			ar << parametersBindingData.mTextures;
			ar << parametersBindingData.mSRVs;
			ar << parametersBindingData.mUAVs;
			ar << parametersBindingData.mSamplers;
			ar << parametersBindingData.mGraphTextures;
			ar << parametersBindingData.mGraphSRVs;
			ar << parametersBindingData.mGraphUAVs;
			ar << parametersBindingData.mParameterReferences;
			ar << parametersBindingData.mRootParameterBufferIndex;
			return ar;
		}

		void bindForLegacyShaderParameters(const Shader* shader, const ShaderParameterMap& parameterMaps, const ShaderParametersMetadata& structMetaData, bool bShouldBindEverything = false);

		void bindForRootShaderParameters(const Shader* shader, const ShaderParameterMap& parameterMaps);
	};

	struct ShaderPermutationParameters
	{
		const EShaderPlatform mPlatform;
		const int32 mPermutationId;
		explicit ShaderPermutationParameters(EShaderPlatform inPlatform, int32 inPermutationId = 0)
			:mPlatform(inPlatform)
			,mPermutationId(inPermutationId)
		{

		}
	};

#define SHADER_DECLARE_VTABLE(ShaderClass) \
	static Shader* constructSerializedInstance() {return new ShaderClass();} \
	static Shader* constructCompiledInstance(const typename Shader::CompiledShaderInitializerType& initializer) \
	{ return new ShaderClass(static_cast<const typename ShaderMetaType::CompiledShaderInitializerType&>(initializer)); }\
	static void modifyCompilationEnvironmentImpl(const ShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment) \
	{\
		const typename ShaderClass::PermutationDomain permutationVector(parameters.mPermutationId); \
		mPermutationVector.modifyCompilationEnvironment(outEnvironment); \
		ShaderClass::modifyCompilationEnvironment(static_cast<const typename ShaderClass::PermutationParameters&>(parameters), outEnvironment); \
	} \
	static bool shouldCompilePermutationImple(const ShaderPermutationParameters& parameters) \
	{return ShaderClass::shouldCompilePermutation(static_cast<const typename ShaderClass::PermutationParameters&>(parameters));}

	struct ShaderCompiledShaderInitializerType
	{
		ShaderType* mType;
		ShaderTarget mTarget;
		const TArray<uint8>& mCode;
		const ShaderParameterMap& mParameterMap;
		const SHAHash& mOutputHash;
		SHAHash mMaterialShaderMapHash;
		const ShaderPipelineType* mShaderPipeline;
		VertexFactoryType* mVertexFactoryType;
		uint32 mNumInstructions;
		uint32 mNumTextureSamplers;
		uint32 mCodeSize;
		int32 mPermutationId;

		RENDER_CORE_API ShaderCompiledShaderInitializerType(
			ShaderType* inType,
			int32 inPermutationId,
			const ShaderCompilerOutput& compilerOutput,
			const SHAHash& inMaterialShaderMapHash,
			const ShaderPipelineType* inShaderPipeline,
			VertexFactoryType* inVertexFactoryType
		);
	};


	class RENDER_CORE_API Shader
	{
		friend class ShaderType;
	public:
		using PermutationDomain = ShaderPermutationNone;
		using PermutationParameters = ShaderPermutationParameters;
		using CompiledShaderInitializerType = ShaderCompiledShaderInitializerType;
		using ShaderMetaType = ShaderType;

		Shader();

		Shader(const ShaderCompiledShaderInitializerType& initializer);

		virtual ~Shader();

		static void modifyCompilationEnvironment(const ShaderPermutationParameters&, ShaderCompilerEnvironment&) {}

		static bool shouldCompilePermutation(const ShaderPermutationParameters&) { return true; }

		static bool validateCompiledResult(EShaderPlatform inPlatform, const ShaderParameterMap& inParameterMap, TArray<wstring>& outError) {
			return true;
		}

		const SHAHash& getHash() const;

		const SHAHash& getVertexFactoryHash() const;

		const SHAHash& getOutputHash() const;

		void finalize(const ShaderMapResourceCode* code);

		inline ShaderType* getType()const { return mType; }

		inline VertexFactoryType* getVertexFactoryType()const { return mVFType; }

		inline int32 getResourceIndex() const { BOOST_ASSERT(mResourceIndex != INDEX_NONE); return mResourceIndex; }

		inline EShaderPlatform getShaderPlatform() const { return mTarget.getPlatform(); }

		inline EShaderFrequency GetFrequency() const { return mTarget.getFrequency(); }

		inline const ShaderTarget getTarget() const { return mTarget; }

		inline inline uint32 getNumInstructions() const { return mNumInstructions; }

		inline uint32 getNumTextureSamplers() const { return mNumTextureSamplers; }

		inline uint32 getCodeSize() const { return mCodeSize; }

		inline void setNumInstructions(uint32 value) { mNumInstructions = value; }

		template<typename ConstantBufferStructType>
		FORCEINLINE_DEBUGGABLE const TShaderConstantBufferParameter<ConstantBufferStructType>& getConstantBufferParameter() const
		{
			const ShaderConstantBufferParameter& foundParameter = getConstantBufferParameter(&ConstantBufferStructType::StaticStructMetadata);
			return static_cast<const TShaderConstantBufferParameter<ConstantBufferStructType>&>(foundParameter);
		}

		FORCEINLINE_DEBUGGABLE const ShaderConstantBufferParameter& getConstantBufferParameter(const ShaderParametersMetadata* searchStruct) const
		{
			const HashedName searchName = searchStruct->getShaderVariableHashedName();
			return getConstantBufferParameter(searchName);
		}

		FORCEINLINE_DEBUGGABLE const ShaderConstantBufferParameter& getConstantBufferParameter(const HashedName searchName) const
		{
			int32 foundIndex = INDEX_NONE;
			TArrayView<const HashedName> constantBufferParameterStructsView(mConstantBufferParameterStructs);
			for (int32 structIndex = 0, count = constantBufferParameterStructsView.size(); structIndex < count; structIndex++)
			{
				if (constantBufferParameterStructsView[structIndex] == searchName)
				{
					foundIndex = structIndex;
					break;
				}
			}
			if (foundIndex != INDEX_NONE)
			{
				const ShaderConstantBufferParameter& foundParameter = mConstantBufferParameters[foundIndex];
				return foundParameter;
			}
			else
			{
				static ShaderConstantBufferParameter UnboundParamter;
				return UnboundParamter;
			}
		}

		const ShaderParametersMetadata* findAutomaticallyBoundConstantBufferStruct(int32 baseIndex) const;

		void dumpDebugInfo() const;

		static inline const ShaderParametersMetadata* getRootParametersMetadata()
		{
			return nullptr;
		}

	private:
		void buildParameterMapInfo(const TMap<wstring, ParameterAllocation>& parameterMap);

	protected:
		ShaderParameterBindings mBindings;
		ShaderParameterMapInfo mParameterMapInfos;

		TArray<HashedName> mConstantBufferParameterStructs;
		TArray<ShaderConstantBufferParameter> mConstantBufferParameters;

		SHAHash mOutputHash;

		SHAHash mVFSourceHash;

		SHAHash mSourceHash;

	private:
		ShaderType* mType;

		VertexFactoryType* mVFType;

		ShaderTarget mTarget;

		int32 mResourceIndex;

		uint32 mNumInstructions;

		uint32 mNumTextureSamplers;

		uint32 mCodeSize;
	};





	class RENDER_CORE_API ShaderType
	{
	public:
		enum class EShaderTypeForDynamicCast : uint32
		{
			Global,
			Material,
			MeshMaterial,
			Niagara,
			OCIO,
			NumShaderTypes,
		};

		typedef class Shader* (*ConstructSerializedType)();
		typedef Shader* (*ConstructCompiledType)(const Shader::CompiledShaderInitializerType& initializer);
		typedef bool (*ShouldCompilePermutationType)(const ShaderPermutationParameters&);
		typedef void (*ModifyCompilationEnvironmentType)(const ShaderPermutationParameters&, ShaderCompilerEnvironment&);
		typedef void (*validateCompiledResultType)(EShaderPlatform, const ShaderParameterMap&, TArray<wstring>&);



		ShaderType(
			EShaderTypeForDynamicCast inShaderTypeForDynamicCast,
			const TCHAR* inName,
			const TCHAR* inSourceFilename,
			const TCHAR* inFounctionName,
			uint32 inFrequency,
			int32 totalPermutationCount,
			ConstructSerializedType inConstructSerializeRef,
			ConstructCompiledType inConstructCompiledRef,
			ModifyCompilationEnvironmentType inModifyCompilationEnvironmentRef,
			ShouldCompilePermutationType inShouldCompilePermutationRef,
			validateCompiledResultType inValidateCompiledResultRef,
			uint32 inTypeSize,
			const ShaderParametersMetadata* inRootParametersMetadata);
		virtual ~ShaderType();

		Shader* constructForDeserialization() const;

		Shader* constructCompiled(const Shader::CompiledShaderInitializerType& initiaizer) const;

		bool shouldCompilePermutation(const ShaderPermutationParameters& parameters)const;

		void modifyCompilationEnvironment(const ShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)const;

		bool validateCompiledResult(EShaderPlatform platform, const ShaderParameterMap& paramterMap, TArray<wstring>& outError)const;

		const SHAHash& getSourceHash(EShaderPlatform shaderPlatform) const;

		/** Serializes a shader type reference by name. */
		RENDER_CORE_API friend Archive& operator<<(Archive& Ar, ShaderType*& Ref);

		/** Hashes a pointer to a shader type. */
		friend uint32 getTypeHash(ShaderType* Ref)
		{
			return Ref ? getTypeHash(Ref->mHashedName) : 0u;
		}

		// Dynamic casts.
		FORCEINLINE GlobalShaderType* getGlobalShaderType()
		{
			return (mShaderTypeForDynamicCast == EShaderTypeForDynamicCast::Global) ? reinterpret_cast<GlobalShaderType*>(this) : nullptr;
		}
		FORCEINLINE const GlobalShaderType* getGlobalShaderType() const
		{
			return (mShaderTypeForDynamicCast == EShaderTypeForDynamicCast::Global) ? reinterpret_cast<const GlobalShaderType*>(this) : nullptr;
		}
		FORCEINLINE MaterialShaderType* getMaterialShaderType()
		{
			return (mShaderTypeForDynamicCast == EShaderTypeForDynamicCast::Material) ? reinterpret_cast<MaterialShaderType*>(this) : nullptr;
		}
		FORCEINLINE const MaterialShaderType* getMaterialShaderType() const
		{
			return (mShaderTypeForDynamicCast == EShaderTypeForDynamicCast::Material) ? reinterpret_cast<const MaterialShaderType*>(this) : nullptr;
		}
		FORCEINLINE MeshMaterialShaderType* getMeshMaterialShaderType()
		{
			return (mShaderTypeForDynamicCast == EShaderTypeForDynamicCast::MeshMaterial) ? reinterpret_cast<MeshMaterialShaderType*>(this) : nullptr;
		}
		FORCEINLINE const MeshMaterialShaderType* getMeshMaterialShaderType() const
		{
			return (mShaderTypeForDynamicCast == EShaderTypeForDynamicCast::MeshMaterial) ? reinterpret_cast<const MeshMaterialShaderType*>(this) : nullptr;
		}
		/*FORCEINLINE const FNiagaraShaderType* GetNiagaraShaderType() const
		{
			return (ShaderTypeForDynamicCast == EShaderTypeForDynamicCast::Niagara) ? reinterpret_cast<const FNiagaraShaderType*>(this) : nullptr;
		}
		FORCEINLINE FNiagaraShaderType* GetNiagaraShaderType()
		{
			return (ShaderTypeForDynamicCast == EShaderTypeForDynamicCast::Niagara) ? reinterpret_cast<FNiagaraShaderType*>(this) : nullptr;
		}
		FORCEINLINE const FOpenColorIOShaderType* GetOpenColorIOShaderType() const
		{
			return (ShaderTypeForDynamicCast == EShaderTypeForDynamicCast::OCIO) ? reinterpret_cast<const FOpenColorIOShaderType*>(this) : nullptr;
		}
		FORCEINLINE FOpenColorIOShaderType* GetOpenColorIOShaderType()
		{
			return (ShaderTypeForDynamicCast == EShaderTypeForDynamicCast::OCIO) ? reinterpret_cast<FOpenColorIOShaderType*>(this) : nullptr;
		}*/

		inline EShaderTypeForDynamicCast getTypeForDynamicCast() const
		{
			return mShaderTypeForDynamicCast;
		}

		
		inline EShaderFrequency getFrequency() const
		{
			return (EShaderFrequency)mFrequency;
		}
		inline const TCHAR* getName() const
		{
			return mName;
		}
		inline const wstring& getFName() const
		{
			return mTypeName;
		}
		inline const HashedName& getHashedName() const
		{
			return mHashedName;
		}
		inline const TCHAR* getShaderFilename() const
		{
			return mSourceFilename;
		}
		inline const HashedName& getHashedShaderFilename() const
		{
			return mHashedSourceFilename;
		}
		inline const TCHAR* getFunctionName() const
		{
			return mFunctionName;
		}
		inline uint32 getTypeSize() const
		{
			return mTypeSize;
		}

		inline int32 getNumShaders() const
		{
			// TODO count this
			return 0;
		}

		inline int32 getPermutationCount() const
		{
			return mTotalPermutationCount;
		}

		inline const TMap<const TCHAR*, CachedConstantBufferDeclaration>& getReferencedUniformBufferStructsCache() const
		{
			return mReferencedConstantBufferStructsCache;
		}

		/** Returns the meta data for the root shader parameter struct. */
		inline const ShaderParametersMetadata* getRootParametersMetadata() const
		{
			return mRootParametersMetadata;
		}

		/** Adds include statements for uniform buffers that this shader type references, and builds a prefix for the shader file with the include statements. */
		void addReferencedUniformBufferIncludes(ShaderCompilerEnvironment& OutEnvironment, wstring& OutSourceFilePrefix, EShaderPlatform Platform);

		void flushShaderFileCache(const TMap<wstring, TArray<const TCHAR*> >& ShaderFileToUniformBufferVariables)
		{
			mReferencedConstantBufferStructsCache.empty();
			generateReferencedConstantBuffers(mSourceFilename, mName, ShaderFileToUniformBufferVariables, mReferencedConstantBufferStructsCache);
			bCachedConstantBufferStructDeclarations = false;
		}

		void dumpDebugInfo();
		void getShaderStableKeyParts(struct FStableShaderKeyAndValue& SaveKeyVal);
	private:
		EShaderTypeForDynamicCast mShaderTypeForDynamicCast;
		const TCHAR* mName;
		wstring mTypeName;
		HashedName mHashedName;
		HashedName mHashedSourceFilename;
		const TCHAR* mSourceFilename;
		const TCHAR* mFunctionName;
		uint32 mFrequency;
		uint32 mTypeSize;
		int32 mTotalPermutationCount;

		ConstructSerializedType mConstructSerializedRef;
		ConstructCompiledType mConstructCompiledRef;
		ModifyCompilationEnvironmentType mModifyCompilationEnvironmentRef;
		ShouldCompilePermutationType mShouldCompilePermutationRef;
		validateCompiledResultType mValidateCompiledResultRef;

		const ShaderParametersMetadata* const mRootParametersMetadata;

		TLinkedList<ShaderType*> mGlobalListLink;

		friend void RENDER_CORE_API dumpShaderStats(EShaderPlatform platform, EShaderFrequency frequency);

		static bool bInitializedSerializationHistory;

	protected:
		bool bCachedConstantBufferStructDeclarations;

		TMap<const TCHAR*, CachedConstantBufferDeclaration> mReferencedConstantBufferStructsCache;
	};


	class RENDER_CORE_API ShaderPipelineType
	{
	public:
		// Set bShouldOptimizeUnusedOutputs to true if we want unique FShaders for each shader pipeline
	// Set bShouldOptimizeUnusedOutputs to false if the FShaders will point to the individual shaders in the map
		ShaderPipelineType(
			const TCHAR* InName,
			const ShaderType* InVertexShader,
			const ShaderType* InHullShader,
			const ShaderType* InDomainShader,
			const ShaderType* InGeometryShader,
			const ShaderType* InPixelShader,
			bool bInShouldOptimizeUnusedOutputs);
		~ShaderPipelineType();

		FORCEINLINE bool hasTessellation() const { return mAllStages[SF_Domain] != nullptr; }
		FORCEINLINE bool hasGeometry() const { return mAllStages[SF_Geometry] != nullptr; }
		FORCEINLINE bool hasPixelShader() const { return mAllStages[SF_Pixel] != nullptr; }

		FORCEINLINE const ShaderType* getShader(EShaderFrequency Frequency) const
		{
			BOOST_ASSERT(Frequency < SF_NumFrequencies);
			return mAllStages[Frequency];
		}

		FORCEINLINE wstring getFName() const { return mTypeName; }
		FORCEINLINE TCHAR const* getName() const { return mName; }
		FORCEINLINE const HashedName& getHashedName() const { return mHashedName; }
		FORCEINLINE const HashedName& getHashedPrimaryShaderFilename() const { return mHashedPrimaryShaderFilename; }

		// Returns an array of valid stages, sorted from PS->GS->DS->HS->VS, no gaps if missing stages
		FORCEINLINE const TArray<const ShaderType*>& getStages() const { return mStages; }

		static TLinkedList<ShaderPipelineType*>*& getTypeList();

		static const TArray<ShaderPipelineType*>& getSortedTypes(ShaderType::EShaderTypeForDynamicCast Type);

		/** @return The global shader pipeline name to type map */
		static TMap<HashedName, ShaderPipelineType*>& getNameToTypeMap();
		static const ShaderPipelineType* getShaderPipelineTypeByName(const HashedName& Name);

		/** Initialize static members, this must be called before any shader types are created. */
		static void initialize();
		static void uninitialize();

		static TArray<const ShaderPipelineType*> GetShaderPipelineTypesByFilename(const TCHAR* Filename);

		/** Serializes a shader type reference by name. */
		RENDER_CORE_API friend Archive& operator<<(Archive& Ar, const ShaderPipelineType*& Ref);

		/** Hashes a pointer to a shader type. */
		friend uint32 GetTypeHash(ShaderPipelineType* Ref) { return Ref ? Ref->mHashIndex : 0; }
		friend uint32 GetTypeHash(const ShaderPipelineType* Ref) { return Ref ? Ref->mHashIndex : 0; }

		// Check if this pipeline is built of specific types
		bool IsGlobalTypePipeline() const { return mStages[0]->getGlobalShaderType() != nullptr; }
		bool IsMaterialTypePipeline() const { return mStages[0]->getMaterialShaderType() != nullptr; }
		bool IsMeshMaterialTypePipeline() const { return mStages[0]->getMeshMaterialShaderType() != nullptr; }

		FORCEINLINE bool ShouldOptimizeUnusedOutputs(EShaderPlatform Platform) const
		{
			return bShouldOptimizeUnusedOutputs && RHISupportsShaderPipelines(Platform);
		}

		/** Calculates a Hash based on this shader pipeline type stages' source code and includes */
		const SHAHash& GetSourceHash(EShaderPlatform ShaderPlatform) const;
	private:
		const TCHAR* const mName;
		wstring mTypeName;
		HashedName mHashedName;
		HashedName mHashedPrimaryShaderFilename;

		TArray<const ShaderType*> mStages;

		const ShaderType* mAllStages[SF_NumFrequencies];

		TLinkedList<ShaderPipelineType*> mGlobalListLink;

		uint32 mHashIndex;
		bool bShouldOptimizeUnusedOutputs;

		static bool bInitialized;
	};

	class CompareShaderPipelineType
	{
	public:
		FORCEINLINE bool operator()(const ShaderPipelineType& _Left, const ShaderPipelineType& _Right) const
		{
			bool bNullA = &_Left == nullptr;
			bool bNullB = &_Right == nullptr;
			if (bNullA && bNullB)
			{
				return false;
			}
			else if (bNullA)
			{
				return true;
			}
			else if (bNullB)
			{
				return false;
			}
			int32 al = CString::strlen(_Left.getName());
			int32 bl = CString::strlen(_Right.getName());

			if (al == bl)
			{
				return CString::strncmp(_Left.getName(), _Right.getName(), al) > 0;
			}
			return al > bl;
		}
	};

	class RENDER_CORE_API ShaderPipeline
	{
	public:
		explicit ShaderPipeline(const ShaderPipelineType* InType) : mTypeName(InType->getHashedName()) { Memory::memzero(&mPermutationIds, sizeof(mPermutationIds)); }
		~ShaderPipeline();

		void AddShader(Shader* shader, int32 PermutationId);

		inline uint32 GetNumShaders() const
		{
			uint32 NumShaders = 0u;
			for (uint32 i = 0u; i < SF_NumGraphicsFrequencies; ++i)
			{
				if (mShaders[i])
				{
					++NumShaders;
				}
			}
			return NumShaders;
		}

		// Find a shader inside the pipeline
		template<typename ShaderType>
		ShaderType* getShader()
		{
			const ShaderType& Type = ShaderType::mStaticType;
			const EShaderFrequency Frequency = Type.getFrequency();
			if (Frequency < SF_NumGraphicsFrequencies && mShaders[Frequency].IsValid())
			{
				Shader* shader = mShaders[Frequency];
				if (shader->getType() == &Type)
				{
					return static_cast<ShaderType*>(shader);
				}
			}
			return nullptr;
		}

		Shader* getShader(EShaderFrequency Frequency)
		{
			BOOST_ASSERT(Frequency < SF_NumGraphicsFrequencies);
			return mShaders[Frequency];
		}

		const Shader* getShader(EShaderFrequency Frequency) const
		{
			BOOST_ASSERT(Frequency < SF_NumGraphicsFrequencies);
			return mShaders[Frequency];
		}

		inline TArray<TShaderRef<Shader>> getShaders(const ShaderMapBase& InShaderMap) const
		{
			TArray<TShaderRef<Shader>> Result;
			for (uint32 i = 0u; i < SF_NumGraphicsFrequencies; ++i)
			{
				if (mShaders[i])
				{
					Result.add(TShaderRef<Shader>(mShaders[i], InShaderMap));
				}
			}
			return Result;
		}

		void validate(const ShaderPipelineType* InPipelineType) const;

		void finalize(const ShaderMapResourceCode* Code);

		enum EFilter
		{
			EAll,			// All pipelines
			EOnlyShared,	// Only pipelines with shared shaders
			EOnlyUnique,	// Only pipelines with unique shaders
		};

		/** Saves stable keys for the shaders in the pipeline */
#if WITH_EDITOR
		void saveShaderStableKeys(const ShaderMapPointerTable& InPtrTable, EShaderPlatform TargetShaderPlatform, const struct StableShaderKeyAndValue& SaveKeyVal) const;
#endif // WITH_EDITOR

		HashedName mTypeName;
		Shader* mShaders[SF_NumGraphicsFrequencies];
		int32 mPermutationIds[SF_NumGraphicsFrequencies];

	};

	inline bool operator<(const ShaderPipeline& lhs, const ShaderPipeline& rhs)
	{
		return lhs.mTypeName.getHash() < rhs.mTypeName.getHash();
	}
	class ShaderPipelineRef
	{
	public:
		ShaderPipelineRef() : mShaderPipeline(nullptr), mShaderMap(nullptr) {}
		ShaderPipelineRef(ShaderPipeline* inPipeline, const ShaderMapBase& inShaderMap) :mShaderPipeline(inPipeline), mShaderMap(&inShaderMap)
		{}

		inline bool isValid() const { return mShaderPipeline != nullptr; }
		inline bool isNull() const { return mShaderPipeline == nullptr; }

		template<typename ShaderType>
		TShaderRef<ShaderType> getShader() const
		{
			return TShaderRef<ShaderType>(mShaderPipeline->getShader<ShaderType>(), *mShaderMap);
		}

		TShaderRef<Shader> getShader(EShaderFrequency frequency)const
		{
			return TShaderRef<Shader>(mShaderPipeline->getShader(frequency), *mShaderMap);
		}

		inline TArray<TShaderRef<Shader>> getShaders() const
		{
			return mShaderPipeline->getShaders(*mShaderMap);
		}

		inline ShaderPipeline* getPipeline() const {
			return mShaderPipeline;
		}

		ShaderMapResource* getResource() const;

		const ShaderMapPointerTable& getPointerTable() const;

		inline ShaderPipeline* operator->() const { BOOST_ASSERT(mShaderPipeline); return mShaderPipeline; }

	private:
		ShaderPipeline* mShaderPipeline;
		const ShaderMapBase* mShaderMap;
	};

	
	
	class SelfContainedShaderId
	{
	public:
		SHAHash mMaterialShaderMapHash;
		wstring mVertexFactoryTypeName;
		wstring mShaderPipelineName;
		SHAHash mVFSourceHash;
		SerializationHistory mVFSerializationHistory;
		wstring mShaderTypeName;
		SHAHash mSourceHash;
		SerializationHistory mSerializationHistory;
		ShaderTarget mTarget;
		RENDER_CORE_API SelfContainedShaderId();

		RENDER_CORE_API SelfContainedShaderId(const ShaderId & inShaderId);

		RENDER_CORE_API bool isValid();

		RENDER_CORE_API friend Archive& operator << (Archive& ar, class SelfContainedShaderId& ref);
	};


	

}

namespace Air
{
	class GlobalShaderType;

	
	//class CompareShaderTypes
	//{
	//public:
	//	FORCEINLINE bool operator()(const )
	//};
	class CompareShaderTypes
	{	
	public:
		FORCEINLINE bool operator()(const ShaderType& _Left, const ShaderType& _Right) const
		{
			int32 al = CString::strlen(_Left.getName());
			int32 bl = CString::strlen(_Right.getName());
			if (al == bl)
			{
				return CString::strncmp(_Left.getName(), _Right.getName(), al) > 0;
			}
			return al > bl;
		}
	};
	

	template<typename ShaderMetaType>
	class TShaderMap
	{
		struct SerializedShaderPipeline 
		{
			const ShaderPipelineType* mShaderPipelineType;
			TArray<TRefCountPtr<Shader>> mShaderStages;
			SerializedShaderPipeline()
				:mShaderPipelineType(nullptr)
			{}
		};

		


	public:
		template<typename ShaderType>
		ShaderType* getShader(int permutationId = 0) const
		{
			BOOST_ASSERT(bHasBeenRegistered);
			const auto& it = mShaders.find(ShaderPrimaryKey(&ShaderType::mStaticType, permutationId));
			BOOST_ASSERT(it != mShaders.end());
			const TRefCountPtr<Shader> shaderRef = it->second;

			return (ShaderType*)(shaderRef->getShaderChecked());
		}
		TShaderMap(EShaderPlatform inPlatform)
			:bHasBeenRegistered(true)
			, mPlatform(inPlatform)
		{}

	public:
		using ShaderPrimaryKey = TShaderTypePermutation<ShaderType>;

		class CompareShaderPrimaryKey
		{
		public:
			FORCEINLINE bool operator()(const ShaderPrimaryKey& a, const ShaderPrimaryKey& b) const
			{
				int32 al = CString::strlen(a.mType->getName());
				int32 bl = CString::strlen(b.mType->getName());
				if (al == bl)
				{
					return CString::strncmp(a.mType->getName(), b.mType->getName(), al) > 0 || a.mPermutationId > b.mPermutationId;
				}
				return al > bl;
			}
		};

		Shader* getShader(ShaderType* shaderType, int32 permutationId = 0) const
		{
			BOOST_ASSERT(bHasBeenRegistered);

			auto shaderRef = mShaders.find(ShaderPrimaryKey(shaderType, permutationId));
			return shaderRef != mShaders.end() ? (shaderRef->second)->getShaderChecked() : nullptr;
		}

		inline const TMap<ShaderPrimaryKey, TRefCountPtr<Shader>>& getShaders() const
		{
			BOOST_ASSERT(bHasBeenRegistered);
			return mShaders;
		}

		inline ShaderPipeline* getShaderPipeline(const ShaderPipelineType* pipelineType) const
		{
			BOOST_ASSERT(bHasBeenRegistered);
			auto it = mShaderPipelines.find(pipelineType);
			return it != mShaderPipelines.end() ? it->second : nullptr;
		}

		inline bool hasShaderPipeline(const ShaderPipelineType* pipelineType) const
		{
			BOOST_ASSERT(bHasBeenRegistered);
			return mShaderPipelines.end() != mShaderPipelines.find(pipelineType);
		}

		inline uint32 getNumShaders() const
		{
			BOOST_ASSERT(bHasBeenRegistered);
			return mShaders.size();
		}


		inline uint32 getNumShaderPipelines() const
		{
			BOOST_ASSERT(bHasBeenRegistered);
			return mShaderPipelines.size();
		}
		
		inline bool isEmpty() const
		{
			BOOST_ASSERT(bHasBeenRegistered);
			return mShaders.empty();
		}

		void serializeInline(Archive& ar, bool bInlineShaderResource, bool bHandleShaderKeyChanges, bool bLoadedKeyCookedMaterial, const TArray<ShaderPrimaryKey>* shaderKeysToSave = nullptr)
		{
			if (ar.isSaving())
			{
				TArray<ShaderPrimaryKey> sortedShaderKeys;
				if (shaderKeysToSave)
				{
					sortedShaderKeys = *shaderKeysToSave;
				}
				else
				{
					mShaders.generateKeyArray(sortedShaderKeys);
				}

				int32 numShaders = sortedShaderKeys.size();
				ar << numShaders;

				sortedShaderKeys.sort(CompareShaderPrimaryKey());

				for (ShaderPrimaryKey key : sortedShaderKeys)
				{
					ShaderType* type = key.mType;
					BOOST_ASSERT(type);
					BOOST_ASSERT(type->getName() != nullptr);
					ar << type;
					Shader* currentShader = mShaders.findChecked(key);
					serializeShaderForSaving(currentShader, ar, bHandleShaderKeyChanges, bInlineShaderResource);
				}
				TArray<ShaderPipeline*> sortedPielines;
				getShaderPipelineList(sortedPielines, ShaderPipeline::EFilter::EAll);
				int32 numPipelines = sortedPielines.size();
				ar << numPipelines;
				sortedPielines.sort();

				for (ShaderPipeline* currentPipeline : sortedPielines)
				{
					const ShaderPipelineType* pipelineType = currentPipeline->mPipelineType;
					ar << pipelineType;

					auto & pipelineStages = pipelineType->getStages();
					int32 numStatges = pipelineStages.size();
					ar << numStatges;
					for (int32 index = 0; index < numStatges; ++index)
					{
						auto * shader = currentPipeline->getShader(pipelineStages[index]->getFrequency());
						ShaderType* type = shader->getType();
						ar << type;
						serializeShaderForSaving(shader, ar, bHandleShaderKeyChanges, bInlineShaderResource);
					}
				}
			}
			if (ar.isLoading())
			{
				bHasBeenRegistered = false;
				int32 numShaders = 0;
				ar << numShaders;
				mSerializedShaders.reserve(numShaders);
				for (int32 shaderIndex = 0; shaderIndex < numShaders; shaderIndex++)
				{
					ShaderType* type = nullptr;
					ar << type;
					Shader* shader = serializeShaderForLoad(type, ar, bHandleShaderKeyChanges, bInlineShaderResource);
					if (shader)
					{
						mSerializedShaders.push_back(shader);
					}
				}
				int32 numPipelines = 0;
				ar << numPipelines;
				for (int32 pipelineIndex = 0; pipelineIndex < numPipelines; pipelineIndex++)
				{
					const ShaderPipelineType* shaderPipelineType = nullptr;
					ar << shaderPipelineType;
					int32 numStages = 0;
					ar << numStages;
					TArray<TRefCountPtr<Shader>> shaderStages;
					for (int32 index = 0; index < numStages; index++)
					{
						ShaderType* type = nullptr;
						ar << type;
						Shader* shader = serializeShaderForLoad(type, ar, bHandleShaderKeyChanges, bInlineShaderResource);
						if (shader)
						{
							shaderStages.push_back(shader);
						}
					}
					if (shaderPipelineType && shaderStages.size() == shaderPipelineType->getStages().size())
					{
						SerializedShaderPipeline* serializedPipeline = new SerializedShaderPipeline();
						serializedPipeline->mShaderPipelineType = shaderPipelineType;
						serializedPipeline->mShaderStages = std::move(shaderStages);
						mSerializedShaderPipelines.push_back(serializedPipeline);
					}
				}
			}
		}
		virtual void registerSerializedShaders()
		{
			bHasBeenRegistered = true;
			BOOST_ASSERT(isInGameThread());
			for (Shader* shader : mSerializedShaders)
			{
				shader->registerSerializedResource();
				ShaderType* type = shader->getType();
				Shader* existingShader = type->findShaderById(shader->getId());
				if (existingShader != nullptr)
				{
					delete shader;
					shader = existingShader;
				}
				else
				{
					shader->Register();
				}
				addShader(shader->getType(), shader->getPermutationId(), shader);
			}
			mSerializedShaders.clear();
			for (SerializedShaderPipeline* serializedPipeline : mSerializedShaderPipelines)
			{
				for (TRefCountPtr<Shader> shader : serializedPipeline->mShaderStages)
				{
					shader->registerSerializedResource();
				}
				ShaderPipeline* shaderPipeline = new ShaderPipeline(serializedPipeline->mShaderPipelineType, serializedPipeline->mShaderStages);
				addShaderPipeline(serializedPipeline->mShaderPipelineType, shaderPipeline);
				delete serializedPipeline;
			}
			mSerializedShaderPipelines.clear();
		}

		void addShader(ShaderType* type, int32 permutationId, Shader* shader)
		{
			BOOST_ASSERT(type != nullptr);
			mShaders.emplace(ShaderPrimaryKey(type, permutationId), shader);
		}

		inline void addShaderPipeline(const ShaderPipelineType* type, ShaderPipeline* shaderPipeline)
		{
			BOOST_ASSERT(bHasBeenRegistered);
			BOOST_ASSERT(type);
			BOOST_ASSERT(!shaderPipeline || shaderPipeline->mPipelineType == type);
			mShaderPipelines.emplace(type, shaderPipeline);
		}


		void getShaderPipelineList(TArray<ShaderPipeline*>& outShaderPipelines, ShaderPipeline::EFilter filter) const
		{
			BOOST_ASSERT(bHasBeenRegistered);
			for (auto& pair : mShaderPipelines)
			{
				ShaderPipeline* pipeline = pair.second;
				if (pipeline->mPipelineType->shoudlOptimizeUnusedOutputs(mPlatform) && filter == ShaderPipeline::EOnlyShared)
				{
					continue;
				}
				else if (!pipeline->mPipelineType->shoudlOptimizeUnusedOutputs(mPlatform) && filter == ShaderPipeline::EOnlyUnique)
				{
					continue;
				}
				outShaderPipelines.push_back(pipeline);
			}
		}

		void removeShaderTypePermutation(ShaderType* shaderType, int32 permutationId)
		{
			mShaders.erase(ShaderPrimaryKey(shaderType, permutationId));
		}

		void removeShaderPipelineType(const ShaderPipelineType* shaderPipelineType)
		{
			auto found = mShaderPipelines.find(shaderPipelineType);
			if (found != mShaderPipelines.end())
			{
				if (found->second != nullptr)
				{
					delete found->second;
				}
				mShaderPipelines.erase(found);
			}
		}


		inline Shader* serializeShaderForLoad(ShaderType* type, Archive& ar, bool bHandleShaderKeyChanges, bool bInlineShaderSource)
		{
			int32 endOffset = 0;
			ar << endOffset;

			SelfContainedShaderId selfContainedKey;
			if (bHandleShaderKeyChanges)
			{
				ar << selfContainedKey;
			}
			Shader* shader = nullptr;
			if (type && (!bHandleShaderKeyChanges || selfContainedKey.isValid()))
			{
				shader = type->constructForDeserialization();
				BOOST_ASSERT(shader != nullptr);
				shader->serializeBase(ar, bInlineShaderSource);
			}
			else
			{
				ar.seek(endOffset);
			}
			return shader;
		}

		inline void serializeShaderForSaving(Shader* currentShader, Archive& ar, bool bHandleShaderKeyChanges, bool bInlineShaderResource)
		{
			int32 skipOffset = ar.tell();
			{
				ar << skipOffset;
			}
			if (bHandleShaderKeyChanges)
			{
				SelfContainedShaderId selfContainedKey = currentShader->getId();
				ar << selfContainedKey;
			}
			currentShader->serializeBase(ar, bInlineShaderResource);
			int32 endOffset = ar.tell();
			ar.seek(skipOffset);
			ar << endOffset;
		
			ar.seek(endOffset);
		}

		bool hasShader(ShaderType* type, int32 permutationId) const
		{
			BOOST_ASSERT(bHasBeenRegistered);
			const auto it = mShaders.find(ShaderPrimaryKey(type, permutationId));
			if (it != mShaders.end())
			{
				if (it->second.getReference() != nullptr)
				{
					return true;
				}
			}
			return false;
		}
	
	protected:
		EShaderPlatform mPlatform;
	private:

		bool bHasBeenRegistered;
		TMap<ShaderPrimaryKey, TRefCountPtr<Shader>> mShaders;

		TArray<Shader*> mSerializedShaders;

		TArray<SerializedShaderPipeline*> mSerializedShaderPipelines;
		TMap<const ShaderPipelineType*, ShaderPipeline*> mShaderPipelines;
	};


	template<typename ShaderType>
	class TShaderMapRef
	{
	public:

		TShaderMapRef(const TShaderMap<typename ShaderType::ShaderMetaType>* shaderIndex)
			:mShader(shaderIndex->template getShader<ShaderType>(0))
		{
			static_assert(std::is_same<typename ShaderType::PermutationDomain, ShaderPermutationNone>::value, "Missing permutation vector argument for shader that have a permutation domain");
		}


		TShaderMapRef(const TShaderMap<typename ShaderType::ShaderMetaType>* shaderIndex, const typename ShaderType::PermutationDomain& permutationVector)
			:mShader(shaderIndex->template getShader<ShaderType>(permutationVector.toDimensionValueId()))
		{}

		FORCEINLINE ShaderType* operator->() const
		{
			return mShader;
		}

		FORCEINLINE ShaderType* operator*() const
		{
			return mShader;
		}


		RENDER_CORE_API friend Archive& operator << (Archive& ar, ShaderType* & ref);

	private:
		ShaderType* mShader;
	};

	template<typename ShaderType>
	class TOptionalShaderMapRef
	{
	public:
		TOptionalShaderMapRef(const TShaderMap<typename ShaderType::ShaderMetaType>* shaderIndex)
			:mShader((ShaderType*)shaderIndex->getShader(&ShaderType::mStaticType))
		{}

		FORCEINLINE bool isValid() const
		{
			return mShader != nullptr;
		}

		FORCEINLINE ShaderType* operator->() const
		{
			return mShader;
		}

		FORCEINLINE ShaderType* operator*() const
		{
			return mShader;
		}
	private:
		ShaderType* mShader;
	};

	class ShaderTypeDependency
	{
	public:
		ShaderTypeDependency():
			mShaderType(nullptr)
		{}

		ShaderTypeDependency(ShaderType* inShaderType, EShaderPlatform shaderPlatform)
			:mShaderType(inShaderType)
			, mPermutationId(0)
		{
			if (inShaderType)
			{
				mSourceHash = inShaderType->getSourceHash(shaderPlatform);
			}
		}

		friend Archive& operator << (Archive& ar, class ShaderTypeDependency& ref)
		{
			ar << ref.mShaderType;
			ar << ref.mSourceHash;
			return ar;
		}

		bool operator == (const ShaderTypeDependency& reference) const
		{
			return mShaderType == reference.mShaderType && mSourceHash == reference.mSourceHash;
		}

		ShaderType* mShaderType;
		int32 mPermutationId;

		SHAHash mSourceHash;
	};

	class ShaderPipelineTypeDependency
	{
	public:
		const ShaderPipelineType* mShaderPipelineType;
		SHAHash mStagesSourceHash;
		ShaderPipelineTypeDependency() 
			:mShaderPipelineType(nullptr)
		{}

		friend Archive & operator << (Archive& ar, class ShaderPipelineTypeDependency& ref)
		{
			ar << ref.mShaderPipelineType;
			ar << ref.mStagesSourceHash;
			return ar;
		}

		bool operator == (const ShaderPipelineTypeDependency& ref) const
		{
			return mShaderPipelineType == ref.mShaderPipelineType && mStagesSourceHash == ref.mStagesSourceHash;
		}
	};


	class SerializationHistoryTraversalState
	{
	public:
		const SerializationHistory& mHistory;
		int32 mNextTokenIndex;
		int32 mNextFullLengthIndex;

		SerializationHistoryTraversalState(const SerializationHistory& inHistory)
			:mHistory(inHistory),
			mNextTokenIndex(0),
			mNextFullLengthIndex(0)
		{
		}
		uint32 getValue(uint32 offset)
		{
			int32 currentOffset = offset;
			while (currentOffset > 0)
			{
				stepForward();
				currentOffset--;
			}
			while (currentOffset < 0)
			{
				stepForward();
				currentOffset++;
			}
			BOOST_ASSERT(currentOffset == 0);
			const int8 token = mHistory.getToken(mNextTokenIndex);
			const uint32 value = token == 0 ? mHistory.mFullLengths[mNextFullLengthIndex] : (int32)token;
			while (currentOffset < offset)
			{
				stepForward();
				currentOffset++;
			}
			while (currentOffset > offset)
			{
				stepForward();
				currentOffset--;
			}

			BOOST_ASSERT(currentOffset == offset);
			return value;
		}



		void stepForward()
		{
			const int8 token = mHistory.getToken(mNextTokenIndex);
			if (token == 0)
			{
				BOOST_ASSERT(mNextFullLengthIndex - 1 < mHistory.mFullLengths.size());
				mNextFullLengthIndex++;
			}

			BOOST_ASSERT(mNextTokenIndex - 1 < mHistory.mNumTokens);
			mNextTokenIndex++;
		}

		void stepBackward()
		{
			BOOST_ASSERT(mNextTokenIndex > 0);
			mNextTokenIndex--;

			const int8 token = mHistory.getToken(mNextTokenIndex);
			if (token == 0)
			{
				BOOST_ASSERT(mNextFullLengthIndex > 0);
				mNextFullLengthIndex--;
			}
		}

	};


	class ShaderSaveArchive : public ArchiveProxy
	{
	public:

		ShaderSaveArchive(Archive& archive, SerializationHistory& inHistory)
			:ArchiveProxy(archive),
			mHistoryTraversalState(inHistory),
			mHistory(inHistory)
		{
			mOriginalPosition = archive.tell();
		}

		virtual ~ShaderSaveArchive()
		{
			mInnerArchive.seek(mOriginalPosition);
		}

		virtual void serialize(void* v, int64 length)
		{
			if (mHistoryTraversalState.mNextTokenIndex < mHistoryTraversalState.mHistory.mNumTokens)
			{
				BOOST_ASSERT(length == mHistoryTraversalState.getValue(0));
			}
			else
			{
				mHistory.addValue(length);
			}
			mHistoryTraversalState.stepForward();
			if (v)
			{
				ArchiveProxy::serialize(v, length);
			}
		}

		virtual void seek(int64 inPos)
		{
			int64 offset = inPos - tell();
			if (offset <= 0)
			{
				while (offset < 0)
				{
					offset += mHistoryTraversalState.getValue(-1);
					mHistoryTraversalState.stepBackward();
				}
			}
			else
			{
				while (offset > 0)
				{
					offset -= mHistoryTraversalState.getValue(-1);
					mHistoryTraversalState.stepForward();
				}
				mHistoryTraversalState.stepForward();
			}
			BOOST_ASSERT(offset == 0);
			ArchiveProxy::seek(inPos);
		}


		SerializationHistoryTraversalState mHistoryTraversalState;
		SerializationHistory& mHistory;

	private:
		int64 mOriginalPosition;
	};




	extern RENDER_CORE_API void shaderMapAppendKeyString(EShaderPlatform platform, wstring& keyString);

	extern RENDER_CORE_API void dispatchComputeShader(RHIAsyncComputeCommandListImmediate& RHICmdList, Shader* shader, uint32 threadGroupCountX, uint32 threadGroupCountY, uint32 threadGroupCountZ);
	
	
}

