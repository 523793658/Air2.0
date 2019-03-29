#include "Rendering/DrawElements.h"
#include <memory>
namespace Air
{
	


	void SlateWindowElementList::postDraw_ParallelThread()
	{
		BOOST_ASSERT(isInParallelRenderingThread);
		for (auto & entry : mDrawLayers)
		{
			entry.first->mBatchMap = nullptr;
		}
		for (auto& handle : mCachedRenderHandlesInUse)
		{
			handle->endUsing();
		}

		mCachedRenderHandlesInUse.clear();
	}

	void SlateWindowElementList::resetBuffers()
	{
		mRootDrawLayer.mDrawElements.clear();
		for (auto& entry : mDrawLayers)
		{
			TArray<SlateDrawElement>& drawElements = entry.second->mDrawElements;
			drawElements.clear();
			mDrawLayerPool.push_back(entry.second);
		}
		mDrawLayers.clear();
		mDrawStack.clear();
		mDrawStack.push_back(&mRootDrawLayer);
		mMemManager.flush();
	}


}