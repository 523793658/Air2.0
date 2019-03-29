#pragma once
#include "GenericPlatform/GenericApplication.h"
#include "SlateCore.h"
#include "Containers/Set.h"
#include "InputCoreType.h"
namespace Air
{
	struct InputEvent
	{
	public:
		InputEvent()
			:mModifierKeys(ModifierKeysState())
			,mUserIndex(0)
			,bIsRepeat(false)
		{}

		InputEvent(const ModifierKeysState& inModifierKeys, const int32 inUserIndex, const bool bInIsRepeat)
			:mModifierKeys(inModifierKeys)
			,mUserIndex(inUserIndex)
			,bIsRepeat(bInIsRepeat)
		{}

		virtual ~InputEvent() {}

	public:
		bool isRepeat() const
		{
			return bIsRepeat;
		}

		bool isShiftDown() const
		{
			return mModifierKeys.isShiftDown();
		}

		bool isLeftShiftDown() const
		{
			return mModifierKeys.isLeftShiftDown();
		}

		bool isRightShiftDown() const
		{
			return mModifierKeys.isRightShiftDown();
		}

		bool isControlDown() const
		{
			return mModifierKeys.isControlDown();
		}

		bool isLeftControlDown() const
		{
			return mModifierKeys.isLeftControlDown();
		}
		bool isRightControlDown() const
		{
			return mModifierKeys.isRightControlDown();
		}

		bool isAltDown() const
		{
			return mModifierKeys.isAltDown();
		}

		bool isLeftAltDown() const 
		{
			return mModifierKeys.isLeftAltDown();
		}

		bool isRightAltDown() const
		{
			return mModifierKeys.isRightAltDown();
		}

		bool isCommandDown() const
		{
			return mModifierKeys.isCommandDown();
		}

		bool isLeftCommandDown() const
		{
			return mModifierKeys.isLeftCommandDown();
		}

		bool isRightCommandDown() const
		{
			return mModifierKeys.isRightCommandDown();
		}

		bool areCapsLocked() const
		{
			return mModifierKeys.areCapsLocked();
		}

		uint32 getUserIndex() const
		{
			return mUserIndex;
		}

		SLATE_CORE_API virtual bool isPointerEvent() const;

	protected:
		ModifierKeysState mModifierKeys;
		uint32 mUserIndex;
		bool bIsRepeat;
	};

	struct KeyEvent : public InputEvent
	{
	public:
		KeyEvent()
			:InputEvent(ModifierKeysState(), 0, false)
			,mKey()
			,mCharacterCode(0)
			,mKeyCode(0)
		{}

		KeyEvent(const Key inKey,
			const ModifierKeysState& inModifierKeys,
			const uint32 inUserIndex,
			const bool bInIsRepeat,
			const uint32 inCharacterCode,
			const uint32 inKeyCode)
			:InputEvent(inModifierKeys, inUserIndex, bInIsRepeat)
			,mKey(inKey)
			,mCharacterCode(inCharacterCode)
			,mKeyCode(inKeyCode)
		{
			
		}

		Key getKey() const
		{
			return mKey;
		}

		uint32 getCharacter() const
		{
			return mCharacterCode;
		}

		uint32 getKeyCode() const
		{
			return mKeyCode;
		}


	private:

		Key mKey;

		uint32 mCharacterCode;

		uint32 mKeyCode;
	};

	class TouchKeySet
		:public TSet<Key>
	{
	public:
		TouchKeySet(Key key)
		{
			this->add(key);
		}
	public:
		SLATE_CORE_API static const TouchKeySet StandardSet;

		SLATE_CORE_API static const TouchKeySet EmptySet;
	};

	struct VirtualPointerPosition
	{
		VirtualPointerPosition()
			:mCurrentCursorPosition(int2::Zero)
			,mLastCursorPosition(int2::Zero)
		{}

		VirtualPointerPosition(const int2& inCurrentCursorPosition, const int2& inLastCursorPosition)
			:mCurrentCursorPosition(inCurrentCursorPosition)
			,mLastCursorPosition(inLastCursorPosition)
		{

		}
		int2 mCurrentCursorPosition;
		int2 mLastCursorPosition;
	};

