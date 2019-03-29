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
namespace Air
{
	extern SHADER_CORE_API SHAHash GGlobalShaderMapHash;

	class VertexFactoryType;
	class ShaderType;
	class Shader;
	class ShaderConstantBufferParameter;
#define DECLARE_EXPORTED_SHADER_TYPE(ShaderClass, ShaderMetaTypeShortcut, RequiredAPI)  \
	public :\
	typedef ShaderMetaTypeShortcut##ShaderType ShaderMetaType;	\
	static RequiredAPI ShaderMetaType mStaticType;	\
	static Shader* constructSerializedInstance() {return new ShaderClass();}	\
	static Shader* constructCompiledInstance(const ShaderMetaType::CompiledShaderInitializerType& initializer)	 \
	{\
		return new ShaderClass(initializer);\
	}\
	virtual uint32 getTypeSize() const override{return sizeof(*this);}

#define DECLARE_SHADER_TYPE(ShaderClass, ShaderMetaTypeShortcut)	\
	DECLARE_EXPORTED_SHADER_TYPE(ShaderClass, ShaderMetaTypeShortcut,)

#define IMPLEMENT_SHADER_TYPE(TemplatePrefix, ShaderClass, SourceFilename, FunctionName, Frequency) \
	TemplatePrefix	\
	ShaderClass::ShaderMetaType	ShaderClass::mStaticType(\
		TEXT(#ShaderClass),	\
		SourceFilename,	\
		FunctionName,	\
		Frequency,	\
		ShaderClass::constructSerializedInstance,	\
		ShaderClass::constructCompiledInstance,	\
		ShaderClass::modifyCompilationEnvironment,\
		ShaderClass::shouldCache,	\
		ShaderClass::getStreamOutElements\
	);

#define IMPLEMENT_SHADER_TYPE2(ShaderClass, Frequency)	\
	template<>	\
	ShaderClass::ShaderMetaType ShaderClass::mStaticType(	\
	TEXT(#ShaderClass),	\
	ShaderClass::getSourceFilename(),	\
	ShaderClass::getFunctionName(),	\
	Frequency,	\
	ShaderClass::constructSerializedInstance,	\
	ShaderClass::constructCompiledInstance,	\
	ShaderClass::modifyCompilationEnvironment,	\
	ShaderClass::shouldCache, \
	ShaderClass::getStreamOutElements	\
		);

	class ShaderResourceId
	{
	public:
		ShaderResourceId() {}
		ShaderResourceId(const ShaderCompilerOutput& output, const TCHAR* inSpecificShaderTypeName)
			:mTarget(output.mTarget),
			mOutputHash(output.mOutputHash),
			mSpecificShaderTypeName(inSpecificShaderTypeName)
		{}

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
	};

	class ShaderResource : public RenderResource, public DeferredCleanupInterface
	{
		friend class Shader;

	public:
		SHADER_CORE_API ShaderResource();

		ShaderResource(const ShaderCompilerOutput& output, ShaderType* inSpecificType);

		~ShaderResource();
		SHADER_CORE_API void serialize(Archive& ar);

		SHADER_CORE_API void AddRef();
		SHADER_CORE_API void Release();
		SHADER_CORE_API void Register();


		FORCEINLINE const VertexShaderRHIParamRef getVertexShader()
		{
			BOOST_ASSERT(mTarget.mFrequency == SF_Vertex);
			if (!isInitialized())
			{
				initializeShaderRHI();
			}
			return mVertexShader;
		}

		FORCEINLINE const PixelShaderRHIParamRef getPixelShader()
		{
			BOOST_ASSERT(mTarget.mFrequency == SF_Pixel);
			if (!isInitialized())
			{
				initializeShaderRHI();
			}
			return mPixelShader;
		}

		FORCEINLINE const HullShaderRHIParamRef getHullShader()
		{
			BOOST_ASSERT(mTarget.mFrequency == SF_Hull);
			if (!isInitialized())
			{
				initializeShaderRHI();
			}
			return mHullShader;
		}

		FORCEINLINE const DomainShaderRHIParamRef getDomainShader()
		{
			BOOST_ASSERT(mTarget.mFrequency == SF_Domain);
			if (!isInitialized())
			{
				initializeShaderRHI();
			}
			return mDomainShader;
		}

		FORCEINLINE const GeometryShaderRHIParamRef getGeometryShader()
		{
			BOOST_ASSERT(mTarget.mFrequency == SF_Geometry);
			if (!isInitialized())
			{
				initializeShaderRHI();
			}
			return mGeometryShader;
		}

		FORCEINLINE const ComputeShaderRHIParamRef getComputeShader()
		{
			BOOST_ASSERT(mTarget.mFrequency == SF_Compute);
			if (!isInitialized())
			{
				initializeShaderRHI();
			}
			return mComputeShader;
		}

		SHADER_CORE_API ShaderResourceId getId() const;

		SHADER_CORE_API void initializeShaderRHI();

		uint32 getSizeBytes() const
		{
			return mCode.getAllocatedSize() + sizeof(ShaderResource);
		}

		virtual void initRHI();
		virtual void releaseRHI();

		virtual void finishCleanup();

		SHADER_CORE_API static ShaderResource* findShaderResourceById(const ShaderResourceId& id);

		SHADER_CORE_API static ShaderResource* findOrCreateShaderResource(const ShaderCompilerOutput& output, class ShaderType* specificType);

		SHADER_CORE_API static void getAllShaderResourceId(TArray<ShaderResourceId>& ids);

		SHADER_CORE_API static bool arePlatformsCompatible(EShaderPlatform currentPlatform, EShaderPlatform targetPlatform);

	public:

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
		const SerializationHistory& mSerializationHistory;

		ShaderTarget mTarget;

		ShaderId(const SerializationHistory& inSerializationHistory) :
			mSerializationHistory(inSerializationHistory)
		{}

		SHADER_CORE_API ShaderId(const SHAHash& inMaterialShaderMapHash, const ShaderPipelineType * inShaderPipeline, VertexFactoryType* inVertexFactoryType, ShaderType* inShaderType, ShaderTarget inTarget);

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
	class SHADER_CORE_API Shader : public DeferredCleanupInterface
	{
		friend class ShaderType;
	public:
		struct CompiledShaderInitializerType
		{
			ShaderType* mType;
			ShaderTarget mTarget;
			const TArray<uint8> & mCode;
			const ShaderParameterMap& mParameterMap;
			const SHAHash& mOutputHash;
			ShaderResource* mResource;
			SHAHash mMaterialShaderMapHash;
			const ShaderPipelineType* mShaderPipeline;
			VertexFactoryType* mVertexFactoryType;

			CompiledShaderInitializerType(
				ShaderType* inType,
				const ShaderCompilerOutput& compilerOutput,
				ShaderResource* inResource,
				const SHAHash& inMaterialShaderMapHash,
				const ShaderPipelineType* inShaderPipeline,
				VertexFactoryType* inVertexFactoryType
			) :
				mType(inType),
				mTarget(compilerOutput.mTarget),
				mCode(compilerOutput.mShaderCode.getReadAccess()),
				mParameterMap(compilerOutput.mParameterMap),
				mOutputHash(compilerOutput.mOutputHash),
				mResource(inResource),
				mMaterialShaderMapHash(inMaterialShaderMapHash),
				mShaderPipeline(inShaderPipeline),
				mVertexFactoryType(inVertexFactoryType)
			{}
		};

		Shader();

		Shader(const CompiledShaderInitializerType& initializer);

		virtual ~Shader();

		virtual uint32 getTypeSize() const
		{
			return sizeof(*this);
		}
		virtual void finishCleanup();

		void beginInitializeResources()
		{
			beginInitResource(mResource);
		}

		inline ShaderType* getType() const { return mType; }

		ShaderId getId() const;

		inline const ShaderTarget getTarget() const { return mTarget; }

		inline void checkShaderIsValid() const
		{
			BOOST_ASSERT(mCanary == ShaderMagic_Initialized);
		}

		inline Shader* getShaderChecked()
		{
			checkShaderIsValid();
			return this;
		}

		void setResource(ShaderResource* inResource);

		void AddRef();
		void Release();
		void Register();
		void deRegister();
		void registerSerializedResource();

		bool serializeBase(Archive& ar, bool bShadersInline);
		virtual bool serialize(Archive& ar) { return false; }
	public:
		inline const VertexShaderRHIParamRef getVertexShader()
		{
			return mResource->getVertexShader();
		}

		inline const HullShaderRHIParamRef getHullShader()
		{
			return mResource->getHullShader();
		}

		inline const DomainShaderRHIParamRef getDomainShader()
		{
			return mResource->getDomainShader();
		}

		inline const GeometryShaderRHIParamRef getGeometryShader()
		{
			return mResource->getGeometryShader();
		}

		inline const PixelShaderRHIParamRef getPixelShader()
		{
			return mResource->getPixelShader();
		}

		template<typename ConstantBufferStructType>
		FORCEINLINE_DEBUGGABLE const TShaderConstantBufferParameter<ConstantBufferStructType>& getConstantBufferParameter() const
		{
			ConstantBufferStruct* searchStruct = &ConstantBufferStructType::mStaticStruct;
			int32 foundIndex = INDEX_NONE;
			for (int32 structIndex = 0, count = mConstantBufferParameterStructs.size(); structIndex < count; structIndex++)
			{
				if (mConstantBufferParameterStructs[structIndex] == searchStruct)
				{
					foundIndex = structIndex;
					break;
				}
			}
			if (foundIndex != INDEX_NONE)
			{
				const TShaderConstantBufferParameter<ConstantBufferStructType>& foundParameter = (const TShaderConstantBufferParameter<ConstantBufferStructType>&)*mConstantBufferParameters[foundIndex];
				foundParameter.mSetParametersId = mSetParametersId;
				return foundParameter;
			}
			else
			{
				static TShaderConstantBufferParameter<ConstantBufferStructType> unboundParameter;
				unboundParameter.setInitialized();
				return unboundParameter;
			}
		}

		static void getStreamOutElements(StreamOutElementList& elementList, TArray<uint32>& streamStrides, int32& rasterizedStream) {}

	protected:
		TArray<ConstantBufferStruct*> mConstantBufferParameterStructs;
		TArray<ShaderConstantBufferParameter*> mConstantBufferParameters;

	private:
		SHAHash mOutputHash;
		ShaderResource* mSerializedResource;

		TRefCountPtr<ShaderResource> mResource;

		SHAHash mMaterialShaderMapHash;
		const ShaderPipelineType* mShaderPipeline;
		VertexFactoryType* mVFType;

		SHAHash mVFSourceHash;
		ShaderType* mType;
		SHAHash mSourceHash;

		ShaderTarget mTarget;

		mutable uint32 mNumRefs;

		mutable uint32 mSetParametersId;

		uint32 mCanary;

	public:
		static const uint32 ShaderMagic_Uninitialized = 0xbd9922df;
		static const uint32 ShaderMagic_CleaningUp = 0xdc67f93b;
		static const uint32 ShaderMagic_Initialized = 0x335b43ab;
	};





	class SHADER_CORE_API ShaderType
	{
	public:
		enum class EShaderTypeForDynamicCast : uint32
		{
			Global,
			Material,
			MeshMaterial
		};

		typedef class Shader* (*ConstructSerializedType)();
		typedef void(*GetStreamOutElementsType)(StreamOutElementList& elementList, TArray<uint32>& streamStrides, int32 & RasterizerStream);

		GetStreamOutElementsType mGetStreamOutElementsRef;

		ShaderType(EShaderTypeForDynamicCast inShaderTypeForDynamicCast, const TCHAR* inName, const TCHAR* inSourceFilename, const TCHAR* inFunctionName, uint32 inFrequency, ConstructSerializedType inConstructSerializedRef, GetStreamOutElementsType inGetStreamOutElementsRef);
		~ShaderType();

		FORCEINLINE MeshMaterialShaderType* getMeshMaterialShaderType()
		{
			return (mShaderTypeForDynamicCast == EShaderTypeForDynamicCast::MeshMaterial) ? reinterpret_cast<MeshMaterialShaderType*>(this) : nullptr;
		}



		FORCEINLINE const MeshMaterialShaderType* getMeshMaterialShaderType() const
		{
			return (mShaderTypeForDynamicCast == EShaderTypeForDynamicCast::MeshMaterial) ? reinterpret_cast<const MeshMaterialShaderType*>(this) : nullptr;
		}

		FORCEINLINE const MaterialShaderType* getMaterialShaderType() const
		{
			return (mShaderTypeForDynamicCast == EShaderTypeForDynamicCast::Material) ? reinterpret_cast<const MaterialShaderType*>(this) : nullptr;
		}

		FORCEINLINE MaterialShaderType* getMaterialShaderType()
		{
			return (mShaderTypeForDynamicCast == EShaderTypeForDynamicCast::Material) ? reinterpret_cast<MaterialShaderType*>(this) : nullptr;
		}

		static TLinkedList<ShaderType*>*& getTypeList();

		bool limitShaderResourceToThisType() const
		{
			return mGetStreamOutElementsRef != &Shader::getStreamOutElements;
		}

		inline const TCHAR* getShaderFilename() const
		{
			return mSourceFilename;
		}

		inline const TCHAR* getFunctionName() const
		{
			return mFunctionName;
		}

		inline const SerializationHistory& getSerializationHistory() const
		{
			return mSerializationHistory;
		}

		inline const TCHAR* getName() const
		{
			return mName;
		}

		inline EShaderFrequency getFrequency() const
		{
			return (EShaderFrequency)mFrequency;
		}

		inline void removeFromShaderIdMap(ShaderId id)
		{
			BOOST_ASSERT(isInGameThread());
			mShaderIdMap.erase(id);
		}

		void flushShaderFileCache(const TMap<wstring, TArray<const TCHAR*>>& shaderFileToConstantBufferVariables);

		SHADER_CORE_API friend Archive& operator << (Archive& ar, ShaderType*& type);

		Shader* findShaderById(const ShaderId& id);

		Shader* constructForDeserialization() const
		{
			return (*mConstructSerializedRef)();
		}

		const SHAHash& getSourceHash() const;

		void addToShaderIdMap(ShaderId id, Shader* shader);


		static TMap<wstring, ShaderType*> & getNameToTypeMap();

		static void initialize(const TMap<wstring, TArray<const TCHAR*>>& shaderFileToConstantBufferVariables);

		void getStreamOutElements(StreamOutElementList& elementList, TArray<uint32>& streamStrides, int32& rasterizedStream)
		{
			(*mGetStreamOutElementsRef)(elementList, streamStrides, rasterizedStream);
		}

		inline const TMap<const TCHAR*, CachedConstantBufferDeclaration>& getReferencedConstantBufferStructsCache() const
		{
			return mReferencedConstantBufferStructsCache;
		}

		FORCEINLINE const GlobalShaderType* getGlobalShaderType() const
		{
			return (mShaderTypeForDynamicCast == EShaderTypeForDynamicCast::Global) ? reinterpret_cast<const GlobalShaderType*>(this) : nullptr;
		}

		FORCEINLINE GlobalShaderType* getGlobalShaderType()
		{
			return (mShaderTypeForDynamicCast == EShaderTypeForDynamicCast::Global) ? reinterpret_cast<GlobalShaderType*>(this) : nullptr;
		}

		void addReferencedConstantBufferIncludes(ShaderCompilerEnvironment& outEnvironment, wstring& outSourceFilePrefix, EShaderPlatform platform);

	private:
		EShaderTypeForDynamicCast mShaderTypeForDynamicCast;
		uint32 mHashIndex;
		const TCHAR* mName;
		wstring mTypeName;
		const TCHAR* mSourceFilename;
		const TCHAR* mFunctionName;
		uint32 mFrequency;

		ConstructSerializedType mConstructSerializedRef;
		TMap<ShaderId, Shader*> mShaderIdMap;
		TLinkedList<ShaderType*> mGlobalListLink;

		SerializationHistory mSerializationHistory;

		TMap<const TCHAR*, CachedConstantBufferDeclaration> mReferencedConstantBufferStructsCache;

		bool bCachedConstantBufferStructDeclarations[SP_NumPlatforms];


		static bool bInitialiezedSerializationHistory;
	};


	class SHADER_CORE_API ShaderPipelineType
	{
	public:
		static TLinkedList<ShaderPipelineType*>*& getTypeList();

		static void initialize();
		static void uninitialize();
		FORCEINLINE const TArray<const ShaderType*> & getStages() const
		{
			return mStages;
		}

		const TCHAR* getName() const { return mName; }

		const SHAHash& getSourceHash()const;

		SHADER_CORE_API friend Archive& operator << (Archive& ar, const ShaderPipelineType*& type);

		FORCEINLINE bool hasTessellation() const { return mAllStages[SF_Domain] != nullptr; }

		FORCEINLINE bool hasGeometry() const { return mAllStages[SF_Geometry] != nullptr; }

		FORCEINLINE bool hasPixelShader() const { return mAllStages[SF_Pixel] != nullptr; }

		bool shoudlOptimizeUnusedOutputs() const { return bShoudlOptimizeUnusedOutputs; }

		bool isMeshMaterialTypePipeline() const { return mStages[0]->getMaterialShaderType() != nullptr; }

		bool isGlobalTypePipeline() const { return mStages[0]->getGlobalShaderType() != nullptr; }

		bool isMaterialTypePipeline() const { return mStages[0]->getMaterialShaderType() != nullptr; }


	private:
		const TCHAR* const mName;

		TArray<const ShaderType*> mStages;

		const ShaderType* mAllStages[SF_NumFrequencies];

		bool bShoudlOptimizeUnusedOutputs;

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

	class SHADER_CORE_API ShaderPipeline
	{
	public:
		const ShaderPipelineType* mPipelineType;
		TRefCountPtr<Shader> mVertexShader;
		TRefCountPtr<Shader> mHullShader;
		TRefCountPtr<Shader> mDomainShader;
		TRefCountPtr<Shader> mGeometryShader;
		TRefCountPtr<Shader> mPixelShader;


		ShaderPipeline(const ShaderPipelineType* inPipelineType, const TArray<Shader*>& inStages);

		ShaderPipeline(const ShaderPipelineType* inPipelineType, const TArray<TRefCountPtr<Shader>>& inStages);

		~ShaderPipeline();

		Shader* getShader(EShaderFrequency frequency)
		{
			switch (frequency)
			{
			case Air::SF_Vertex:
				return mVertexShader.getReference();
			case Air::SF_Hull:
				return mHullShader.getReference();
			case Air::SF_Domain:
				return mDomainShader.getReference();
			case Air::SF_Pixel:
				return mPixelShader.getReference();
			case Air::SF_Geometry:
				return mGeometryShader.getReference();
			default:
				BOOST_ASSERT(false);
			}
			return nullptr;
		}
		enum EFilter
		{
			EAll,
			EOnlyShared,
			EOnlyUnique,
		};

		
		void validate();

	};

	inline bool operator<(const ShaderPipeline& lhs, const ShaderPipeline& rhs)
	{
		CompareShaderPipelineType comparator;
		return comparator(*lhs.mPipelineType, *rhs.mPipelineType);
	}

	
	
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
		SHADER_CORE_API SelfContainedShaderId();

		SHADER_CORE_API SelfContainedShaderId(const ShaderId & inShaderId);

		SHADER_CORE_API bool isValid();

		SHADER_CORE_API friend Archive& operator << (Archive& ar, class SelfContainedShaderId& ref);
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
		ShaderType* getShader() const
		{
			BOOST_ASSERT(bHasBeenRegistered);
			const auto& it = mShaders.find(&ShaderType::mStaticType);
			BOOST_ASSERT(it != mShaders.end());
			const TRefCountPtr<Shader> shaderRef = it->second;

			return (ShaderType*)(shaderRef->getShaderChecked());
		}
		TShaderMap()
			:bHasBeenRegistered(true)
		{}

		Shader* getShader(ShaderType* shaderType) const
		{
			BOOST_ASSERT(bHasBeenRegistered);

			auto shaderRef = mShaders.find(shaderType);
			return shaderRef != mShaders.end() ? (shaderRef->second)->getShaderChecked() : nullptr;
		}

		inline const TMap<ShaderType*, TRefCountPtr<Shader>>& getShaders() const
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

		void serializeInline(Archive& ar, bool bInlineShaderResource, bool bHandleShaderKeyChanges)
		{
			if (ar.isSaving())
			{
				int32 numShaders = mShaders.size();
				ar << numShaders;
				auto sortedShaders = mShaders;
				for (auto it = sortedShaders.begin(); it != sortedShaders.end(); ++it)
				{
					ShaderType* type = it->first;
					BOOST_ASSERT(type);
					BOOST_ASSERT(type->getName() != nullptr);
					ar << type;
					Shader* currentShader = it->second;
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
				addShader(shader->getType(), shader);
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

		void addShader(ShaderType* type, Shader* shader)
		{
			BOOST_ASSERT(type != nullptr);
			mShaders.emplace(type, shader);
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
				if (pipeline->mPipelineType->shoudlOptimizeUnusedOutputs() && filter == ShaderPipeline::EOnlyShared)
				{
					continue;
				}
				else if (!pipeline->mPipelineType->shoudlOptimizeUnusedOutputs() && filter == ShaderPipeline::EOnlyUnique)
				{
					continue;
				}
				outShaderPipelines.push_back(pipeline);
			}
		}

		void removeShaderType(const ShaderType* shaderType)
		{
			mShaders.erase(const_cast<ShaderType*>(shaderType));
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

		bool hasShader(ShaderType* type) const
		{
			BOOST_ASSERT(bHasBeenRegistered);
			const auto it = mShaders.find(type);
			if (it != mShaders.end())
			{
				if (it->second.getReference() != nullptr)
				{
					return true;
				}
			}
			return false;
		}

	private:

		bool bHasBeenRegistered;
		TMap<ShaderType*, TRefCountPtr<Shader>> mShaders;

		TArray<Shader*> mSerializedShaders;

		TArray<SerializedShaderPipeline*> mSerializedShaderPipelines;
		TMap<const ShaderPipelineType*, ShaderPipeline*> mShaderPipelines;
	};


	template<typename ShaderType>
	class TShaderMapRef
	{
	public:
		TShaderMapRef(const TShaderMap<typename ShaderType::ShaderMetaType>* shaderIndex)
			:mShader(shaderIndex->template getShader<ShaderType>())
		{}

		FORCEINLINE ShaderType* operator->() const
		{
			return mShader;
		}

		FORCEINLINE ShaderType* operator*() const
		{
			return mShader;
		}


		SHADER_CORE_API friend Archive& operator << (Archive& ar, ShaderType* & ref);

	private:
		ShaderType* mShader;
	};



	class ShaderTypeDependency
	{
	public:
		ShaderTypeDependency():
			mShaderType(nullptr)
		{}

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




	extern SHADER_CORE_API void shaderMapAppendKeyString(EShaderPlatform platform, wstring& keyString);

	
}

