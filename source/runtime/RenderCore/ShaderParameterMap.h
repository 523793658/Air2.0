#pragma once
#include "CoreType.h"
#include "ShaderCoreConfig.h"
#include "RHIDefinitions.h"
#include "Misc/SecureHash.h"
#include "Misc/CString.h"
#include "Template/RefCounting.h"
#include "ConstantBuffer.h"
#include "Containers/Map.h"
#include "boost/lexical_cast.hpp"
namespace Air
{

	enum ECompilerFlags
	{
		CFLAG_PreferFlowControl = 0,
		CFLAG_Debug,
		CFLAG_AvoidFlowControl,
		/** Disable shader validation */
		CFLAG_SkipValidation,
		/** Only allows standard optimizations, not the longest compile times. */
		CFLAG_StandardOptimization,
		/** Shader should use on chip memory instead of main memory ring buffer memory. */
		CFLAG_OnChip,
		CFLAG_KeepDebugInfo,
		CFLAG_NoFastMath,
		/** Explicitly enforce zero initialisation on shader platforms that may omit it. */
		CFLAG_ZeroInitialise,
		/** Explicitly enforce bounds checking on shader platforms that may omit it. */
		CFLAG_BoundsChecking,
		// Compile ES2 with ES3.1 features
		CFLAG_FeatureLevelES31,
		// Force removing unused interpolators for platforms that can opt out
		CFLAG_ForceRemoveUnusedInterpolators,
		// Set default precision to highp in a pixel shader (default is mediump on ES2 platforms)
		CFLAG_UseFullPrecisionInPS,
		// Hint that its a vertex to geometry shader
		CFLAG_VertexToGeometryShader
	};

	

	
	

	struct BaseShaderResourceTable
	{
		uint32 mResourceTableBits;
		TArray<uint32> mShaderResourceViewMap;
		TArray<uint32> mSamplerMap;
		TArray<uint32> mUnorderedAccessViewMap;
		TArray<uint32> mResourceTableLayoutHashes;
		BaseShaderResourceTable() :
			mResourceTableBits(0)
		{}

		friend bool operator == (const BaseShaderResourceTable& lhs, const BaseShaderResourceTable& rhs)
		{
			bool bEqual = true;
			bEqual &= (lhs.mResourceTableBits == rhs.mResourceTableBits);
			bEqual &= (lhs.mShaderResourceViewMap.size() == rhs.mShaderResourceViewMap.size());
			bEqual &= (lhs.mSamplerMap.size() == rhs.mSamplerMap.size());
			bEqual &= (lhs.mUnorderedAccessViewMap.size() == rhs.mUnorderedAccessViewMap.size());
			bEqual &= (lhs.mResourceTableLayoutHashes.size() == rhs.mResourceTableLayoutHashes.size());
			if (!bEqual)
			{
				return false;
			}

			bEqual &= (Memory::memcmp(lhs.mShaderResourceViewMap.data(), rhs.mShaderResourceViewMap.data(), lhs.mShaderResourceViewMap.getTypeSize() * lhs.mShaderResourceViewMap.size()) == 0);
			bEqual &= (Memory::memcmp(lhs.mSamplerMap.data(), rhs.mSamplerMap.data(), lhs.mSamplerMap.getTypeSize() * lhs.mSamplerMap.size()) == 0);
			bEqual &= (Memory::memcmp(lhs.mUnorderedAccessViewMap.data(), rhs.mUnorderedAccessViewMap.data(), lhs.mUnorderedAccessViewMap.getTypeSize() * lhs.mUnorderedAccessViewMap.size()) == 0);
			return bEqual;
		}
	};

	inline Archive& operator <<(Archive& ar, BaseShaderResourceTable& srt)
	{
		ar << srt.mResourceTableBits;
		ar << srt.mShaderResourceViewMap;
		ar << srt.mSamplerMap;
		ar << srt.mUnorderedAccessViewMap;
		ar << srt.mResourceTableLayoutHashes;
		return ar;
	}


	class ShaderCodeReader
	{
		const TArray<uint8>& mShaderCode;
	public:
		ShaderCodeReader(const TArray<uint8>& inShaderCode)
			:mShaderCode(inShaderCode)
		{
			BOOST_ASSERT(mShaderCode.size());
		}

