#include "PostProcess/RenderTargetPool.h"
#include "RHIUtilities.h"

namespace Air
{
	TGlobalResource<RenderTargetPool> GRenderTargetPool;

	RenderTargetPool::RenderTargetPool()
	{

	}

	void RenderTargetPool::addPhaseEvent(const TCHAR* inPhaseName)
	{

	}

	void RenderTargetPool::transitionTargetsWritable(RHICommandListImmediate& RHICmdList)
	{
		BOOST_ASSERT(isInRenderingThread());
		waitForTransitionFence();
		mTransitionTargets.clear();
		for (int32 i = 0; i < mPooledRenderTargets.size(); ++i)
		{
			PooledRenderTarget* pooledRT = mPooledRenderTargets[i];
			if (pooledRT && pooledRT->getDesc().bAutoWritable)
			{
				RHITexture* renderTarget = pooledRT->getRenderTargetItem().mTargetableTexture;
				if (renderTarget)
				{
					mTransitionTargets.push_back(renderTarget);
				}
			}
		}
		if (mTransitionTargets.size() > 0)
		{
			RHICmdList.transitionResourceArrayNoCopy(EResourceTransitionAccess::EWritable, mTransitionTargets);
			if (GRHIThread)
			{
				mTransitionFence = RHICmdList.RHIThreadFence(false);
			}
		}

	}

	static uint32 computeSizeInKB(PooledRenderTarget& element)
	{
		return (element.computeMemorySize() + 1023) / 1024;
	}

	bool RenderTargetPool::findFreeElement(RHICommandList& RHICmdList, const PooledRenderTargetDesc& desc, TRefCountPtr<IPooledRenderTarget>& out, const TCHAR* inDebugName, bool bDoWriteableBarrier /* = true */)
	{
		BOOST_ASSERT(isInRenderingThread());
		if (!desc.isValid())
		{
			return true;
		}
		if (out)
		{
			PooledRenderTarget* current = (PooledRenderTarget*)out.getReference();
			BOOST_ASSERT(!current->isSnapshot());
			const bool bExactMatch = true;
			if (out->getDesc().compare(desc, bExactMatch))
			{
				current->mDesc.mDebugName = inDebugName;
				RHIBindDebugLabelName(current->getRenderTargetItem().mTargetableTexture, inDebugName);
				BOOST_ASSERT(!out->isFree());
				return true;
			}
			else
			{
				out = 0;
				if (current->isFree())
				{
					mAllocationLevelInKB -= computeSizeInKB(*current);
					int32 index = findIndex(current);
					BOOST_ASSERT(index >= 0);
					mPooledRenderTargets[index] = 0;
					verifyAllocationLevel();
				}
			}
		}
		PooledRenderTarget* found = 0;
		uint32 foundIndex = -1;
		bool bReusingExistingTarget = false;

		{
			uint32 passCount = ((desc.mFlags & TexCreate_FastVRAM) && PlatformProperties::supportsFastVRAMMemory()) ? 2 : 1;
			for (uint32 pass = 0; pass < passCount; ++pass)
			{
				bool bExactMatch = (pass == 0);
				for (uint32 i = 0, num = (uint32)mPooledRenderTargets.size(); i < num; ++i)
				{
					PooledRenderTarget* element = mPooledRenderTargets[i];
					if (element && element->isFree() && element->getDesc().compare(desc, bExactMatch))
					{
						BOOST_ASSERT(!element->isSnapshot());
						found = element;
						foundIndex = i;
						bReusingExistingTarget = true;
						break;
					}
				}
			}
		}
		if (!found)
		{
			found = new PooledRenderTarget(desc);
			mPooledRenderTargets.push_back(found);
			BOOST_ASSERT(!(desc.mFlags & TexCreate_UAV));
			RHIResourceCreateInfo createInfo(desc.mClearValue);
			if (desc.mTargetableFlags & (TexCreate_RenderTargetable | TexCreate_DepthStencilTargetable | TexCreate_UAV))
			{
				if (desc.is2DTexture())
				{
					if (!desc.isArray())
					{
						RHICreateTargetableShaderResource2D(
							desc.mExtent.x,
							desc.mExtent.y,
							desc.mFormat,
							desc.mNumMips,
							desc.mFlags,
							desc.mTargetableFlags,
							desc.bForceSeparateTargetAndShaderResource,
							createInfo,
							(Texture2DRHIRef&)found->mRenderTargetItem.mTargetableTexture,
							(Texture2DRHIRef&)found->mRenderTargetItem.mShaderResourceTexture,
							desc.mNumSamples
						);
					}
					else
					{
						//RHICreateTargetableShaderResource2DArray()
					}
					if (GSupportsRenderTargetWriteMask && desc.bCreateRenderTargetWriteMask)
					{

					}
					if (desc.mNumMips > 1)
					{
						found->mRenderTargetItem.MipSRVs.setNum(desc.mNumMips);
						for (uint16 i = 0; i < desc.mNumMips; i++)
						{
							found->mRenderTargetItem.MipSRVs[i] = RHICreateShaderResourceView((Texture2DRHIRef&)found->mRenderTargetItem.mShaderResourceTexture, i);
						}
					}
				}
				else if (desc.is3DTexture())
				{

				}
				else
				{
					BOOST_ASSERT(desc.isCubemap());
					if (desc.isArray())
					{

					}
					else
					{
						RHICreateTargetableShaderResourceCube(
							desc.mExtent.x,
							desc.mFormat,
							desc.mNumMips,
							desc.mFlags,
							desc.mTargetableFlags,
							false,
							createInfo,
							(TextureCubeRHIRef&)found->mRenderTargetItem.mTargetableTexture,
							(TextureCubeRHIRef&)found->mRenderTargetItem.mShaderResourceTexture
						);
					}
				}
				RHIBindDebugLabelName(found->mRenderTargetItem.mTargetableTexture, inDebugName);
			}
			else
			{
				if (desc.is2DTexture())
				{
					found->mRenderTargetItem.mShaderResourceTexture = RHICreateTexture2D(desc.mExtent.x, desc.mExtent.y, desc.mFormat, desc.mNumMips, desc.mNumSamples, desc.mFlags, createInfo);
				}
				else if (desc.is3DTexture())
				{

				}
				else
				{

				}
				RHIBindDebugLabelName(found->mRenderTargetItem.mTargetableTexture, inDebugName);
			}
			if (desc.mTargetableFlags & TexCreate_UAV)
			{

			}
			mAllocationLevelInKB += computeSizeInKB(*found);
			verifyAllocationLevel();
			foundIndex = mPooledRenderTargets.size();
			found->mDesc.mDebugName = inDebugName;
			
		}
		BOOST_ASSERT(found->isFree());
		BOOST_ASSERT(!found->isSnapshot());
		found->mDesc.mDebugName = inDebugName;
		found->mUnusedForNFrames = 0;
		addAllocEvent(foundIndex, found);
		out = found;
		BOOST_ASSERT(!found->isFree());
		if (bReusingExistingTarget && bDoWriteableBarrier)
		{
			RHICmdList.transitionResource(EResourceTransitionAccess::EWritable, found->getRenderTargetItem().mTargetableTexture);
		}
		return false;
	}

