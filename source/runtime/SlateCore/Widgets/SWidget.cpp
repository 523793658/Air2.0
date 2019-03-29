#include "Widgets/SWidget.h"
#include "Application/SlateApplicationBase.h"
namespace Air
{
	SWidget::SWidget()
	{
		int x = 2;
	}
	Reply SWidget::onKeyDown(const Geometry& myGeometry, const KeyEvent& inKeyEvent)
	{
		return Reply::unhandled();
	}

	Reply SWidget::onKeyUp(const Geometry& myGeometry, const KeyEvent& inKeyEvent)
	{
		return Reply::unhandled();
	}

	Reply SWidget::onMouseButtonDown(const Geometry& myGeometry, const PointerEvent& mouseEvent)
	{
		return Reply::unhandled();
	}

	Reply SWidget::onMouseButtonUp(const Geometry& myGeometry, const PointerEvent& mouseEvent)
	{
		return Reply::unhandled();
	}

	Reply SWidget::onMouseMove(const Geometry& myGeometry, const PointerEvent& mouseEvent)
	{
		return Reply::unhandled();
	}

	Reply SWidget::onMouseWheel(const Geometry& myGeometry, const PointerEvent& mouseEvent)
	{
		return Reply::unhandled();
	}

	void SWidget::onMouseLeave(const PointerEvent& mouseEvent)
	{
		
	}

	void SWidget::onMouseEnter(const Geometry& myGeometry, const PointerEvent& mouseEvent)
	{
	}

	void SWidget::onMouseCaptureLost()
	{

	}

	void SWidget::onFinishedPointerInput(){}

	bool SWidget::hasMouseCapture() const
	{
		return SlateApplicationBase::get().doesWidgetHaveMouseCapture(this->shared_from_this());
	}
}