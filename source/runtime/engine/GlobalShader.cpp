#include "HAL/PlatformProperties.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Misc/CoreMisc.h"
#include "Serialization/MemoryReader.h"
#include "GlobalShader.h"
#include "Serialization/MemoryWriter.h"
#include "Interface/ITargetPlatformManagerModule.h"
#include "ShaderCompiler.h"
#include "boost/lexical_cast.hpp"
#include "DerivedDataCacheInterface.h"

namespace Air
{

	const int32 mGlobalShaderMapId = 0;

	TShaderMap<GlobalShaderType>* GGlobalShaderMap[SP_NumPlatforms];

	static wstring getGlobalShaderCacheFilename(EShaderPlatform platform)
	{
		return TEXT("GlobalShaderCache-") + legacyShaderPlatformToShaderFormat(platform) + TEXT(".bin");
	}

	static void serializeGlobalShaders(Archive& ar, TShaderMap<GlobalShaderType>* globalShaderMap)
	{
		BOOST_ASSERT(isInGameThread());
		static const uint32 referenceTag = 0x47534D42;
		if (ar.isLoading())
		{
			uint32 tag = 0;
			ar << tag;
		}
		else
		{
			uint32 tag = referenceTag;
			ar << tag;
		}
		globalShaderMap->serializeInline(ar, true, false);
		globalShaderMap->registerSerializedShaders();
	}

	static inline Shader* processCompiledJob(ShaderCompileJob* singleJob, const ShaderPipelineType* pipeline, TArray<EShaderPlatform>& shaderPlatformsProcessed, TArray<const ShaderPipelineType*>& outSharedPipelines)
	{
		GlobalShaderType* globalShaderType = singleJob->mShaderType->getGlobalShaderType();
		Shader* shader = globalShaderType->finishCompileShader(*singleJob, pipeline);
		if (shader)
		{
			EShaderPlatform platform = (EShaderPlatform)singleJob->mInput.mTarget.mPlatform;
			if (!pipeline || !pipeline->shoudlOptimizeUnusedOutputs())
			{
				GGlobalShaderMap[platform]->addShader(globalShaderType, shader);
				if (!platform)
				{
					auto jobSharedPipelines = singleJob->mSharingPipelines.find(nullptr);
					if (jobSharedPipelines != singleJob->mSharingPipelines.end())
					{
						for (auto* sharedPipeline : jobSharedPipelines->second)
						{
							outSharedPipelines.addUnique(sharedPipeline);
						}
					}
				}
			}
			shaderPlatformsProcessed.addUnique(platform);
		}
		else
		{

		}
		return shader;
	}

	wstring getGlobalShaderMapKeyString(const GlobalShaderMapId& shaderMapId, EShaderPlatform platform)
	{
		wstring format = legacyShaderPlatformToShaderFormat(platform);
		wstring shaderMapKeyString = format + TEXT("_") + boost::lexical_cast<wstring>(getTargetPlatformManagerRef().shaderFormatVersion(format)) + TEXT("_");
		shaderMapAppendKeyString(platform, shaderMapKeyString);
		shaderMapId.appendKeyString(shaderMapKeyString);
		return shaderMapKeyString;
	}


	void saveGlobalShaderMapToDerivedDataCache(EShaderPlatform platform)
	{
		TArray<uint8> saveData;
		MemoryWriter ar(saveData, true);
		serializeGlobalShaders(ar, GGlobalShaderMap[platform]);
		GlobalShaderMapId shaderMapId(platform);

		getDerivedDataCacheRef().put(getGlobalShaderMapKeyString(shaderMapId, platform).c_str(), saveData);
	}

