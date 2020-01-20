#include "HAL/PlatformProperties.h"
#include "HAL/PlatformProcess.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/Paths.h"
#include "Misc/ScopeLock.h"
#include "ShaderParameterMap.h"
#include "VertexFactory.h"
#include "Containers/Map.h"
#include "Misc/FileHelper.h"
#include "Shader.h"
#include "Template/RefCounting.h"
#include "Containers/LinkList.h"
#include "boost/algorithm/string.hpp"
#include "Modules/ModuleManager.h"
#include "Interface/IShaderFormat.h"
namespace Air
{
	SHAHash GGlobalShaderMapHash;

	class ShaderHashCache
	{
	public:
		ShaderHashCache()
			:bInitialized(false)
		{}

		void initialize()
		{
			const wstring emptyDirectory;
			for (auto& platform : mPlatforms)
			{
				platform.mIncludeDirectory = emptyDirectory;
				platform.mShaderHashCache.clear();
			}

			TArray<wstring> modules;
			ModuleManager::get().findModules(SHADERFORMAT_MODULE_WILDCARD, modules);

			if (!modules.size())
			{
				AIR_LOG(LogShaders, Error, TEXT("No target shader formats found!"));
			}

			TArray<wstring> supportedFormats;
			for (int32 moduleIndex = 0; moduleIndex < modules.size(); moduleIndex++)
			{
			}
			bInitialized = true;
		}

		SHAHash* findHash(EShaderPlatform shaderPlatform, const wstring& virtualFilePath)
		{
			auto it = mPlatforms[shaderPlatform].mShaderHashCache.find(virtualFilePath);
			if (mPlatforms[shaderPlatform].mShaderHashCache.end() != it)
			{
				return &it->second;
			}
			return nullptr;
		}

	private:
		struct Platform
		{
			wstring mIncludeDirectory;
			TMap<wstring, SHAHash> mShaderHashCache;
		};

		Platform mPlatforms[EShaderPlatform::SP_NumPlatforms];
		bool bInitialized;

		const wstring& getPlatformIncludeDirectory(EShaderPlatform shaderPlatform)
		{
			return mPlatforms[shaderPlatform].mIncludeDirectory;
		}
	};

	TMap<wstring, SHAHash> GShaderHashCache;
	TMap<wstring, wstring> GShaderFileCache;

	CriticalSection FileCacheCriticalSection;

	class ShaderCoreModule : public DefaultModuleImpl
	{
	public:
		virtual void startupModule()
		{
			{
				SHA1 hashState;
				const TCHAR* GlobalShaderString = TEXT("GlobalShaderMap");
				hashState.updateWithString(GlobalShaderString, CString::strlen(GlobalShaderString));
				hashState.finalize();
				hashState.getHash(&GGlobalShaderMapHash.mHash[0]);
			}
		}
	};

	IMPLEMENT_MODULE(ShaderCoreModule, ShaderCore);

	const TCHAR* skipToCharOnCurrentLine(const TCHAR* inStr, TCHAR targetChar)
	{
		const TCHAR* str = inStr;
		if (str)
		{
			while (*str && *str != targetChar && *str != TEXT('\n'))
			{
				++str;
			}
			if (*str != targetChar)
			{
				str = nullptr;
			}
		}
		return str;
	}

	wstring getRelativeShaderFilename(const wstring& inFilename)
	{
		wstring shaderDir = PlatformProcess::shaderDir();
		boost::replace_all(shaderDir, TEXT("\\"), TEXT("/"));
		int32 charIndex = shaderDir.find(TEXT("/"));
		if (charIndex != INDEX_NONE)
		{
			shaderDir = shaderDir.substr(charIndex + 1);
		}
		wstring relativeFilename = boost::replace_all_copy(inFilename, TEXT("\\"), TEXT("/"));
		relativeFilename = IFileManager::get().convertToRelativePath(relativeFilename.c_str());
		charIndex = relativeFilename.find(shaderDir);
		if (charIndex != INDEX_NONE)
		{
			charIndex += shaderDir.length();
			if (relativeFilename[charIndex] == TEXT('/'))
			{
				charIndex++;
			}
			if (boost::contains(relativeFilename, TEXT("WorkingDirectory")))
			{
				const int32 numDirsToSkip = 3;
				int32 numDirsSkipped = 0;
				int32 newCharIndex = charIndex;
				do
				{
					newCharIndex = relativeFilename.find(TEXT('/'), charIndex);
					charIndex = (newCharIndex == INDEX_NONE) ? charIndex : newCharIndex + 1;
				} while (newCharIndex != INDEX_NONE && ++numDirsSkipped < numDirsToSkip);
			}
			relativeFilename = relativeFilename.substr(charIndex);
		}
		return relativeFilename;
	}

