#include "InputCoreType.h"
#include "HAL/PlatformMisc.h"
namespace Air
{
	const Key EKeys::TouchKeys[NUM_TOUCH_KEYS] = 
	{
		Key("Touch1"),
		Key("Touch2"),
		Key("Touch3"),
		Key("Touch4"),
		Key("Touch5"),
		Key("Touch6"),
		Key("Touch7"),
		Key("Touch8"),
		Key("Touch9"),
		Key("Touch10"),
	};

	const Key EKeys::AnyKey("AnyKey");
	const Key EKeys::LeftMouseButton("LeftMouseButton");
	const Key EKeys::RightMouseButton("RightMouseButton");
	const Key EKeys::MiddleMouseButton("MiddleMouseButton");
	const Key EKeys::MouseX("MouseX");
	const Key EKeys::MouseY("MouseY");
	const Key EKeys::MouseScrollDown("MouseScrollDown");
	const Key EKeys::MouseScrollUp("MouseScrollUp");
	const Key EKeys::MouseWheelAxis("MouseWheelAxis");
	const Key EKeys::ThumbMouseButton("ThumbMouseButton");
	const Key EKeys::ThumbMouseButton2("ThumbMouseButton2");


	const Key EKeys::LeftAlt("LeftAlt");
	const Key EKeys::RightAlt("RightAlt");
	const Key EKeys::LeftShift("LeftShift");
	const Key EKeys::RightShift("RightShift");
	const Key EKeys::LeftCtrl("LeftControl");
	const Key EKeys::RightCtrl("RightControl");
	const Key EKeys::LeftCmd("LeftCommand");
	const Key EKeys::RightCmd("RightCommand");

	const Key EKeys::BackSpace("BackSpace");
	const Key EKeys::Tab("Tab");
	const Key EKeys::Enter("Enter");
	const Key EKeys::Pause("Pause");

	const Key EKeys::CapsLock("CapsLock");
	const Key EKeys::Escape("Escape");
	const Key EKeys::SpaceBar("SpaceBar");
	const Key EKeys::PageUp("PageUp");
	const Key EKeys::PageDown("PageDown");
	const Key EKeys::Home("Home");
	const Key EKeys::End("End");

	const Key EKeys::Left("Left");
	const Key EKeys::Right("Right");
	const Key EKeys::Up("Up");
	const Key EKeys::Down("Down");

	const Key EKeys::Insert("Insert");
	const Key EKeys::Delete("Delete");

	const Key EKeys::Zero("Zero");
	const Key EKeys::One("One");
	const Key EKeys::Two("Two");
	const Key EKeys::Three("Three");
	const Key EKeys::Four("Four");
	const Key EKeys::Five("Five");
	const Key EKeys::Six("Six");
	const Key EKeys::Seven("Seven");
	const Key EKeys::Eight("Eight");
	const Key EKeys::Nine("Nine");

	const Key EKeys::A("A");
	const Key EKeys::B("B");
	const Key EKeys::C("C");
	const Key EKeys::D("D");
	const Key EKeys::E("E");
	const Key EKeys::F("F");
	const Key EKeys::G("G");
	const Key EKeys::H("H");
	const Key EKeys::I("I");
	const Key EKeys::J("J");
	const Key EKeys::K("K");
	const Key EKeys::L("L");
	const Key EKeys::M("M");
	const Key EKeys::N("N");
	const Key EKeys::O("O");
	const Key EKeys::P("P");
	const Key EKeys::Q("Q");
	const Key EKeys::R("R");
	const Key EKeys::S("S");
	const Key EKeys::T("T");
	const Key EKeys::U("U");
	const Key EKeys::V("V");
	const Key EKeys::W("W");
	const Key EKeys::X("X");
	const Key EKeys::Y("Y");
	const Key EKeys::Z("Z");

	const Key EKeys::NumPadZero("NumPadZero");
	const Key EKeys::NumPadOne("NumPadOne");
	const Key EKeys::NumPadTwo("NumPadTwo");
	const Key EKeys::NumPadThree("NumPadThree");
	const Key EKeys::NumPadFour("NumPadFour");
	const Key EKeys::NumPadFive("NumPadFive");
	const Key EKeys::NumPadSix("NumPadSix");
	const Key EKeys::NumPadSeven("NumPadSeven");
	const Key EKeys::NumPadEight("NumPadEight");
	const Key EKeys::NumPadNine("NumPadNine");