	struct PointerEvent
		: public InputEvent
	{

		PointerEvent()
			:mScreenSpacePosition(int2::Zero)
			, mLastScreenSpacePosition(int2::Zero)
			, mCursorDelta(int2::Zero)
			, mPressedButtons(TouchKeySet::EmptySet)
			, mEffectingButton()
			, mPointerIndex(0)
			, mTouchpadIndex(0)
			, bIsTouchEvent(false)
			, mGestureType(EGestureEvent::None)
			, mWheelOrGestureDelta(0, 0)
			, bIsDirectionInvertedFromDevice(false)
		{}

		PointerEvent(
			uint32 inPointerIndex,
			const int2& inScreenSpacePosition,
			const int2& inLastScreenSpacePosition,
			const TSet<Key>& inPressedButtons,
			Key inEffectingButton,
			float inWhellDelta,
			const ModifierKeysState& inModifierKeys
		)
			:InputEvent(inModifierKeys, 0, false)
			, mScreenSpacePosition(inScreenSpacePosition)
			, mLastScreenSpacePosition(inLastScreenSpacePosition)
			, mCursorDelta(inScreenSpacePosition - inLastScreenSpacePosition)
			, mPressedButtons(inPressedButtons)
			, mEffectingButton(inEffectingButton)
			, mPointerIndex(inPointerIndex)
			, mTouchpadIndex(0)
			, bIsTouchEvent(false)
			, mGestureType(EGestureEvent::None)
			, mWheelOrGestureDelta(0, inWhellDelta)
			, bIsDirectionInvertedFromDevice(false)
		{}

		PointerEvent(uint32 inUserIndex, uint32 inPointerIndex, const int2& inScreenSpacePosition,
			int2& inLastScreenSpacePosition,
			const TSet<Key>& inPressedButtons,
			Key inEffectingButton,
			float inWheelDelta,
			const ModifierKeysState& inModifierKeys)
			:InputEvent(inModifierKeys, inUserIndex, false)
			, mScreenSpacePosition(inScreenSpacePosition)
			, mLastScreenSpacePosition(inLastScreenSpacePosition)
			, mCursorDelta(inScreenSpacePosition - inLastScreenSpacePosition)
			, mPressedButtons(inPressedButtons)
			, mEffectingButton(inEffectingButton)
			, mPointerIndex(inPointerIndex)
			, mTouchpadIndex(0)
			, bIsTouchEvent(false)
			, mGestureType(EGestureEvent::None)
			, mWheelOrGestureDelta(0, inWheelDelta)
			, bIsDirectionInvertedFromDevice(false)
		{

		}

		PointerEvent(uint32 inPointerIndex, const int2& inScreenSpacePosition, const int2 & inLastScreenSpacePosition,
			const int2& inDelta, const TSet<Key>& inPressedButtons, const ModifierKeysState& inModifierKeys)
			:InputEvent(inModifierKeys, 0, false)
			, mScreenSpacePosition(inScreenSpacePosition)
			, mLastScreenSpacePosition(inLastScreenSpacePosition)
			, mCursorDelta(inDelta)
			, mPressedButtons(inPressedButtons)
			, mPointerIndex(inPointerIndex)
			, mTouchpadIndex(0)
			, bIsTouchEvent(false)
			, mGestureType(EGestureEvent::None)
			, mWheelOrGestureDelta(0, 0)
			, bIsDirectionInvertedFromDevice(false)
		{

		}

		PointerEvent(uint32 inUserIndex, uint32 inPointerIndex,
			const int2& inScreenSpacePosition, const int2& inLastScreenSpacePosition, bool bPressLeftMouseButton, const ModifierKeysState& inModifierKeys = ModifierKeysState(), uint32 inTouchpadIndex = 0)
			:InputEvent(inModifierKeys, inUserIndex, false)
			, mScreenSpacePosition(inScreenSpacePosition)
			, mLastScreenSpacePosition(inLastScreenSpacePosition)
			, mCursorDelta(inScreenSpacePosition - inLastScreenSpacePosition)
			, mPressedButtons(bPressLeftMouseButton ? TouchKeySet::StandardSet : TouchKeySet::EmptySet)
			, mEffectingButton(EKeys::LeftMouseButton)
			, mPointerIndex(inPointerIndex)
			, mTouchpadIndex(inTouchpadIndex)
			, bIsTouchEvent(true)
			, mGestureType(EGestureEvent::None)
			, mWheelOrGestureDelta(0, 0)
			, bIsDirectionInvertedFromDevice(false)
		{
		}

		PointerEvent(
			const int2& inScreenSpacePosition,
			const int2& inLastScreenSpacePosition,
			const TSet<Key>& inPressedButtons,
			const ModifierKeysState& inModifiersKeys,
			EGestureEvent::Type inGestureType,
			const int2& inGestureDelta,
			bool bInIsDirectionInvertedFromdevice
		)
			:InputEvent(inModifiersKeys, 0, false)
			, mScreenSpacePosition(inScreenSpacePosition)
			, mLastScreenSpacePosition(inLastScreenSpacePosition)
			, mCursorDelta(inLastScreenSpacePosition - inScreenSpacePosition)
			, mPressedButtons(inPressedButtons)
			, mPointerIndex(0)
			, bIsTouchEvent(false)
			, mGestureType(inGestureType)
			, mWheelOrGestureDelta(inGestureDelta)
			, bIsDirectionInvertedFromDevice(bInIsDirectionInvertedFromdevice)
		{}
	public:
		const int2& getScreenSpacePosition()const { return mScreenSpacePosition; }

		const int2& getLastScreenSpacePosition() const { return mLastScreenSpacePosition; }

		int2 getCursorDelta() const { return mCursorDelta; }
	
		bool isMouseButtonDown(Key mouseButton) const { return mPressedButtons.contains(mouseButton); }

		Key getEffectingButton() const { return mEffectingButton; }


		float getWheelDelta() const { return mWheelOrGestureDelta.y; }

		int32 getUserIndex() const { return mUserIndex; }

		int32 getPointerIndex() const { return mPointerIndex; }

		uint32 getTouchpadIndex() const { return mTouchpadIndex; }

		bool isTouchEvent() const { return bIsTouchEvent; }

		EGestureEvent::Type getGestureType() const { return mGestureType; }

		const int2& getGestureDelta() const { return mWheelOrGestureDelta; }

		bool isDirectionInvertedFromDevice() const { return bIsDirectionInvertedFromDevice; }

		void operator =(const PointerEvent& rhs)
		{
			InputEvent::operator=(rhs);
			mScreenSpacePosition = rhs.mScreenSpacePosition;
			mLastScreenSpacePosition = rhs.mLastScreenSpacePosition;
			mCursorDelta = rhs.mCursorDelta;
			const_cast<TSet<Key>&>(mPressedButtons) = rhs.mPressedButtons;
			mEffectingButton = rhs.mEffectingButton;
			mUserIndex = rhs.mUserIndex;
			mPointerIndex = rhs.mPointerIndex;
			mTouchpadIndex = rhs.mTouchpadIndex;
			bIsTouchEvent = rhs.bIsTouchEvent;
			mGestureType = rhs.mGestureType;
			mWheelOrGestureDelta = rhs.mWheelOrGestureDelta;
			bIsDirectionInvertedFromDevice = rhs.bIsDirectionInvertedFromDevice;
		}

		SLATE_CORE_API virtual bool isPointerEvent() const override;

		template<typename PointerEventType>
		static PointerEventType makeTranslatedEvent(const PointerEventType& inPointerEvent, const VirtualPointerPosition& virtualPosition)
		{
			PointerEventType newEvent = inPointerEvent;
			newEvent.mScreenSpacePosition = virtualPosition.mCurrentCursorPosition;
			newEvent.mLastScreenSpacePosition = virtualPosition.mLastCursorPosition;
			return newEvent;
		}
	private:
		int2 mScreenSpacePosition;
		int2 mLastScreenSpacePosition;
		int2 mCursorDelta;
		const TSet<Key>& mPressedButtons;
		Key mEffectingButton;
		uint32 mPointerIndex;
		uint32 mTouchpadIndex;
		bool bIsTouchEvent;
		int2 mWheelOrGestureDelta;
		EGestureEvent::Type mGestureType;
		bool bIsDirectionInvertedFromDevice;

	};

	enum class EFocusCause : uint8
	{
		Mouse,
		Navigation,
		SetDirectly,
		Cleard,
		OtherWidgetLostFocus,
		WindowActivate,
	};
}