#include "Widgets/SViewport.h"

namespace Air
{
	Reply SViewport::onKeyDown(const Geometry& myGeometry, const KeyEvent& inKeyEvent)
	{
		return !mViewportInterface.expired() ? mViewportInterface.lock()->onKeyDown(myGeometry, inKeyEvent) : Reply::unhandled();
	}

	Reply SViewport::onKeyUp(const Geometry& myGeometry, const KeyEvent& inKeyEvent)
	{
		return !mViewportInterface.expired() ? mViewportInterface.lock()->onKeyUp(myGeometry, inKeyEvent) : Reply::unhandled();
	}

	Reply SViewport::onMouseButtonDown(const Geometry& myGeometry, const PointerEvent& mouseEvent)
	{
		return !mViewportInterface.expired() ? mViewportInterface.lock()->onMouseButtonDown(myGeometry, mouseEvent) : Reply::unhandled();
	}

	Reply SViewport::onMouseButtonUp(const Geometry& myGeometry, const PointerEvent& mouseEvent)
	{
		return !mViewportInterface.expired() ? mViewportInterface.lock()->onMouseButtonUp(myGeometry, mouseEvent) : Reply::unhandled();
	}

	Reply SViewport::onMouseMove(const Geometry& myGeometry, const PointerEvent& mouseEvent)
	{
		return !mViewportInterface.expired() ? mViewportInterface.lock()->onMouseMove(myGeometry, mouseEvent) : Reply::unhandled();
	}

	Reply SViewport::onMouseWheel(const Geometry& myGeometry, const PointerEvent& mouseEvent)
	{
		return !mViewportInterface.expired() ? mViewportInterface.lock()->onMouseWheel(myGeometry, mouseEvent) : Reply::unhandled();
	}

	void SViewport::onMouseEnter(const Geometry& myGeometry, const PointerEvent& mouseEvent)
	{
		SCompoundWidget::onMouseEnter(myGeometry, mouseEvent);
		if (!mViewportInterface.expired())
		{
			mViewportInterface.lock()->onMouseEnter(myGeometry, mouseEvent);
		}
	}

	void SViewport::onMouseLeave(const PointerEvent& mouseEvent)
	{
		SCompoundWidget::onMouseLeave(mouseEvent);
		if (!mViewportInterface.expired())
		{
			mViewportInterface.lock()->onMouseLeave(mouseEvent);
		}
	}

	void SViewport::setActive(bool bActive)
	{

	}

	void SViewport::onFinishedPointerInput()
	{
		std::shared_ptr<ISlateViewport> pinnedInterface = mViewportInterface.lock();
		if (pinnedInterface)
		{
			pinnedInterface->onFinishedPointerInput();
		}
	}
}