	void RenderTargetPool::verifyAllocationLevel()
	{

	}

	int32 RenderTargetPool::findIndex(IPooledRenderTarget* In) const
	{
		BOOST_ASSERT(isInRenderingThread());
		if (In)
		{
			for (uint32 i = 0, num = (uint32)mPooledRenderTargets.size(); i < num; ++i)
			{
				const PooledRenderTarget* element = mPooledRenderTargets[i];
				if (element == In)
				{
					BOOST_ASSERT(!element->isSnapshot());
					return i;
				}
			}
		}
		return -1;
	}

	void RenderTargetPool::waitForTransitionFence()
	{
		BOOST_ASSERT(isInRenderingThread());
		if (mTransitionFence)
		{
			RHICommandListExecutor::waitOnRHIThreadFence(mTransitionFence);
			mTransitionFence = nullptr;
		}
		mTransitionTargets.clear();
		mDeferredDeleteArray.clear();
	}

	void RenderTargetPool::tickPoolElements()
	{
		BOOST_ASSERT(isInRenderingThread());
		waitForTransitionFence();
		if (bStartEventRecordingNextTick)
		{
			bStartEventRecordingNextTick = false;
			bEventRecordingStarted = true;
		}

		uint32 minmumPoolSizeInKB;
		{
			minmumPoolSizeInKB = 1536 * 1024;
		}

		compactPool();

		for (uint32 i = 0; i < (uint32)mPooledRenderTargets.size(); ++i)
		{
			PooledRenderTarget* element = mPooledRenderTargets[i];
			if (element)
			{
				BOOST_ASSERT(!element->isSnapshot());
				element->onFrameStart();
			}
		}
		while (mAllocationLevelInKB > minmumPoolSizeInKB)
		{
			int32 oldestElementIndex = -1;
			for (uint32 i = 0, num = (uint32)mPooledRenderTargets.size(); i < num; ++i)
			{
				PooledRenderTarget* element = mPooledRenderTargets[i];
				if (element && element->mUnusedForNFrames > 2)
				{
					if (oldestElementIndex != -1)
					{
						if (mPooledRenderTargets[oldestElementIndex]->mUnusedForNFrames < element->mUnusedForNFrames)
						{
							oldestElementIndex = i;
						}
					}
					else
					{
						oldestElementIndex = i;
					}
				}
			}
			if (oldestElementIndex != -1)
			{
				mAllocationLevelInKB -= computeSizeInKB(*mPooledRenderTargets[oldestElementIndex]);
				mPooledRenderTargets[oldestElementIndex] = nullptr;
				verifyAllocationLevel();
			}
			else
			{
				if (!bCurrentlyOverBudget)
				{
					bCurrentlyOverBudget = true;
				}
				break;
			}
		}
		if (mAllocationLevelInKB <= minmumPoolSizeInKB)
		{
			if (bCurrentlyOverBudget)
			{
				bCurrentlyOverBudget = false;
			}
		}
		addPhaseEvent(TEXT("FromLastFrom"));
		addPhaseEvent(TEXT("Rendering"));
	}

