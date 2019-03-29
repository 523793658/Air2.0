#pragma once
#include "SlateCore.h"
#include "Rendering/DrawElements.h"
namespace Air
{
	class SlateBatchData;
	class SlateRHIRenderingPolicy;
	class SLATE_CORE_API SlateElementBatcher
	{
	public:
		SlateElementBatcher(std::shared_ptr<SlateRHIRenderingPolicy> inRenderPolicy);

		void addElements(SlateWindowElementList& windowElementList);

		bool requirestVsync() const;

		void resetBatches();

	private:
		void addElements(const SlateWindowElementList& elementList, SlateDrawLayer& inDrawLayer);

	private:
		void addViewportElement(const SlateDrawElement& drawElement);
	private:
		SlateBatchData* mBatchData;

		SlateDrawLayer* mDrawLayer;

		SlateRHIRenderingPolicy* mRenderingPolicy;

		int32 numDrawnBatchesStat;
	};
}