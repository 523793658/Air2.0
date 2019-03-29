#include "SlateApplication.h"
#include "GenericPlatform/GenericApplication.h"
#include "Misc/App.h"
#include "Layout/WidgetPath.h"
#include "HAL/PlatformTime.h"
#include "HAL/PlatformMisc.h"
#include "Template/SharePointer.h"
#include "InputCoreType.h"
#include "Input/Events.h"
#include "Application/SlateWondowHelper.h"
#include "GenericPlatform/ICursor.h"
#include "Slate/SceneViewport.h"
namespace Air
{
	std::shared_ptr<SlateApplication> SlateApplication::mCurrentApplication;

	SlateApplication::SlateApplication()
		:mSynthesizeMouseMovePending(0)
		, bAppIsActive(true)
		, bSlateWindowActive(true)
		, mScale(1.0f)
		, mCursorRadius(0.0f)
		, mLastMouseMoveTime (0.0)
		, mCurrentTime(PlatformTime::seconds())
		, mLastTickTime(0.0)
		, mAverageDeltaTime(1.0f / 30.f)
	{
		mPointerIndexLastPositionMap.emplace(CursorPointerIndex, float2::zero());
	}


	std::shared_ptr<SWindow> SlateApplication::addWindow(std::shared_ptr<SWindow> inSlateWindow, const bool bShowImmediately /* = true */)
	{
		mSlateWindows.push_back(inSlateWindow);
		std::shared_ptr<GenericWindow> NewWindow = makeWindow(inSlateWindow, bShowImmediately);
		if (bShowImmediately)
		{
			inSlateWindow->showWindow();
		}
		return inSlateWindow;
	}

	std::shared_ptr<SWindow> SlateApplication::getWindow(int32 index)
	{
		return mSlateWindows[index];
	}

	std::shared_ptr<GenericWindow> SlateApplication::makeWindow(std::shared_ptr<SWindow> inSlateWindow, const bool bShowImmediately)
	{
		std::shared_ptr<GenericWindow> nativeParent = nullptr;
		std::shared_ptr<SWindow> parentWindow = inSlateWindow->getParentWindow();
		if (parentWindow)
		{
			nativeParent = parentWindow->getNativeWindow();
		}
		shared_ptr<GenericWindowDefinition> definition = MakeSharedPtr<GenericWindowDefinition>(inSlateWindow->getWindowsDesc());

		std::shared_ptr<GenericWindow> newWindow = mPlatformApplication->makeWindow();
		if (App::canEverRender())
		{
			inSlateWindow->setNativeWindow(newWindow);

			inSlateWindow->setCachedScreenPosition(inSlateWindow->getWindowsDesc().DesiredPositionOnScree);

			inSlateWindow->setCachedSize(inSlateWindow->getWindowsDesc().SizeDesiredOnScreen);

			mPlatformApplication->initializeWindow(newWindow, definition, nativeParent, bShowImmediately);
		}
		else
		{
			inSlateWindow->setNativeWindow(MakeSharedPtr<GenericWindow>());
		}
		return newWindow;
	}

	void SlateApplication::tickApplication(float deltaTime)
	{
		mLastTickTime = mCurrentTime;
		mCurrentTime = PlatformTime::seconds();

		{
			const float runningAverageScale = 0.1f;
			mAverageDeltaTime = mAverageDeltaTime * (1.0f - runningAverageScale) + getDeltaTime() * runningAverageScale;

		}
		const double maxQuantumBeforeClamp = 1.0 / 8.0;
		if (getDeltaTime() > maxQuantumBeforeClamp)
		{
			mLastTickTime = mCurrentTime - maxQuantumBeforeClamp;
		}


		{
			drawWindows();
		}
	}

	void SlateApplication::tick()
	{
		const float deltaTime = getDeltaTime();
		
		tickApplication(deltaTime);
	}

	void SlateApplication::create()
	{
		GenericApplication* app = PlatformMisc::createApplication();
		std::shared_ptr<GenericApplication> appPtr = std::shared_ptr<GenericApplication>(app);
		create(appPtr);
	}

	void SlateApplication::drawWindows()
	{
		privateDrawWindows();
	}

	struct DrawWindowArgs
	{
		DrawWindowArgs(SlateDrawBuffer& inDrawBuffer, const WidgetPath& inWidgetsUnderCursor)
			:mOutDrawBuffer(inDrawBuffer),
			mWidgetsUnderCursor(inWidgetsUnderCursor)
		{}

		SlateDrawBuffer& mOutDrawBuffer;
		const WidgetPath& mWidgetsUnderCursor;
	};

