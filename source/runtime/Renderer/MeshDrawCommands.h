#pragma once
#include "MeshPassProcessor.h"
#include "SceneCore.h"

namespace Air
{
	class ViewInfo;

	class MeshDrawCommandPassSetupTaskContext
	{
	public:
		const ViewInfo* mView;
		EShadingPath mShadingPath;
		EMeshPass::Type mPassType;
		bool bUseGPUScene;
		bool bDynamicInstancing;
		bool bReverseCulling;
		bool bRenderSceneTwoSided;
		FExclusiveDepthStencil::Type mBasePassDepthStencilAccess;
		FExclusiveDepthStencil::Type mDefaultBasePassDepthStencilAccess;
		
		MeshPassProcessor* mMeshPassProcessor;

		void* mPrimitiveIdBufferData;
		int32 mPrimitiveIdBufferDataSize;
		MeshCommandOneFrameArray mMeshDrawCommands;
		MeshCommandOneFrameArray mMobileBasePassCSMMeshDrawCommands;

		TArray<const StaticMeshBatch*, SceneRenderingAllocator> mDynamicMeshCommandBuildRequests;
		TArray<const StaticMeshBatch*, SceneRenderingAllocator> mMobileBasePassCSMDynamicMeshCommandBuildRequests;
		DynamicMeshDrawCommandStorage mMeshDrawCommandStorage;
		GraphicsMinimalPipelineStateSet mMinimalPipelineStatePassSet;

		int32 mInstanceFactor;
		int32 mNumDynamicMeshElements;
		int32 mNumDynamicMeshCommandBuildRequestElements;


	};

	class ParallelCommandListSet;

	class ParallelMeshDrawCommandPass
	{

	public:
		void dispatchDraw(ParallelCommandListSet* parallelCommandListSet, RHICommandList& RHICmdList) const;

	private:
		RHIVertexBuffer* mPrimitiveIdVertexBufferRHI;

		MeshDrawCommandPassSetupTaskContext mTaskContext;

		GraphEventRef mTaskEventRef;

		mutable bool bPrimitiveIdBufferDataOwnedByRHIThread;

		int32 mMaxNumDraws;

		void waitForMeshPassSetupTask() const;
	};
}