	void processCompiledGlbalShaders(const TArray<ShaderCommonCompileJob*>& compilationResults)
	{
		TArray<EShaderPlatform> shaderPlatformsProcessed;
		TArray<const ShaderPipelineType*> sharedPipelines;

		for (int32 resultIndex = 0; resultIndex < compilationResults.size(); resultIndex++)
		{
			const ShaderCommonCompileJob& currentJob = *compilationResults[resultIndex];
			ShaderCompileJob* singleJob = nullptr;
			if ((singleJob = (ShaderCompileJob*)currentJob.getSingleShaderJob()) != nullptr)
			{
				processCompiledJob(singleJob, nullptr, shaderPlatformsProcessed, sharedPipelines);
			}
			else
			{
				const auto* pipelineJob = currentJob.getShaderPipelineJob();
				BOOST_ASSERT(pipelineJob);
				TArray<Shader*> shaderStages;
				for (int32 index = 0; index < pipelineJob->mStageJobs.size(); index++)
				{
					singleJob = pipelineJob->mStageJobs[index]->getSingleShaderJob();
					Shader* shader = processCompiledJob(singleJob, pipelineJob->mShaderPipeline, shaderPlatformsProcessed, sharedPipelines);
					shaderStages.add(shader);
				}
				ShaderPipeline* shaderPipeline = new ShaderPipeline(pipelineJob->mShaderPipeline, shaderStages);
				if (shaderPipeline)
				{
					EShaderPlatform platform = (EShaderPlatform)pipelineJob->mStageJobs[0]->getSingleShaderJob()->mInput.mTarget.mPlatform;
					BOOST_ASSERT(shaderPipeline && !GGlobalShaderMap[platform]->hasShaderPipeline(shaderPipeline->mPipelineType));
					GGlobalShaderMap[platform]->addShaderPipeline(pipelineJob->mShaderPipeline, shaderPipeline);
				}
			}
		}
		for (int32 platformIndex = 0; platformIndex < shaderPlatformsProcessed.size(); platformIndex++)
		{
			{
				EShaderPlatform platform = shaderPlatformsProcessed[platformIndex];
				auto* globalShaderMap = GGlobalShaderMap[platform];
				for (const ShaderPipelineType* shaderPipelineType : sharedPipelines)
				{
					BOOST_ASSERT(shaderPipelineType->isGlobalTypePipeline());
					if (!globalShaderMap->hasShaderPipeline(shaderPipelineType))
					{
						auto& stageTypes = shaderPipelineType->getStages();
						TArray<Shader*> shaderStages;
						for (int32 index = 0; index < stageTypes.size(); ++index)
						{
							GlobalShaderType* globalShaderType = ((ShaderType*)(stageTypes[index]))->getGlobalShaderType();
							if (globalShaderType->shouldCache(platform))
							{
								Shader* shader = globalShaderMap->getShader(globalShaderType);
								BOOST_ASSERT(shader);
								shaderStages.add(shader);
							}
							else
							{
								break;
							}
						}
						BOOST_ASSERT(stageTypes.size() == shaderStages.size());
						ShaderPipeline* shaderPipeline = new ShaderPipeline(shaderPipelineType, shaderStages);
						globalShaderMap->addShaderPipeline(shaderPipelineType, shaderPipeline);
					}
				}
			}
			//saveGlobalShaderMapToDerivedDataCache(shaderPlatformsProcessed[platformIndex]);
		}
	}

	

	