	void SlateApplication::drawWindowAndChildren(const std::shared_ptr<SWindow>& windowToDraw, DrawWindowArgs& drawWindowArgs)
	{
		bool bDrawChildWindow = false;
		if (windowToDraw->isVisible() && (!windowToDraw->isWindowMinimized()))
		{
			SlateWindowElementList& windowElementList = drawWindowArgs.mOutDrawBuffer.addWindowElementList(windowToDraw);
		}
	}

	void SlateApplication::privateDrawWindows(std::shared_ptr<SWindow> drawOnlyThisWindow /* = nullptr */)
	{
		BOOST_ASSERT(mRenderer);
		{
		}
		DrawWindowArgs drawWindowArgs(mRenderer->getDrawBuffer(), WidgetPath());
		for (auto& it : mSlateWindows)
		{
			if (it)
			{
				drawWindowAndChildren(it, drawWindowArgs);
			}
		}

		mRenderer->drawWindows(drawWindowArgs.mOutDrawBuffer);

	}

	
	std::shared_ptr<SlateApplication> SlateApplication::create(const std::shared_ptr<class GenericApplication>& inPlatformApplication)
	{
		EKeys::initialize();
		mCurrentApplication = MakeSharedPtr<SlateApplication>();
		mCurrentBaseApplication = mCurrentApplication;
		mPlatformApplication = inPlatformApplication;
		mPlatformApplication->setMessageHandler(mCurrentApplication);

		return mCurrentApplication;
	}

	bool SlateApplication::initializeRenderer(std::shared_ptr<SlateRenderer> inRenderer, bool bQuietMode)
	{
		mRenderer = inRenderer;
		bool result = mRenderer->initialize();
		if (!result && !bQuietMode)
		{

		}
		return result;
	}


	bool SlateApplication::onKeyDown(const int32 keyCode, const uint32 characterCode, const bool isRepeat)
	{
		Key const key = InputKeyManager::get().getKeyFromCodes(keyCode, characterCode);
		KeyEvent keyEvent(key, mPlatformApplication->getModifierKeys(), 0, isRepeat, characterCode, keyCode);
		return processKeyDownEvent(keyEvent);
	}

	bool SlateApplication::onKeyUp(const int32 keyCode, const uint32 characterCode, const bool isRepeat)
	{
		Key const key = InputKeyManager::get().getKeyFromCodes(keyCode, characterCode);
		KeyEvent keyEvent(key, mPlatformApplication->getModifierKeys(), 0, isRepeat, characterCode, keyCode);
		return processKeyUpEvent(keyEvent);
	}

	bool SlateApplication::onMouseMove()
	{
		bool result = true;
		const int2 currentCursorPosition = getCursorPos();
		const int2 lastCursorPosition = getLastCursorPos();
		if (lastCursorPosition != currentCursorPosition)
		{
			mLastMouseMoveTime = getCurrentTime();

			PointerEvent mouseEvent(CursorPointerIndex, currentCursorPosition, lastCursorPosition, mPressedMouseButtons, EKeys::Invalid, 0, mPlatformApplication->getModifierKeys());
		
			result = processMouseMoveEvent(mouseEvent);
		}
		return result;
	}

	Key translateMouseButtonToKey(const EMouseButtons::Type button)
	{
		Key key = EKeys::Invalid;
		switch (button)
		{
		case EMouseButtons::Left:
			key = EKeys::LeftMouseButton;
			break;
		case EMouseButtons::Middle:
			key = EKeys::MiddleMouseButton;
			break;
		case EMouseButtons::Right:
			key = EKeys::RightMouseButton;
			break;
		case EMouseButtons::Thumb01:
			key = EKeys::ThumbMouseButton;
			break;
		case EMouseButtons::Thumb02:
			key = EKeys::ThumbMouseButton2;
			break;
		}
		return key;
	}

	bool SlateApplication::onMouseDown(const std::shared_ptr<GenericWindow>& window, const EMouseButtons::Type btton)
	{
		return onMouseDown(window, btton, getCursorPos());
	}
	bool SlateApplication::onMouseDown(const std::shared_ptr<GenericWindow>& window, const EMouseButtons::Type btton, const float2 cursorPos)
	{
		Key key = translateMouseButtonToKey(btton);
		PointerEvent mouseEvent(
			CursorPointerIndex,
			cursorPos,
			getLastCursorPos(),
			mPressedMouseButtons,
			key,
			0,
			mPlatformApplication->getModifierKeys()
		);
		return processMouseButtonDownEvent(window, mouseEvent);
	}

