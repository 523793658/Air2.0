#pragma once
#include "SlateCore.h"
#include "Input/ReplyBase.h"
#include "Misc/Optional.h"
#include "InputCoreType.h"
#include "Input/Events.h"
namespace Air
{
	class SLATE_CORE_API Reply : public TReplyBase<Reply>
	{
	public:


		static Reply unhandled()
		{
			return Reply(false);
		}

		static Reply handled()
		{
			return Reply(true);
		}

		Reply& preventThrottling()
		{
			this->bPreventThrottling = true;
			return me();
		}

		Reply& releaseMouseCapture();

		Reply& releaseMouseLock();

		Reply& setMousePos(const int2& newMousePos);

		Reply& setUserFocus(std::shared_ptr<SWidget> giveMeFocus, EFocusCause reasonFocusChanging, bool bInAllUsers);

		Reply& captureMouse(std::shared_ptr<SWidget> inMouseCapture);

		Reply& lockMouseToWidget(std::shared_ptr<SWidget> inWidget);

		Reply& useHighPrecisionMouseMovement(std::shared_ptr<SWidget> inMouseCaptor);

		const std::shared_ptr<SWidget>& getMouseCaptor() const { return mMouseCaptor; }

		bool shouldUseHighPrecisionMouse() const { return bUseHighPrecisionMouse; }

		const TOptional<int2> getRequestedMousePos() const {
			return mRequestedMousePos;
		}

		bool shouldReleaseMouse() const { return bReleaseMouseCapture; }

	private:
		Reply(bool bIsHandled)
			:TReplyBase<Reply>(bIsHandled)
			,mRequestedMousePos()
			,mEventHandler(nullptr)
			,mMouseCaptor(nullptr)
			,mFocusRecipient(nullptr)
			,mMouseLockWidget(nullptr)
			,bReleaseMouseCapture(false)
			,bSetUserFocus(false)
			,bReleaseUserFocus(false)
			,bAllUser(false)
			,bShouldReleaseMouseLock(false)
			,bUseHighPrecisionMouse(false)
			,bPreventThrottling(false)
			,bEndDragDrop(false)
		{

		}
	private:
		TOptional<int2> mRequestedMousePos;
		std::shared_ptr<SWidget> mEventHandler;
		std::shared_ptr<SWidget> mMouseCaptor;
		std::shared_ptr<SWidget> mFocusRecipient;
		std::shared_ptr<SWidget> mMouseLockWidget;
		std::shared_ptr<SWidget> mDetectDrawForWidget;

		Key mDetectDrawForMouseButton;

		uint32 bReleaseMouseCapture : 1;
		uint32 bSetUserFocus : 1;
		uint32 bReleaseUserFocus : 1;
		uint32 bAllUser : 1;
		uint32 bShouldReleaseMouseLock : 1;
		uint32 bUseHighPrecisionMouse : 1;
		uint32 bPreventThrottling : 1;
		uint32 bEndDragDrop : 1;

	};
}