	TShaderMap<GlobalShaderType>* getGlobalShaderMap(EShaderPlatform platform, bool bRefreshShaderMap /* = false */)
	{
		if (bRefreshShaderMap)
		{
			delete GGlobalShaderMap[platform];
			GGlobalShaderMap[platform] = nullptr;
			flushShaderFileCache();
		}

		if (!GGlobalShaderMap[platform])
		{
			BOOST_ASSERT(isInGameThread());
			ScopedSlowTask slowTask(70);
			slowTask.enterProgressFrame(20);
			verifyShaderSourceFiles();
			GGlobalShaderMap[platform] = new TShaderMap<GlobalShaderType>();
			bool bLoadedFromCacheFile = false;

			if (PlatformProperties::requiresCookedData())
			{
				slowTask.enterProgressFrame(50);
				TArray<uint8> globalShaderData;
				wstring globalShaderCacheFilename = Paths::getRelativePathToRoot() + getGlobalShaderCacheFilename(platform);
				Paths::makeStandardFilename(globalShaderCacheFilename);
				bLoadedFromCacheFile = FileHelper::loadFileToArray(globalShaderData, globalShaderCacheFilename.c_str(), FILEREAD_Silent);
				if (!bLoadedFromCacheFile)
				{

				}

				MemoryReader memoryReader(globalShaderData);
				serializeGlobalShaders(memoryReader, GGlobalShaderMap[platform]);
			}
			else
			{
				GlobalShaderMapId shaderMapId(platform);
				TArray<uint8> cachedData;
				slowTask.enterProgressFrame(40);
				const wstring dataKey = getGlobalShaderMapKeyString(shaderMapId, platform);
				slowTask.enterProgressFrame(10);

			}

			verifyGlobalShaders(platform, bLoadedFromCacheFile);
			extern int32 GCreateShadersOnLoad;
			if (GCreateShadersOnLoad && platform == GMaxRHIShaderPlatform)
			{
				for (auto & pair : GGlobalShaderMap[platform]->getShaders())
				{
					Shader* shader = pair.second;
					if (shader)
					{
						shader->beginInitializeResources();
					}
				}
			}
		}
		return GGlobalShaderMap[platform];
	}

	
	void verifyGlobalShaders(EShaderPlatform platform, bool bLoadedFromCacheFile)
	{
		BOOST_ASSERT(isInGameThread());
		BOOST_ASSERT(GGlobalShaderMap[platform]);
		TShaderMap<GlobalShaderType>* globalShaderMap = getGlobalShaderMap(platform);
		const bool bEmptyMap = globalShaderMap->isEmpty();
		if (bEmptyMap)
		{
			AIR_LOG(LogShaders, Warning, TEXT("Empty global shader map, recompiling all global shaders\n"));
		}


		bool bErrorOnMissing = bLoadedFromCacheFile;
		if (PlatformProperties::requiresCookedData())
		{
			bErrorOnMissing = true;
		}
		TArray<ShaderCommonCompileJob*> globalShaderJobs;
		TMap<ShaderType*, ShaderCompileJob*> sharedShaderJobs;

		for (TLinkedList<ShaderType*>::TIterator it(ShaderType::getTypeList()); it; it.next())
		{
			GlobalShaderType* globalShaderType = it->getGlobalShaderType();
			if (globalShaderType && globalShaderType->shouldCache(platform))
			{
				if (!globalShaderMap->hasShader(globalShaderType))
				{
					if (bErrorOnMissing)
					{
						AIR_LOG(LogShaders, Fatal, TEXT("Missing global shader %s, please make sure cooking was successful"));
					}
					if (!bEmptyMap)
					{
						AIR_LOG(LogShaders, Warning, TEXT(" %s"), globalShaderType->getName());
					}
					auto * job = globalShaderType->beginCompileShader(platform, nullptr, globalShaderJobs);
					BOOST_ASSERT(sharedShaderJobs.end() == sharedShaderJobs.find(globalShaderType));
					sharedShaderJobs.emplace(globalShaderType, job);
				}
			}
		}

		for (TLinkedList<ShaderPipelineType*>::TIterator shaderPipelineIt(ShaderPipelineType::getTypeList()); shaderPipelineIt; shaderPipelineIt.next())
		{
			const ShaderPipelineType* pipeline = *shaderPipelineIt;
			if (pipeline->isGlobalTypePipeline())
			{
				if (!globalShaderMap->getShaderPipeline(pipeline))
				{
					auto& stageTypes = pipeline->getStages();
					TArray<GlobalShaderType*> shaderStages;
					for (int32 index = 0; index < stageTypes.size(); ++index)
					{
						GlobalShaderType* globalShaderType = ((ShaderType*)(stageTypes[index]))->getGlobalShaderType();
						if (globalShaderType->shouldCache(platform))
						{
							shaderStages.add(globalShaderType);
						}
						else
						{
							break;
						}
					}
					if (shaderStages.size() == stageTypes.size())
					{
						if (bErrorOnMissing)
						{
							AIR_LOG(logShaders, Fatal, TEXT("Missing global shader pipeline %s", pipeline->getName()));
						}
						if (pipeline->shoudlOptimizeUnusedOutputs())
						{
							GlobalShaderType::beginCompileShaderPipeline(platform, pipeline, shaderStages, globalShaderJobs);
						}
						else
						{
							for (const ShaderType* shaderType : stageTypes)
							{
								auto jobIt = sharedShaderJobs.find(const_cast<ShaderType*>(shaderType));
								BOOST_ASSERT(jobIt != sharedShaderJobs.end());
								auto* singleJob = jobIt->second->getSingleShaderJob();
								BOOST_ASSERT(singleJob);
								auto& sharedPipelinesInJob = singleJob->mSharingPipelines.findOrAdd(nullptr);
								BOOST_ASSERT(!sharedPipelinesInJob.contains(pipeline));
								sharedPipelinesInJob.add(pipeline);
							}
						}
					}
				}
			}
		}

		if (globalShaderJobs.size() > 0)
		{
			GShaderCompilingManager->addJobs(globalShaderJobs, true, true, true);
			const bool bAllowAsynchronousGlobalShaderCompiling = !isOpenGLPlatform(GMaxRHIShaderPlatform) && !isVulkanPlatform(GMaxRHIShaderPlatform) && !isMetalPlatform(GMaxRHIShaderPlatform) && GShaderCompilingManager->allowAsynchronousShaderCompiling();

			//if (!bAllowAsynchronousGlobalShaderCompiling)
			{
				TArray<int32> shaderMapIds;
				shaderMapIds.add(mGlobalShaderMapId);
				GShaderCompilingManager->finishCompilation(TEXT("Global"), shaderMapIds);
			}
		}
	}