		template<class T>
		const T* findOptionalData() const
		{
			return (const T*)findOptionalData(T::key, sizeof(T));
		}

		const uint8* findOptionalData(uint8 inKey, uint8 valueSize) const
		{
			BOOST_ASSERT(valueSize);
			const uint8* end = &mShaderCode[0] + mShaderCode.size();
			int32 localOptionalDataSize = getOptionalDataSize();
			const uint8* start = end - localOptionalDataSize;
			end = end - sizeof(localOptionalDataSize);
			const uint8* current = start;
			while (current < end)
			{
				uint8 key = *current++;
				uint32 size = *((const uint32*)current);
				current += sizeof(size);
				if (key == inKey && size == valueSize)
				{
					return current;
				}
				current += size;
			}
			return 0;
		}

		const uint8* findOptionalDataAndSize(uint8 inKey, int32& outSize) const
		{
			BOOST_ASSERT(mShaderCode.size() >= 4);
			const uint8* end = &mShaderCode[0] + mShaderCode.size();
			int32 localOptionalDataSize = getOptionalDataSize();
			const uint8* start = end - localOptionalDataSize;
			end = end - sizeof(localOptionalDataSize);
			const uint8* current = start;
			while (current < end)
			{
				uint8 key = *current++;
				uint32 size = *((const uint32*)current);
				current += sizeof(size);
				if (key == inKey)
				{
					outSize = size;
					return current;
				}
				current += size;
			}
			outSize = -1;
			return nullptr;
		}

		int32 getOptionalDataSize() const
		{
			if (mShaderCode.size() < sizeof(int32))
			{
				return 0;
			}
			const uint8* end = &mShaderCode[0] + mShaderCode.size();
			int32 localOptionalDataSize = ((const uint32*)end)[-1];
			BOOST_ASSERT(localOptionalDataSize >= 0);
			BOOST_ASSERT(mShaderCode.size() >= localOptionalDataSize);
			return localOptionalDataSize;
		}

		int32 getShaderCodeSize() const
		{
			return mShaderCode.size() - getOptionalDataSize();
		}

		uint32 getActualShaderCodeSize() const
		{
			return mShaderCode.size() - getOptionalDataSize();
		}

		const ANSICHAR* findOptionalData(uint8 inKey) const
		{
			BOOST_ASSERT(mShaderCode.size() >= 4);
			const uint8* end = &mShaderCode[0] + mShaderCode.size();
			int32 localOptionalDataSize = getOptionalDataSize();
			const uint8* start = end - localOptionalDataSize;
			end = end - sizeof(localOptionalDataSize);
			const uint8* current = start;
			while (current < end)
			{
				uint8 key = *current++;
				uint32 size = *((const uint32*)current);
				current += sizeof(size);
				if (key == inKey)
				{
					return (ANSICHAR*)current;
				}
				current += size;
			}
			return 0;
		}

	};

	struct ShaderCodePackedResourceCounts
	{
		static const uint8 key = 'p';
		bool bGlobalConstantBufferUsed;
		uint8 mNumSamplers;
		uint8 mNumSRVs;
		uint8 mNumCBs;
		uint8 mNumUAVs;
	};

	class ShaderCode
	{
		mutable int32 mOptionalDataSize;
		mutable TArray<uint8> mShaderCodeWithOptionalData;
	public:
		ShaderCode()
			:mOptionalDataSize(0)
		{}

		void finalizeShaderCode() const
		{
			if (mOptionalDataSize != -1)
			{
				mOptionalDataSize += sizeof(mOptionalDataSize);
				mShaderCodeWithOptionalData.append((const uint8*)& mOptionalDataSize, sizeof(mOptionalDataSize));
				mOptionalDataSize = -1;
			}
		}

		TArray<uint8>& getWriteAccess()
		{
			return mShaderCodeWithOptionalData;
		}

		int32 getShaderCodeSize() const
		{
			finalizeShaderCode();
			ShaderCodeReader wrapper(mShaderCodeWithOptionalData);
			return wrapper.getShaderCodeSize();
		}

		void getShaderCodeLegacy(TArray<uint8>& out) const
		{
			out.clear();
			out.addUninitialized(getShaderCodeSize());
			Memory::memcpy(out.data(), getReadAccess().data(), mShaderCodeWithOptionalData.size());
		}