	bool loadShaderSourceFile(const TCHAR* filename, wstring& outfileContents)
	{
		if (PlatformProperties::requiresCookedData())
		{
			return false;
		}

		bool bResult = false;

		{
			wstring shaderFilename = Paths::combine(PlatformProcess::baseDir(), PlatformProcess::shaderDir(), filename);

			if (Paths::getExtension(shaderFilename) == TEXT(""))
			{
				shaderFilename += TEXT(".hlsl");
			}

			ScopeLock scopeLock(&FileCacheCriticalSection);
			auto it = GShaderFileCache.find(shaderFilename);
			if (it != GShaderFileCache.end())
			{
				wstring* cachedFile = &it->second;
				outfileContents = cachedFile->c_str();
				bResult = true;
			}
			else
			{
				if (FileHelper::loadFileToString(outfileContents, shaderFilename.c_str(), FileHelper::EHashOptions::EnableVerify | FileHelper::EHashOptions::ErrorMissingHash))
				{
					GShaderFileCache.emplace(shaderFilename, outfileContents);
					bResult = true;
				}
			}
		}
		return bResult;
	}


	void loadShaderSourceFileChecked(const TCHAR* filename, wstring& outFileContents)
	{
		if (!loadShaderSourceFile(filename, outFileContents))
		{

		}
	}

	void getShaderIncludes(const TCHAR* filename, TArray<wstring>& includeFilenames, uint32 depthLimit)
	{
		wstring fileContents;
		loadShaderSourceFileChecked(filename, fileContents);
		if (fileContents.length() > 0)
		{
			const TCHAR* includeBegin = CString::strstr(fileContents.c_str(), TEXT("#include"));
			uint32 searchCount = 0;
			const uint32 maxSearchCount = 20;
			while (includeBegin != nullptr && searchCount < maxSearchCount && depthLimit > 0)
			{
				const TCHAR* includeFilenameBegin = skipToCharOnCurrentLine(includeBegin, TEXT('\"'));
				if (includeFilenameBegin)
				{
					const TCHAR* includeFilenameEnd = skipToCharOnCurrentLine(includeFilenameBegin + 1, TEXT('\"'));
					if (includeFilenameEnd)
					{
						wstring extractedIncludeFilename(includeFilenameBegin + 1, includeFilenameEnd - includeFilenameBegin - 1);
						if (extractedIncludeFilename == TEXT("Material.hlsl"))
						{
							extractedIncludeFilename = TEXT("MaterialTemplate.hlsl");
						}
						bool bIgnoreInclude = extractedIncludeFilename == TEXT("VertexFactory.hlsl") || extractedIncludeFilename == TEXT("GeneratedConstantBuffers.hlsl") || extractedIncludeFilename == TEXT("GeneratedInstancedStereo.hlsl") || boost::starts_with(extractedIncludeFilename, TEXT("ConstantBuffers/"));

						const bool bIsOptionalInclude = false;
						if (bIsOptionalInclude)
						{
							wstring shaderFilename = Paths::combine(PlatformProcess::baseDir(), PlatformProcess::shaderDir(), extractedIncludeFilename.c_str());
							if (!Paths::fileExists(shaderFilename))
							{
								bIgnoreInclude = true;
							}
						}
						if (!bIgnoreInclude)
						{
							getShaderIncludes(extractedIncludeFilename.c_str(), includeFilenames, depthLimit - 1);
							extractedIncludeFilename = Paths::getBaseFilename(extractedIncludeFilename, false);
							includeFilenames.addUnique(extractedIncludeFilename);
						}
					}
				}
				includeBegin = skipToCharOnCurrentLine(includeBegin, TEXT('\n'));
				if (includeBegin && *includeBegin != 0)
				{
					includeBegin = CString::strstr(includeBegin + 1, TEXT("#include"));
				}
				searchCount++;
			}
		}
	}

