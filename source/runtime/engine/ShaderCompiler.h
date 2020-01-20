#pragma once
#include "EngineMininal.h"
#include "Template/RefCounting.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "ShaderCore.h"
#include "HAL/PlatformProcess.h"
#include "Shader.h"
namespace Air

{
	class ShaderCompileJob;
	class ShaderPipelineCompileJob;
	class ShaderCompilingManager;
	struct ShaderCompileWorkerInfo;

	class ShaderCommonCompileJob : public RefCountedObject
	{
	public:
		uint32 mId;
		bool bFinalized;
		bool bSucceeded;
		bool bOptimizeForLowLatency;

		ShaderCommonCompileJob(uint32 inId)
			:mId(inId),
			bFinalized(false),
			bSucceeded(false),
			bOptimizeForLowLatency(false)
		{}

		virtual ShaderCompileJob* getSingleShaderJob() { return nullptr; }
		virtual const ShaderCompileJob* getSingleShaderJob() const { return nullptr; }
		virtual ShaderPipelineCompileJob* getShaderPipelineJob() { return nullptr; }
		virtual const ShaderPipelineCompileJob* getShaderPipelineJob() const { return nullptr; }
	};

	class ShaderCompileJob : public ShaderCommonCompileJob
	{
	public:
		VertexFactoryType* mVFType;
		ShaderType* mShaderType;
		int32 mPermutationId;
		ShaderCompilerInput mInput;
		ShaderCompilerOutput mOutput;
		TMap<const VertexFactoryType*, TArray<const ShaderPipelineType*>> mSharingPipelines;

		ShaderCompileJob(uint32 inId, VertexFactoryType* inVFType, ShaderType* inShaderType, int32 inPermutationId)
			:ShaderCommonCompileJob(inId)
			,mVFType(inVFType)
			,mShaderType(inShaderType)
			, mPermutationId(inPermutationId)
		{}
		virtual ShaderCompileJob* getSingleShaderJob() override { return this; }
		virtual const ShaderCompileJob* getSingleShaderJob() const override { return this; }
	};

	class ShaderPipelineCompileJob : public ShaderCommonCompileJob
	{
	public:
		TArray<ShaderCommonCompileJob*> mStageJobs;
		bool bFailedRemovingUnused;
		const ShaderPipelineType* mShaderPipeline;
		ShaderPipelineCompileJob(uint32 inId, const ShaderPipelineType* inShaderPipeline, int32 numStages)
			:ShaderCommonCompileJob(inId),
			bFailedRemovingUnused(false),
			mShaderPipeline(inShaderPipeline)
		{
			BOOST_ASSERT(inShaderPipeline && inShaderPipeline->getName());
			BOOST_ASSERT(numStages > 0);
			mStageJobs.empty(numStages);
		}

		~ShaderPipelineCompileJob()
		{
			for (int32 index = 0; index < mStageJobs.size(); ++index)
			{
				delete mStageJobs[index];
			}
			mStageJobs.clear();
		}

		virtual ShaderPipelineCompileJob* getShaderPipelineJob() override { return this; }
		virtual const ShaderPipelineCompileJob* getShaderPipelineJob() const override 
		{
			return this;
		}
	};


	class ShaderCompileThreadRunnableBase : public Runnable
	{
		friend class ShaderCompilingManager;
	protected:
		ShaderCompilingManager* mManager;
		RunnableThread* mThread;

		wstring mErrorMessage;
		bool bTerminatedByError;
		volatile bool bForceFinish;

	public:
		ShaderCompileThreadRunnableBase(ShaderCompilingManager* inManager);
		virtual ~ShaderCompileThreadRunnableBase() {}
		void startThread();

		virtual void stop() { bForceFinish = true; }
		virtual uint32 run();
		inline void waitForCompletion() const
		{
			if (mThread)
			{
				mThread->waitForCompletion();
			}
		}
		void checkHealth() const;
		virtual int32 compilingLoop() = 0;
	};

	class ShaderCompileThreadRunnable : public ShaderCompileThreadRunnableBase
	{
		friend class ShaderCompilingManager;
	private:
		TArray<ShaderCompileWorkerInfo*> mWorkerInfos;
		double mLastCheckForWorkersTime;

	public:
		ShaderCompileThreadRunnable(ShaderCompilingManager* inManager);
		virtual ~ShaderCompileThreadRunnable();

	private:
		int32 pullTaskFromQueue();
		void writeNewTasks();

		bool launchWorkersIfNeeded();

		void readAvailableResults();

		void compileDirectlyThroughDll();

		virtual int32 compilingLoop() override;
	};

	struct ShaderMapCompileResults
	{
		ShaderMapCompileResults()
		{
		
		}

		ShaderMapCompileResults(const ShaderMapCompileResults& rhs)
			:mNumJobsQueued(rhs.mNumJobsQueued),
			bAllJobsSucceeded(rhs.bAllJobsSucceeded),
			bApplyCompletedShaderMapForRendering(rhs.bApplyCompletedShaderMapForRendering),
			bRecreateComponentRenderStateOnCompletion(rhs.bRecreateComponentRenderStateOnCompletion),
			mFinishedJobs(rhs.mFinishedJobs)
		{

		}

		ShaderMapCompileResults(ShaderMapCompileResults && rhs)
			:mNumJobsQueued(rhs.mNumJobsQueued),
			bAllJobsSucceeded(rhs.bAllJobsSucceeded),
			bApplyCompletedShaderMapForRendering(rhs.bApplyCompletedShaderMapForRendering),
			bRecreateComponentRenderStateOnCompletion(rhs.bRecreateComponentRenderStateOnCompletion),
			mFinishedJobs(std::move(rhs.mFinishedJobs))
		{

		}

