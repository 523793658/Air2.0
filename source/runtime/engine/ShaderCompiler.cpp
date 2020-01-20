#include "RHI.h"
#include "VertexFactory.h"
#include "ShaderCompiler.h"
#include "ShaderCore.h"
#include "Containers/StringConv.h"
#include "HAL/PlatformTime.h"
#include "HAL/PlatformProperties.h"
#include "Misc/ScopedSlowTask.h"
#include "HAL/FileManager.h"
#include "Misc/Guid.h"
#include "SceneInterface.h"
#include "Interface/IShaderFormat.h"
#include "HAL/PlatformProcess.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Modules/ModuleManager.h"
#include "Interface/IShaderFormatModule.h"
#include "HAL/PlatformFileManager.h"
#include "Classes/Materials/MaterialShader.h"
#include "EngineModule.h"
#include "Misc/Paths.h"
#include "RendererInterface.h"
#include "boost/algorithm/string.hpp"
namespace Air
{

	ShaderCompilingManager* GShaderCompilingManager = nullptr;

	const int32 ShaderCompileWorkerInputVersion = 1;

	const int32 ShaderCompileWorkerOutputVersion = 1;

	const int32 ShaderCompileWorkerSingleJobHeader = 'S';

	const int32 ShaderCompileWorkerPipelineJobHeader = 'P';

	static void generateShaderParametersMetadataMemeber(wstring& result, const ShaderParametersMetadata::Member& member)
	{
		wstring baseTypeName;
		switch (member.getBaseType())
		{
		case CBMT_BOOL: baseTypeName = TEXT("bool"); break;
		case CBMT_INT32: baseTypeName = TEXT("int"); break;
		case CBMT_UINT32: baseTypeName = TEXT("uint"); break;
		case CBMT_FLOAT32: 
			if (member.getPrecision() == EShaderPrecisionModifier::Float)
			{
				baseTypeName = TEXT("float");
			}
			else if (member.getPrecision() == EShaderPrecisionModifier::Half)
			{
				baseTypeName = TEXT("half");
			}
			else if (member.getPrecision() == EShaderPrecisionModifier::Fixed)
			{
				baseTypeName = TEXT("fixed");
			}
			break;
		default:
			break;
		}

		wstring TypeDim;
		if (member.getNumRows() > 1)
		{
			TypeDim = printf(TEXT("%ux%u"), member.getNumRows(), member.getNumColumns());
		}
		else
		{
			TypeDim = printf(TEXT("%u"), member.getNumColumns());
		}
		wstring arrayDim;
		if (member.getNumElements() > 0)
		{
			arrayDim = printf(TEXT("[%u]"), member.getNumElements());
		}

		result = printf(L"%s%s %s%s", baseTypeName.c_str(), TypeDim.c_str(), member.getName(), arrayDim.c_str());
	}

	static void generateInstancedStereoCode(wstring& result)
	{
		const ShaderParametersMetadata* InstanceView = nullptr;
		for (TLinkedList<ShaderParametersMetadata*>::TIterator structIt(ShaderParametersMetadata::getStructList()); structIt; structIt.next())
		{
			if (CString::strcmp(structIt->getShaderVariableName(), TEXT("InstancedView")) == 0)
			{
				InstanceView = *structIt;
				break;
			}
		}
		BOOST_ASSERT(InstanceView != nullptr);
		const TArray<ShaderParametersMetadata::Member>& structMembers = InstanceView->getMembers();
		result = L"struct ViewState\r\n";
		result += L"{\r\n";
		for (int32 memberIndex = 0; memberIndex < structMembers.size(); memberIndex++)
		{
			const ShaderParametersMetadata::Member& member = structMembers[memberIndex];
			wstring memberDecl;
			generateShaderParametersMetadataMemeber(memberDecl, member);
			result += printf(L"\t%s;\r\n", memberDecl.c_str());
		}
		result += L"};\r\n";
		result += L"ViewState getPrimaryView()\r\n";
		result += L"{\r\n";
		result += L"\tViewState Result;\r\n";
		for (int32 memberIndex = 0; memberIndex < structMembers.size(); ++memberIndex)
		{
			const ShaderParametersMetadata::Member & member = structMembers[memberIndex];
			result += printf(L"\tResult.%s = View.%s;\r\n", member.getName(), member.getName());
		}
		result += L"\treturn Result;\r\n";
		result += L"}\r\n";

		result += L"ViewState getInstancedView()\r\n";
		result += L"{\r\n";
		result += L"\tViewState Result;\r\n";
		for (int32 memberIndex = 0; memberIndex < structMembers.size(); ++memberIndex)
		{
			const ShaderParametersMetadata::Member& member = structMembers[memberIndex];
			result += printf(L"\tResult.%s = InstancedView.%s;\r\n", member.getName(), member.getName());
		}

		result += L"\treturn Result;\r\n";
		result += L"}\r\n";
	}