	const Key EKeys::Multiply("Multiply");
	const Key EKeys::Subtract("Subtract");
	const Key EKeys::Add("Add");
	const Key EKeys::Decimal("Decimal");
	const Key EKeys::Divide("Divide");

	const Key EKeys::F1("F1");
	const Key EKeys::F2("F2");
	const Key EKeys::F3("F3");
	const Key EKeys::F4("F4");
	const Key EKeys::F5("F5");
	const Key EKeys::F6("F6");
	const Key EKeys::F7("F7");
	const Key EKeys::F8("F8");
	const Key EKeys::F9("F9");
	const Key EKeys::F10("F10");
	const Key EKeys::F11("F11");
	const Key EKeys::F12("F12");

	const Key EKeys::NumLock("NumLock");
	const Key EKeys::ScrollLock("ScrollLock");

	const Key EKeys::Semicolon("Semicolon");
	const Key EKeys::Equals("Equals");
	const Key EKeys::Comma("Comma");
	const Key EKeys::Underscore("Underscore");
	const Key EKeys::Hyphen("Hyphen");

	const Key EKeys::Invalid("");

	bool EKeys::bInitialized = false;

	void EKeys::initialize()
	{
		if (bInitialized)
		{
			return;
		}
		bInitialized = true;

		addKey(KeyDetails(EKeys::AnyKey, TEXT("AnyKey")));
		addKey(KeyDetails(EKeys::MouseX, TEXT("MouseX"), KeyDetails::FloatAxits | KeyDetails::MouseButton | KeyDetails::UpdateAxisWithoutSamples));
		addKey(KeyDetails(EKeys::MouseY, TEXT("MouseY"), KeyDetails::FloatAxits | KeyDetails::MouseButton | KeyDetails::UpdateAxisWithoutSamples));
		addKey(KeyDetails(EKeys::MouseScrollUp, TEXT("MouseScrollUp"), KeyDetails::MouseButton));
		addKey(KeyDetails(EKeys::MouseScrollDown, TEXT("MouseScrollDown"), KeyDetails::MouseButton));
		addKey(KeyDetails(EKeys::MouseWheelAxis, TEXT("MouseWheelAxis"), KeyDetails::FloatAxits | KeyDetails::MouseButton | KeyDetails::UpdateAxisWithoutSamples));
		
		addKey(KeyDetails(EKeys::LeftMouseButton, TEXT("LeftMouseButton"), KeyDetails::MouseButton));
		addKey(KeyDetails(EKeys::RightMouseButton, TEXT("RightMouseButton"), KeyDetails::MouseButton));
		addKey(KeyDetails(EKeys::MiddleMouseButton, TEXT("MiddleMouseButton"), KeyDetails::MouseButton));
		addKey(KeyDetails(EKeys::ThumbMouseButton, TEXT("ThumbMouseButtonTEXT"), KeyDetails::MouseButton));
		addKey(KeyDetails(EKeys::ThumbMouseButton2, TEXT("ThumbMouseButton2"), KeyDetails::MouseButton));

		addKey(KeyDetails(EKeys::BackSpace, TEXT("BackSpace")));
		addKey(KeyDetails(EKeys::Tab, TEXT("Tab")));
		addKey(KeyDetails(EKeys::Enter, TEXT("Enter")));
		addKey(KeyDetails(EKeys::Pause, TEXT("Pause")));

		addKey(KeyDetails(EKeys::CapsLock, TEXT("CapsLock")));
		addKey(KeyDetails(EKeys::Escape, TEXT("Escape")));
		addKey(KeyDetails(EKeys::SpaceBar, TEXT("SpaceBar")));
		addKey(KeyDetails(EKeys::PageUp, TEXT("PageUp")));
		addKey(KeyDetails(EKeys::PageDown, TEXT("PageDown")));
		addKey(KeyDetails(EKeys::Home, TEXT("Home")));
		addKey(KeyDetails(EKeys::End, TEXT("End")));

		addKey(KeyDetails(EKeys::Left, TEXT("Left")));
		addKey(KeyDetails(EKeys::Right, TEXT("Right")));
		addKey(KeyDetails(EKeys::Up, TEXT("Up")));
		addKey(KeyDetails(EKeys::Down, TEXT("Down")));

		addKey(KeyDetails(EKeys::Insert, TEXT("Insert")));
		addKey(KeyDetails(EKeys::Delete, TEXT("Delete")));

		addKey(KeyDetails(EKeys::Zero, TEXT("Zero")));
		addKey(KeyDetails(EKeys::One, TEXT("One")));
		addKey(KeyDetails(EKeys::Two, TEXT("Two")));
		addKey(KeyDetails(EKeys::Three, TEXT("Three")));
		addKey(KeyDetails(EKeys::Four, TEXT("Four")));
		addKey(KeyDetails(EKeys::Five, TEXT("Five")));
		addKey(KeyDetails(EKeys::Six, TEXT("Six")));
		addKey(KeyDetails(EKeys::Seven, TEXT("Seven")));
		addKey(KeyDetails(EKeys::Eight, TEXT("Eight")));
		addKey(KeyDetails(EKeys::Nine, TEXT("Delete")));

		addKey(KeyDetails(EKeys::A, TEXT("A")));
		addKey(KeyDetails(EKeys::B, TEXT("B")));
		addKey(KeyDetails(EKeys::C, TEXT("C")));
		addKey(KeyDetails(EKeys::D, TEXT("D")));
		addKey(KeyDetails(EKeys::E, TEXT("E")));
		addKey(KeyDetails(EKeys::F, TEXT("F")));
		addKey(KeyDetails(EKeys::G, TEXT("G")));
		addKey(KeyDetails(EKeys::H, TEXT("H")));
		addKey(KeyDetails(EKeys::I, TEXT("I")));
		addKey(KeyDetails(EKeys::J, TEXT("J")));
		addKey(KeyDetails(EKeys::K, TEXT("K")));
		addKey(KeyDetails(EKeys::L, TEXT("L")));
		addKey(KeyDetails(EKeys::M, TEXT("M")));
		addKey(KeyDetails(EKeys::N, TEXT("N")));
		addKey(KeyDetails(EKeys::O, TEXT("O")));
		addKey(KeyDetails(EKeys::P, TEXT("P")));
		addKey(KeyDetails(EKeys::Q, TEXT("Q")));
		addKey(KeyDetails(EKeys::R, TEXT("R")));
		addKey(KeyDetails(EKeys::S, TEXT("S")));
		addKey(KeyDetails(EKeys::T, TEXT("T")));
		addKey(KeyDetails(EKeys::U, TEXT("U")));
		addKey(KeyDetails(EKeys::V, TEXT("V")));
		addKey(KeyDetails(EKeys::W, TEXT("W")));
		addKey(KeyDetails(EKeys::X, TEXT("X")));
		addKey(KeyDetails(EKeys::Y, TEXT("Y")));
		addKey(KeyDetails(EKeys::Z, TEXT("Z")));

		addKey(KeyDetails(EKeys::NumPadZero, TEXT("NumPadZero")));
		addKey(KeyDetails(EKeys::NumPadOne, TEXT("NumPadOne")));
		addKey(KeyDetails(EKeys::NumPadTwo, TEXT("NumPadTwo")));
		addKey(KeyDetails(EKeys::NumPadThree, TEXT("NumPadThree")));
		addKey(KeyDetails(EKeys::NumPadFour, TEXT("NumPadFour")));
		addKey(KeyDetails(EKeys::NumPadFive, TEXT("NumPadFive")));
		addKey(KeyDetails(EKeys::NumPadSix, TEXT("NumPadSix")));
		addKey(KeyDetails(EKeys::NumPadSeven, TEXT("NumPadSeven")));
		addKey(KeyDetails(EKeys::NumPadEight, TEXT("NumPadEight")));
		addKey(KeyDetails(EKeys::NumPadNine, TEXT("NumPadNine")));

		addKey(KeyDetails(EKeys::Multiply, TEXT("Multiply")));
		addKey(KeyDetails(EKeys::Subtract, TEXT("Subtract")));
		addKey(KeyDetails(EKeys::Add, TEXT("Add")));
		addKey(KeyDetails(EKeys::Decimal, TEXT("Decimal")));
		addKey(KeyDetails(EKeys::Divide, TEXT("Divide")));

		addKey(KeyDetails(EKeys::F1, TEXT("F1")));
		addKey(KeyDetails(EKeys::F2, TEXT("F2")));
		addKey(KeyDetails(EKeys::F3, TEXT("F3")));
		addKey(KeyDetails(EKeys::F4, TEXT("F4")));
		addKey(KeyDetails(EKeys::F5, TEXT("F5")));
		addKey(KeyDetails(EKeys::F6, TEXT("F6")));
		addKey(KeyDetails(EKeys::F7, TEXT("F7")));
		addKey(KeyDetails(EKeys::F8, TEXT("F8")));
		addKey(KeyDetails(EKeys::F9, TEXT("F9")));
		addKey(KeyDetails(EKeys::F10, TEXT("F10")));
		addKey(KeyDetails(EKeys::F11, TEXT("F11")));
		addKey(KeyDetails(EKeys::F12, TEXT("F12")));

		addKey(KeyDetails(EKeys::NumLock, TEXT("NumLock")));
		addKey(KeyDetails(EKeys::ScrollLock, TEXT("ScrollLock")));

		addKey(KeyDetails(EKeys::LeftAlt, TEXT("LeftAlt"), KeyDetails::ModifierKey));
		addKey(KeyDetails(EKeys::RightAlt, TEXT("RightAlt"), KeyDetails::ModifierKey));
		addKey(KeyDetails(EKeys::LeftShift, TEXT("LeftShift"), KeyDetails::ModifierKey));
		addKey(KeyDetails(EKeys::RightShift, TEXT("RightShift"), KeyDetails::ModifierKey));
		addKey(KeyDetails(EKeys::LeftCtrl, TEXT("LeftCtrl"), KeyDetails::ModifierKey));
		addKey(KeyDetails(EKeys::RightCtrl, TEXT("RightCtrl"), KeyDetails::ModifierKey));
		addKey(KeyDetails(EKeys::LeftCmd, TEXT("LeftCmd"), KeyDetails::ModifierKey));
		addKey(KeyDetails(EKeys::RightCmd, TEXT("RightCmd"), KeyDetails::ModifierKey));

		addKey(KeyDetails(EKeys::Semicolon, TEXT("Semicolon")));
		addKey(KeyDetails(EKeys::Equals, TEXT("Equals")));
		addKey(KeyDetails(EKeys::Comma, TEXT("Comma")));
		addKey(KeyDetails(EKeys::Underscore, TEXT("Underscore")));
		addKey(KeyDetails(EKeys::Hyphen, TEXT("Hyphen")));


		addKey(KeyDetails(EKeys::Invalid, TEXT("Invalid")));
	}


