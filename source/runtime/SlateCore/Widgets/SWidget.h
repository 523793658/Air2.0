#pragma once
#include "SlateCore.h"
#include "Input/Reply.h"
#include "Input/Events.h"
#include "Layout/Geometry.h"
namespace Air
{
	class SLATE_CORE_API SWidget
		: public std::enable_shared_from_this<SWidget>
	{
	public:
		SWidget();

		virtual ~SWidget() {}

		virtual Reply onKeyDown(const Geometry& myGeometry, const KeyEvent& inKeyEvent);

		virtual Reply onKeyUp(const Geometry& myGeometry, const KeyEvent& inKeyEvent);

		virtual Reply onMouseButtonDown(const Geometry& myGeometry, const PointerEvent& mouseEvent);

		virtual Reply onMouseButtonUp(const Geometry& myGeometry, const PointerEvent& mouseEvent);

		virtual Reply onMouseMove(const Geometry& myGeometry, const PointerEvent& mouseEvent);

		virtual Reply onMouseWheel(const Geometry& myGeometry, const PointerEvent& mouseEvent);

		virtual void onMouseLeave(const PointerEvent& mouseEvent);

		virtual void onMouseEnter(const Geometry& myGeometry, const PointerEvent& mouseEvent);

		bool hasMouseCapture() const;

		virtual void onFinishedPointerInput();

		virtual void onMouseCaptureLost();
	};
}