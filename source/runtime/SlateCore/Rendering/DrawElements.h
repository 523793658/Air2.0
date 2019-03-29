#pragma once
#include "SlateCore.h"
#include "Widgets/SWindow.h"
#include "Misc/MemStack.h"
#include <map>
#include <array>
namespace Air
{
	class SlateDrawElement
	{
	public:
		enum EElementType
		{
			ET_Viewport,
		};
		FORCEINLINE EElementType getElementType() const { return mType; }

	private:

		EElementType mType;
	};


	class SlateElementBatch
	{};


	typedef TArray<SlateElementBatch> ElementBatchArray;

	class ElementBatchMap
	{
	public:
		TArray<uint32> mActiveLayers;
		std::array< std::unique_ptr<ElementBatchArray>, 256> mLayers;
		std::map<uint32, std::unique_ptr<ElementBatchArray>> mOverflowLayers;
		uint32 mMinLayer;
		uint32 mMaxLayer;
	};


	class SlateDrawLayerHandle : public std::enable_shared_from_this<SlateDrawLayerHandle>
	{
	public:
		SlateDrawLayerHandle()
			:mBatchMap(nullptr)
		{}

		ElementBatchMap* mBatchMap;
	};

	class SlateDrawLayer
	{
	public:
		ElementBatchMap& getElementBatchMap()
		{
			return mLayerToElementBatches;
		}
	public:
		ElementBatchMap mLayerToElementBatches;

		TArray<SlateDrawElement> mDrawElements;
	};

	class SLATE_CORE_API SlateRenderDataHandle : public std::enable_shared_from_this<SlateRenderDataHandle>
	{
	public:

		void endUsing() { PlatformAtomics::interLockedDecrement(&mUsageCount); }
		void beginUsing() { PlatformAtomics::interlockedIncrement(&mUsageCount); }

		bool isInUse() const { return mUsageCount > 0; }
	private:
		volatile int32 mUsageCount;
	};
	class SlateBatchData
	{

	};

	class SlateWindowElementList
	{
		friend class SlateElementBatcher;
	public:
		explicit SlateWindowElementList(std::shared_ptr<SWindow> inWindow = std::shared_ptr<SWindow>())
			:mTopLevelWindow(inWindow)
		{
			mDrawStack.push_back(&mRootDrawLayer);
		}


		SLATE_CORE_API void postDraw_ParallelThread();

		FORCEINLINE std::shared_ptr<SWindow> getWindow() const
		{
			return mTopLevelWindow.lock();
		}

		SLATE_CORE_API void resetBuffers();

		SlateBatchData& getBatchData() { return mBatchData; }
		SlateDrawLayer& getRootDrawLayer() { return mRootDrawLayer; }

		std::map<std::shared_ptr<SlateDrawLayerHandle>, std::shared_ptr<SlateDrawLayer>>& getChildDrawLayers() { return mDrawLayers; }

	public:
		std::shared_ptr<SlateDrawLayerHandle> mLayerHandle;
	private:
		std::map<std::shared_ptr<SlateDrawLayerHandle>, std::shared_ptr<SlateDrawLayer>> mDrawLayers;

		TArray < std::shared_ptr<SlateRenderDataHandle>> mCachedRenderHandlesInUse;

		std::weak_ptr<SWindow> mTopLevelWindow;

		TArray<SlateDrawLayer*> mDrawStack;

		TArray<std::shared_ptr<SlateDrawLayer>> mDrawLayerPool;

		SlateDrawLayer mRootDrawLayer;

		MemStackBase mMemManager;

		SlateBatchData mBatchData;
	};


}