	void Key::conditionalLookupKeyDetails() const
	{
		if (!mKeyDetails)
		{
			mKeyDetails = EKeys::getKeyDetails(*this);
		}
	}

	bool Key::isModifierKey() const
	{
		conditionalLookupKeyDetails();
		return (mKeyDetails ? mKeyDetails->isModifierKey() : false);
	}


	TMap<Key, std::shared_ptr<KeyDetails>> EKeys::mInputKeys;

	std::shared_ptr<KeyDetails> EKeys::getKeyDetails(const Key key)
	{
		auto& it = mInputKeys.find(key);
		if (it == mInputKeys.end())
		{
			return std::shared_ptr<KeyDetails>();
		}
		return it->second;
	}

	std::shared_ptr<InputKeyManager> InputKeyManager::mInstance;

	InputKeyManager& InputKeyManager::get()
	{
		if (!mInstance)
		{
			InputKeyManager* m = new InputKeyManager();
			mInstance.reset(m);
		}
		return *mInstance;
	}

	void InputKeyManager::initKeyMappings()
	{
		static const uint32 MAX_KEY_MAPPINGS(256);
		uint32 keyCodes[MAX_KEY_MAPPINGS], charCodes[MAX_KEY_MAPPINGS];
		wstring keyNames[MAX_KEY_MAPPINGS], charKeyNames[MAX_KEY_MAPPINGS];
		uint32 const charKeyMapSize(PlatformMisc::getCharKeyMap(charCodes, charKeyNames, MAX_KEY_MAPPINGS));

		uint32 const keyMapSize(PlatformMisc::getKeyMap(keyCodes, keyNames, MAX_KEY_MAPPINGS));

		for (uint32 idx = 0; idx < keyMapSize; ++idx)
		{
			Key key(keyNames[idx].c_str());
			if (!key.isValid())
			{
				EKeys::addKey(KeyDetails(key, key.getDisplayName()));
			}
			mKeyMapVirtualToEnum.emplace(keyCodes[idx], key);
		}
		for (uint32 idx = 0; idx < charKeyMapSize; ++idx)
		{
			const Key key(charKeyNames[idx].c_str());
			if (key.isValid())
			{
				mKeyMapCharToEnum.emplace(charCodes[idx], key);
			}
		}
	}
	wstring Key::getDisplayName() const
	{
		conditionalLookupKeyDetails();
		return (mKeyDetails ? mKeyDetails->getDisplayName() : mKeyName);
	}

