#pragma once
#include "InputCoreMinimal.h"
#include "Containers/StringConv.h"
namespace Air
{

	
	struct INPUT_CORE_API Key
	{
		Key()
		{

		}

		Key(const wstring inName)
			:mKeyName(inName)
		{

		}

		Key(const TCHAR* inName)
			: mKeyName(inName)
		{

		}
		Key(const ANSICHAR* inName)
			:mKeyName(ANSI_TO_TCHAR(inName))
		{

		}

		friend bool operator == (const Key& keyA, const Key& keyB)
		{
			return keyA.mKeyName == keyB.mKeyName;
		}

		friend bool operator != (const Key& keyA, const Key& keyB)
		{
			return keyA.mKeyName != keyB.mKeyName;
		}

		friend bool operator <(const Key& keyA, const Key& keyB)
		{
			return keyA.mKeyName < keyB.mKeyName;
		}

		friend uint32 getTypeHash(const Key& key)
		{
			return getTypeHash(key.mKeyName);
		}

		bool isValid() const
		{
			if (!mKeyName.empty())
			{
				conditionalLookupKeyDetails();
				return !!mKeyDetails;
			}
			return false;
		}
		bool isFloatAxis() const;

		bool isVectorAxis() const;

		bool isGamepadKey() const;

		bool isModifierKey() const;

		bool isMouseButton() const;

		bool shouldUpdateAxisWithoutSamples() const;

		wstring getDisplayName() const;



		friend struct EKeys;
	private:

		wstring mKeyName;
		mutable class std::shared_ptr<struct KeyDetails> mKeyDetails;

		void conditionalLookupKeyDetails() const;

	};


	struct INPUT_CORE_API KeyDetails
	{
		enum EKeyFlags
		{
			GamepadKey = 0x01,
			MouseButton = 0x02,
			ModifierKey = 0x04,
			FloatAxits = 0x08,
			VectorAxis = 0x10,
			UpdateAxisWithoutSamples = 0x40,
			NoFlags = 0,
		};

		KeyDetails(const Key inKey, const wstring& inDisplayName, const uint8 inKeyFlags = 0, const wstring inmenuCategory = TEXT(""));

		bool isModifierKey() const { return bIsModifierKey != 0; }

		bool isFloatAxis() const { return mAxisType == EInputAxisType::Float; }

		bool isVectorAxis() const { return mAxisType == EInputAxisType::Vector; }

		bool isGamepadKey() const { return bIsGamepadKey != 0; }

		bool isMouseButton() const { return bIsMouseButton != 0; }

		bool shouldUpdateAxisWithoutSamples() const { return bShouldUpdateAxisWithoutSamples != 0; }

		const Key& getKey() const { return mKey; }

		wstring getDisplayName() const
		{
			return mDispayName;
		}

	private:
		enum class EInputAxisType : uint8
		{
			None,
			Float,
			Vector
		};

		Key mKey;
		wstring mDispayName;

		int32 bIsModifierKey : 1;
		int32 bIsGamepadKey : 1;
		int32 bIsMouseButton : 1;
		int32 bShouldUpdateAxisWithoutSamples : 1;
		EInputAxisType mAxisType;
	};

	struct INPUT_CORE_API EKeys
	{
		static const Key AnyKey;
		static const Key MouseX;
		static const Key MouseY;
		static const Key MouseScrollUp;
		static const Key MouseScrollDown;
		static const Key MouseWheelAxis;


		static const Key LeftMouseButton;
		static const Key RightMouseButton;
		static const Key MiddleMouseButton;
		static const Key ThumbMouseButton;
		static const Key ThumbMouseButton2;

		static const Key BackSpace;
		static const Key Tab;
		static const Key Enter;
		static const Key Pause;

		static const Key CapsLock;
		static const Key Escape;
		static const Key SpaceBar;
		static const Key PageUp;
		static const Key PageDown;
		static const Key Home;
		static const Key End;

		static const Key Left;
		static const Key Right;
		static const Key Up;
		static const Key Down;

		static const Key Insert;
		static const Key Delete;

		static const Key Zero;
		static const Key One;
		static const Key Two;
		static const Key Three;
		static const Key Four;
		static const Key Five;
		static const Key Six;
		static const Key Seven;
		static const Key Eight;
		static const Key Nine;
		
		static const Key A;
		static const Key B;
		static const Key C;
		static const Key D;
		static const Key E;
		static const Key F;
		static const Key G;
		static const Key H;
		static const Key I;
		static const Key J;
		static const Key K;
		static const Key L;
		static const Key M;
		static const Key N;
		static const Key O;
		static const Key P;
		static const Key Q;
		static const Key R;
		static const Key S;
		static const Key T;
		static const Key U;
		static const Key V;
		static const Key W;
		static const Key X;
		static const Key Y;
		static const Key Z;

		static const Key NumPadZero;
		static const Key NumPadOne;
		static const Key NumPadTwo;
		static const Key NumPadThree;
		static const Key NumPadFour;
		static const Key NumPadFive;
		static const Key NumPadSix;
		static const Key NumPadSeven;
		static const Key NumPadEight;
		static const Key NumPadNine;

		static const Key Multiply;
		static const Key Subtract;
		static const Key Add;
		static const Key Decimal;
		static const Key Divide;

		static const Key F1;
		static const Key F2;
		static const Key F3;
		static const Key F4;
		static const Key F5;
		static const Key F6;
		static const Key F7;
		static const Key F8;
		static const Key F9;
		static const Key F10;
		static const Key F11;
		static const Key F12;

		static const Key NumLock;
		static const Key ScrollLock;


		static const Key LeftAlt;		
		static const Key RightAlt;
		static const Key LeftShift;
		static const Key RightShift;
		static const Key LeftCtrl;
		static const Key RightCtrl;
		static const Key LeftCmd;
		static const Key RightCmd;

		static const Key Semicolon;
		static const Key Equals;
		static const Key Comma;
		static const Key Underscore;
		static const Key Hyphen;



		static const Key Invalid;

		static const int32 NUM_TOUCH_KEYS = 11;

		static const Key TouchKeys[NUM_TOUCH_KEYS];

		static std::shared_ptr<KeyDetails> getKeyDetails(const Key key);

		static void initialize();

		static void addKey(const KeyDetails& keyDetails);
	private:
		static TMap<Key, std::shared_ptr<KeyDetails>> mInputKeys;

		static bool bInitialized;
	};


	struct INPUT_CORE_API InputKeyManager : std::enable_shared_from_this<InputKeyManager>
	{
	public:
		static InputKeyManager& get();

		void getCodesFromKey(const Key key, const uint32* & keyCode, const uint32*& charCode) const;

		Key getKeyFromCodes(const uint32 keyCode, const uint32 charCode) const;

		void initKeyMappings();

	private:
		InputKeyManager()
		{
			initKeyMappings();
		}

		static std::shared_ptr<InputKeyManager> mInstance;
		TMap<uint32, Key> mKeyMapVirtualToEnum;
		TMap<uint32, Key> mKeyMapCharToEnum;
	};
}