		const TArray<uint8>& getReadAccess() const
		{
			finalizeShaderCode();
			return mShaderCodeWithOptionalData;
		}
		template<class T>
		void addOptionalData(const T& In)
		{
			addOptionalData(T::key, (uint8*)& In, sizeof(T));
		}

		void addOptionalData(uint8 key, const uint8* valuePtr, uint32 valueSize)
		{
			BOOST_ASSERT(valuePtr);
			BOOST_ASSERT(mOptionalDataSize >= 0);
			mShaderCodeWithOptionalData.push_back(key);
			mShaderCodeWithOptionalData.append((const uint8*)& valueSize, sizeof(valueSize));
			mShaderCodeWithOptionalData.append(valuePtr, valueSize);
			mOptionalDataSize += sizeof(uint8) + sizeof(valueSize) + (uint32)valueSize;
		}

		void addOptionalData(uint8 key, const ANSICHAR* inString)
		{
			uint32 size = CStringAnsi::strlen(inString) + 1;
			addOptionalData(key, (uint8*)inString, size);
		}

		friend Archive& operator << (Archive& ar, ShaderCode& output)
		{
			if (ar.isLoading())
			{
				output.mOptionalDataSize = -1;
			}
			else
			{
				output.finalizeShaderCode();
			}
			ar << output.mShaderCodeWithOptionalData;
			return ar;
		}

	};

	struct ShaderTarget
	{
		uint32 mFrequency : SF_NumBits;
		uint32 mPlatform : SP_NumBits;
		ShaderTarget() {}
		ShaderTarget(EShaderFrequency inFrequency, EShaderPlatform inPlatform)
			:mFrequency(inFrequency),
			mPlatform(inPlatform)
		{}

		friend bool operator == (const ShaderTarget& lhs, const ShaderTarget& rhs)
		{
			return lhs.mFrequency == rhs.mFrequency && lhs.mPlatform == rhs.mPlatform;
		}

		friend Archive& operator << (Archive& ar, ShaderTarget& target)
		{
			uint32 targetFrequency = target.mFrequency;
			uint32 targetPlatform = target.mPlatform;
			ar << targetFrequency << targetPlatform;
			return ar;
		}

		EShaderPlatform getPlatform() const
		{
			return (EShaderPlatform)mPlatform;
		}
	};


	class ShaderParameterMap
	{
	public:
		ShaderParameterMap() {}
		RENDER_CORE_API bool findParameterAllocation(const TCHAR* inParameterName, uint16& outBufferIndex, uint16& outBaseIndex, uint16& outSize) const;

		RENDER_CORE_API bool containsParameterAllocation(const TCHAR* inParameterName)const;
		RENDER_CORE_API void addParameterAllocation(const TCHAR* inParameterName, uint16 inBufferIndex, uint16 BaseIndex, uint16 size);

		RENDER_CORE_API void removeParameterAllocation(const TCHAR* parameterName);

		RENDER_CORE_API void verifyBindingAreComplete(const TCHAR* shaderTypeName, ShaderTarget target, class VertexFactoryType* inVertexFactoryType) const;

		void updateHash(SHA1& hashState) const;

		friend Archive& operator << (Archive& ar, ShaderParameterMap& inParameterMap)
		{
			return ar << inParameterMap.mParameterMap;
		}

		inline void getAllParameterNames(TArray<wstring>& outNames) const
		{
			mParameterMap.generateKeyArray(outNames);
		}

	private:
		struct ParameterAllocation
		{
			uint16 mBufferIndex;
			uint16 mBaseIndex;
			uint16 mSize;
			mutable bool bBound;
			ParameterAllocation()
				:bBound(false) {}

			friend Archive& operator << (Archive& ar, ParameterAllocation& allocation)
			{
				return ar << allocation.mBufferIndex << allocation.mBaseIndex << allocation.mSize << allocation.bBound;
			}
		};
		TMap<wstring, ParameterAllocation> mParameterMap;
	};

	


	



	struct ShaderCompilerError
	{

		ShaderCompilerError(const TCHAR* inStrippedErrorMessage = TEXT(""))
			:mErrorFile(TEXT(""))
			, mErrorLineString(TEXT(""))
			, mStrippedErrorMessage(inStrippedErrorMessage)
		{
		}
		wstring mErrorFile;
		wstring mErrorLineString;
		wstring mStrippedErrorMessage;

