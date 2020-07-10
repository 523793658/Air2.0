#include "MeshDrawCommands.h"
#include "sceneRendering.h"
namespace Air
{

	struct RHICommandUpdatePrimitiveIdBuffer : public RHICommand<RHICommandUpdatePrimitiveIdBuffer>
	{
		RHIVertexBuffer* mVertexBuffer;
		void* mVertexBufferData;
		int32 mVertexBufferDataSize;

		virtual ~RHICommandUpdatePrimitiveIdBuffer() {}

		FORCEINLINE_DEBUGGABLE RHICommandUpdatePrimitiveIdBuffer(
			RHIVertexBuffer* inVertexBuffer,
			void* inVertexBufferData,
			int32 inVertexBufferDataSize
		):mVertexBuffer(inVertexBuffer)
			,mVertexBufferData(inVertexBufferData)
			,mVertexBufferDataSize(inVertexBufferDataSize)
		{}

		void execute(RHICommandListBase& cmdList)
		{
			void* RESTRICT data = (void* RESTRICT)GDynamicRHI->RHILockVertexBuffer(mVertexBuffer, 0, mVertexBufferDataSize, RLM_WriteOnly);
			Memory::memcpy(data, mVertexBufferData, mVertexBufferDataSize);
			GDynamicRHI->RHIUnlockVertexBuffer(mVertexBuffer);

			Memory::free(mVertexBufferData);
		}

	};

	class DrawVisibleMeshCommandsAnyThreadTask : public RenderTask
	{
		RHICommandList& mRHICmdList;
		const MeshCommandOneFrameArray& mVisibleMeshDrawCommands;
		const GraphicsMinimalPipelineStateSet& mGraphicsMinimalPipelineStateSet;
		RHIVertexBuffer* mPrimitiveIdsBuffers;
		int32 mBasePrimitiveIdsOffset;
		bool bDynamicInstancing;
		uint32 mInstanceFactor;
		uint32 mTaskIndex;
		uint32 mTaskNum;
	public:
		DrawVisibleMeshCommandsAnyThreadTask(
			RHICommandList& inRHICmdList,
			const MeshCommandOneFrameArray& inVisibleMeshDrawCommands,
			const GraphicsMinimalPipelineStateSet& inGraphicsMinimalPipelineStateSet,
			RHIVertexBuffer* inPrimitiveIdsBuffers,
			int32 inBasePrimitiveIdsOffset,
			bool bInDynamicInstancing,
			uint32 inInstanceFactor,
			uint32 inTaskIndex,
			uint32 inTaskNum)
		:mRHICmdList(inRHICmdList)
		,mVisibleMeshDrawCommands(inVisibleMeshDrawCommands)
		,mGraphicsMinimalPipelineStateSet(inGraphicsMinimalPipelineStateSet)
		,mPrimitiveIdsBuffers(inPrimitiveIdsBuffers)
		,mBasePrimitiveIdsOffset(inBasePrimitiveIdsOffset)
		,bDynamicInstancing(bInDynamicInstancing)
		,mInstanceFactor(inInstanceFactor)
		,mTaskIndex(inTaskIndex)
		,mTaskNum(inTaskNum)
		{

		}

		static ESubsequentsMode::Type getSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

		void doTask(ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent)
		{
			BOOST_ASSERT(mRHICmdList.isInsideRenderPass());
			const int32 drawNum = mVisibleMeshDrawCommands.size();
			const int32 numDrawPerTask = Math::divideAndRoundUp<int32>(drawNum, mTaskNum);
			const int32 startIndex = mTaskIndex * numDrawPerTask;
			const int32 numDraws = Math::min(numDrawPerTask, drawNum - startIndex);
			submitMeshDrawCommandsRange(mVisibleMeshDrawCommands, mGraphicsMinimalPipelineStateSet, mPrimitiveIdsBuffers, mBasePrimitiveIdsOffset, bDynamicInstancing, startIndex, numDraws, mInstanceFactor, mRHICmdList);
			mRHICmdList.endRenderPass();
			mRHICmdList.handleRTThreadTaskCompletion(myCompletionGraphEvent);
		}
	};

