#pragma once
#include "CoreType.h"
#include "GenericPlatform/GenericWindows.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"

namespace Air
{
	class ICursor;

	namespace EModifierKey
	{
		typedef uint8 Type;
		const Type None = 0;

		const Type Control = 1 << 0;
		const Type Alt = 1 << 1;
		const Type Shift = 1 << 2;
		const Type Command = 1 << 3;

		FORCEINLINE EModifierKey::Type fromBools(const bool bControl, const bool bAlt, const bool bShift, const bool bCommand)
		{
			EModifierKey::Type modifierMask = EModifierKey::None;
			if (bControl) modifierMask |= EModifierKey::Control;
			if (bAlt) modifierMask |= EModifierKey::Alt;
			if (bShift) modifierMask |= EModifierKey::Shift;
			if (bCommand) modifierMask |= EModifierKey::Command;
			return modifierMask;
		}
	}

	class ModifierKeysState
	{
	public:
		ModifierKeysState(
			const bool bInIsLeftShiftDown,
			const bool bInIsRightShiftDown,
			const bool bInIsLeftControlDown,
			const bool bInIsRightControlDown,
			const bool bInIsLeftAltDown,
			const bool bInIsRightAltDown,
			const bool bInIsLeftCommandDown,
			const bool bInIsRightCommandDown,
			const bool bInAreCapsLocked
		)
			:bIsLeftShiftDown(bInIsLeftShiftDown)
			,bIsRightShiftDown(bInIsRightShiftDown)
			,bIsLeftControlDown(bInIsLeftControlDown)
			,bIsRightControlDown(bInIsRightControlDown)
			,bIsLeftAltDown(bInIsLeftAltDown)
			,bIsRightAltDown(bInIsRightAltDown)
			,bIsLeftCommandDown(bInIsLeftCommandDown)
			,bIsRightCommandDown(bInIsRightCommandDown)
			, bAreCapsLocked(bInAreCapsLocked)
		{

		}

		ModifierKeysState()
			:bIsLeftShiftDown(false)
			,bIsRightShiftDown(false)
			,bIsLeftControlDown(false)
			,bIsRightControlDown(false)
			,bIsLeftAltDown(false)
			,bIsRightAltDown(false)
			,bIsLeftCommandDown(false)
			,bIsRightCommandDown(false)
			,bAreCapsLocked(false)
		{}

		bool isShiftDown() const
		{
			return bIsLeftShiftDown || bIsRightShiftDown;
		}

		bool isLeftShiftDown() const
		{
			return bIsLeftShiftDown;
		}

		bool isRightShiftDown() const
		{
			return bIsRightShiftDown;
		}

		bool isControlDown() const
		{
			return bIsLeftControlDown || bIsRightControlDown;
		}

		bool isLeftControlDown() const
		{
			return bIsLeftControlDown;
		}

		bool isRightControlDown() const
		{
			return bIsRightControlDown;
		}
		bool isAltDown() const
		{
			return bIsLeftAltDown || bIsRightAltDown;
		}

		bool isLeftAltDown() const
		{
			return bIsLeftAltDown;
		}

		bool isRightAltDown() const
		{
			return bIsRightAltDown;
		}



		bool isCommandDown() const
		{
			return bIsLeftCommandDown || bIsRightCommandDown;
		}

		bool isLeftCommandDown() const
		{
			return bIsLeftCommandDown;
		}

		bool isRightCommandDown() const
		{
			return bIsRightCommandDown;
		}

		bool areCapsLocked() const
		{
			return bAreCapsLocked;
		}

		bool areModifiersDown(EModifierKey::Type modifierKeys) const
		{
			bool allModifiersDown = true;
			if ((modifierKeys & EModifierKey::Shift) == EModifierKey::Shift)
			{
				allModifiersDown &= isShiftDown();
			}
			if ((modifierKeys & EModifierKey::Command) == EModifierKey::Command)
			{
				allModifiersDown &= isCommandDown();
			}

			if ((modifierKeys & EModifierKey::Control) == EModifierKey::Control)
			{
				allModifiersDown &= isControlDown();
			}

			if ((modifierKeys & EModifierKey::Alt) == EModifierKey::Alt)
			{
				allModifiersDown &= isAltDown();
			}
			return allModifiersDown;
		}

	private:
		uint16 bIsLeftShiftDown : 1;
		uint16 bIsRightShiftDown : 1;

		uint16 bIsLeftControlDown : 1;
		uint16 bIsRightControlDown : 1;

		uint16 bIsLeftAltDown : 1;
		uint16 bIsRightAltDown : 1;

		uint16 bIsLeftCommandDown : 1;
		uint16 bIsRightCommandDown : 1;

		uint16 bAreCapsLocked : 1;
	};

	class CORE_API GenericApplication
	{
	public:
		
		GenericApplication(const std::shared_ptr<ICursor>& inCursor);

		
		virtual std::shared_ptr<GenericWindow> makeWindow() = 0;
		
		virtual void setCapture(const std::shared_ptr<GenericWindow>& inWindow) {}

		virtual void* getCapture() const { return nullptr; }

		virtual void setHighPrecisionMouseMode(const bool enable, const std::shared_ptr<GenericWindow>& inWindow) {}

		virtual void pollGameDeviceState(const float timeDelta) {}

		virtual void initializeWindow(const std::shared_ptr<GenericWindow>& window, const std::shared_ptr<GenericWindowDefinition>& inDesc, const std::shared_ptr<GenericWindow>& inParent, const bool bShowImmediately);

		virtual ModifierKeysState getModifierKeys() const { return ModifierKeysState(); }

		virtual void setMessageHandler(const std::shared_ptr<GenericApplicationMessageHandler>& inMessageHandler)
		{
			mMessageHandler = inMessageHandler;
		}

		virtual bool isCursorDirectlyOverSlateWindow() const { return true; }
	public:
		const std::shared_ptr<ICursor> mCursor;
	protected:
		std::shared_ptr<class GenericApplicationMessageHandler> mMessageHandler;
	};
}