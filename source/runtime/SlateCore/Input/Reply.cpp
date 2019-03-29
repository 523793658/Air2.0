#include "Input/Reply.h"
namespace Air
{
	Reply& Reply::releaseMouseCapture()
	{
		this->mMouseCaptor.reset();
		this->bReleaseMouseCapture = true;
		return me();
	}

	Reply& Reply::releaseMouseLock()
	{
		this->bShouldReleaseMouseLock = true;
		mMouseLockWidget.reset();
		return me();
	}

	Reply& Reply::setMousePos(const int2& newMousePos)
	{
		this->mRequestedMousePos = newMousePos;
		return me();
	}

	Reply& Reply::setUserFocus(std::shared_ptr<SWidget> giveMeFocus, EFocusCause reasonFocusChanging, bool bInAllUsers)
	{
		this->bSetUserFocus = true;
		this->mFocusRecipient = giveMeFocus;
		this->bReleaseUserFocus = false;
		this->bAllUser = bInAllUsers;
		return me();
	}

	Reply& Reply::captureMouse(std::shared_ptr<SWidget> inMouseCapture)
	{
		this->mMouseCaptor = inMouseCapture;
		return me();
	}

	Reply& Reply::lockMouseToWidget(std::shared_ptr<SWidget> inWidget)
	{
		this->mMouseLockWidget = inWidget;
		this->bShouldReleaseMouseLock = false;
		return me();
	}

	Reply& Reply::useHighPrecisionMouseMovement(std::shared_ptr<SWidget> inMouseCaptor)
	{
		this->mMouseCaptor = inMouseCaptor;
		return me();
	}
}