	static inline Shader* processCompiledJob(ShaderCompileJob* singleJob, const ShaderPipelineType* pipeline, TArray<EShaderPlatform>& shaderPlatformsProcessed, TArray<const ShaderPipelineType*>& outSharedPipelines);

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
							if (globalShaderType->shouldCompilePermutation(platform, kUniquePermutationId))
							{
								Shader* shader = globalShaderMap->getShader(globalShaderType, kUniquePermutationId);
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
			if (!globalShaderType)
			{
				continue;
			}
			int32 permutationCountToCompile = 0;
			for(int32 permutationId = 0; permutationId < globalShaderType->getPermutationCount(); permutationId++)
			{
				if (globalShaderType->shouldCompilePermutation(platform, permutationId) && !globalShaderMap->hasShader(globalShaderType, permutationId))
				{
					if (bErrorOnMissing)
					{
						AIR_LOG(LogShaders, Fatal, TEXT("Missing global shader %s, please make sure cooking was successful"));
					}
					if (!bEmptyMap)
					{
						AIR_LOG(LogShaders, Warning, TEXT(" %s"), globalShaderType->getName());
					}
					auto* job = GlobalShaderTypeCompiler::beginCompileShader(globalShaderType, permutationId, platform, nullptr, globalShaderJobs);
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
						if (globalShaderType->shouldCompilePermutation(platform, kUniquePermutationId))
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
						if (pipeline->shoudlOptimizeUnusedOutputs(platform))
						{
							GlobalShaderTypeCompiler::beginCompileShaderPipeline(platform, pipeline, shaderStages, globalShaderJobs);
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

	static inline Shader* processCompiledJob(ShaderCompileJob* singleJob, const ShaderPipelineType* pipeline, TArray<EShaderPlatform>& shaderPlatformsProcessed, TArray<const ShaderPipelineType*>& outSharedPipelines)
	{
		GlobalShaderType* globalShaderType = singleJob->mShaderType->getGlobalShaderType();
		Shader* shader = GlobalShaderTypeCompiler::finishCompileShader(globalShaderType, *singleJob, pipeline);
		if (shader)
		{
			EShaderPlatform platform = (EShaderPlatform)singleJob->mInput.mTarget.mPlatform;
			if (!pipeline || !pipeline->shoudlOptimizeUnusedOutputs(platform))
			{
				GGlobalShaderMap[platform]->addShader(globalShaderType, singleJob->mPermutationId, shader);
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

	void globalBeginCompileShader(const wstring & debugGroupName, class VertexFactoryType* inVFType, class ShaderType* inShaderType, const class ShaderPipelineType* inShaderPipelineType, const TCHAR* inSourceFilename, const TCHAR* inFunctionName, ShaderTarget inTarget, ShaderCompileJob* inNewJob, TArray<ShaderCommonCompileJob*>& inNewJobs, bool bAllowDevelopmentShaderCompile /* = true */)
	{
		ShaderCompilerInput& input = inNewJob->mInput;
		input.mTarget = inTarget;
		input.mShaderFormat = legacyShaderPlatformToShaderFormat(EShaderPlatform(inTarget.mPlatform));
		input.mSourceFilename = inSourceFilename;
		input.mEntryPointName = inFunctionName;
		input.bCompilingForShaderPipeline = false;
		input.bIncludeUsedOutputs = false;
		input.bGenerateDirectCompileFile = false;
		input.mDumpDebugInfoRootPath = GShaderCompilingManager->getAbsoluteShaderDebugInfoDirectory() + input.mShaderFormat;
		input.mDumpDebugInfoPath = TEXT("D:/Test/");
		input.mDebugGroupName = debugGroupName;
		if (inShaderPipelineType)
		{
			input.mDebugGroupName = input.mDebugGroupName + inShaderPipelineType->getName();
		}
		if (inVFType)
		{
			wstring vfName = inVFType->getName();
			input.mDebugGroupName = input.mDebugGroupName + vfName;
		}



		{
			wstring shaderTypeName = inShaderType->getName();
			input.mDebugGroupName += shaderTypeName;
		}
		{
			input.mEnvironment.setDefine(TEXT("PIXELSHADER"), inTarget.mFrequency == SF_Pixel);
			input.mEnvironment.setDefine(TEXT("DOMAINSHADER"), inTarget.mFrequency == SF_Domain);
			input.mEnvironment.setDefine(TEXT("HULLSHADER"), inTarget.mFrequency == SF_Hull);
			input.mEnvironment.setDefine(TEXT("GEOMETRYSHADER"), inTarget.mFrequency == SF_Geometry);
			input.mEnvironment.setDefine(TEXT("VERTEXSHADER"), inTarget.mFrequency == SF_Vertex);
			input.mEnvironment.setDefine(TEXT("COMPUTESHADER"), inTarget.mFrequency == SF_Compute);
		}

		input.mEnvironment.setDefine(TEXT("COMPILER_DEFINE"), TEXT("#define"));
		{

		}
		{
			static const auto Cvar = 0;
			input.mEnvironment.setDefine(TEXT("SELECTIVE_BASEPASS_OUTPUTS"), Cvar);
		}
		inShaderType->addReferencedConstantBufferIncludes(input.mEnvironment, input.mSourceFilePrefix, (EShaderPlatform)inTarget.mPlatform);
		if (inVFType)
		{
			inVFType->addReferencedConstantBufferIncludes(input.mEnvironment, input.mSourceFilePrefix, (EShaderPlatform)inTarget.mPlatform);
		}

		wstring generatedInstancedStereoCode;
		generateInstancedStereoCode(generatedInstancedStereoCode);
		input.mEnvironment.mIncludeVirtualPathToContentsMap.emplace(TEXT("GeneratedInstancedStereo.hlsl"), generatedInstancedStereoCode);
		{

		}
		inNewJobs.push_back(inNewJob);
	}

	void ShaderCompilingManager::finishCompilation(const TCHAR* materialName, const TArray<int32>& shaderMapIdsToFinishCompiling)
	{
		BOOST_ASSERT(!PlatformProperties::requiresCookedData());
		const double startTime = PlatformTime::seconds();

		Text statusUpdate;
		ScopedSlowTask slowTask(0, statusUpdate, false);
		TMap<int32, ShaderMapFinalizeResults> compiledShaderMaps;
		compiledShaderMaps.append(mPendingFinalizeShaderMaps);
		mPendingFinalizeShaderMaps.empty();
		blockOnShaderMapCompletion(shaderMapIdsToFinishCompiling, compiledShaderMaps);
		bool bRetry = false;
		do 
		{
			bRetry = handlePotentialRetryOnError(compiledShaderMaps);
		} while (bRetry);

		processCompiledShaderMap(compiledShaderMaps, FLT_MAX);
		BOOST_ASSERT(compiledShaderMaps.size() == 0);
		const double endTime = PlatformTime::seconds();
	}

	ShaderCompileThreadRunnableBase::ShaderCompileThreadRunnableBase(ShaderCompilingManager* inManager)
		:mManager(inManager),
		mThread(nullptr),
		bTerminatedByError(false),
		bForceFinish(false)
	{

	}

	void ShaderCompileThreadRunnableBase::startThread()
	{
		if (mManager->bAllowAsynchronousShaderCompiling && !PlatformProperties::requiresCookedData())
		{
			mThread = RunnableThread::create(this, TEXT("ShaderCompilingThread"), 0, TPri_Normal);
		}
	}

	uint32 ShaderCompileThreadRunnableBase::run()
	{
#ifdef _MSC_VER
		if (!PlatformMisc::isDebuggerPresent())
		{
#if !PLATFORM_SEH_EXCEPTIONS_DISABLED
			__try
#endif
			{
				BOOST_ASSERT(mManager->bAllowAsynchronousShaderCompiling);
				while (!bForceFinish)
				{
					compilingLoop();
				}
			}
#if !PLATFORM_SEH_EXCEPTIONS_DISABLED
			__except (
#if PLATFORM_WINDOWS
				1
#endif
				)
			{
				PlatformMisc::memoryBarrier();
				bTerminatedByError = true;
			}
#endif
		}
		else
#endif
		{
			BOOST_ASSERT(mManager->bAllowAsynchronousShaderCompiling);
			while (!bForceFinish)
			{
				compilingLoop();
			}
		}
		return 0;
	}

	void ShaderCompileThreadRunnableBase::checkHealth() const
	{
		if (bTerminatedByError)
		{
			GIsCriticalError = false;
		}
	}

	struct ShaderCompileWorkerInfo
	{
		ProcHandle mWorkerProcess;
		bool bIssuedTasksToWorker;

		bool bLaunchedworker;

		bool bComplete;

		double mStartTime;

		TArray<ShaderCommonCompileJob*> mQueuedJobs;

		ShaderCompileWorkerInfo() :
			bIssuedTasksToWorker(false),
			bLaunchedworker(false),
			bComplete(false),
			mStartTime(0.0)
		{

		}

		~ShaderCompileWorkerInfo()
		{
			if (mWorkerProcess.isValid())
			{
				PlatformProcess::terminateProc(mWorkerProcess);
				PlatformProcess::closeProc(mWorkerProcess);
			}
		}

	};

	ShaderCompileThreadRunnable::ShaderCompileThreadRunnable(ShaderCompilingManager* inManager)
		:ShaderCompileThreadRunnableBase(inManager)
		, mLastCheckForWorkersTime(0)
	{
		for (uint32 workerIndex = 0; workerIndex < mManager->mNumShaderCompilingThreads; workerIndex++)
		{
			mWorkerInfos.add(new ShaderCompileWorkerInfo());
		}
	}

	ShaderCompileThreadRunnable::~ShaderCompileThreadRunnable()
	{
		for (int32 index = 0; index < mWorkerInfos.size(); index++)
		{
			delete mWorkerInfos[index];
		}
		mWorkerInfos.empty(0);
	}

	static const TArray<const IShaderFormat*>& getShaderFormats()
	{
		static bool bInitailzied = false;
		static TArray<const IShaderFormat*> results;
		if (!bInitailzied)
		{
			bInitailzied = true;
			results.empty(results.size());
			TArray<wstring> modules;
			ModuleManager::get().findModules(SHADERFORMAT_MODULE_WILDCARD, modules);
			if (!modules.size())
			{
				AIR_LOG(logShaders, Error, TEXT("No target shader formats found!"));
			}

			for (int32 index = 0; index < modules.size(); index++)
			{
				IShaderFormat* format = ModuleManager::loadModuleChecked<IShaderFormatModule>(modules[index]).getShaderFormat();
				if (format != nullptr)
				{
					results.add(format);
				}
			}
			return results;
		}
	}

	static inline void getFormatVersionMap(TMap<wstring, uint16>& outFormatVersionMap)
	{
		if (outFormatVersionMap.size() == 0)
		{
			const TArray<const class IShaderFormat*>& shaderFormats = getShaderFormats();
			BOOST_ASSERT(shaderFormats.size());
			for (int32 index = 0; index < shaderFormats.size(); index++)
			{
				TArray<wstring> outFormats;
				shaderFormats[index]->getSupportedFormats(outFormats);
				BOOST_ASSERT(outFormats.size());
				for (int32 innerIndex = 0; innerIndex < outFormats.size(); innerIndex++)
				{
					uint16 version = shaderFormats[index]->getVersion(outFormats[innerIndex]);
					outFormatVersionMap.emplace(outFormats[innerIndex], version);
				}
			}
		}
	}

	static void splitJobsByType(const TArray<ShaderCommonCompileJob*> & queuedJobs, TArray<ShaderCompileJob*>& outQueuedSingleJobs, TArray<ShaderPipelineCompileJob*>& outQueuedPipelineJobs)
	{
		for (int32 index = 0; index < queuedJobs.size(); ++index)
		{
			auto* commonJob = queuedJobs[index];
			auto* pipelineJob = commonJob->getShaderPipelineJob();
			if (pipelineJob)
			{
				outQueuedPipelineJobs.add(pipelineJob);
			}
			else
			{
				auto* singleJob = commonJob->getSingleShaderJob();
				BOOST_ASSERT(singleJob);
				outQueuedSingleJobs.add(singleJob);
			}
		}
	}

	static bool doWriteTasks(const TArray<ShaderCommonCompileJob*>& ququedJobs, Archive& transferFile)
	{
		int32 inputVersion = ShaderCompileWorkerInputVersion;
		transferFile << inputVersion;

		static TMap<wstring, uint16> mFormatVersionMap;
		getFormatVersionMap(mFormatVersionMap);
		transferFile << mFormatVersionMap;

		TArray<ShaderCompileJob*> queuedSingleJobs;

		TArray<ShaderPipelineCompileJob*> queuedPipelineJobs;

		splitJobsByType(ququedJobs, queuedSingleJobs, queuedPipelineJobs);
		{
			int32 singleJobHeader = ShaderCompileWorkerSingleJobHeader;
			transferFile << singleJobHeader;
			int32 numBatches = queuedSingleJobs.size();
			transferFile << numBatches;

			for (int32 jobIndex = 0; jobIndex < queuedSingleJobs.size(); jobIndex++)
			{
				transferFile << queuedSingleJobs[jobIndex]->mInput;
			}
		}
		{
			int32 pipelineJobHeader = ShaderCompileWorkerPipelineJobHeader;
			transferFile << pipelineJobHeader;
			int32 numBatches = queuedPipelineJobs.size();
			transferFile << numBatches;
			for (int32 jobIndex = 0; jobIndex < queuedPipelineJobs.size(); jobIndex++)
			{
				auto* pipelineJob = queuedPipelineJobs[jobIndex];
				wstring pipelineName = pipelineJob->mShaderPipeline->getName();
				transferFile << pipelineName;
				int32 numStageJobs = pipelineJob->mStageJobs.size();
				transferFile << numStageJobs;
				for (int32 index = 0; index < numStageJobs; index++)
				{
					transferFile << pipelineJob->mStageJobs[index]->getSingleShaderJob()->mInput;
				}
			}
		}
		return transferFile.close();
	}

	int32 ShaderCompileThreadRunnable::pullTaskFromQueue()
	{
		int32 numAtiveThreads = 0;
		{
			std::lock_guard<std::mutex> lock(mManager->mCompileQueueSection);
			const int32 numWorkersToFeed = mManager->bCompilingOuringGame ? mManager->mNumShaderCompilingThreadsDuringGame : mWorkerInfos.size();
			for (int32 workerIndex = 0; workerIndex < mWorkerInfos.size(); workerIndex++)
			{
				ShaderCompileWorkerInfo& currentWorkerInfo = *mWorkerInfos[workerIndex];
				if (currentWorkerInfo.mQueuedJobs.size() == 0 && workerIndex < numWorkersToFeed)
				{
					BOOST_ASSERT(!currentWorkerInfo.bComplete);
					if (mManager->mCompileQueue.size() > 0)
					{
						bool bAddedLowLatencyTask = false;
						int32 jobIndex = 0; 
						for (; jobIndex < mManager->mMaxShaderJobsBatchSize && jobIndex < mManager->mCompileQueue.size() && !bAddedLowLatencyTask; jobIndex++)
						{
							bAddedLowLatencyTask |= mManager->mCompileQueue[jobIndex]->bOptimizeForLowLatency;
							currentWorkerInfo.mQueuedJobs.add(mManager->mCompileQueue[jobIndex]);
						}

						currentWorkerInfo.bIssuedTasksToWorker = false;
						currentWorkerInfo.bLaunchedworker = false;
						currentWorkerInfo.mStartTime = PlatformTime::seconds();
						numAtiveThreads++;
						mManager->mCompileQueue.removeAt(0, jobIndex);
					}
				}
				else
				{
					if (currentWorkerInfo.mQueuedJobs.size() > 0)
					{
						numAtiveThreads++;
					}
					if (currentWorkerInfo.bComplete)
					{
for (int32 jobIndex = 0; jobIndex < currentWorkerInfo.mQueuedJobs.size(); jobIndex++)
{
	ShaderMapCompileResults& shaderMapResults = mManager->mShaderMapJobs.findChecked(currentWorkerInfo.mQueuedJobs[jobIndex]->mId);
	shaderMapResults.mFinishedJobs.add(currentWorkerInfo.mQueuedJobs[jobIndex]);
	shaderMapResults.bAllJobsSucceeded = shaderMapResults.bAllJobsSucceeded && currentWorkerInfo.mQueuedJobs[jobIndex]->bSucceeded;
}
const float elapsedTime = PlatformTime::seconds() - currentWorkerInfo.mStartTime;
mManager->mWorkersBusyTime += elapsedTime;
if (false)
{

}

PlatformAtomics::interLockedAdd(&mManager->mNumOutstandingJobs, -currentWorkerInfo.mQueuedJobs.size());
currentWorkerInfo.bComplete = false;
currentWorkerInfo.mQueuedJobs.empty();
					}
				}
			}
		}
		return numAtiveThreads;
	}

	void ShaderCompileThreadRunnable::writeNewTasks()
	{
		for (int32 workerIndex = 0; workerIndex < mWorkerInfos.size(); workerIndex++)
		{
			ShaderCompileWorkerInfo& currentWorkerInfo = *mWorkerInfos[workerIndex];
			if (!currentWorkerInfo.bIssuedTasksToWorker && currentWorkerInfo.mQueuedJobs.size() > 0)
			{
				currentWorkerInfo.bIssuedTasksToWorker = true;
				const wstring woringDirectory = mManager->mAbsoluteShaderBaseWorkingDirectory + boost::lexical_cast<wstring>(workerIndex);
				wstring transferFileName;
				do
				{
					Guid guid;
					PlatformMisc::createGuid(guid);
					transferFileName = woringDirectory + guid.toString();
				} while (IFileManager::get().fileSize(transferFileName.c_str()) != INDEX_NONE);
				Archive* transferFile = nullptr;
				int32 retryCount = 0;
				while (transferFile == nullptr && retryCount < 2000)
				{
					if (retryCount > 0)
					{
						PlatformProcess::sleep(0.01f);
					}
					transferFile = IFileManager::get().createFileWriter(transferFileName.c_str(), FILEWRITE_EvenIfReadOnly);
					retryCount++;
					if (transferFile == nullptr)
					{
						AIR_LOG(logShaderCompilers, Warning, TEXT("could not create the shader compiler transfer file '%s', retrying...", transferFileName.c_str()));
					}
				}
				if (transferFile == nullptr)
				{
					AIR_LOG(logShaderCompilers, Fatal, TEXT("could not create the shader compiler transfer file '%s'."), transferFileName);
				}

				BOOST_ASSERT(transferFile);

				if (!doWriteTasks(currentWorkerInfo.mQueuedJobs, *transferFile))
				{
					uint64 totalDiskSpace = 0;
					uint64 freeDiskSpace = 0;
					PlatformMisc::getDiskTotalAndFreeSpace(transferFileName, totalDiskSpace, freeDiskSpace);
					AIR_LOG(logShaderCompilers, Fatal, TEXT("could not write the shader compiler transfer filename to '%s' (free disk space : %llu."), transferFileName.c_str(), freeDiskSpace);
				}
				delete transferFile;

				wstring properTransferFilename = woringDirectory + TEXT("WorkerInputOnly.in");
				if (!IFileManager::get().move(properTransferFilename.c_str(), transferFileName.c_str()))
				{
					uint64 totalDiskSpace = 0;
					uint64 freeDiskSpace = 0;
					PlatformMisc::getDiskTotalAndFreeSpace(transferFileName, totalDiskSpace, freeDiskSpace);
					AIR_LOG(logShaderCompilers, Fatal, TEXT("could not rename the shader compiler transfer filename to '%s' from '%s' (Free Disk Space: %llu)."), properTransferFilename.c_str(), transferFileName.c_str(), freeDiskSpace);

				}

			}
		}
	}

	bool ShaderCompileThreadRunnable::launchWorkersIfNeeded()
	{
		const double currentTime = PlatformTime::seconds();
		const bool bCheckForWorkerRunning = (currentTime - mLastCheckForWorkersTime > .1f);
		bool bAbandonWorkers = false;
		if (bCheckForWorkerRunning)
		{
			mLastCheckForWorkersTime = currentTime;
		}
		for (int32 workerIndex = 0; workerIndex < mWorkerInfos.size(); workerIndex++)
		{
			ShaderCompileWorkerInfo& currentWorkerInfo = *mWorkerInfos[workerIndex];

			if (currentWorkerInfo.mQueuedJobs.size() == 0)
			{
				if (currentWorkerInfo.mWorkerProcess.isValid() && !ShaderCompilingManager::isShaderCompilerWorkerRunning(currentWorkerInfo.mWorkerProcess))
				{
					PlatformProcess::closeProc(currentWorkerInfo.mWorkerProcess);
					currentWorkerInfo.mWorkerProcess = ProcHandle();
				}
				continue;
			}
			if (!currentWorkerInfo.mWorkerProcess.isValid() || (bCheckForWorkerRunning && !ShaderCompilingManager::isShaderCompilerWorkerRunning(currentWorkerInfo.mWorkerProcess)))
			{
				bool bLaunchAgain = true;
				if (currentWorkerInfo.mWorkerProcess.isValid())
				{
					PlatformProcess::closeProc(currentWorkerInfo.mWorkerProcess);
					currentWorkerInfo.mWorkerProcess = ProcHandle();
					if (currentWorkerInfo.bLaunchedworker)
					{
						const wstring workingDirectory = mManager->mAbsoluteShaderBaseWorkingDirectory + boost::lexical_cast<wstring>(workerIndex) + TEXT("/");
						const wstring outputFileNameAndPath = workingDirectory + TEXT("WorkerOutputOnly.out");

						if (PlatformFileManager::get().getPlatformFile().fileExists(outputFileNameAndPath.c_str()))
						{
							bLaunchAgain = false;
						}
						else
						{
							bAbandonWorkers = true;
							break;
						}
					}
				}
				if (bLaunchAgain)
				{
					const wstring workingDirectory = mManager->mShaderBaseWorkingDirectory + boost::lexical_cast<wstring>(workerIndex) + TEXT("/");
					wstring inputFileName(TEXT("WorkerInputOnly.in"));
					wstring outputFileName(TEXT("WorkerOutputOnly.out"));
					currentWorkerInfo.mWorkerProcess = mManager->launchWorker(workingDirectory, mManager->mProcessId, workerIndex, inputFileName, outputFileName);
				}
			}
		}
		return bAbandonWorkers;
	}

	static void modalErrorOrLog(const wstring& text)
	{
		if (PlatformProperties::supportsWindowedMode())
		{
			AIR_LOG(LogShaderCompilers, ERROR, TEXT("%s"), text.c_str());
			PlatformMisc::requestExit(false);
			return;
		}
		else
		{
			AIR_LOG(logShaderCompilers, Fatal, TEXT("%s"), text.c_str());
		}
	}

	namespace SCWErrorCode
	{
		enum EErors
		{
			Success,
			GeneralCrash,
			BadShaderFormatVersion,
			BadInputVersion,
			BadSingleJobHeader,
			BadPipelineJobHeader,
			CantDeleteInputFile,
			CantSaveOutputFile,
			NoTargetShaderFormatsFound,
			CantCompileForSpecificFormat,
		};
	}

	static void readSingleJob(ShaderCompileJob* currentJob, Archive& outputFile)
	{
		BOOST_ASSERT(!currentJob->bFinalized);
		currentJob->bFinalized = true;
		outputFile << currentJob->mOutput;
		currentJob->mOutput.generateOutputHash();
		currentJob->bSucceeded = currentJob->mOutput.bSucceeded;
	}

	static void doReadTaskResults(const TArray<ShaderCommonCompileJob*>& queuedJobs, Archive& outputFile)
	{
		int32 outputVersion = ShaderCompileWorkerOutputVersion;
		outputFile << outputVersion;
		if (ShaderCompileWorkerOutputVersion != outputVersion)
		{
			wstring text = printf(TEXT("Expecting ShaderComi;erWorker output version %d, got %d instead1! forgot to build ShaderCompilerWorker?"), ShaderCompileWorkerOutputVersion, outputVersion);
			modalErrorOrLog(text);
		}
		int32 errorCode;
		outputFile << errorCode;
		int32 callstackLength = 0;
		outputFile << callstackLength;
		int32 expectionInfoLength = 0;
		outputFile << expectionInfoLength;

		if (errorCode != SCWErrorCode::Success)
		{
			TArray<TCHAR> callStack;
			callStack.addUninitialized(callstackLength + 1);
			outputFile.serialize(callStack.getData(), callstackLength * sizeof(TCHAR));
			callStack[callstackLength] = 0;
		
			TArray<TCHAR> exceptionInfo;
			exceptionInfo.addUninitialized(expectionInfoLength + 1);
			outputFile.serialize(exceptionInfo.getData(), expectionInfoLength * sizeof(TCHAR));
			exceptionInfo[expectionInfoLength] = 0;

			switch (errorCode)
			{
			default:
			case SCWErrorCode::GeneralCrash:
			{
				BOOST_ASSERT(false);
			}
				break;
			}
		}
		TArray<ShaderCompileJob*> queuedSingleJobs;
		TArray<ShaderPipelineCompileJob*> queuedPipelineJobs;
		splitJobsByType(queuedJobs, queuedSingleJobs, queuedPipelineJobs);
		{
			int32 singleJobHeader = -1;
			outputFile << singleJobHeader;
			if (singleJobHeader != ShaderCompileWorkerSingleJobHeader)
			{
				wstring text = printf(TEXT("Expecting ShaderCompilerWorker Single Jobs %d, got %d instead! forget to build shadercompilerWorker?"), ShaderCompileWorkerSingleJobHeader, singleJobHeader);
			}
			int32 numJobs;
			outputFile << numJobs;
			BOOST_ASSERT(numJobs == queuedSingleJobs.size());
			for (int32 jobIndex = 0; jobIndex < numJobs; jobIndex++)
			{
				auto* currentJob = queuedSingleJobs[jobIndex];
				readSingleJob(currentJob, outputFile);
			}
		}
		{
			int32 pipelineJobHeader = -1;
			outputFile << pipelineJobHeader;
			if (pipelineJobHeader != ShaderCompileWorkerPipelineJobHeader)
			{
				BOOST_ASSERT(false);
			}
			int32 numJobs;
			outputFile << numJobs;
			BOOST_ASSERT(numJobs == queuedPipelineJobs.size());
			for (int32 jobIndex = 0; jobIndex < numJobs; jobIndex++)
			{
				ShaderPipelineCompileJob* currentJob = queuedPipelineJobs[jobIndex];
				wstring pipelineName;
				outputFile << pipelineName;
				BOOST_ASSERT(pipelineName == currentJob->mShaderPipeline->getName());
				BOOST_ASSERT(!currentJob->bFinalized);
				currentJob->bFinalized = true;
				currentJob->bFailedRemovingUnused = false;
				int32 numStageJobs = -1;
				outputFile << numStageJobs;
				if (numStageJobs != currentJob->mStageJobs.size())
				{
					BOOST_ASSERT(numJobs == queuedPipelineJobs.size());
				}
				currentJob->bSucceeded = true;
				for (int32 index = 0; index < numStageJobs; index++)
				{
					auto* singleJob = currentJob->mStageJobs[index]->getSingleShaderJob();
					readSingleJob(singleJob, outputFile);
					currentJob->bFailedRemovingUnused = currentJob->bFailedRemovingUnused | singleJob->mOutput.bFailedRemovingUnused;
				}
			}
		}
	}


	void ShaderCompileThreadRunnable::readAvailableResults()
	{
		for (int32 workerIndex = 0; workerIndex < mWorkerInfos.size(); workerIndex++)
		{
			ShaderCompileWorkerInfo& currentWorkerInfo = *mWorkerInfos[workerIndex];
			if (currentWorkerInfo.mQueuedJobs.size() > 0)
			{
				const wstring workingDirectory = mManager->mAbsoluteShaderBaseWorkingDirectory + boost::lexical_cast<wstring>(workerIndex) + TEXT("/");
				const TCHAR* inputFileName = TEXT("WorkerInputOnly.in");
				const wstring outputFileNameAndPath = workingDirectory + TEXT("WorkerOutputOnly.out");

				if (PlatformFileManager::get().getPlatformFile().fileExists(outputFileNameAndPath.c_str()))
				{
					Archive* outputFilePtr = IFileManager::get().createFileReader(outputFileNameAndPath.c_str(), FILEREAD_Silent);
					if (outputFilePtr)
					{
						Archive& outputFile = *outputFilePtr;
						BOOST_ASSERT(!currentWorkerInfo.bComplete);
						doReadTaskResults(currentWorkerInfo.mQueuedJobs, outputFile);
						delete outputFilePtr;
						bool bdeletedOutput = IFileManager::get().del(outputFileNameAndPath.c_str(), true, true);
						int32 retryCount = 0;
						while (!bdeletedOutput && retryCount < 200)
						{
							PlatformProcess::sleep(0.01f);
							bdeletedOutput = IFileManager::get().del(outputFileNameAndPath.c_str(), true, true);
							retryCount++;
						}
						BOOST_ASSERT(bdeletedOutput, TEXT("failed to delete %d!"), outputFileNameAndPath.c_str());
						currentWorkerInfo.bComplete = true;
					}
				}
			}
		}
	}

	extern bool compileShaderPipeline(const IShaderFormat* compiler, wstring format, ShaderPipelineCompileJob* pipelineJob, const wstring & dir)
	{
		BOOST_ASSERT(pipelineJob && pipelineJob->mStageJobs.size() > 0);
		ShaderCompileJob* currentJob = pipelineJob->mStageJobs[0]->getSingleShaderJob();
		currentJob->mInput.bCompilingForShaderPipeline = true;
		currentJob->mInput.bIncludeUsedOutputs = false;
		if (isValidRef(currentJob->mInput.mSharedEnvironment))
		{
			currentJob->mInput.mEnvironment.merge(*currentJob->mInput.mSharedEnvironment);
		}
		compiler->compileShader(format, currentJob->mInput, currentJob->mOutput, dir);

		currentJob->bSucceeded = currentJob->mOutput.bSucceeded;
		if (!currentJob->mOutput.bSucceeded)
		{
			return false;
		}
		currentJob->mOutput.generateOutputHash();
		bool bEnableRemovingUnused = true;
		bool bJobFailedRemovingUnused = false;

		for (int32 index = 0; index < pipelineJob->mStageJobs.size(); index++)
		{
			auto stage = pipelineJob->mStageJobs[index]->getSingleShaderJob()->mInput.mTarget.mFrequency;
			if (stage != SF_Vertex && stage != SF_Pixel)
			{
				bEnableRemovingUnused = false;
				break;
			}
		}
		for (int32 index = 0; index < pipelineJob->mStageJobs.size(); index++)
		{
			auto* previousJob = currentJob;
			currentJob = pipelineJob->mStageJobs[index]->getSingleShaderJob();
			bEnableRemovingUnused = bEnableRemovingUnused && previousJob->mOutput.bSupportsQueryingUsedAttributes;
			if (bEnableRemovingUnused)
			{
				currentJob->mInput.bIncludeUsedOutputs = true;
				currentJob->mInput.bCompilingForShaderPipeline = true;
				currentJob->mInput.mUsedOutputs = previousJob->mOutput.mUsedAttributes;
			}
			if (isValidRef(currentJob->mInput.mSharedEnvironment))
			{
				currentJob->mInput.mEnvironment.merge(*currentJob->mInput.mSharedEnvironment);
			}
			compiler->compileShader(format, currentJob->mInput, currentJob->mOutput, dir);

			currentJob->bSucceeded = currentJob->mOutput.bSucceeded;
			if (!currentJob->mOutput.bSucceeded)
			{
				return false;
			}
			bJobFailedRemovingUnused = currentJob->mOutput.bFailedRemovingUnused || bJobFailedRemovingUnused;
			currentJob->mOutput.generateOutputHash();
		}
		pipelineJob->bSucceeded = true;
		pipelineJob->bFailedRemovingUnused = bJobFailedRemovingUnused;
		return true;
	}

	void ShaderCompileThreadRunnable::compileDirectlyThroughDll()
	{
		for (int32 workerIndex = 0; workerIndex < mWorkerInfos.size(); workerIndex++)
		{
			ShaderCompileWorkerInfo& currentWorkerInfo = *mWorkerInfos[workerIndex];
			if (currentWorkerInfo.mQueuedJobs.size() > 0)
			{
				for (int32 jobIndex = 0; jobIndex < currentWorkerInfo.mQueuedJobs.size(); jobIndex++)
				{
					ShaderCommonCompileJob& currentJob = *currentWorkerInfo.mQueuedJobs[jobIndex];
					BOOST_ASSERT(!currentJob.bFinalized);
					currentJob.bFinalized = true;
					static ITargetPlatformManagerModule& TPM = getTargetPlatformManagerRef();
					auto* singleJob = currentJob.getSingleShaderJob();
					if (singleJob)
					{
						const wstring format = legacyShaderPlatformToShaderFormat(EShaderPlatform(singleJob->mInput.mTarget.mPlatform));
						const IShaderFormat *compiler = TPM.findShaderFormat(format);
						if (!compiler)
						{
							BOOST_ASSERT(false);
						}
						if (isValidRef(singleJob->mInput.mSharedEnvironment))
						{
							singleJob->mInput.mEnvironment.merge(*singleJob->mInput.mSharedEnvironment);
						}
						compiler->compileShader(format, singleJob->mInput, singleJob->mOutput, wstring(PlatformProcess::shaderDir()));
						singleJob->bSucceeded = singleJob->mOutput.bSucceeded;
						if (singleJob->mOutput.bSucceeded)
						{
							singleJob->mOutput.generateOutputHash();
						}
					}
					else
					{
						auto* pipelineJob = currentJob.getShaderPipelineJob();
						BOOST_ASSERT(pipelineJob);
						EShaderPlatform platform = (EShaderPlatform)pipelineJob->mStageJobs[0]->getSingleShaderJob()->mInput.mTarget.mPlatform;
						const wstring format = legacyShaderPlatformToShaderFormat(platform);
						const IShaderFormat* compiler = TPM.findShaderFormat(format);
						BOOST_ASSERT(compiler);
						for (int32 index = 1; index < pipelineJob->mStageJobs.size(); ++index)
						{
							auto* singleStage = pipelineJob->mStageJobs[index]->getSingleShaderJob();
							if (!singleStage)
							{

							}
							if (platform != singleStage->mInput.mTarget.mPlatform)
							{

							}
						}
						compileShaderPipeline(compiler, format, pipelineJob, wstring(PlatformProcess::shaderDir()));
					}
				}
				currentWorkerInfo.bComplete = true;
			}
		}
	}

	int32 ShaderCompileThreadRunnable::compilingLoop()
	{
		const int32 numActiveThreads = pullTaskFromQueue();
		if (numActiveThreads == 0 && mManager->bAllowAsynchronousShaderCompiling)
		{
			PlatformProcess::sleep(0.010f);
		}
		if (mManager->bAllowCompilingThroughWorkers)
		{
			writeNewTasks();
			bool bAbandonWorkers = launchWorkersIfNeeded();
			if (bAbandonWorkers)
			{
				mManager->bAllowCompilingThroughWorkers = false;
			}
			else
			{
				readAvailableResults();
			}
		}
		else
		{
			compileDirectlyThroughDll();
		}
		return numActiveThreads;
	}

	void ShaderCompilingManager::addJobs(TArray<ShaderCommonCompileJob*>& newJobs, bool bApplyCompletedShaderMapForRendering, bool bOptimizeForLowLatency, bool bRecreateComponentRenderStateOnCompletion)
	{
		std::lock_guard<std::mutex> lock(mCompileQueueSection);
		if (bOptimizeForLowLatency)
		{
			int32 insertIndex = 0; 
			for (; insertIndex < mCompileQueue.size(); insertIndex++)
			{
				if (!mCompileQueue[insertIndex]->bOptimizeForLowLatency)
				{
					break;
				}
			}
			mCompileQueue.insertZeroed(insertIndex, newJobs.size());
			for (int32 jobIndex = 0; jobIndex < newJobs.size(); jobIndex++)
			{
				mCompileQueue[insertIndex + jobIndex] = newJobs[jobIndex];
			}
		}
		else
		{
			mCompileQueue.append(newJobs);
		}

		PlatformAtomics::interLockedAdd(&mNumOutstandingJobs, newJobs.size());
		for (int32 jobIndex = 0; jobIndex < newJobs.size(); jobIndex++)
		{
			newJobs[jobIndex]->bOptimizeForLowLatency = bOptimizeForLowLatency;
			ShaderMapCompileResults& shaderMapInfo = mShaderMapJobs.findOrAdd(newJobs[jobIndex]->mId);
			shaderMapInfo.bApplyCompletedShaderMapForRendering = bApplyCompletedShaderMapForRendering;
			shaderMapInfo.bRecreateComponentRenderStateOnCompletion = bRecreateComponentRenderStateOnCompletion;
			auto* pipelineJob = newJobs[jobIndex]->getShaderPipelineJob();
			if (pipelineJob)
			{
				shaderMapInfo.mNumJobsQueued += pipelineJob->mStageJobs.size();
			}
			else
			{
				shaderMapInfo.mNumJobsQueued++;
			}
		}
	}

	static int32 getNumTotalJobs(const TArray<ShaderCommonCompileJob*>& jobs)
	{
		int32 numJobs = 0;
		for (int32 index = 0; index < jobs.size(); index++)
		{
			auto* pipelineJob = jobs[index]->getShaderPipelineJob();
			numJobs += pipelineJob ? pipelineJob->mStageJobs.size() : 1;
		}
		return numJobs;
	}

	void ShaderCompilingManager::blockOnShaderMapCompletion(const TArray<int32>& shaderMapIdsToFinishCompiling, TMap<int32, ShaderMapFinalizeResults>& compiledShaderMaps)
	{
		if (bAllowAsynchronousShaderCompiling)
		{
			int32 numPendingJobs = 0;
			do 
			{
				mThread->checkHealth();
				numPendingJobs = 0;
				{
					std::lock_guard<std::mutex> lock(mCompileQueueSection);
					for (int32 shaderMapIndex = 0; shaderMapIndex < shaderMapIdsToFinishCompiling.size(); shaderMapIndex++)
					{
						const auto resultsPtr = mShaderMapJobs.find(shaderMapIdsToFinishCompiling[shaderMapIndex]);
						if (resultsPtr != mShaderMapJobs.end())
						{
							const ShaderMapCompileResults& result = resultsPtr->second;
							int32 finishedJobs = getNumTotalJobs(result.mFinishedJobs);
							if (finishedJobs == result.mNumJobsQueued)
							{
								compiledShaderMaps.emplace(shaderMapIdsToFinishCompiling[shaderMapIndex], ShaderMapFinalizeResults(result));
								mShaderMapJobs.erase(shaderMapIdsToFinishCompiling[shaderMapIndex]);
							}
							else
							{
								numPendingJobs += result.mNumJobsQueued;
							}
						}
					}
				}
				if (numPendingJobs > 0)
				{
					PlatformProcess::sleep(0.01f);
				}
			} while (numPendingJobs > 0);
		}
		else
		{
			int32 numActiveWorkers = 0; 
			do 
			{
				numActiveWorkers = mThread->compilingLoop();
			} while (numActiveWorkers > 0);
			BOOST_ASSERT(mCompileQueue.size() == 0);
			for (int32 shaderMapIndex = 0; shaderMapIndex < shaderMapIdsToFinishCompiling.size(); shaderMapIndex++)
			{
				const auto resultIt = mShaderMapJobs.find(shaderMapIdsToFinishCompiling[shaderMapIndex]);
				if (resultIt != mShaderMapJobs.end())
				{
					const ShaderMapCompileResults& result = resultIt->second;
					BOOST_ASSERT(getNumTotalJobs(result.mFinishedJobs) == result.mNumJobsQueued);
					compiledShaderMaps.emplace(shaderMapIdsToFinishCompiling[shaderMapIndex], ShaderMapFinalizeResults(result));
					mShaderMapJobs.erase(shaderMapIdsToFinishCompiling[shaderMapIndex]);
				}
			}
		}
	}

	static void addErrorsForFailedJob(const ShaderCompileJob& currentJob, TArray<EShaderPlatform>& errorPlatforms, TArray<wstring>& uniqueErorrs, TArray< const ShaderCommonCompileJob*>& errorJobs)
	{
		errorPlatforms.addUnique((EShaderPlatform)currentJob.mInput.mTarget.mPlatform);
		for (int32 errorIndex = 0; errorIndex < currentJob.mOutput.mErrors.size(); errorIndex++)
		{
			const ShaderCompilerError& currentError = currentJob.mOutput.mErrors[errorIndex];
			uniqueErorrs.addUnique(currentJob.mOutput.mErrors[errorIndex].getErrorString());
			errorJobs.addUnique(&currentJob);
		}
	}

	static void processErrors(const ShaderCompileJob& currentJob, TArray<wstring>& uniqueErrors, wstring& errorString)
	{
		for (int32 errorIndex = 0; errorIndex < currentJob.mOutput.mErrors.size(); errorIndex++)
		{
			ShaderCompilerError currentError = currentJob.mOutput.mErrors[errorIndex];
			int32 uniqueError = INDEX_NONE;
			if (uniqueErrors.find(currentError.getErrorString(), uniqueError))
			{
				uniqueErrors.removeAt(uniqueError);
				if (currentError.mErrorFile == TEXT("Material.hlsl"))
				{
					currentError.mErrorFile = TEXT("MaterialTemplate.hlsl");
				}
				else if (boost::contains(currentError.mErrorFile, TEXT("memory")))
				{
					BOOST_ASSERT(currentJob.mShaderType);
					currentError.mErrorFile = wstring(currentJob.mShaderType->getShaderFilename()) + TEXT(".hlsl");
				}
				else if (currentError.mErrorFile == TEXT("VertexFactory.hlsl"))
				{
					BOOST_ASSERT(currentJob.mVFType);
					currentError.mErrorFile = currentJob.mVFType->getShaderFilename() + wstring(TEXT(".usf"));
				}
				else if (currentError.mErrorFile.empty() && currentJob.mShaderType)
				{
					currentError.mErrorFile = wstring(currentJob.mShaderType->getShaderFilename()) + TEXT(".hlsl");
				}
				wstring uniqueErrorPrefix;
				if (currentJob.mShaderType)
				{
					const wstring solutionPath = Paths::rootDir();
					wstring shaderPath = PlatformProcess::shaderDir();
					Paths::makePathRelativeTo(shaderPath, solutionPath.c_str());
					uniqueErrorPrefix = printf(TEXT("%s(%s): Shader %s, VF %s:\n\t"), currentError.mErrorFile.c_str(), currentError.mErrorLineString.c_str(), currentJob.mShaderType->getName(), currentJob.mVFType ? currentJob.mVFType->getName() : TEXT("None"));
				}
				else
				{
					uniqueErrorPrefix = printf(TEXT("%s(0): "), currentJob.mInput.mSourceFilename);
				}
				wstring uniqueErrorString = uniqueErrorPrefix + currentError.mStrippedErrorMessage + TEXT("\n");
				if (GIsBuildMachine)
				{
				}
				else
				{
					AIR_LOG(LogShaderCompilers, Warning, TEXT("%s"), uniqueErrorString.c_str());
				}
				errorString += uniqueErrorString;
			}
		}
	}


	bool ShaderCompilingManager::handlePotentialRetryOnError(TMap<int32, ShaderMapFinalizeResults>& completedShaderMaps)
	{
		bool bRetryCompile = false;
		for (auto& it : completedShaderMaps)
		{
			ShaderMapFinalizeResults& result = it.second;
			if (!result.bAllJobsSucceeded)
			{
				bool bSpecialEngineMaterial = false;
				const MaterialShaderMap* shaderMap = nullptr;
				for (auto& shaderMapIt : MaterialShaderMap::mShaderMapsBeingCompiled)
				{
					const MaterialShaderMap* testShaderMap = shaderMapIt.first;
					BOOST_ASSERT(testShaderMap);
					if (testShaderMap->mCompilingId == it.first)
					{
						shaderMap = testShaderMap;
						for (int32 materialIndex = 0; materialIndex < shaderMapIt.second.size(); materialIndex++)
						{
							FMaterial* material = shaderMapIt.second[materialIndex];
							bSpecialEngineMaterial = bSpecialEngineMaterial | material->isSpecialEngineMaterial();

						}
						break;
					}
				}
				BOOST_ASSERT(shaderMap || it.first == mGlobalShaderMapId);
				if (it.first == mGlobalShaderMapId)
				{
					const TArray<ShaderCommonCompileJob*>& completeJobs = result.mFinishedJobs;
					TArray<const ShaderCommonCompileJob*> errorJobs;
					TArray<wstring> uniqueErorrs;
					TArray<EShaderPlatform> errorPlatforms;
					for (int32 jobIndex = 0; jobIndex < completeJobs.size(); jobIndex++)
					{
						const ShaderCommonCompileJob& currentJob = *completeJobs[jobIndex];
						if (!currentJob.bSucceeded)
						{
							const auto* singleJob = currentJob.getSingleShaderJob();
							if (singleJob)
							{
								addErrorsForFailedJob(*singleJob, errorPlatforms, uniqueErorrs, errorJobs);
							}
							else
							{
								const auto* pipelineJob = currentJob.getShaderPipelineJob();
								BOOST_ASSERT(pipelineJob);
								for (const ShaderCommonCompileJob* commonJob : pipelineJob->mStageJobs)
								{
									addErrorsForFailedJob(*commonJob->getSingleShaderJob(), errorPlatforms, uniqueErorrs, errorJobs);
								}
							}
						}
					}
					wstring targetShaderPlatformString;
					for (int32 platformIndex = 0; platformIndex < errorPlatforms.size(); platformIndex++)
					{
						if (targetShaderPlatformString.empty())
						{
							targetShaderPlatformString = legacyShaderPlatformToShaderFormat(errorPlatforms[platformIndex]);
						}
						else
						{
							targetShaderPlatformString += TEXT(", ") + legacyShaderPlatformToShaderFormat(errorPlatforms[platformIndex]);
						}
					}
					const TCHAR* materialName = shaderMap ? shaderMap->getFriendlyName().c_str() : TEXT("global shaders");

					wstring errorString = printf(TEXT("%i Shader compiler errors compiling %s for platform %s:"), uniqueErorrs.size(), materialName, targetShaderPlatformString.c_str());

					AIR_LOG(LogShaderCompilers, Warning, TEXT("%s"), errorString.c_str());
					errorString += TEXT("\n");

					for (int32 jobIndex = 0; jobIndex < completeJobs.size(); jobIndex++)
					{
						const ShaderCommonCompileJob& currentJob = *completeJobs[jobIndex];
						if (!currentJob.bSucceeded)
						{
							const auto* singleJob = currentJob.getSingleShaderJob();
							if (singleJob)
							{
								processErrors(*singleJob, uniqueErorrs, errorString);
							}
							else
							{
								const auto* pipelineJob = currentJob.getShaderPipelineJob();
								BOOST_ASSERT(pipelineJob);
								for (const ShaderCommonCompileJob* commonJob : pipelineJob->mStageJobs)
								{
									processErrors(*commonJob->getSingleShaderJob(), uniqueErorrs, errorString);
								}
							}
						}
					}
					if (bRetryCompile)
					{
						break;
					}
				}
			}
		}

		if (bRetryCompile)
		{
			flushShaderFileCache();
			TArray<int32> mapsToRemove;
			for (auto& it : completedShaderMaps)
			{
				ShaderMapFinalizeResults &result = it.second;
				if (!result.bAllJobsSucceeded)
				{
					mapsToRemove.add(it.first);
					for (int32 jobIndex = 0; jobIndex < result.mFinishedJobs.size(); jobIndex++)
					{
						ShaderCommonCompileJob& currentJob = *result.mFinishedJobs[jobIndex];
						auto* singleJob = currentJob.getSingleShaderJob();
						if (singleJob)
						{
							singleJob->mOutput = ShaderCompilerOutput();
						}
						else
						{
							auto* pipelineJob = currentJob.getShaderPipelineJob();
							for (ShaderCommonCompileJob* commonJob : pipelineJob->mStageJobs)
							{
								commonJob->getSingleShaderJob()->mOutput = ShaderCompilerOutput();
								commonJob->bFinalized = false;
							}
						}
						currentJob.bFinalized = false;
					}
					addJobs(result.mFinishedJobs, result.bApplyCompletedShaderMapForRendering, true, result.bRecreateComponentRenderStateOnCompletion);

				}
			}
			const int32 originalNumShaderMaps = completedShaderMaps.size();
			for (int32 removeIndex = 0; removeIndex < mapsToRemove.size(); removeIndex++)
			{
				completedShaderMaps.erase(mapsToRemove[removeIndex]);
			}
			BOOST_ASSERT(completedShaderMaps.size() == originalNumShaderMaps - mapsToRemove.size());
			blockOnShaderMapCompletion(mapsToRemove, completedShaderMaps);
			BOOST_ASSERT(completedShaderMaps.size() == originalNumShaderMaps);
		}
		return bRetryCompile;
	}

	static void checkSingleJob(ShaderCompileJob* singleJob, TArray<wstring>& errors)
	{
		if (singleJob->bSucceeded)
		{
			BOOST_ASSERT(singleJob->mOutput.mShaderCode.getShaderCodeSize() > 0);
		}
		if (!singleJob->bSucceeded)
		{
			for (int32 errorIndex = 0; errorIndex < singleJob->mOutput.mErrors.size(); errorIndex++)
			{
				errors.addUnique(singleJob->mOutput.mErrors[errorIndex].getErrorString());
			}
		}
	}


	

	

	void ShaderCompilingManager::processCompiledShaderMap(TMap<int32, ShaderMapFinalizeResults>& compiledShaderMaps, float timeBudget)
	{
		TArray<TRefCountPtr<MaterialShaderMap>> localShaderMapReferences;
		TMap<FMaterial*, MaterialShaderMap*> materialsToUpdate;
		TMap<FMaterial*, MaterialShaderMap*> materialsToApplyToScene;
		for (auto processIt = compiledShaderMaps.begin(); processIt != compiledShaderMaps.end(); processIt++)
		{
			TRefCountPtr<MaterialShaderMap> shaderMap = nullptr;
			TArray<FMaterial*>* materials = nullptr;
			for (auto& shaderMapIt : MaterialShaderMap::mShaderMapsBeingCompiled)
			{
				if (shaderMapIt.first->mCompilingId == processIt->first)
				{
					shaderMap = shaderMapIt.first;
					materials = &shaderMapIt.second;
					break;
				}
			}
			BOOST_ASSERT((shaderMap&& materials) || processIt->first == mGlobalShaderMapId);
			if (shaderMap && materials)
			{
				TArray<wstring> errors;
				ShaderMapFinalizeResults & compileResults = processIt->second;
				const TArray<ShaderCommonCompileJob*>& resultArray = compileResults.mFinishedJobs;
				TArray<FMaterial*> materialsArray = *materials;
				bool bSuccess = true;
				for (int32 jobIndex = 0; jobIndex < resultArray.size(); jobIndex++)
				{
					ShaderCommonCompileJob& currentJob = *resultArray[jobIndex];
					bSuccess = bSuccess && currentJob.bSucceeded;
					auto* singleJob = currentJob.getSingleShaderJob();
					if (singleJob)
					{
						checkSingleJob(singleJob, errors);
					}
					else
					{
						auto* pipelineJob = currentJob.getShaderPipelineJob();
						for (int32 index = 0; index < pipelineJob->mStageJobs.size(); ++index)
						{
							checkSingleJob(pipelineJob->mStageJobs[index]->getSingleShaderJob(), errors);
						}
					}
				}
				bool bShaderMapComplete = true;
				if (bSuccess)
				{
					bShaderMapComplete = shaderMap->processCompilationResults(resultArray, compileResults.mFinalizeJobIndex, timeBudget, compileResults.mShaderPipelines);
				}
				if (bShaderMapComplete)
				{
					shaderMap->bCompiledSuccessfully = bSuccess;
					localShaderMapReferences.add(shaderMap);
					MaterialShaderMap::mShaderMapsBeingCompiled.erase(shaderMap);
					for (int32 materialIndex = 0; materialIndex < materialsArray.size(); materialIndex++)
					{
						FMaterial* material = materialsArray[materialIndex];
						MaterialShaderMap* completedShaderMap = shaderMap;

						material->removeOutstandingCompileId(shaderMap->mCompilingId);

						if (material->getMaterialId() == completedShaderMap->getShaderMapId().mBaseMaterialId)
						{
							if (!bSuccess)
							{
								material->mCompileErrors = errors;
								materialsToUpdate.emplace(material, nullptr);
								if (material->isDefaultMaterial())
								{
									for (int32 errorIndex = 0; errorIndex < errors.size(); errorIndex++)
									{
										AIR_LOG(LogShaderCompiler, Warning, TEXT("	 %s"), errors[errorIndex].c_str());
									}
								}

								for (int32 errorIndex = 0; errorIndex < errors.size(); errorIndex++)
								{

								}
							}
							else
							{
								if (completedShaderMap->isComplete(material, true))
								{
									materialsToUpdate.emplace(material, completedShaderMap);
									if (compileResults.bApplyCompletedShaderMapForRendering)
									{
										materialsToApplyToScene.emplace(material, completedShaderMap);
									}
								}
								if (false)
								{

								}
							}
						}
					}
					for (int32 jobIndex = 0; jobIndex < resultArray.size(); jobIndex++)
					{
						delete resultArray[jobIndex];
					}
					processIt = compiledShaderMaps.erase(processIt);
					if (processIt == compiledShaderMaps.end())
					{
						break;
					}
				}
				if (timeBudget < 0)
				{
					break;
				}
			}
			else if (processIt->first == mGlobalShaderMapId)
			{
				auto globalShaderResults = compiledShaderMaps.find(mGlobalShaderMapId);
				if (globalShaderResults != compiledShaderMaps.end())
				{
					const TArray<ShaderCommonCompileJob*>& compilationResults = globalShaderResults->second.mFinishedJobs;
					processCompiledGlbalShaders(compilationResults);
					for (int32 resultIndex = 0; resultIndex < compilationResults.size(); resultIndex++)
					{
						delete compilationResults[resultIndex];
					}
					processIt = compiledShaderMaps.erase(processIt);
					if (processIt == compiledShaderMaps.end())
					{
						break;
					}
				}
			}
		}
		if (materialsToUpdate.size() > 0)
		{
			for (auto& it : materialsToUpdate)
			{
				FMaterial* material = it.first;
				MaterialShaderMap* shaderMap = it.second;
				BOOST_ASSERT(!shaderMap || shaderMap->isValidForRendering());
				material->setGameThreadShaderMap(it.second);
			}
			const TSet<SceneInterface*>& allocatedScenes = getRendererModule().getAllocatedScenes();
			for (auto& sceneIt : allocatedScenes)
			{
				sceneIt->setShaderMapsOnMaterialResources(materialsToApplyToScene);
			}

			for (auto& it : materialsToUpdate)
			{
				FMaterial* material = it.first;
				material->notifyCompilationFinished();
			}
			propagateMaterialChangesToPrimitives(materialsToUpdate);
		}
	}
	void ShaderCompilingManager::propagateMaterialChangesToPrimitives(const TMap<FMaterial*, class MaterialShaderMap*>& materialsToUpdate)
	{
		/*TArray<MaterialInterface*> usedMaterials;
		TindirectArray<ComponentRecreateRenderStateContext> componentContexts;
		for(to)*/
	}
	bool ShaderCompilingManager::isShaderCompilerWorkerRunning(ProcHandle& workerHandle)
	{
		return PlatformProcess::isProcRunning(workerHandle);
	}

	ShaderCompilingManager::ShaderCompilingManager()
		:bCompilingOuringGame(true),
		bAllowAsynchronousShaderCompiling(true),
		mNumOutstandingJobs(0),
#if PLATFORM_MAC
#else
		mShaderCompileWorkerName(TEXT("../"))
#endif
	{
		bAllowCompilingThroughWorkers = false;
		mWorkersBusyTime = 0;
		if (!PlatformProcess::supportsMultithreading())
		{
			bAllowCompilingThroughWorkers = false;
			bAllowAsynchronousShaderCompiling = false;
		}

		mProcessId = PlatformProcess::getCurrentProcessId();
		{
			Guid guid;
			guid = Guid::newGuid();
			wstring legacyShaderWorkingDirectory = Paths::gameIntermediateDir() + TEXT("Shaders/WorkingDirectory/") + boost::lexical_cast<wstring>(mProcessId) + TEXT("/");
			mShaderBaseWorkingDirectory = PlatformProcess::shaderWorkingDir() + guid.toString(EGuidFormats::Digits) + TEXT("/");

		}
		IFileManager::get().deleteDirectory(mShaderBaseWorkingDirectory.c_str(), false, true);
		wstring absoluteBaseDirectory = IFileManager::get().convertToAbsolutePathForExternalApplForWrite(mShaderBaseWorkingDirectory.c_str());
		Paths::normalizeDirectoryName(absoluteBaseDirectory);
		mAbsoluteShaderBaseWorkingDirectory = absoluteBaseDirectory + TEXT("/");

		wstring absoluteDebugInfoDirectory = IFileManager::get().convertToAbsolutePathForExternalApplForWrite((Paths::gameSaveDir() + TEXT("ShaderDebugInfo")).c_str());
		Paths::normalizeDirectoryName(absoluteDebugInfoDirectory);
		mAbsoluteShaderDebugInfoDirectory = absoluteDebugInfoDirectory + TEXT("/");

		const int32 numVirtualCores = PlatformMisc::numberOfCoresIncludingHyperthreads();
		mNumShaderCompilingThreads = bAllowCompilingThroughWorkers ? (numVirtualCores - 0) : 1;
		mNumShaderCompilingThreadsDuringGame = bAllowCompilingThroughWorkers ? (numVirtualCores - 0) : 1;

		if (numVirtualCores <= 4)
		{
			mNumShaderCompilingThreads = numVirtualCores - 1;
			mNumShaderCompilingThreadsDuringGame = numVirtualCores - 1;
		}

		mNumShaderCompilingThreads = Math::max<int32>(1, mNumShaderCompilingThreads);

		mNumShaderCompilingThreadsDuringGame = Math::max<int32>(1, mNumShaderCompilingThreadsDuringGame);

		mNumShaderCompilingThreadsDuringGame = Math::min<int32>(mNumShaderCompilingThreadsDuringGame, mNumShaderCompilingThreads);

		if (false)
		{


		}
		else
		{
			mThread = makeUniquePtr<ShaderCompileThreadRunnable>(this);
		}
		mThread->startThread();
	}

	ProcHandle ShaderCompilingManager::launchWorker(const wstring & workingDirectory, uint32 processId, uint32 threadId, const wstring & workerInputFile, const wstring & workerOutputFile)
	{
		wstring workerAbsoluteDirectory = IFileManager::get().convertToAbsolutePathForExternalApplForWrite(workingDirectory.c_str());
		Paths::normalizeDirectoryName(workerAbsoluteDirectory);
		wstring workerParameters = wstring(TEXT("\"")) + workerAbsoluteDirectory + TEXT("/\" ") + boost::lexical_cast<wstring>(processId) + boost::lexical_cast<wstring>(threadId) + TEXT(" ") + workerInputFile + TEXT(" ") + workerOutputFile;
		workerParameters += TEXT(" -communicatethroughfile ");
		if (GIsBuildMachine)
		{
			workerParameters += TEXT(" -buildmachine ");
		}

		uint32 workerId = 0; 
		ProcHandle workerHandle = PlatformProcess::createProc(mShaderCompileWorkerName.c_str(), workerParameters.c_str(), true, false, false, &workerId, -1, NULL, NULL);
		if (!workerHandle.isValid())
		{
			AIR_LOG(LogShaderCompilers, Fatal, TEXT("Couldn't launch %s! make sure the file is in your binaries folder."), mShaderCompileWorkerName.c_str());
		}
		return workerHandle;
	}

	void GlobalShaderTypeCompiler::beginCompileShaderPipeline(EShaderPlatform platform, const ShaderPipelineType* shaderPipeline, const TArray<GlobalShaderType*>& shaderStages, TArray<ShaderCommonCompileJob*>& newJobs)
	{
		BOOST_ASSERT(shaderStages.size() > 0);
		BOOST_ASSERT(shaderPipeline);
		AIR_LOG(LogShaders, Verbose, TEXT(" Pipeline: %s"), shaderPipeline->getName());

		auto* newPipelineJob = new ShaderPipelineCompileJob(mGlobalShaderMapId, shaderPipeline, shaderStages.size());
		for (int32 index = 0; index < shaderStages.size(); ++index)
		{
			auto* shaderStage = shaderStages[index];
			beginCompileShader(shaderStage, kUniquePermutationId, platform, shaderPipeline, newPipelineJob->mStageJobs);
		}
		newJobs.add(newPipelineJob);
	}

	ShaderCompileJob* GlobalShaderTypeCompiler::beginCompileShader(GlobalShaderType* shaderType, int32 permutationId, EShaderPlatform platform, const ShaderPipelineType* shaderPipeline, TArray<ShaderCommonCompileJob*>& newJobs)
	{
		ShaderCompileJob* newJob = new ShaderCompileJob(mGlobalShaderMapId, nullptr, shaderType, permutationId);
		ShaderCompilerEnvironment& shaderEnvironment = newJob->mInput.mEnvironment;
		AIR_LOG(LogShaders, Verbose, TEXT("  %s"), shaderType->getName());

		shaderType->setupCompileEnvironment(platform, permutationId, shaderEnvironment);
		static wstring GlobalName(TEXT("Global"));

		Air::globalBeginCompileShader(GlobalName, nullptr, shaderType, shaderPipeline, shaderType->getShaderFilename(), shaderType->getFunctionName(), ShaderTarget(shaderType->getFrequency(), platform), newJob, newJobs);
		return newJob;
	}

	Shader* GlobalShaderTypeCompiler::finishCompileShader(GlobalShaderType* shaderType, const ShaderCompileJob& compileJob, const ShaderPipelineType* shaderPipelineType)
	{
		Shader* shader = nullptr;
		if (compileJob.bSucceeded)
		{
			ShaderType* specificType = nullptr;
			int32 specificPermutationId = 0;
			if (compileJob.mShaderType->limitShaderResourceToThisType())
			{
				specificType = compileJob.mShaderType;
				specificPermutationId = compileJob.mPermutationId;
			}

			ShaderResource* resource = ShaderResource::findOrCreateShaderResource(compileJob.mOutput, specificType, specificPermutationId);

			BOOST_ASSERT(resource);

			if (shaderPipelineType && !shaderPipelineType->shoudlOptimizeUnusedOutputs(compileJob.mInput.mTarget.getPlatform()))
			{
				shaderPipelineType = nullptr;
			}

			SHAHash globalShaderMapHash;
			{
				SHA1 hashState;
				const TCHAR* globalShaderString = TEXT("GlobalShaderMap");
				hashState.updateWithString(globalShaderString, CString::strlen(globalShaderString));
				hashState.finalize();
				hashState.getHash(&globalShaderMapHash.mHash[0]);
			}

			shader = compileJob.mShaderType->findShaderById(ShaderId(globalShaderMapHash, shaderPipelineType, nullptr, compileJob.mShaderType, compileJob.mPermutationId, compileJob.mInput.mTarget));
			if (!shader)
			{
				shader = (*(shaderType->mConstructCompiledRef))(GlobalShaderType::CompiledShaderInitializerType(shaderType, compileJob.mPermutationId, compileJob.mOutput, resource, globalShaderMapHash, shaderPipelineType, nullptr));
				compileJob.mOutput.mParameterMap.verifyBindingAreComplete(shaderType->getName(), compileJob.mOutput.mTarget, compileJob.mVFType);
			}
		}

		if (compileJob.mOutput.mErrors.size() > 0)
		{
			if (compileJob.bSucceeded == false)
			{
				AIR_LOG(LogShaderCompilers, Error, TEXT("Errros compiling global shader %s %s %s:\n"), compileJob.mShaderType->getName(), shaderPipelineType ? TEXT("ShaderPipeline") : TEXT(""), shaderPipelineType ? shaderPipelineType->getName() : TEXT(""));
				for (int32 errorIndex = 0; errorIndex < compileJob.mOutput.mErrors.size(); errorIndex++)
				{
					AIR_LOG(LogShaderCompilers, Error, TEXT("		%s"), compileJob.mOutput.mErrors[errorIndex].getErrorString().c_str());
				}
			}
			else if(false)
			{

			}
		}

		return shader;
	}

}