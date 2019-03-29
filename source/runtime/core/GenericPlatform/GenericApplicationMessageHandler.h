#pragma once
#include "CoreType.h"
namespace Air
{
	class GenericWindow;
	namespace EWindowAction
	{
		enum Type
		{
			ClickedNonClientArea	= 1,
			Maximize				= 2,
			Resotre					= 3,
			WindowMenu				= 4,
		};
	}

	namespace EMouseButtons
	{
		enum Type
		{
			Left = 0,
			Middle,
			Right,
			Thumb01,
			Thumb02,
			Invalid,
		};
	}

	namespace EWindowActivation
	{
		enum Type
		{
			Activate = 0,
			ActivateByMouse,
			Deactivate,
		};
	}

	namespace EGestureEvent
	{
		enum Type
		{
			None,
			Scroll,
			Magnify,
			Swipe,
			Rotate,
			Count
		};
	}

	class GenericApplicationMessageHandler
	{
	public:
		virtual bool onWindowAction(const std::shared_ptr<GenericWindow>& window, const EWindowAction::Type inActionType)
		{
			return true;
		}

		virtual void onMovedWindow(const std::shared_ptr<GenericWindow>& window, const int32 x, int32 y)
		{

		}
		virtual void onOSPaint(const std::shared_ptr<GenericWindow>& window)
		{

		}
		virtual bool shouldProcessUserInputMessages(const std::shared_ptr<GenericWindow>& platformWindow) const
		{
			return false;
		}

		virtual bool onKeyChar(const TCHAR character, const bool isRepeat)
		{
			return false;
		}

		virtual bool onKeyDown(const int32 keyCode, const uint32 characterCode, const bool isRepeat)
		{
			return false;
		}

		virtual bool onKeyUp(const int32 keyCode, const uint32 characterCode, const bool isRepeat)
		{
			return false;
		}

		virtual bool onMouseDown(const std::shared_ptr<GenericWindow>& window, const EMouseButtons::Type btton)
		{
			return false;
		}

		virtual bool onMouseDown(const std::shared_ptr<GenericWindow>& window, const EMouseButtons::Type btton, const float2 cursorPos)
		{
			return false;
		}

		virtual bool onMouseUp(const EMouseButtons::Type botton)
		{
			return false;
		}

		virtual bool onMouseUp(const EMouseButtons::Type botton, const float2 cursorPos)
		{
			return false;
		}
		virtual bool onMouseDoubleClick(const std::shared_ptr<GenericWindow>& window, const EMouseButtons::Type button)
		{
			return false;
		}

		virtual bool onMouseDoubleClick(const std::shared_ptr<GenericWindow>& window, const EMouseButtons::Type button, const float2 cursorPos)
		{
			return false;
		}

		virtual bool onMouseWheel(const float delta)
		{
			return false;
		}

		virtual bool onMouseWheel(const float delta, float2 cursorPos)
		{
			return false;
		}

		virtual bool onRawMouseMove(const int32 X, const int32 Y)
		{
			return false;
		}

		virtual bool onMouseMove()
		{
			return false;
		}

		virtual bool onCursorSet()
		{
			return false;
		}

		virtual bool OnWindowActivationChanged(const std::shared_ptr<GenericWindow>& window, const EWindowActivation::Type activationType)
		{
			return false;
		}

		virtual bool onApplicationActivationChanged(const bool isActive)
		{
			return false;
		}

		virtual void onWindowClose(const std::shared_ptr<GenericWindow>& window)
		{

		}

		virtual bool onSizeChanged(const std::shared_ptr<GenericWindow>& window, const int32 width, const int32 height, bool bWasMinimized = false)
		{
			return false;
		}

		virtual void onResizingWindow(const std::shared_ptr<GenericWindow>& window)
		{

		}

		virtual bool beginReshapingWindow(const std::shared_ptr<GenericWindow>& window)
		{
			return true;
		}

		virtual void finishedReshapingWindow(const std::shared_ptr<GenericWindow>& window)
		{

		}
	};
}