		wstring getErrorString() const
		{
			return mErrorFile + TEXT("(") + mErrorLineString + TEXT("): ") + mStrippedErrorMessage;
		}

		friend Archive& operator << (Archive& ar, ShaderCompilerError& error)
		{
			return ar << error.mErrorFile << error.mErrorLineString << error.mStrippedErrorMessage;
		}
	};

	
	struct ShaderCompilerOutput
	{
		ShaderParameterMap mParameterMap;
		TArray<ShaderCompilerError> mErrors;
		ShaderTarget mTarget;
		ShaderCode mShaderCode;
		SHAHash mOutputHash;
		uint32 mNumInstructions{ 0 };
		uint32 mNumTextureSamplers{ 0 };
		bool bSucceeded{ false };
		bool bFailedRemovingUnused{ false };
		bool bSupportsQueryingUsedAttributes{ false };
		TArray<wstring> mUsedAttributes;
		RENDER_CORE_API void generateOutputHash();

		friend Archive& operator << (Archive& ar, ShaderCompilerOutput& output)
		{
			ar << output.mParameterMap << output.mErrors << output.mTarget << output.mShaderCode << output.mNumInstructions << output.mNumTextureSamplers << output.bSucceeded;
			ar << output.bFailedRemovingUnused << output.bSupportsQueryingUsedAttributes << output.mUsedAttributes;
			return ar;
		}
	};
	

	class ShaderCompilerDefinitions
	{
	public:
		ShaderCompilerDefinitions()
		{
			mDefinitions.reserve(50);
		}

		void setDefine(const TCHAR* name, const TCHAR* value)
		{
			mDefinitions.emplace(name, value);
		}

		void setDefine(const TCHAR* name, uint32 value)
		{
			mDefinitions.emplace(name, boost::lexical_cast<wstring>(value));
		}

		void setDefine(const TCHAR* name, int32 value)
		{
			mDefinitions.emplace(name, boost::lexical_cast<wstring>(value));
		}

		void setFloatDefine(const TCHAR* name, float value)
		{
			mDefinitions.emplace(name, boost::lexical_cast<wstring>(value));
		}

		const TMap<wstring, wstring>& getDefinionMap() const
		{
			return mDefinitions;
		}

		friend Archive& operator << (Archive& ar, ShaderCompilerDefinitions& defs)
		{
			return ar << defs.mDefinitions;
		}
		void merge(const ShaderCompilerDefinitions& other)
		{
			mDefinitions.append(other.mDefinitions);
		}

	private:
		TMap<wstring, wstring> mDefinitions;
	};

	struct ShaderCompilerEnvironment : public RefCountedObject
	{
		TMap<wstring, wstring> mIncludeVirtualPathToContentsMap;

		TMap<wstring, wstring> mIncludeVirtualPathToExternalContentsMap;


		TArray<uint32> mCompilerFlags;
		TMap<uint32, uint8> mRenderTargetOutputFormatsMap;
		TMap<wstring, ResourceTableEntry> mResourceTableMap;
		TMap<wstring, uint32> mResourceTableLayoutHashes;

		ShaderCompilerEnvironment()
		{
			mIncludeVirtualPathToContentsMap.reserve(15);
		}


		friend Archive& operator <<(Archive& ar, ShaderCompilerEnvironment& environment)
		{
			return ar << environment.mIncludeVirtualPathToContentsMap << environment.mDefinitions << environment.mCompilerFlags << environment.mRenderTargetOutputFormatsMap << environment.mResourceTableMap << environment.mResourceTableLayoutHashes;
		}

		RENDER_CORE_API void merge(const ShaderCompilerEnvironment& other);

		void setDefine(const TCHAR* name, const TCHAR* value) { mDefinitions.setDefine(name, value); }

		void setDefine(const TCHAR* name, uint32 value)
		{
			mDefinitions.setDefine(name, value);
		}
		void setDefine(const TCHAR* name, int32 value)
		{
			mDefinitions.setDefine(name, value);
		}

		void setDefine(const TCHAR* name, bool value) {
			mDefinitions.setDefine(name, (uint32)value);
		}
		void setDefine(const TCHAR* name, float value)
		{
			mDefinitions.setFloatDefine(name, value);
		}