	static void addShaderSourceFileEntry(TArray<wstring>& shaderSourceFiles, const wstring& shaderFileName)
	{
		wstring shaderFilenameBase(Paths::getBaseFilename(shaderFileName));
		if (!shaderSourceFiles.contains(shaderFilenameBase))
		{
			shaderSourceFiles.push_back(shaderFilenameBase);
			TArray<wstring> shaderIncludes;
			getShaderIncludes(shaderFilenameBase.c_str(), shaderIncludes);
			for (int32 includeIdx = 0; includeIdx < shaderIncludes.size(); ++includeIdx)
			{
				shaderSourceFiles.addUnique(shaderIncludes[includeIdx]);
			}
		}
	}




	static void getAllShaderSourceFile(TArray<wstring>& shaderSourceFile)
	{
		for (TLinkedList<VertexFactoryType*>::TIterator factoryIt(VertexFactoryType::getTypeList()); factoryIt; factoryIt.next())
		{
			VertexFactoryType* vertexFactoryType = *factoryIt;
			if (vertexFactoryType)
			{
				wstring shaderFilename(vertexFactoryType->getShaderFilename());
				addShaderSourceFileEntry(shaderSourceFile, shaderFilename);
			}
		}
		for (TLinkedList<ShaderType*>::TIterator shaderIt(ShaderType::getTypeList()); shaderIt; shaderIt.next())
		{
			ShaderType* shaderType = *shaderIt;
			if (shaderType)
			{
				wstring shaderFilename(shaderType->getShaderFilename());
				addShaderSourceFileEntry(shaderSourceFile, shaderFilename);
			}
		}
		addShaderSourceFileEntry(shaderSourceFile, TEXT("MaterialTemplate"));
		addShaderSourceFileEntry(shaderSourceFile, TEXT("Common"));
		addShaderSourceFileEntry(shaderSourceFile, TEXT("Definitions"));
	}

	void verifyShaderSourceFiles()
	{
		if (!PlatformProperties::requiresCookedData())
		{
			TArray<wstring> shaderSourceFiles;
			getAllShaderSourceFile(shaderSourceFiles);
			ScopedSlowTask slowTask(shaderSourceFiles.size());
			for (int32 shaderFileIdx = 0; shaderFileIdx < shaderSourceFiles.size(); shaderFileIdx++)
			{
				slowTask.enterProgressFrame(1);
				wstring fileContents;
				loadShaderSourceFileChecked(shaderSourceFiles[shaderFileIdx].c_str(), fileContents);
			}
		}
	}
	void buildShaderFileToConstantBufferMap(TMap<wstring, TArray<const TCHAR*>>& shaderFileToConstantBufferVariables)
	{
		if (!PlatformProperties::requiresCookedData())
		{
			TArray<wstring> shaderSourceFiles;
			getAllShaderSourceFile(shaderSourceFiles);
			struct ShaderVariable
			{
				ShaderVariable(const TCHAR* shaderVariable)
					:mOriginalShaderVariable(shaderVariable),
					mSearchKey(boost::to_upper_copy(wstring(shaderVariable) + TEXT(".")))
				{

				}
				const TCHAR* mOriginalShaderVariable;
				wstring mSearchKey;
			};
			std::vector<ShaderVariable> searchKeys;
			for (TLinkedList<ShaderParametersMetadata*>::TIterator structIt(ShaderParametersMetadata::getStructList()); structIt; structIt.next())
			{
				searchKeys.push_back(ShaderVariable(structIt->getShaderVariableName()));
			}

			for (int32 fileIndex = 0; fileIndex < shaderSourceFiles.size(); fileIndex++)
			{
				wstring shaderFileContents;
				loadShaderSourceFileChecked(shaderSourceFiles[fileIndex].c_str(), shaderFileContents);
				boost::to_upper(shaderFileContents);
				TArray<const TCHAR*>& referencedConstantBuffers = shaderFileToConstantBufferVariables.findOrAdd(shaderSourceFiles[fileIndex]);
				for (int32 searchKeyIndex = 0; searchKeyIndex < searchKeys.size(); ++searchKeyIndex)
				{
					if (boost::contains(shaderFileContents, searchKeys[searchKeyIndex].mSearchKey))
					{
						referencedConstantBuffers.addUnique(searchKeys[searchKeyIndex].mOriginalShaderVariable);
					}
				}
			}
		}
	}