	bool Key::isFloatAxis() const
	{
		conditionalLookupKeyDetails();
		return (mKeyDetails ? mKeyDetails->isFloatAxis() : false);
	}

	bool Key::isVectorAxis() const
	{
		conditionalLookupKeyDetails();
		return (mKeyDetails ? mKeyDetails->isVectorAxis() : false);
	}

	bool Key::isGamepadKey() const
	{
		conditionalLookupKeyDetails();
		return (mKeyDetails ? mKeyDetails->isGamepadKey() : false);
	}

	bool Key::isMouseButton() const
	{
		conditionalLookupKeyDetails();
		return (mKeyDetails ? mKeyDetails->isGamepadKey() : false);
	}

	KeyDetails::KeyDetails(const Key inKey, const wstring& inDisplayName, const uint8 inKeyFlags /* = 0 */, const wstring inmenuCategory /* = TEXT("") */)
		:mKey(inKey)
		,mDispayName(inDisplayName)
		,bIsModifierKey((inKeyFlags & EKeyFlags::ModifierKey) != 0)
		,bIsGamepadKey((inKeyFlags & EKeyFlags::GamepadKey) != 0)
		,bIsMouseButton((inKeyFlags &EKeyFlags::MouseButton) != 0)
		,mAxisType(EInputAxisType::None)
		,bShouldUpdateAxisWithoutSamples((inKeyFlags & EKeyFlags::UpdateAxisWithoutSamples) != 0)
	{
		if ((inKeyFlags & EKeyFlags::FloatAxits) != 0)
		{
			mAxisType = EInputAxisType::Float;
		}

		else if ((inKeyFlags& EKeyFlags::VectorAxis) != 0)
		{
			mAxisType = EInputAxisType::Vector;
		}

	}

