#pragma once
#include "PostProcess/SceneRenderTargets.h"
#include "RHICommandList.h"
namespace Air
{
	struct PooledRenderTarget : public IPooledRenderTarget 
	{
		PooledRenderTarget(const PooledRenderTargetDesc& inDesc)
			:mNumRef(0)
			,mUnusedForNFrames(0)
			, mDesc(inDesc)
			,bSnapshot(false)
		{

		}
		PooledRenderTarget(const PooledRenderTarget& snaphotSource)
			:mNumRef(1)
			, mUnusedForNFrames(0)
			, mDesc(snaphotSource.mDesc)
			, bSnapshot(true)
		{
			BOOST_ASSERT(isInRenderingThread());
			mRenderTargetItem = snaphotSource.mRenderTargetItem;
		}
		virtual ~PooledRenderTarget()
		{
			BOOST_ASSERT(!mNumRef || (bSnapshot && mNumRef == 1));
			mRenderTargetItem.safeRelease();
		}
		bool isSnapshot() const
		{
			return bSnapshot;
		}

		uint32 getUnusedForNFrames() const
		{
			BOOST_ASSERT(!bSnapshot);
			return mUnusedForNFrames;
		}

		virtual uint32 AddRef() const override final;
		virtual uint32 Release() const override final;
		virtual uint32 GetRefCount() const override final;
		virtual bool isFree() const override final;
		virtual void setDebugName(const TCHAR* inName);
		virtual const PooledRenderTargetDesc& getDesc() const;
		virtual uint32 computeMemorySize() const;


	private:
		mutable int32 mNumRef;
		uint32 mUnusedForNFrames;
		PooledRenderTargetDesc mDesc;
		bool bSnapshot;
		bool onFrameStart();

		friend class RenderTargetPool;
	};


	class RenderTargetPool : public RenderResource
	{
	public:
		RenderTargetPool();

		void transitionTargetsWritable(RHICommandListImmediate& RHICmdList);

		void addPhaseEvent(const TCHAR* inPhaseName);

		void waitForTransitionFence();

		int32 findIndex(IPooledRenderTarget* in) const;

		void verifyAllocationLevel();

		void addAllocEvent(uint32 inPoolEntryId, PooledRenderTarget* In)
		{
			BOOST_ASSERT(In);
			
		}

		bool findFreeElement(RHICommandList& RHICmdList, const PooledRenderTargetDesc& desc, TRefCountPtr<IPooledRenderTarget>& out, const TCHAR* inDebugName, bool bDoWriteableBarrier = true);

		void tickPoolElements();

		void createUntrackedElement(const PooledRenderTargetDesc& desc, TRefCountPtr<IPooledRenderTarget>& out, const SceneRenderTargetItem& item);

		void freeUnusedResource(TRefCountPtr<IPooledRenderTarget>& inTarget);

		void freeUnusedResources();

	private:
		void compactPool();
	private:
		TArray<TRefCountPtr<PooledRenderTarget>> mPooledRenderTargets;
		TArray<TRefCountPtr<PooledRenderTarget>> mDeferredDeleteArray;
		TArray<RHITexture*> mTransitionTargets;

		TArray<PooledRenderTarget*> mPooledRenderTargetSnapshots;

		uint32 mAllocationLevelInKB{ 0 };
		GraphEventRef mTransitionFence;
		bool bCurrentlyOverBudget{ false };
		bool bStartEventRecordingNextTick{ false };
		uint32 mEventRecordingSizeThreshold{ 0 };
		bool bEventRecordingActive{ false };
		bool bEventRecordingStarted{ false };
		uint32 mCurrentEvnetRecorrdingTime{ 0 };
	};



	extern TGlobalResource<RenderTargetPool> GRenderTargetPool;
}