	void flushShaderFileCache()
	{
		GShaderHashCache.clear();
		GShaderFileCache.clear();
		if (!PlatformProperties::requiresCookedData())
		{
			TMap<wstring, TArray<const TCHAR*>> shaderFileToConstantBufferVariables;
			buildShaderFileToConstantBufferMap(shaderFileToConstantBufferVariables);
			for (TLinkedList<ShaderPipelineType*>::TConstIterator it(ShaderPipelineType::getTypeList()); it; it.next())
			{
				const auto& stages = it->getStages();
				for (const ShaderType* shaderType : stages)
				{
					((ShaderType*)shaderType)->flushShaderFileCache(shaderFileToConstantBufferVariables);
				}
			}
			for (TLinkedList<ShaderType*>::TIterator it(ShaderType::getTypeList()); it; it.next())
			{
				it->flushShaderFileCache(shaderFileToConstantBufferVariables);
			}

			for (TLinkedList<VertexFactoryType*>::TIterator it(VertexFactoryType::getTypeList()); it; it.next())
			{
				it->flushShaderFileCache(shaderFileToConstantBufferVariables);
			}

		}
	}

	bool ShaderParameterMap::findParameterAllocation(const TCHAR* inParameterName, uint16& outBufferIndex, uint16& outBaseIndex, uint16& outSize) const
	{
		auto allocationIt = mParameterMap.find(inParameterName);
		if (allocationIt != mParameterMap.end())
		{
			const ParameterAllocation& allocation = allocationIt->second;
			outBufferIndex = allocation.mBufferIndex;
			outBaseIndex = allocation.mBaseIndex;
			outSize = allocation.mSize;

			if (allocation.bBound)
			{

			}
			allocation.bBound = true;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool ShaderParameterMap::containsParameterAllocation(const TCHAR* inParameterName) const
	{
		return mParameterMap.find(inParameterName) != mParameterMap.end();
	}

	void ShaderParameterMap::addParameterAllocation(const TCHAR* inParameterName, uint16 inBufferIndex, uint16 BaseIndex, uint16 size)
	{
		ParameterAllocation allocation;
		allocation.mBufferIndex = inBufferIndex;
		allocation.mBaseIndex = BaseIndex;
		allocation.mSize = size;
		mParameterMap.emplace(inParameterName, allocation);
	}

	void ShaderParameterMap::removeParameterAllocation(const TCHAR* parameterName)
	{
		mParameterMap.erase(parameterName);
	}

	void ShaderParameterMap::verifyBindingAreComplete(const TCHAR* shaderTypeName, ShaderTarget target, class VertexFactoryType* inVertexFactoryType) const
	{

	}

	void ShaderParameterMap::updateHash(SHA1& hashState) const
	{
		for (auto& it : mParameterMap)
		{
			const wstring& paramName = it.first;
			const ParameterAllocation& paramValue = it.second;
			hashState.update((const uint8*)paramName.c_str(), paramName.length() * sizeof(TCHAR));
			hashState.update((const uint8*)& paramValue.mBufferIndex, sizeof(paramValue.mBufferIndex));
			hashState.update((const uint8*)& paramValue.mBaseIndex, sizeof(paramValue.mBaseIndex));
			hashState.update((const uint8*)& paramValue.mSize, sizeof(paramValue.mSize));
		}
	}



	void ShaderCompilerOutput::generateOutputHash()
	{
		SHA1 hashState;
		const TArray<uint8>& code = mShaderCode.getReadAccess();
		uint32 shaderCodeSize = mShaderCode.getShaderCodeSize();
		hashState.update(code.data(), shaderCodeSize * code.getTypeSize());
		hashState.finalize();
		hashState.getHash(&mOutputHash.mHash[0]);
	}
	static void updateSingleShaderFileHash(SHA1& inOutHashState, const TCHAR* filename)
	{
		TArray<wstring> includeFilenames;
		getShaderIncludes(filename, includeFilenames);
		for (auto& includeFile : includeFilenames)
		{
			wstring includeFileContents;
			loadShaderSourceFileChecked(includeFile.c_str(), includeFileContents);
			inOutHashState.updateWithString(includeFileContents.c_str(), includeFileContents.length());
		}
		wstring fileContents;
		loadShaderSourceFileChecked(filename, fileContents);
		inOutHashState.updateWithString(fileContents.c_str(), fileContents.length());
	}


	const SHAHash& getShaderFileHash(const TCHAR* filename, EShaderPlatform shaderPlatform)
	{
		auto it = GShaderHashCache.find(filename);
		if (it != GShaderHashCache.end())
		{
			return it->second;
		}

		SHA1 hashState;
		updateSingleShaderFileHash(hashState, filename);
		hashState.finalize();
		SHAHash& newHash = GShaderHashCache.emplace(filename, SHAHash()).first->second;
		hashState.getHash(&newHash.mHash[0]);
		return newHash;
	}

	const SHAHash& getShaderFilesHash(const TArray<wstring>& filenames)
	{
		wstring key;
		for (const wstring& filename : filenames)
		{
			key += filename;
		}
		auto it = GShaderHashCache.find(key);
		if (it != GShaderHashCache.end())
		{
			return it->second;
		}
		SHA1 hashState;
		for (const wstring& filename : filenames)
		{
			updateSingleShaderFileHash(hashState, filename.c_str());
		}
		hashState.finalize();

		SHAHash& newHash = GShaderHashCache.emplace(key, SHAHash()).first->second;
		hashState.getHash(&newHash.mHash[0]);
		return newHash;
	}

	void generateReferencedConstantBuffers(const TCHAR* sourceFileName, const TCHAR* shaderTypeName, const TMap<wstring, TArray<const TCHAR*>>& shaderFileToConstantBufferVariables, TMap<const TCHAR*, CachedConstantBufferDeclaration>& constantBufferEntries)
	{
		TArray<wstring> filesToSearch;
		getShaderIncludes(sourceFileName, filesToSearch);
		filesToSearch.push_back(sourceFileName);
		for (int32 fileIndex = 0; fileIndex < filesToSearch.size(); fileIndex++)
		{
			const TArray<const TCHAR*>& foundConstantBufferVariables = shaderFileToConstantBufferVariables.findChecked(filesToSearch[fileIndex]);
			for (int32 variableIndex = 0; variableIndex < foundConstantBufferVariables.size(); variableIndex++)
			{
				constantBufferEntries.emplace(foundConstantBufferVariables[variableIndex], CachedConstantBufferDeclaration());
			}
		}
	}



	void ShaderCompilerEnvironment::merge(const ShaderCompilerEnvironment& other)
	{
		for (auto& it : other.mIncludeVirtualPathToContentsMap)
		{
			auto existing = mIncludeVirtualPathToContentsMap.find(it.first);
			if (existing != mIncludeVirtualPathToContentsMap.end())
			{
				wstring& existingContents = existing->second;


				existingContents.append(it.second);
			}
			else
			{
				mIncludeVirtualPathToContentsMap.emplace(it.first, it.second);
			}
		}
		mCompilerFlags.append(other.mCompilerFlags);
		mResourceTableMap.append(other.mResourceTableMap);
		mResourceTableLayoutHashes.append(other.mResourceTableLayoutHashes);
		mDefinitions.merge(other.mDefinitions);
	}

	void serializeConstantBufferInfo(ShaderSaveArchive& ar, const TMap<const TCHAR*, CachedConstantBufferDeclaration>& constantBufferEntries)
	{
		for (auto& it : constantBufferEntries)
		{
			for (TLinkedList<ShaderParametersMetadata*>::TIterator structIt(ShaderParametersMetadata::getStructList()); structIt; structIt.next())
			{
				if (it.first == structIt->getShaderVariableName())
				{
					const ShaderParametersMetadata& Struct = **structIt;
					const TArray<ShaderParametersMetadata::Member>& members = Struct.getMembers();
					int32 numMembers = members.size();
					ar.serialize(NULL, numMembers);
					for (int32 memberIndex = 0; memberIndex < members.size(); memberIndex++)
					{
						const ShaderParametersMetadata::Member& member = members[memberIndex];
						int32 memberSize = member.getNumColumns() * member.getNumRows();
						ar.serialize(NULL, memberSize);
						int32 memberType = (int32)member.getBaseType();
						ar.serialize(NULL, memberType);
					}
					break;
				}
			}
		}
	}



	void initializeShaderType()
	{
		TMap<wstring, TArray<const TCHAR*>> shaderFileToConstantBufferVariables;
		buildShaderFileToConstantBufferMap(shaderFileToConstantBufferVariables);
		ShaderType::initialize(shaderFileToConstantBufferVariables);
		VertexFactoryType::initialize(shaderFileToConstantBufferVariables);
		ShaderPipelineType::initialize();
	}
}