	GlobalShaderMapId::GlobalShaderMapId(EShaderPlatform platform)
	{
		TArray<ShaderType*> shaderTypes;
		TArray<const ShaderPipelineType*> shaderPipelineTypes;

		for (TLinkedList<ShaderType*>::TIterator shaderTypeIt(ShaderType::getTypeList()); shaderTypeIt; shaderTypeIt.next())
		{
			GlobalShaderType* globalShaderType = shaderTypeIt->getGlobalShaderType();
			if (globalShaderType && globalShaderType->shouldCache(platform))
			{
				shaderTypes.push_back(globalShaderType);
			}
		}
		for (TLinkedList<ShaderPipelineType*>::TIterator shaderPipelineIt(ShaderPipelineType::getTypeList()); shaderPipelineIt; shaderPipelineIt.next())
		{
			const ShaderPipelineType* pipeline = *shaderPipelineIt;
			if (pipeline->isGlobalTypePipeline())
			{
				int32 numStagesNeeded = 0;
				auto& stageTypes = pipeline->getStages();
				for (const ShaderType* shader : stageTypes)
				{
					const GlobalShaderType* globalShaderType = shader->getGlobalShaderType();
					if (globalShaderType->shouldCache(platform))
					{
						++numStagesNeeded;
					}
					else
					{
						break;
					}
				}
				if (numStagesNeeded == stageTypes.size())
				{
					shaderPipelineTypes.push_back(pipeline);
				}
			}
		}
		shaderTypes.sort(CompareShaderTypes());

		for (int32 index = 0; index < shaderTypes.size(); index++)
		{
			ShaderTypeDependency dependency;
			dependency.mShaderType = shaderTypes[index];
			dependency.mSourceHash = shaderTypes[index]->getSourceHash();
			mShaderTypeDependencies.push_back(dependency);
		}

		shaderPipelineTypes.sort(CompareShaderPipelineType());
		for (int32 index = 0; index < shaderPipelineTypes.size(); index++)
		{
			const ShaderPipelineType* pipeline = shaderPipelineTypes[index];
			ShaderPipelineTypeDependency dependency;
			dependency.mShaderPipelineType = pipeline;
			dependency.mStagesSourceHash = pipeline->getSourceHash();
			mShaderPipelineTypeDependencies.push_back(dependency);
		}
	}