		const TMap<wstring, wstring>& getDefinitions() const
		{
			return mDefinitions.getDefinionMap();
		}

	private:
		ShaderCompilerDefinitions mDefinitions;

	};

	struct ShaderCompilerInput
	{
		ShaderTarget mTarget;
		wstring mShaderFormat;
		wstring mSourceFilePrefix;
		wstring mSourceFilename;
		wstring mEntryPointName;
		bool bSkipPreprocessedCache;
		bool bGenerateDirectCompileFile;

		bool bCompilingForShaderPipeline;

		bool bIncludeUsedOutputs;
		TArray<wstring> mUsedOutputs;
		wstring mDumpDebugInfoRootPath;
		wstring mDumpDebugInfoPath;
		wstring mDebugGroupName;
		ShaderCompilerEnvironment mEnvironment;
		TRefCountPtr<ShaderCompilerEnvironment> mSharedEnvironment;
		ShaderCompilerInput() :
			bGenerateDirectCompileFile(false),
			bCompilingForShaderPipeline(false),
			bIncludeUsedOutputs(false),
			bSkipPreprocessedCache(false)
		{}
		wstring generateShaderName() const
		{
			wstring name;
			if (mDebugGroupName == TEXT("Global"))
			{
				name = mSourceFilename + TEXT(".hlsl|") + mEntryPointName;
			}
			else
			{
				name = mDebugGroupName + TEXT(":") + mSourceFilename + TEXT(".hlsl");
			}
			return name;
		}



		friend Archive& operator << (Archive& ar, ShaderCompilerInput& input)
		{
			ar << input.mTarget;
			{
				wstring shaderFormatString = input.mShaderFormat;
				ar << shaderFormatString;
				input.mShaderFormat = shaderFormatString;
			}
			ar << input.mSourceFilePrefix;
			ar << input.mSourceFilename;
			ar << input.mEntryPointName;
			ar << input.bSkipPreprocessedCache;
			ar << input.bCompilingForShaderPipeline;
			ar << input.bGenerateDirectCompileFile;
			ar << input.bIncludeUsedOutputs;
			ar << input.mUsedOutputs;
			ar << input.mDumpDebugInfoRootPath;
			ar << input.mDumpDebugInfoPath;
			ar << input.mDebugGroupName;
			ar << input.mEnvironment;

			bool bHasSharedEnvironment = isValidRef(input.mSharedEnvironment);
			ar << bHasSharedEnvironment;
			if (bHasSharedEnvironment)
			{
				if (ar.isSaving())
				{
					ar << *(input.mSharedEnvironment);
				}
				if (ar.isLoading())
				{
					input.mSharedEnvironment = new ShaderCompilerEnvironment();
					ar << *(input.mSharedEnvironment);
				}
			}
			return ar;
		}
	};



	struct CachedConstantBufferDeclaration
	{
		wstring mDeclaration;
	};

	extern RENDER_CORE_API void flushShaderFileCache();
	extern RENDER_CORE_API void verifyShaderSourceFiles();
	extern RENDER_CORE_API void getShaderIncludes(const TCHAR* filename, TArray<wstring>& includeFilenames, uint32 depthLimits = 7);
	extern RENDER_CORE_API const SHAHash& getShaderFileHash(const TCHAR* filename, EShaderPlatform shaderPlatform);
	extern RENDER_CORE_API const class SHAHash& getShaderFilesHash(const TArray<wstring>& filenames);
	extern void generateReferencedConstantBuffers(const TCHAR* sourceFile, const TCHAR* shaderTypeName, const TMap<wstring, TArray<const TCHAR*>>& shaderFileToConstantBufferVaribles, TMap<const TCHAR*, CachedConstantBufferDeclaration>& constantBufferEntries);

	extern RENDER_CORE_API void serializeConstantBufferInfo(class ShaderSaveArchive& ar, const TMap<const TCHAR*, CachedConstantBufferDeclaration>& constantBufferEntries);

	extern RENDER_CORE_API bool loadShaderSourceFile(const TCHAR* filename, wstring& outFileContents);

	extern RENDER_CORE_API void loadShaderSourceFileChecked(const TCHAR* filename, wstring& outFileContents);

	extern RENDER_CORE_API wstring getRelativeShaderFilename(const wstring& inFilename);

	extern RENDER_CORE_API void initializeShaderType();
}