	bool PooledRenderTarget::isFree() const
	{
		uint32 refCount = GetRefCount();
		BOOST_ASSERT(refCount >= 1);
		return !bSnapshot && refCount == 1;
	}

	const PooledRenderTargetDesc& PooledRenderTarget::getDesc() const
	{
		return mDesc;
	}

	uint32 PooledRenderTarget::computeMemorySize() const
	{
		uint32 size = 0;
		if (!bSnapshot)
		{
			if (mDesc.is2DTexture())
			{
				size += RHIComputeMemorySize((const Texture2DRHIRef&)mRenderTargetItem.mTargetableTexture);
				if (mRenderTargetItem.mShaderResourceTexture != mRenderTargetItem.mTargetableTexture)
				{
					size += RHIComputeMemorySize((const Texture2DRHIRef&)mRenderTargetItem.mShaderResourceTexture);
				}
			}
			else if (mDesc.is3DTexture())
			{
			}
			else
			{

			}
		}
		return size;
	}
	void PooledRenderTarget::setDebugName(const TCHAR* inName)
	{
		mDesc.mDebugName = inName;
	}

	uint32 PooledRenderTarget::GetRefCount() const
	{
		return uint32(mNumRef);
	}
	uint32 PooledRenderTarget::Release() const
	{
		if (!bSnapshot)
		{
			BOOST_ASSERT(isInRenderingThread());
			uint32 refs = uint32(--mNumRef);
			if (refs == 0)
			{
				SceneRenderTargetItem& nonConstItem = (SceneRenderTargetItem&)mRenderTargetItem;
				nonConstItem.safeRelease();
				delete this;
			}
			return refs;
		}
		BOOST_ASSERT(mNumRef == 1);
		return 1;
	}

	uint32 PooledRenderTarget::AddRef() const
	{
		if (!bSnapshot)
		{
			BOOST_ASSERT(isInRenderingThread());
			return uint32(++mNumRef);
		}
		BOOST_ASSERT(mNumRef == 1);
		return 1;
	}

	void RenderTargetPool::createUntrackedElement(const PooledRenderTargetDesc& desc, TRefCountPtr<IPooledRenderTarget>& out, const SceneRenderTargetItem& item)
	{
		BOOST_ASSERT(isInRenderingThread());
		out = 0;
		PooledRenderTarget* found = new PooledRenderTarget(desc);
		found->mRenderTargetItem = item;
		BOOST_ASSERT(!found->isSnapshot());
		out = found;
	}

	void RenderTargetPool::compactPool()
	{
		for (uint32 i = 0, num = (uint32)mPooledRenderTargets.size(); i < num; ++i)
		{
			PooledRenderTarget* element = mPooledRenderTargets[i];
			if (!element)
			{
				mPooledRenderTargets.removeAtSwap(i);
				--num;
			}
		}
	}

	bool PooledRenderTarget::onFrameStart()
	{
		BOOST_ASSERT(isInRenderingThread() && !bSnapshot);
		if (!isFree())
		{
			BOOST_ASSERT(!mUnusedForNFrames);
			return false;
		}
		++mUnusedForNFrames;
		if (mUnusedForNFrames > 10)
		{
			return true;
		}
		return false;
	}

	void RenderTargetPool::freeUnusedResource(TRefCountPtr<IPooledRenderTarget>& inTarget)
	{
		BOOST_ASSERT(isInRenderingThread());
		int32 index = findIndex(inTarget);
		if (index != -1)
		{
			PooledRenderTarget* element = mPooledRenderTargets[index];
			if (element)
			{
				BOOST_ASSERT(!element->isSnapshot());
				mAllocationLevelInKB -= computeSizeInKB(*element);
				mDeferredDeleteArray.add(mPooledRenderTargets[index]);
				mPooledRenderTargets[index] = nullptr;
				inTarget.safeRelease();
				verifyAllocationLevel();
			}
		}
	}

	void RenderTargetPool::freeUnusedResources()
	{
		BOOST_ASSERT(isInRenderingThread());
		for (uint32 i = 0, num = (uint32)mPooledRenderTargets.size(); i < num; ++i)
		{
			PooledRenderTarget* element = mPooledRenderTargets[i];
			if (element && element->isFree())
			{
				BOOST_ASSERT(!element->isSnapshot());
				mAllocationLevelInKB -= computeSizeInKB(*element);
				mDeferredDeleteArray.add(mPooledRenderTargets[i]);
				mPooledRenderTargets[i] = nullptr;
			}
		}
		verifyAllocationLevel();
	}
}