#pragma once
#include "SlateCore.h"
#include "Input/Events.h"
#include "Layout/Geometry.h"
#include "Input/Reply.h"
namespace Air
{
	class ISlateViewport
	{
	public:
		virtual Reply onKeyDown(const Geometry & myGeometry, const KeyEvent& inKeyEvent)
		{
			return Reply::unhandled();
		}

		virtual Reply onKeyUp(const Geometry & myGeometry, const KeyEvent& inKeyEvent)
		{
			return Reply::unhandled();
		}

		virtual Reply onMouseButtonDown(const Geometry& myGeometry, const PointerEvent& mouseEvent)
		{
			return Reply::unhandled();
		}

		virtual Reply onMouseButtonUp(const Geometry& myGeometry, const PointerEvent& mouseEvent)
		{
			return Reply::unhandled();
		}

		virtual Reply onMouseMove(const Geometry& myGeometry, const PointerEvent& mouseEvent)
		{
			return Reply::unhandled();
		}

		virtual Reply onMouseWheel(const Geometry& myGeometry, const PointerEvent& mouseEvent)
		{
			return Reply::unhandled();
		}

		virtual Reply onMouseButtonDoubleClick(const Geometry& inMyGeometry, const PointerEvent& inMouseEvent)
		{
			return Reply::unhandled();
		}

		virtual void onMouseEnter(const Geometry& myGeometry, const PointerEvent& mouseEvent)
		{

		}

		virtual void onMouseLeave(const PointerEvent& mouseEvent)
		{

		}


		virtual void onFinishedPointerInput()
		{

		}
		virtual ~ISlateViewport() {}
	};

	namespace ESlateBatchDrawFlag
	{
		typedef uint8 Type;

		const Type Wireframe = 1 << 3;
	}
}