	bool SlateApplication::onMouseUp(const EMouseButtons::Type botton)
	{
		return onMouseUp(botton, getCursorPos());
	}

	bool SlateApplication::onMouseUp(const EMouseButtons::Type botton, const float2 cursorPos)
	{
		Key key = translateMouseButtonToKey(botton);
		PointerEvent mouseEvent(
			CursorPointerIndex,
			cursorPos,
			getLastCursorPos(),
			mPressedMouseButtons,
			key,
			0,
			mPlatformApplication->getModifierKeys());
		return processMouseButtonUpEvent(mouseEvent);
	}

	void SlateApplication::queueSynthesizedMouseMove()
	{
		mSynthesizeMouseMovePending = 2;
	}

	bool SlateApplication::processMouseMoveEvent(PointerEvent& mouseEvent, bool bIsSynthetic /* = false */)
	{
		if (!bIsSynthetic)
		{
			queueSynthesizedMouseMove();
			const bool allowSpawningOfToolTips = true;
		}

		const bool bOverSlateWindow = !bIsSynthetic || mPlatformApplication->isCursorDirectlyOverSlateWindow();
		if (bOverSlateWindow)
		{
			Reply reply = Reply::unhandled();
			reply = mSlateWindows[0]->onMouseMove(mSlateWindows[0]->getWindowGeometryInScreen(), mouseEvent);
			mPointerIndexLastPositionMap[mouseEvent.getPointerIndex()] = mouseEvent.getScreenSpacePosition();
			return reply.isEventHandled();
		}
		return false;
	}

	bool SlateApplication::processMouseButtonUpEvent(PointerEvent& inMouseEvent)
	{
		queueSynthesizedMouseMove();
		setLastUserInteractionTime(this->getCurrentTime());
		mLastUserInteractionTimeForThrottling = mLastUserInteractionTime;
		mPressedMouseButtons.remove(inMouseEvent.getEffectingButton());
		Reply reply = mSlateWindows[0]->onMouseButtonUp(mSlateWindows[0]->getWindowGeometryInScreen(), inMouseEvent);

		processReply(reply, &*mSlateWindows[0], &inMouseEvent);

		const bool bHanded = reply.isEventHandled();

		if (mPressedMouseButtons.size() == 0)
		{
			mPlatformApplication->setCapture(nullptr);
		}
		return bHanded;
	}
	bool SlateApplication::isDragDropping()const
	{
		return false;
	}

	bool SlateApplication::processMouseButtonDownEvent(const std::shared_ptr<GenericWindow>& platformWindow, PointerEvent& inMouseEvent)
	{
		queueSynthesizedMouseMove();
		setLastUserInteractionTime(this->getCurrentTime());
		mLastUserInteractionTimeForThrottling = mLastUserInteractionTime;
		if (platformWindow)
		{
			mPlatformApplication->setCapture(platformWindow);
		}
		mPressedMouseButtons.add(inMouseEvent.getEffectingButton());
		bool bInGame = false;

		if (!isDragDropping())
		{
			Reply reply = Reply::unhandled();
			if (mMouseCaptor.hasCaptureForPointerIndex(inMouseEvent.getUserIndex(), inMouseEvent.getPointerIndex()))
			{
				std::shared_ptr<SWidget> mouseCaptor = mMouseCaptor.toWidgetPath(&inMouseEvent);
				bInGame = App::isGame();
				reply = mSlateWindows[0]->onMouseButtonDown(mSlateWindows[0]->getWindowGeometryInScreen(), inMouseEvent);
			}
			else
			{
				reply = mSlateWindows[0]->onMouseButtonDown(mSlateWindows[0]->getWindowGeometryInScreen(), inMouseEvent);
			}
			processReply(reply, &*mSlateWindows[0], &inMouseEvent);
		}
		mPointerIndexLastPositionMap.emplace(inMouseEvent.getPointerIndex(), inMouseEvent.getScreenSpacePosition());
		return true;
	}

	bool SlateApplication::MouseCaptureHelper::hasCaptureForPointerIndex(uint32 userIndex, uint32 pointerIndex) const
	{
		auto & it = mPointerIndexToMouseCaptorWeakMap.find(UserAndPointer(userIndex, pointerIndex));
		return it != mPointerIndexToMouseCaptorWeakMap.end() && !it->second.expired();
	}
	std::shared_ptr<SWidget> SlateApplication::MouseCaptureHelper::toWidgetPath(const PointerEvent* pointerEvent)
	{
		auto& it = mPointerIndexToMouseCaptorWeakMap.find(UserAndPointer(pointerEvent->getUserIndex(), pointerEvent->getPointerIndex()));
		if (it != mPointerIndexToMouseCaptorWeakMap.end() && !it->second.expired())
		{
			return it->second.lock();
		}
		return std::shared_ptr<SWidget>();
	}