	void GlobalShaderMapId::appendKeyString(wstring keyString) const
	{
		TMap<const TCHAR*, CachedConstantBufferDeclaration> referenceConstantBuffers;
		for (int32 shaderIndex = 0; shaderIndex < mShaderTypeDependencies.size(); shaderIndex++)
		{
			const ShaderTypeDependency& shaderTypeDependency = mShaderTypeDependencies[shaderIndex];
			keyString += TEXT("_");
			keyString += shaderTypeDependency.mShaderType->getName();
			keyString += shaderTypeDependency.mSourceHash.toString();

			shaderTypeDependency.mShaderType->getSerializationHistory().appendKeyString(keyString);

			const TMap<const TCHAR*, CachedConstantBufferDeclaration>& referencedConstantBufferStructsCache = shaderTypeDependency.mShaderType->getReferencedConstantBufferStructsCache();
			for (auto& it : referencedConstantBufferStructsCache)
			{
				referenceConstantBuffers.emplace(it.first, it.second);
			}
		}
		for (int32 index = 0; index < mShaderPipelineTypeDependencies.size(); ++index)
		{
			const ShaderPipelineTypeDependency& dependency = mShaderPipelineTypeDependencies[index];
			keyString += TEXT("_");
			keyString += dependency.mShaderPipelineType->getName();
			keyString += dependency.mStagesSourceHash.toString();
			for (auto& shaderType : dependency.mShaderPipelineType->getStages())
			{
				const TMap<const TCHAR*, CachedConstantBufferDeclaration>& referencedConstantBufferStructsCache = shaderType->getReferencedConstantBufferStructsCache();
				for (auto& it : referencedConstantBufferStructsCache)
				{
					referenceConstantBuffers.emplace(it.first, it.second);
				}
			}
		}
		{
			TArray<uint8> tempData;
			SerializationHistory serializationHistory;
			MemoryWriter ar(tempData, true);
			ShaderSaveArchive saveArchive(ar, serializationHistory);
			serializeConstantBufferInfo(saveArchive, referenceConstantBuffers);
			serializationHistory.appendKeyString(keyString);
		}
	}

	void GlobalShaderType::beginCompileShaderPipeline(EShaderPlatform platform, const ShaderPipelineType* shaderPipeline, const TArray<GlobalShaderType *>& shaderStages, TArray<ShaderCommonCompileJob *>& newJobs)
	{
		BOOST_ASSERT(shaderStages.size() > 0);
		BOOST_ASSERT(shaderPipeline);
		AIR_LOG(LogShaders, Verbose, TEXT(" Pipeline: %s"), shaderPipeline->getName());

		auto* newPipelineJob = new ShaderPipelineCompileJob(mGlobalShaderMapId, shaderPipeline, shaderStages.size());
		for (int32 index = 0; index < shaderStages.size(); ++index)
		{
			auto* shaderStage = shaderStages[index];
			shaderStage->beginCompileShader(platform, shaderPipeline, newPipelineJob->mStageJobs);
		}
		newJobs.add(newPipelineJob);
	}

	ShaderCompileJob* GlobalShaderType::beginCompileShader(EShaderPlatform platform, const ShaderPipelineType* shaderPipeline, TArray<ShaderCommonCompileJob*>& newJobs)
	{
		ShaderCompileJob* newJob = new ShaderCompileJob(mGlobalShaderMapId, nullptr, this);
		ShaderCompilerEnvironment& shaderEnvironment = newJob->mInput.mEnvironment;
		setCompileEnvironment(platform, shaderEnvironment);
		static wstring globalName(TEXT("Global"));
		globalBeginCompileShader(globalName, nullptr, this, shaderPipeline, getShaderFilename(), getFunctionName(), ShaderTarget(getFrequency(), platform),
			newJob,
			newJobs);
		return newJob;
	}

	Shader* GlobalShaderType::finishCompileShader(const ShaderCompileJob& compileJob, const ShaderPipelineType* shaderPipelineType)
	{
		Shader* shader = nullptr;
		if (compileJob.bSucceeded)
		{
			ShaderType* specificType = compileJob.mShaderType->limitShaderResourceToThisType() ? compileJob.mShaderType : nullptr;
			ShaderResource* resource = ShaderResource::findOrCreateShaderResource(compileJob.mOutput, specificType);
			BOOST_ASSERT(resource);
			if (shaderPipelineType && !shaderPipelineType->shoudlOptimizeUnusedOutputs())
			{
				shaderPipelineType = nullptr;
			}
			shader = compileJob.mShaderType->findShaderById(ShaderId(GGlobalShaderMapHash, shaderPipelineType, nullptr, compileJob.mShaderType, compileJob.mInput.mTarget));
			if (!shader)
			{
				shader = (*mConstructCompiledRef)(CompiledShaderInitializerType(this, compileJob.mOutput, resource, GGlobalShaderMapHash, shaderPipelineType, nullptr));
				compileJob.mOutput.mParameterMap.verifyBindingAreComplete(getName(), compileJob.mOutput.mTarget, compileJob.mVFType);

			}
		}
		return shader;
	}

	GlobalShader::GlobalShader(const ShaderMetaType::CompiledShaderInitializerType& initializer)
		:Shader(initializer)
	{

	}
}