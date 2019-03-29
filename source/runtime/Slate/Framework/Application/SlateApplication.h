#pragma once
#include "Slate.h"
#include "Application/SlateApplicationBase.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"
namespace Air
{
	struct DrawWindowArgs;
	class SViewport;

	



	class SLATE_API SlateApplication
		: public SlateApplicationBase,
			public GenericApplicationMessageHandler
	{
	public:
		SlateApplication();

		static void create();
		static std::shared_ptr<SlateApplication> create(const std::shared_ptr<class GenericApplication>& inPlatformApplication);

		static SlateApplication& get()
		{
			isInGameThread();
			return *mCurrentApplication;
		}

		virtual std::shared_ptr<SWindow> addWindow(std::shared_ptr<SWindow> inSlateWindow, const bool bShowImmediately = true) override;

		virtual std::shared_ptr<SWindow> getWindow(int32 index);

		void tick();

		virtual bool initializeRenderer(std::shared_ptr<SlateRenderer> inRenderer, bool bQuietMode = false) override;

		std::shared_ptr<SWindow> findWidgetWindow(std::shared_ptr<const SWidget> inWidget) const;

		void tickApplication(float deltaTime);

		virtual float getApplicationScale() const override
		{
			return mScale;
		}
		std::shared_ptr<SWidget> getUserFocusedWidget(uint32 userIndex) const override;

		virtual bool isActive() const override
		{
			return bAppIsActive;
		}

		ModifierKeysState getModifierKeys() const;

		const float getDeltaTime() const
		{
			return (float)(mCurrentTime - mLastTickTime);
		}

		static bool isInitialized()
		{
			return !!mCurrentApplication;
		}

		void drawWindows();

		void drawWindowAndChildren(const std::shared_ptr<SWindow>& windowToDraw, DrawWindowArgs& drawWindowArgs);

		void registerGameViewport(std::shared_ptr<SViewport> inViewport);

		void registerViewport(std::shared_ptr<SViewport> inViewport);


		virtual const double getCurrentTime() const override
		{
			return mCurrentTime;
		}
	public:
		virtual bool onKeyDown(const int32 keyCode, const uint32 characterCode, const bool isRepeat) override;

		virtual bool onKeyUp(const int32 keyCode, const uint32 characterCode, const bool isRepeat) override;

		virtual bool onMouseMove() override;

		virtual bool onRawMouseMove(const int32 X, const int32 Y) override;

		virtual bool onMouseDown(const std::shared_ptr<GenericWindow>& window, const EMouseButtons::Type btton, const float2 cursorPos) override;

		virtual bool onMouseDown(const std::shared_ptr<GenericWindow>& window, const EMouseButtons::Type btton) override;

		virtual bool onMouseUp(const EMouseButtons::Type botton) override;

		virtual bool onMouseUp(const EMouseButtons::Type botton, const float2 cursorPos);

		virtual float2 getCursorPos() const override;

		void setCursorPos(const float2& mouseCoordinate);

		virtual float2 getLastCursorPos() const override;

		virtual float2 getCursorSize() const override;
		bool processKeyDownEvent(KeyEvent& inKeyEvent);
		bool processKeyUpEvent(KeyEvent& inKeyEvent);
		bool processMouseMoveEvent(PointerEvent& mouseEvent, bool bIsSynthetic = false);
		bool processMouseButtonDownEvent(const std::shared_ptr<GenericWindow>& platformWindow, PointerEvent& inMouseEvent);

		bool processMouseButtonUpEvent(PointerEvent& inMouseEvent);

		void setLastUserInteractionTime(double inCurrentTime);

		bool shouldProcessUserInputMessages(const std::shared_ptr<GenericWindow>& platformWindow) const override;

		void pollGameDeviceState();

		void finishedInputThisFrame();

		bool isDragDropping() const;

		virtual bool doesWidgetHaveMouseCapture(const std::shared_ptr<const SWidget> widget) const;

		void processReply(const Reply theReply, const SWidget* widget, const PointerEvent* inMouseEvent, uint32 userIndex = 0);
	private:
		std::shared_ptr<GenericWindow> makeWindow(std::shared_ptr<SWindow> inSlateWindow, const bool bShowImmediately);

		void privateDrawWindows(std::shared_ptr<SWindow> drawOnlyThisWindow = nullptr);
		void queueSynthesizedMouseMove();



	private:
		class UserAndPointer
		{
		public:
			uint32 mUserIndex;
			uint32 mPointerIndex;

			UserAndPointer(uint32 inUserIndex, uint32 inPointerIndex)
				:mUserIndex(inUserIndex)
				,mPointerIndex(inPointerIndex)
			{}
			bool operator == (const UserAndPointer & other)const
			{
				return mUserIndex == other.mUserIndex && mPointerIndex == other.mPointerIndex;
			}

			friend size_t getTypeHash(const UserAndPointer& userAndPointer)
			{
				return userAndPointer.mUserIndex << 16 | userAndPointer.mPointerIndex;
			}
			friend class MouseCaptureHelper;
		};

		class MouseCaptureHelper
		{
		public:
			bool hasCaptureForPointerIndex(uint32 userIndex, uint32 pointerIndex) const;
			std::shared_ptr<SWidget> toWidgetPath(const PointerEvent* pointerEvent);

			void setMouseCaptor(uint32 userIndex, uint32 pointerIndex, const std::shared_ptr<SWidget> widget);

			bool hasCapture() const;

			TArray<std::shared_ptr<SWidget>> toSharedWidgets() const;

			void informCurrentCaptorOfCaptureLoss(uint32 userIndex, uint32 pointerIndex) const;

			void invalidateCaptureForPointer(uint32 userIndex, uint32 pointerIndex);

			bool doesWidgetHaveMouseCapture(const std::shared_ptr<const SWidget> widget) const;
		private:
		protected:
			TMap<UserAndPointer, std::weak_ptr<SWidget>> mPointerIndexToMouseCaptorWeakMap;
		};
	private:
		static std::shared_ptr<SlateApplication> mCurrentApplication;

		TArray<std::shared_ptr<SWindow>> mSlateWindows;

		std::mutex mSlateTickMutex;

		TArray<std::shared_ptr<SWindow>> mSlateVirtualWindows;

		TArray<std::shared_ptr<SWindow>> mActiveModalWindows;

		std::weak_ptr<SViewport> mGameViewportWidget;

		float mScale{ 1.0f };

		double mCurrentTime;
		double mLastTickTime;

		float mAverageDeltaTime;

		int32 mSynthesizeMouseMovePending{ 0 };

		double mLastMouseMoveTime;

		double mLastUserInteractionTimeForThrottling{ 0 };

		double mLastUserInteractionTime;

		TSet<Key> mPressedMouseButtons;

		TMap<uint32, int2> mPointerIndexLastPositionMap;

		bool bAppIsActive;

		bool bSlateWindowActive;

		float mCursorRadius;

		MouseCaptureHelper mMouseCaptor;
	};
}