	void SlateApplication::setLastUserInteractionTime(double inCurrentTime)
	{
		if (mLastUserInteractionTime != inCurrentTime)
		{
			mLastUserInteractionTime = inCurrentTime;
		}
	}

	float2 SlateApplication::getCursorPos() const
	{
		if (mPlatformApplication->mCursor)
		{
			return mPlatformApplication->mCursor->getPosition();
		}
		return float2(0, 0);
	}

	float2 SlateApplication::getLastCursorPos() const
	{
		return mPointerIndexLastPositionMap.at(CursorPointerIndex);
	}

	void SlateApplication::setCursorPos(const float2& mouseCoordinate)
	{
		if (mPlatformApplication->mCursor)
		{
			return mPlatformApplication->mCursor->setPosition(mouseCoordinate.x, mouseCoordinate.y);
		}
	}

	float2 SlateApplication::getCursorSize() const
	{
		if (mPlatformApplication->mCursor)
		{
			int32 x;
			int32 y;
			mPlatformApplication->mCursor->getSize(x, y);
			return float2(x, y);
		}
		return float2(1, 1);
	}



	bool SlateApplication::processKeyDownEvent(KeyEvent& inKeyEvent)
	{
		//事件路由，暂未实现
		Reply reply = Reply::unhandled();
		reply = mSlateWindows[0]->onKeyDown(mSlateWindows[0]->getWindowGeometryInScreen(), inKeyEvent);
		return reply.isEventHandled();
	}

	bool SlateApplication::processKeyUpEvent(KeyEvent& inKeyEvent)
	{
		Reply reply = Reply::unhandled();
		reply = mSlateWindows[0]->onKeyUp(mSlateWindows[0]->getWindowGeometryInScreen(), inKeyEvent);
		return reply.isEventHandled();
	}

	bool SlateApplication::shouldProcessUserInputMessages(const std::shared_ptr<GenericWindow>& platformWindow) const
	{
		std::shared_ptr<SWindow> window;
		if (platformWindow)
		{
			window = SlateWindowHelper::findWindowByPlatformWindow(mSlateWindows, platformWindow);
		}
		if (mActiveModalWindows.size() == 0)
		{
			return true;
		}
		return false;
	}

	void SlateApplication::registerViewport(std::shared_ptr<SViewport> inViewport)
	{
		std::shared_ptr<SWindow> parentWindow = findWidgetWindow(inViewport);
		if (parentWindow)
		{
			std::weak_ptr<ISlateViewport> slateViewport = inViewport->getViewportInterface();
			if (!slateViewport.expired())
			{
				parentWindow->setViewport(slateViewport.lock());
			}
		}
	}

	void SlateApplication::registerGameViewport(std::shared_ptr<SViewport> inViewport)
	{
		registerViewport(inViewport);
		if (mGameViewportWidget.lock() != inViewport)
		{
			inViewport->setActive(true);
			mGameViewportWidget = inViewport;
		}
	}

	std::shared_ptr<SWindow> SlateApplication::findWidgetWindow(std::shared_ptr<const SWidget> inWidget) const
	{
		return mSlateWindows[0];
	}

	bool SlateApplication::onRawMouseMove(const int32 X, const int32 Y)
	{
		if (X != 0 || Y != 0)
		{
			PointerEvent mouseEvent(CursorPointerIndex, getCursorPos(), getLastCursorPos(), float2(X, Y), mPressedMouseButtons, mPlatformApplication->getModifierKeys());
			processMouseMoveEvent(mouseEvent);
		}
		return true;
	}

	std::shared_ptr<SWidget> SlateApplication::getUserFocusedWidget(uint32 userIndex) const
	{
		return mSlateWindows[0];
	}

	ModifierKeysState SlateApplication::getModifierKeys() const
	{
		return mPlatformApplication->getModifierKeys();
	}

	bool SlateApplication::doesWidgetHaveMouseCapture(const std::shared_ptr<const SWidget> widget) const
	{
		return mMouseCaptor.doesWidgetHaveMouseCapture(widget);
	}

