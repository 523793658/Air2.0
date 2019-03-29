#include "windows/WindowsCursor.h"
#include "windows/WindowsHWrapper.h"
#include <windef.h>
namespace Air
{
	using namespace Windows;

	WindowsCursor::WindowsCursor()
	{
		for (int32 curCursorIndex = 0; curCursorIndex < EMouseCursor::TotalCursorCount; ++curCursorIndex)
		{
			mCursorHandles[curCursorIndex] = nullptr;
			Windows::HCURSOR cursorHandle = NULL;
			switch (curCursorIndex)
			{
			case EMouseCursor::None:
			case EMouseCursor::Custom:
				break;
			case EMouseCursor::Default:
				cursorHandle = ::LoadCursor(NULL, IDC_ARROW);
				break;
			case EMouseCursor::TextEditBeam:
				cursorHandle = ::LoadCursor(NULL, IDC_IBEAM);
				break;
			case EMouseCursor::ResizeLeftRight:
				cursorHandle = ::LoadCursor(NULL, IDC_SIZEWE);
				break;
			case EMouseCursor::ResizeUpDown:
				cursorHandle = ::LoadCursor(NULL, IDC_SIZENS);
				break;
			case EMouseCursor::ResizeSouthEast:
				cursorHandle = ::LoadCursor(NULL, IDC_SIZENWSE);
				break;
			case EMouseCursor::ResizeSouthWest:
				cursorHandle = ::LoadCursor(NULL, IDC_SIZENESW);
				break;
			case EMouseCursor::CardinalCross:
				cursorHandle = ::LoadCursor(NULL, IDC_SIZEALL);
				break;
			case EMouseCursor::Crosshairs:
				cursorHandle = ::LoadCursor(NULL, IDC_CROSS);
				break;
			case EMouseCursor::Hand:
				cursorHandle = ::LoadCursor(NULL, IDC_HAND);
				break;
			case EMouseCursor::GrabHand:
			case EMouseCursor::GrabHandClosed:
				cursorHandle = ::LoadCursor(NULL, IDC_HAND);
				break;

			case EMouseCursor::SlashedCircle:
				cursorHandle = ::LoadCursor(NULL, IDC_NO);
				break;
			case EMouseCursor::EyeDropper:
				break;
			default:
				BOOST_ASSERT(false);
				break;
			}
			mCursorHandles[curCursorIndex] = cursorHandle;
		}
	}
	WindowsCursor::~WindowsCursor()
	{
		for (int32 curCursorIndex = 0; curCursorIndex < EMouseCursor::TotalCursorCount; ++curCursorIndex)
		{
			switch (curCursorIndex)
			{
			case EMouseCursor::None:
			case EMouseCursor::Default:
			case EMouseCursor::TextEditBeam:
			case EMouseCursor::ResizeLeftRight:
			case EMouseCursor::ResizeUpDown:
			case EMouseCursor::ResizeSouthEast:
			case EMouseCursor::ResizeSouthWest:
			case EMouseCursor::CardinalCross:
			case EMouseCursor::Crosshairs:
			case EMouseCursor::Hand:
			case EMouseCursor::GrabHand:
			case EMouseCursor::GrabHandClosed:
			case EMouseCursor::SlashedCircle:
			case EMouseCursor::EyeDropper:
			case EMouseCursor::Custom:
				break;
			default:
				BOOST_ASSERT(0);
				break;
			}
		}
	}

	float2 WindowsCursor::getPosition() const
	{
		POINT cursorPos;
		::GetCursorPos(&cursorPos);
		return float2(cursorPos.x, cursorPos.y);
	}



	void WindowsCursor::setPosition(const int32 x, const int32 y)
	{
		::SetCursorPos(x, y);
	}


	void WindowsCursor::setType(const EMouseCursor::Type inNewCursr)
	{
		BOOST_ASSERT(inNewCursr < EMouseCursor::TotalCursorCount);
		mCurrentType = inNewCursr;
		::SetCursor(mCursorHandles[mCurrentType]);
	}



	void WindowsCursor::getSize(int32& width, int32& height) const
	{
		width = 16;
		height = 16;
	}


	void WindowsCursor::show(bool bShow)
	{
		if (bShow)
		{
			while (::ShowCursor(true) < 0);
		}
		else
		{
			while (::ShowCursor(false) >= 0);
		}
	}

	void WindowsCursor::lock(const RECT* const bounds)
	{
		::ClipCursor(bounds);
	}

	void WindowsCursor::setCustomShape(void* inCursorHandle)
	{
		Windows::HCURSOR cursorHandle = (Windows::HCURSOR)inCursorHandle;
		mCursorHandles[EMouseCursor::Custom] = cursorHandle;
	}
}