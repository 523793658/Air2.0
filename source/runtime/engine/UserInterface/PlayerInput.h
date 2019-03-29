#pragma once
#include "EngineMininal.h"
#include "KeyState.h"
#include "InputCoreType.h"
#include "Framework/Commands/InputChord.h"
#include "Classes/Components/InputComponent.h"
#include "Object.h"
namespace Air
{
	class InputComponent;
	class APlayerController;

	struct InputActionKeyMapping
	{
		wstring mActionName;

		Key mKey;

		uint8 bShift : 1;

		uint8 bCtrl : 1;

		uint8 bAlt : 1;

		uint8 bCmd : 1;

		bool operator == (const InputActionKeyMapping& other) const
		{
			return (
				mActionName == other.mActionName &&
				mKey == other.mKey &&
				bShift == other.bShift&&
				bCtrl == other.bCtrl&&
				bAlt == other.bAlt&&
				bCmd == other.bCmd);
		}

		bool operator < (const InputActionKeyMapping& other) const
		{
			bool bResult = false;
			if (mActionName < other.mActionName)
			{
				bResult = true;
			}
			else if (mActionName == other.mActionName)
			{
				bResult = (mKey < other.mKey);
			}
			return bResult;
		}

		InputActionKeyMapping(const wstring inActionName = TEXT(""), const Key inKey = EKeys::Invalid, const bool bInShift = false, const bool bInCtrl = false, const bool bInAlt = false, const bool bInCmd = false)
			:mActionName(inActionName),
			mKey(inKey),
			bShift(bInShift),
			bCtrl(bInCtrl),
			bAlt(bInAlt),
			bCmd(bInCmd)
		{}

	};

	struct InputAxisKeyMapping
	{
		wstring mAxisName;
		Key mKey;
		float mScale;

		bool operator == (const InputAxisKeyMapping& other)const
		{
			return (mAxisName == other.mAxisName && mKey == other.mKey && mScale == other.mScale);
		}

		bool operator <(const InputAxisKeyMapping& other) const
		{
			bool bResult = false;
			if (mAxisName < other.mAxisName)
			{
				bResult = true;
			}
			else if (mAxisName == other.mAxisName)
			{
				if (mKey < other.mKey)
				{
					bResult = true;
				}
				else if (mKey == other.mKey)
				{
					bResult = (mScale < other.mScale);
				}
			}
			return bResult;
		}

		InputAxisKeyMapping(const wstring inAxisName = TEXT(""), const Key inKey = EKeys::Invalid, const float inScale = 1.0f)
			:mAxisName(inAxisName),
			mKey(inKey),
			mScale(inScale)
		{

		}
	};

	struct InputAxisProperties
	{
		float mDeadZone;
		float mSensitivity;
		float mExponent;
		uint8 bInvert : 1;

		InputAxisProperties()
			:mDeadZone(0.2f)
			, mSensitivity(1.0f)
			, mExponent(1.0f)
			, bInvert(false)
		{}
	};


	class PlayerInput : public Object
	{
		GENERATED_RCLASS_BODY(PlayerInput, Object)
	public:
		

		bool inputKey(Key key, enum EInputEvent e, float amountDepressed, bool bGamepad);

		bool inputAxis(Key key, float delta, float deltaTime, int32 numSamples, bool bGamepad);

		~PlayerInput();

		virtual World* getWorld() const override;


		static const TArray<InputActionKeyMapping> & getEngineDefinedActionMappings()
		{
			return mEngineDefinedActionMappings;
		}

		static const TArray<InputAxisKeyMapping> & getEngineDefinedAxisMapping()
		{
			return mEngineDefinedAxisMappings;
		}
		bool isKeyHandledByAction(Key key) const;

		bool isPressed(Key inKey) const;

		bool isAltPressed() const;

		bool isCtrlPressed() const;;

		bool isShiftPressed() const;

		bool isCmdPressed() const;

		void tick(float deltaTime);

		void processInputStack(const TArray<InputComponent*>& inputComponentStack, const float deltaTime, const bool bGamePaused);

		static void addEngineDefinedAxisMapping(const InputAxisKeyMapping& axisMapping);

		void conditionalInitAxisProperties();

		APlayerController* getOuterAPlayerController();

	private:
		void getChordsForAction(const InputActionBinding& actionBinding, const bool bGamePaused, TArray<struct DelegateDispatchDetails>& foundChords, TArray<Key>& keysToConsume);

		void getChordsForKeyMapping(const InputActionKeyMapping& keyMapping, const InputActionBinding& actionBinding, const bool bGamePaused, TArray<struct DelegateDispatchDetails>& foundChords, TArray<Key>& keysToConsume);

		void getChordsForKey(const InputKeyBinding& keyBinding, const bool bGamePaused, TArray<struct DelegateDispatchDetails>& foundChords, TArray<Key>& keysToConsume);

		float determineAxisValue(const InputAxisBinding& axisBinding, const bool bGamePaused, TArray<Key>& keysToConsume);

		void processNonAxesKeys(Key inKey, KeyState* keyState);

		bool isKeyConsumed(Key key) const;

		float getKeyValue(Key inKey)const;

		float3 getVectorKeyValue(Key inKey) const;

		void consumeKey(Key key);

		void finishProcessingPlayerInput();

		FORCEINLINE void conditionalBuildKeyMappings()
		{
			if (!bKeyMapsBuilt)
			{
				conditionalBuildKeyMappings_Internal();
			}
		}
		void conditionalBuildKeyMappings_Internal();

		bool keyEventOccurred(Key key, EInputEvent e, TArray<uint32>& inEventIndices) const;

		float massageAxisInput(Key key, float rawValue);
	private:
		struct ActionKeyDetails
		{
			TArray<InputActionKeyMapping> mActions;
			InputChord mCapturingChord;
		};

		struct AxisKeyDetails
		{
			TArray<InputAxisKeyMapping> mKeyMappings;
			uint8 bInverted : 1;
			AxisKeyDetails()
				:bInverted(false)
			{}
		};


		TMap<Key, InputAxisProperties> mAxisProperties;

		TMap<wstring, ActionKeyDetails> mActionKeyMap;
		TMap<wstring, AxisKeyDetails> mAxisKeyMap;

		TMap<Key, KeyState> mKeyStateMap;

		TArray<struct InputActionKeyMapping> mActionMappings;
		TArray<struct InputAxisKeyMapping> mAxisMappings;

		uint8 bKeyMapsBuilt : 1;

		uint32 mEventCount;

		static TArray<InputActionKeyMapping> mEngineDefinedActionMappings;

		static TArray<InputAxisKeyMapping> mEngineDefinedAxisMappings;

	public:
		float mSmoothedMouse[2];
		float mMouseSamplingTotal;
		int32 mMouseSamples;

		TArray<uint32> mEventIndices;

		TArray<wstring> mInvertedAxis;
	};

}