	void ParallelMeshDrawCommandPass::dispatchDraw(ParallelCommandListSet* parallelCommandListSet, RHICommandList& RHICmdList)const
	{
		if (mMaxNumDraws <= 0)
		{
			return;
		}


		RHIVertexBuffer* primitiveIdsBuffer = mPrimitiveIdVertexBufferRHI;
		const int32 basePrimitiveIdsOffset = 0;
		if (parallelCommandListSet)
		{
			if (mTaskContext.bUseGPUScene)
			{
				RHICommandListImmediate& cmdList = getImmediateCommandList_ForRenderCommand();
				if (mTaskEventRef.isValid())
				{
					cmdList.addDispatchPrerequisite(mTaskEventRef);
				}

				new (cmdList.allocCommand<RHICommandUpdatePrimitiveIdBuffer>())RHICommandUpdatePrimitiveIdBuffer(
					mPrimitiveIdVertexBufferRHI,
					mTaskContext.mPrimitiveIdBufferData,
					mTaskContext.mPrimitiveIdBufferDataSize
				);
				cmdList.RHIThreadFence(true);
				bPrimitiveIdBufferDataOwnedByRHIThread = true;
			}
			const ENamedThreads::Type renderThread = ENamedThreads::getRenderThread();

			GraphEventArray prereqs;
			if (parallelCommandListSet->getPrereqs())
			{
				prereqs.append(*parallelCommandListSet->getPrereqs());
			}
			if (mTaskEventRef.isValid())
			{
				prereqs.add(mTaskEventRef);
			}
			const int32 numThreads = Math::min<int32>(TaskGraphInterface::get().getNumWorkerThreads(), parallelCommandListSet->mWidth);
			const int32 numTasks = Math::min<int32>(numThreads, Math::divideAndRoundUp(mMaxNumDraws, parallelCommandListSet->mMinDrawPerCommandList));
			const int32 numDrawsPerTask = Math::divideAndRoundUp(mMaxNumDraws, numThreads);

			for (int32 taskIndex = 0; taskIndex < numTasks; taskIndex++)
			{
				const int32 startIndex = taskIndex * numDrawsPerTask;
				const int32 numDraws = Math::min(numDrawsPerTask, mMaxNumDraws - startIndex);
				BOOST_ASSERT(numDraws > 0);
				RHICommandList* cmdList = parallelCommandListSet->newParallelCommandList();
				GraphEventRef anyThreadCompletionEvent = TGraphTask<DrawVisibleMeshCommandsAnyThreadTask>::createTask(&prereqs, renderThread).constructAndDispatchWhenReady(*cmdList, mTaskContext.mMeshDrawCommands, mTaskContext.mMinimalPipelineStatePassSet, primitiveIdsBuffer, basePrimitiveIdsOffset, mTaskContext.bDynamicInstancing, mTaskContext.mInstanceFactor, taskIndex, numTasks);
				parallelCommandListSet->addParallelCommandList(cmdList, anyThreadCompletionEvent, numDraws);
			}
		}
		else
		{
			waitForMeshPassSetupTask();
			if (mTaskContext.bUseGPUScene)
			{
				void* RESTRICT data = RHILockVertexBuffer(mPrimitiveIdVertexBufferRHI, 0, mTaskContext.mPrimitiveIdBufferDataSize, RLM_WriteOnly);
				Memory::memcpy(data, mTaskContext.mPrimitiveIdBufferData, mTaskContext.mPrimitiveIdBufferDataSize);
				RHIUnlockVertexBuffer(mPrimitiveIdVertexBufferRHI);
			}
			
			submitMeshDrawCommandsRange(mTaskContext.mMeshDrawCommands, mTaskContext.mMinimalPipelineStatePassSet, primitiveIdsBuffer, basePrimitiveIdsOffset, mTaskContext.bDynamicInstancing, 0, mTaskContext.mMeshDrawCommands.size(), mTaskContext.mInstanceFactor, RHICmdList);
		}
	}

	void ParallelMeshDrawCommandPass::waitForMeshPassSetupTask() const
	{
		if (mTaskEventRef.isValid())
		{
			TaskGraphInterface::get().waitUntilTaskCompletes(mTaskEventRef, ENamedThreads::getRenderThread_Local());
		}
	}
}