		ShaderMapCompileResults& operator = (const ShaderMapCompileResults& rhs)
		{
			mNumJobsQueued = rhs.mNumJobsQueued;
			bAllJobsSucceeded = rhs.bAllJobsSucceeded;
			bApplyCompletedShaderMapForRendering = rhs.bApplyCompletedShaderMapForRendering;
			bRecreateComponentRenderStateOnCompletion = rhs.bRecreateComponentRenderStateOnCompletion;
			mFinishedJobs = rhs.mFinishedJobs;
			return *this;
		}

		int32 mNumJobsQueued{ 0 };
		bool bAllJobsSucceeded{ true };
		bool bApplyCompletedShaderMapForRendering{ true };
		bool bRecreateComponentRenderStateOnCompletion{ false };

		TArray<ShaderCommonCompileJob*> mFinishedJobs;
	};

	struct ShaderMapFinalizeResults : ShaderMapCompileResults
	{
		int32 mFinalizeJobIndex;
		TMap<const VertexFactoryType*, TArray<const ShaderPipelineType*>> mShaderPipelines;

		ShaderMapFinalizeResults()
			:mFinalizeJobIndex(0)
		{

		}

		ShaderMapFinalizeResults(const ShaderMapCompileResults& inCompileResults) :
			ShaderMapCompileResults(inCompileResults),
			mFinalizeJobIndex(0)
		{}
	};

	class FMaterial;

	class ShaderCompilingManager
	{
		friend class ShaderCompileThreadRunnableBase;
		friend class ShaderCompileThreadRunnable;
		bool bCompilingOuringGame;
		TArray<ShaderCommonCompileJob*> mCompileQueue;
		TMap<int32, ShaderMapCompileResults> mShaderMapJobs;
		int32 mNumOutstandingJobs;
		std::mutex mCompileQueueSection;

		TMap<int32, ShaderMapFinalizeResults> mPendingFinalizeShaderMaps;
		std::unique_ptr<ShaderCompileThreadRunnableBase> mThread;
		uint32 mNumShaderCompilingThreads;
		uint32 mNumShaderCompilingThreadsDuringGame;
		int32 mMaxShaderJobsBatchSize{ 10 };
		uint32 mProcessId;
		bool bAllowCompilingThroughWorkers;
		bool bAllowAsynchronousShaderCompiling;

		double	mWorkersBusyTime{ 0 };

		wstring mAbsoluteShaderDebugInfoDirectory;
		wstring mShaderBaseWorkingDirectory;
		wstring mAbsoluteShaderBaseWorkingDirectory;
		wstring mShaderCompileWorkerName;

	public:

		ENGINE_API ShaderCompilingManager();


		const wstring & getAbsoluteShaderDebugInfoDirectory() const
		{
			return mAbsoluteShaderDebugInfoDirectory;
		}

		bool allowAsynchronousShaderCompiling() const
		{
			return bAllowAsynchronousShaderCompiling;
		}

		ENGINE_API void addJobs(TArray<ShaderCommonCompileJob*>& newJobs, bool bApplyCompletedShaderMapForRendering, bool bOptimizeForLowLatency, bool bRecreateComponentRenderStateOnCompletion);

		ENGINE_API void finishCompilation(const TCHAR* materialName, const TArray<int32>& shaderMapIdsToFinishCompiling);

		void blockOnShaderMapCompletion(const TArray<int32>& shaderMapIdsToFinishCompiling, TMap<int32, ShaderMapFinalizeResults>& compiledShaderMaps);

		bool handlePotentialRetryOnError(TMap<int32, ShaderMapFinalizeResults>& completedShaderMaps);

		void processCompiledShaderMap(TMap<int32, ShaderMapFinalizeResults>& compiledShaderMaps, float timeBudget);

		void propagateMaterialChangesToPrimitives(const TMap<FMaterial*, class MaterialShaderMap*>& materialsToUpdate);

		static bool isShaderCompilerWorkerRunning(ProcHandle& workerHandle);

		ProcHandle launchWorker(const wstring & workingDirectory, uint32 processId, uint32 threadId, const wstring & workerInputFile, const wstring & workerOutputFile);

		bool isCompiling() const
		{
			return mNumOutstandingJobs > 0 || mPendingFinalizeShaderMaps.size() > 0;
		}
	};

	class GlobalShaderTypeCompiler
	{
	public:
		ENGINE_API static class ShaderCompileJob* beginCompileShader(GlobalShaderType* shaderType, int32 permutationId, EShaderPlatform platform, const ShaderPipelineType* shaderPipeline, TArray<ShaderCommonCompileJob*>& newJobs);

		ENGINE_API static void beginCompileShaderPipeline(EShaderPlatform platform, const ShaderPipelineType* shaderPipeline, const TArray<GlobalShaderType*>& shaderStages, TArray<ShaderCommonCompileJob*>& newJobs);

		static Shader* finishCompileShader(GlobalShaderType* shaderType, const ShaderCompileJob& compileJob, const ShaderPipelineType* shaderPipelineType);
	};

	extern void globalBeginCompileShader(
		const wstring & debugGroupName,
		class VertexFactoryType* inVFType,
		class ShaderType* inShaderType,
		const class ShaderPipelineType* inShaderPipelineType,
		const TCHAR* inSourceFilename,
		const TCHAR* inFunctionName,
		ShaderTarget inTarget,
		ShaderCompileJob* inNewJob,
		TArray<ShaderCommonCompileJob*>& inNewJobs,
		bool bAllowDevelopmentShaderCompile = true
	);

	extern ENGINE_API ShaderCompilingManager* GShaderCompilingManager;

}