	void EKeys::addKey(const KeyDetails& keyDetails)
	{
		const Key& key = keyDetails.getKey();
		BOOST_ASSERT(mInputKeys.find(key) == mInputKeys.end());
		key.mKeyDetails = MakeSharedPtr<KeyDetails>(keyDetails);
		mInputKeys.emplace(key, key.mKeyDetails);
	}
	Key InputKeyManager::getKeyFromCodes(const uint32 keyCode, const uint32 charCode) const
	{
		auto it = mKeyMapVirtualToEnum.find(keyCode);
		if (it != mKeyMapVirtualToEnum.end())
		{
			return it->second;
		}
		else
		{
			auto it2 = mKeyMapCharToEnum.find(charCode);
			if (it2 != mKeyMapCharToEnum.end())
			{
				return it2->second;
			}
		}
		return EKeys::Invalid;
	}

	void InputKeyManager::getCodesFromKey(const Key key, const uint32* & keyCode, const uint32*& charCode) const
	{
		charCode = mKeyMapCharToEnum.findKey(key);
		keyCode = mKeyMapVirtualToEnum.findKey(key);
	}

	bool Key::shouldUpdateAxisWithoutSamples() const
	{
		conditionalLookupKeyDetails();
		return (mKeyDetails ? mKeyDetails->shouldUpdateAxisWithoutSamples() : false);
	}
}