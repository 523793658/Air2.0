#include "Rendering/ElementBatcher.h"
#include <map>
#include "Layout/SlateRect.h"
namespace Air
{
	SlateElementBatcher::SlateElementBatcher(std::shared_ptr<SlateRHIRenderingPolicy> inRenderPolicy)
		:mBatchData(nullptr)
		, mDrawLayer(nullptr)
		, mRenderingPolicy(inRenderPolicy.get())
		, numDrawnBatchesStat(0)
	{

	}


	void SlateElementBatcher::addElements(SlateWindowElementList& windowElementList)
	{
		mBatchData = &windowElementList.getBatchData();
		addElements(windowElementList, windowElementList.getRootDrawLayer());

		std::map<std::shared_ptr<SlateDrawLayerHandle>, std::shared_ptr<SlateDrawLayer>> & drawLayers = windowElementList.getChildDrawLayers();
		for (auto & entry : drawLayers)
		{
			addElements(windowElementList, *entry.second.get());
		}
		mBatchData = nullptr;
	}

	void SlateElementBatcher::addElements(const SlateWindowElementList& elementList, SlateDrawLayer& inDrawLayer)
	{
		mDrawLayer = &inDrawLayer;
		const TArray<SlateDrawElement>& drawElements = inDrawLayer.mDrawElements;
		for (int32 index = 0; index < drawElements.size(); ++index)
		{
			const SlateDrawElement& drawElement = drawElements[index];

			switch (drawElement.getElementType())
			{
			case SlateDrawElement::ET_Viewport:
				addViewportElement(drawElement);
				break;
			default:
				break;
			}
		}
	}

	void SlateElementBatcher::addViewportElement(const SlateDrawElement& drawElement)
	{
		//const 
	}

	void SlateElementBatcher::resetBatches()
	{

	}

	bool SlateElementBatcher::requirestVsync() const
	{
		return false;
	}
}