	bool SlateApplication::MouseCaptureHelper::doesWidgetHaveMouseCapture(const std::shared_ptr<const SWidget> widget) const
	{
		for (const auto& indexPathPair : mPointerIndexToMouseCaptorWeakMap)
		{
			auto lastWidget = indexPathPair.second.lock();
			if (widget == lastWidget)
			{
				return true;
			}
		}
		return false;
	}
	void SlateApplication::MouseCaptureHelper::setMouseCaptor(uint32 userIndex, uint32 pointerIndex, const std::shared_ptr<SWidget> widget)
	{
		invalidateCaptureForPointer(userIndex, pointerIndex);
		if (widget)
		{
			mPointerIndexToMouseCaptorWeakMap.emplace(UserAndPointer(userIndex, pointerIndex), widget);
		}
	}
	void SlateApplication::MouseCaptureHelper::invalidateCaptureForPointer(uint32 userIndex, uint32 pointerIndex)
	{
		informCurrentCaptorOfCaptureLoss(userIndex, pointerIndex);
		mPointerIndexToMouseCaptorWeakMap.erase(UserAndPointer(userIndex, pointerIndex));
	}

	void SlateApplication::MouseCaptureHelper::informCurrentCaptorOfCaptureLoss(uint32 userIndex, uint32 pointerIndex) const
	{
		const auto& it = mPointerIndexToMouseCaptorWeakMap.find(UserAndPointer(userIndex, pointerIndex));
		if (it != mPointerIndexToMouseCaptorWeakMap.end() && !it->second.expired())
		{
			std::shared_ptr<SWidget> sharedWidgetPtr = it->second.lock();
			if (sharedWidgetPtr)
			{
				sharedWidgetPtr->onMouseCaptureLost();
			}
		}


	}
	void SlateApplication::processReply(const Reply theReply, const SWidget* widget, const PointerEvent* inMouseEvent, uint32 userIndex /* = 0 */)
	{
		uint32 pointerIndex = inMouseEvent != nullptr ? inMouseEvent->getPointerIndex() : CursorPointerIndex;
		const bool bIsVirtualInteraction = false;

		if (mMouseCaptor.hasCaptureForPointerIndex(userIndex, pointerIndex) && theReply.shouldReleaseMouse())
		{
			mMouseCaptor.invalidateCaptureForPointer(userIndex, pointerIndex);
		}

		if (bAppIsActive)
		{
			std::shared_ptr<SWidget> requrestedMouseCaptor = theReply.getMouseCaptor();
			if (requrestedMouseCaptor)
			{
				mMouseCaptor.setMouseCaptor(userIndex, pointerIndex, requrestedMouseCaptor);
			}
			if (!bIsVirtualInteraction && requrestedMouseCaptor)
			{
				if (theReply.shouldUseHighPrecisionMouse())
				{
					const std::shared_ptr<SWindow> window = mSlateWindows[0];
					mPlatformApplication->setCapture(window->getNativeWindow());
					mPlatformApplication->setHighPrecisionMouseMode(true, window->getNativeWindow());
				}
			}
			TOptional<int2> requestedMousePos = theReply.getRequestedMousePos();
			if (requestedMousePos.isSet())
			{
				const float2 position = requestedMousePos.getValue();
				mPointerIndexLastPositionMap.emplace(CursorPointerIndex, position);
				setCursorPos(position);
			}
		}
	}

	void SlateApplication::pollGameDeviceState()
	{
		if (mActiveModalWindows.size() == 0 && !GIntraFrameDebuggingGameThread)
		{
			mPlatformApplication->pollGameDeviceState(getDeltaTime());
		}
	}

	bool SlateApplication::MouseCaptureHelper::hasCapture() const
	{
		for (auto it : mPointerIndexToMouseCaptorWeakMap)
		{
			if (!it.second.expired())
			{
				return true;
			}
		}
		return false;
	}

	TArray<std::shared_ptr<SWidget>> SlateApplication::MouseCaptureHelper::toSharedWidgets() const
	{
		TArray < std::shared_ptr<SWidget>> widgets;
		widgets.empty(mPointerIndexToMouseCaptorWeakMap.size());
		for (const auto& indexPathPair : mPointerIndexToMouseCaptorWeakMap)
		{
			std::shared_ptr<SWidget> lastWidget = indexPathPair.second.lock();
			if (lastWidget)
			{
				widgets.add(lastWidget);
			}
		}
		return widgets;

	}

	void SlateApplication::finishedInputThisFrame()
	{
		const float deltaTime = getDeltaTime();
		if (mMouseCaptor.hasCapture())
		{
			TArray<std::shared_ptr<SWidget>> captors = mMouseCaptor.toSharedWidgets();
			for (const auto& captor : captors)
			{
				captor->onFinishedPointerInput();
			}

		}
		else
		{
		}

		
	}
}