#include "PlayerInput.h"
#include "Classes/GameFramework/PlayerController.h"
#include "Classes/GameFramework/InputSettings.h"
#include "Misc/App.h"
#include "Classes/Components/InputComponent.h"
#include "Classes/Engine/World.h"
#include "SimpleReflection.h"
namespace Air
{
	TArray<InputActionKeyMapping> PlayerInput::mEngineDefinedActionMappings;
	TArray<InputAxisKeyMapping> PlayerInput::mEngineDefinedAxisMappings;

	static TArray<PlayerInput*> mGlobalPlayerInputs;

	struct DelegateDispatchDetails
	{
		uint32 mEventIndex;
		uint32 mFoundIndex;

		InputActionUnifiedDelegate mActionDelegate;
		const InputActionBinding* mSourceAction;
		InputChord mChord;
		TEnumAsByte<EInputEvent> mKeyEvent;

		DelegateDispatchDetails(const uint32 inEventIndex, const uint32 inFoundIndex, const InputChord& inChored, const InputActionUnifiedDelegate& inDelegate, const EInputEvent inKeyEvent, const InputActionBinding* inSourceAction = nullptr)
			:mEventIndex(inEventIndex)
			,mFoundIndex(inFoundIndex)
			,mActionDelegate(inDelegate)
			,mSourceAction(inSourceAction)
			,mChord(inChored)
			,mKeyEvent(inKeyEvent)
		{

		}




	};




	PlayerInput::PlayerInput(const ObjectInitializer& initializer)
		:ParentType(initializer)
	{
		mGlobalPlayerInputs.add(this);
	}

	PlayerInput::~PlayerInput()
	{
		mGlobalPlayerInputs.remove(this);
	}


	bool PlayerInput::inputKey(Key key, enum EInputEvent e, float amountDepressed, bool bGamepad)
	{
		KeyState & keyState = mKeyStateMap.findOrAdd(key);
		World* world = getWorld();
		BOOST_ASSERT(world);

		switch (e)
		{
		case Air::IE_Pressed:
		case Air::IE_Repeat:
			keyState.mRawValueAccumulator.x = amountDepressed;
			keyState.mEventAccumulator[e].add(++mEventCount);
			if (keyState.bDownPrevious = false)
			{
				const float worldRealTimeSeconds = world->getRealTimeSeconds();
				if ((worldRealTimeSeconds - keyState.mLastUpDownTransitionTime) < getDefault<InputSettings>()->DoubleClickTime)
				{
					keyState.mEventAccumulator[IE_DoubleClick].add(++mEventCount);
				}
				keyState.mLastUpDownTransitionTime = worldRealTimeSeconds;
			}
			break;
		case Air::IE_Released:
			keyState.mRawValueAccumulator.x = 0.f;
			keyState.mEventAccumulator[IE_Released].add(++mEventCount);
			break;
		case Air::IE_DoubleClick:
			keyState.mRawValueAccumulator.x = amountDepressed;
			keyState.mEventAccumulator[IE_Pressed].add(++mEventCount);
			keyState.mEventAccumulator[IE_DoubleClick].add(++mEventCount);
			break;
		}
		keyState.mSampleCountAccumulator++;

		if (e == IE_Pressed)
		{
			return isKeyHandledByAction(key);
		}
		return true;
	}

	bool PlayerInput::inputAxis(Key key, float delta, float deltaTime, int32 numSamples, bool bGamepad)
	{
		BOOST_ASSERT((key != EKeys::MouseX && key != EKeys::MouseY) || numSamples > 0);
		KeyState& keyState = mKeyStateMap.findOrAdd(key);
		if (keyState.mValue.x == 0.f && delta != 0.f)
		{
			keyState.mEventAccumulator[IE_Released].add(++mEventCount);
		}
		else if (keyState.mValue.x != 0.f && delta == 0.f)
		{
			keyState.mEventAccumulator[IE_Released].add(++mEventCount);
		}
		else
		{
			keyState.mEventAccumulator[IE_Repeat].add(++mEventCount);
		}
		keyState.mSampleCountAccumulator += numSamples;
		keyState.mRawValueAccumulator.x += delta;

		return false;
	}


	bool PlayerInput::isAltPressed() const
	{
		return isPressed(EKeys::LeftAlt) || isPressed(EKeys::RightAlt);
	}

	bool PlayerInput::isCtrlPressed() const
	{
		return isPressed(EKeys::LeftCtrl) || isPressed(EKeys::RightCtrl);
	}

	bool PlayerInput::isShiftPressed() const
	{
		return isPressed(EKeys::LeftShift) || isPressed(EKeys::RightShift);
	}

	bool PlayerInput::isCmdPressed() const
	{
		return isPressed(EKeys::LeftCmd) || isPressed(EKeys::RightCmd);
	}

	bool PlayerInput::isPressed(Key inKey) const
	{
		if (inKey == EKeys::AnyKey)
		{
			for (const auto it : mKeyStateMap)
			{
				if (!it.first.isFloatAxis() && !it.first.isVectorAxis() && it.second.bDown)
				{
					return true;
				}
			}
		}
		else
		{
			auto it = mKeyStateMap.find(inKey);
			if (it != mKeyStateMap.end())
			{
				return it->second.bDown;
			}
			return false;
		}
		return false;
	}

	bool PlayerInput::isKeyHandledByAction(Key key) const
	{
		for (const InputActionKeyMapping& mapping : mActionMappings)
		{
			if ((mapping.mKey == key || mapping.mKey == EKeys::AnyKey) && (mapping.bAlt == false || isAltPressed()) && (mapping.bCtrl == false || isCtrlPressed()) &&
				(mapping.bShift == false || isShiftPressed()) &&
				(mapping.bCmd == false || isCmdPressed()))
			{
				return true;
			}
		}
		return false;
	}

	class World* PlayerInput::getWorld() const
	{
		BOOST_ASSERT(getTypedOuter<APlayerController>());
		World* world = getTypedOuter<APlayerController>()->getWorld();
		return world;
	}

	void PlayerInput::addEngineDefinedAxisMapping(const InputAxisKeyMapping& axisMapping)
	{
		mEngineDefinedAxisMappings.addUnique(axisMapping);
		for (auto playerInput : mGlobalPlayerInputs)
		{
			playerInput->mAxisKeyMap.clear();
			playerInput->bKeyMapsBuilt = false;
		}
	}

	void PlayerInput::tick(float deltaTime)
	{
		conditionalInitAxisProperties();

	}

	void PlayerInput::conditionalInitAxisProperties()
	{
		if (mAxisProperties.size() == 0)
		{
			//for(const inputaxisconfi)
		}
	}

	void PlayerInput::processInputStack(const TArray<std::shared_ptr<InputComponent>>& inputComponentStack, const float deltaTime, const bool bGamePaused)
	{
		APlayerController* playerController = getOuterAPlayerController();

		playerController->preProcessInput(deltaTime, bGamePaused);

		for (auto& it : mKeyStateMap)
		{
			KeyState* keyState = &(it.second);
			Key key = it.first;
			for (uint8 eventIndex = 0; eventIndex < IE_MAX; ++eventIndex)
			{
				keyState->mEventCount[eventIndex].reset();
				exchange(keyState->mEventCount[eventIndex], keyState->mEventAccumulator[eventIndex]);
			}

			if ((keyState->mSampleCountAccumulator > 0) || key.shouldUpdateAxisWithoutSamples())
			{
				keyState->mRawValue = keyState->mRawValueAccumulator;
				if (keyState->mSampleCountAccumulator == 0)
				{
					keyState->mEventCount[IE_Released].add(++mEventCount);
				}
			}

			if ((key == EKeys::MouseX) && keyState->mRawValue.x != 0.f)
			{
				if (mSmoothedMouse[0] != 0)
				{
					mMouseSamplingTotal += App::getDeltaTime();
					mMouseSamples += keyState->mSampleCountAccumulator;
				}
			}
			processNonAxesKeys(key, keyState);

			keyState->mRawValueAccumulator = float3(0.0f);
			keyState->mSampleCountAccumulator = 0;
		}
		mEventCount = 0;

		struct AxisDelegateDetails
		{
			InputAxisUnifiedDelegate mDelegate;
			float mValue;

			AxisDelegateDetails(const InputAxisUnifiedDelegate& inDelegate, const float inValue)
				:mDelegate(inDelegate)
				,mValue(inValue)
			{}
		};

		struct VectorAxisDelegateDetails
		{
			InputVectorAxisUnifiedDelegate mDelegate;
			float3 mValue;
			VectorAxisDelegateDetails(const InputVectorAxisUnifiedDelegate& inDelegate, const float3 inValue)
				:mDelegate(inDelegate)
				,mValue(inValue)
			{}
		};

		static TArray<AxisDelegateDetails> mAxisDelegates;

		static TArray<VectorAxisDelegateDetails> mVectorAxisDelegates;

		static TArray<DelegateDispatchDetails> mNonAxisDelegates;

		static TArray<Key> mKeysToConsume;

		static TArray<DelegateDispatchDetails> mFoundChords;

		BOOST_ASSERT(isInGameThread() && !mAxisDelegates.size() && !mVectorAxisDelegates.size() && !mNonAxisDelegates.size() && !mKeysToConsume.size() && !mFoundChords.size() && !mEventIndices.size());

		struct DelegateDispatchDetailsSorter
		{
			bool operator()(const DelegateDispatchDetails& A, const DelegateDispatchDetails& B) const
			{
				return (A.mEventIndex == B.mEventIndex ? A.mFoundIndex < B.mFoundIndex : A.mEventIndex < B.mEventIndex);
			}
		};

		int32 stackIndex = inputComponentStack.size() - 1;

		for (; stackIndex >= 0; --stackIndex)
		{
			std::shared_ptr<InputComponent> const ic = inputComponentStack[stackIndex];
			if (ic)
			{
				BOOST_ASSERT(!mKeysToConsume.size() && !mFoundChords.size() && !mEventIndices.size());
				for (int32 actionIndex = 0; actionIndex < ic->getNumActionBindings(); ++actionIndex)
				{
					getChordsForAction(ic->getActionBinding(actionIndex), bGamePaused, mFoundChords, mKeysToConsume);
				}

				for (int32 keyIndex = 0; keyIndex < ic->mKeyBindings.size(); ++keyIndex)
				{
					getChordsForKey(ic->mKeyBindings[keyIndex], bGamePaused, mFoundChords, mKeysToConsume);
				}

				mFoundChords.sort(DelegateDispatchDetailsSorter());

				for (int32 chordIndex = 0; chordIndex < mFoundChords.size(); ++chordIndex)
				{
					const DelegateDispatchDetails& foundChord = mFoundChords[chordIndex];
					bool bFireDelegate = true;

					if(foundChord.mSourceAction && foundChord.mSourceAction->bPaired)
					{
						ActionKeyDetails& keyDetails = mActionKeyMap.findChecked(foundChord.mSourceAction->mActionName);
						if (!keyDetails.mCapturingChord.mKey.isValid() || keyDetails.mCapturingChord == foundChord.mChord || !isPressed(keyDetails.mCapturingChord.mKey))
						{
							if (foundChord.mSourceAction->mKeyEvent == IE_Pressed)
							{
								keyDetails.mCapturingChord = foundChord.mChord;
							}
							else
							{
								keyDetails.mCapturingChord.mKey = EKeys::Invalid;
							}
						}
						else
						{
							bFireDelegate = false;
						}
					}
					if (bFireDelegate && mFoundChords[chordIndex].mActionDelegate)
					{
						mFoundChords[chordIndex].mFoundIndex = mNonAxisDelegates.size();
						mNonAxisDelegates.add(mFoundChords[chordIndex]);
					}
				}

				for (InputAxisBinding& ab : ic->mAxisBindings)
				{
					ab.mAxisValue = determineAxisValue(ab, bGamePaused, mKeysToConsume);
					if (ab.mAxisDelegate)
					{
						mAxisDelegates.add(AxisDelegateDetails(ab.mAxisDelegate, ab.mAxisValue));
					}
				}
				for (InputAxisKeyBinding& axisKeyBinding : ic->mAxisKeyBindings)
				{
					if (!isKeyConsumed(axisKeyBinding.mAxisKey))
					{
						if (!bGamePaused || axisKeyBinding.bExecuteWhenPaused)
						{
							axisKeyBinding.mAxisValue = getKeyValue(axisKeyBinding.mAxisKey);
						}
						else
						{
							axisKeyBinding.mAxisValue = 0.f;
						}
						if (axisKeyBinding.bConsumeInput)
						{
							mKeysToConsume.addUnique(axisKeyBinding.mAxisKey);
						}
					}
					if (axisKeyBinding.mAxisDelegate)
					{
						mAxisDelegates.add(AxisDelegateDetails(axisKeyBinding.mAxisDelegate, axisKeyBinding.mAxisValue));
					}
				}
				for (InputVectorAxisBinding& vectorAxisBinding : ic->mVectorAxisBindings)
				{
					if (!isKeyConsumed(vectorAxisBinding.mAxisKey))
					{
						if (!bGamePaused || vectorAxisBinding.bExecuteWhenPaused)
						{
							vectorAxisBinding.mAxisValue = getVectorKeyValue(vectorAxisBinding.mAxisKey);
						}
						else
						{
							vectorAxisBinding.mAxisValue = float3::Zero;
						}

						if (vectorAxisBinding.bConsumeInput)
						{
							mKeysToConsume.addUnique(vectorAxisBinding.mAxisKey);
						}
					}
					if (vectorAxisBinding.mAxisDelegate)
					{
						mVectorAxisDelegates.add(VectorAxisDelegateDetails(vectorAxisBinding.mAxisDelegate, vectorAxisBinding.mAxisValue));
					}
				}
				if (ic->bBlockInput)
				{
					--stackIndex;
					mKeysToConsume.reset();
					mFoundChords.reset();
					break;
				}

				for (int32 keyIndex = 0; keyIndex < mKeysToConsume.size(); ++keyIndex)
				{
					consumeKey(mKeysToConsume[keyIndex]);
				}
				mKeysToConsume.reset();
				mFoundChords.reset();
			}
		}

		for (; stackIndex >= 0; --stackIndex)
		{
			std::shared_ptr<InputComponent> ic = inputComponentStack[stackIndex];
			if (ic)
			{
				for (InputAxisBinding& axisBinding : ic->mAxisBindings)
				{
					axisBinding.mAxisValue = 0.f;
				}
				for (InputAxisKeyBinding& axisKeyBinding : ic->mAxisKeyBindings)
				{
					axisKeyBinding.mAxisValue = 0.f;
				}
				for (InputVectorAxisBinding& vectorAxisBinding : ic->mVectorAxisBindings)
				{
					vectorAxisBinding.mAxisValue = float3::Zero;
				}
			}
		}

		mNonAxisDelegates.sort(DelegateDispatchDetailsSorter());

		for (const DelegateDispatchDetails& details : mNonAxisDelegates)
		{
			if (details.mActionDelegate)
			{
				details.mActionDelegate(details.mChord.mKey);
			}
		}

		for (const AxisDelegateDetails& details : mAxisDelegates)
		{
			if (details.mDelegate)
			{
				details.mDelegate(details.mValue);
			}
		}
		for (const VectorAxisDelegateDetails& details : mVectorAxisDelegates)
		{
			if (details.mDelegate)
			{
				details.mDelegate(details.mValue);
			}
		}

		playerController->postProcessInput(deltaTime, bGamePaused);

		finishProcessingPlayerInput();
		mAxisDelegates.reset();
		mVectorAxisDelegates.reset();
		mNonAxisDelegates.reset();

	}


	void PlayerInput::getChordsForAction(const InputActionBinding& actionBinding, const bool bGamePaused, TArray<struct DelegateDispatchDetails>& foundChords, TArray<Key>& keysToConsume) 
	{
		conditionalBuildKeyMappings();
		auto& keyDetailsIt = mActionKeyMap.find(actionBinding.mActionName);
		if (keyDetailsIt != mActionKeyMap.end())
		{
			for (const InputActionKeyMapping& keyMapping : keyDetailsIt->second.mActions)
			{
				if (keyMapping.mKey == EKeys::AnyKey)
				{
					for (auto keyStateIt : mKeyStateMap)
					{
						if (!keyStateIt.first.isFloatAxis() && !keyStateIt.first.isVectorAxis() && !keyStateIt.second.bConsumed)
						{
							InputActionKeyMapping subKeyMapping(keyMapping);
							subKeyMapping.mKey = keyStateIt.first;
							getChordsForKeyMapping(subKeyMapping, actionBinding, bGamePaused, foundChords, keysToConsume);
						}
					}
				}
				else if (!isKeyConsumed(keyMapping.mKey))
				{
					getChordsForKeyMapping(keyMapping, actionBinding, bGamePaused, foundChords, keysToConsume);
				}
			}
		}
	}

	void PlayerInput::getChordsForKey(const InputKeyBinding& keyBinding, const bool bGamePaused, TArray<struct DelegateDispatchDetails>& foundChords, TArray<Key>& keysToConsume)
	{
		bool bConsumeInput = false;
		if (keyBinding.mChord.mKey == EKeys::AnyKey)
		{
			for (auto keyStateIt : mKeyStateMap)
			{
				if (!keyStateIt.first.isFloatAxis() && !keyStateIt.first.isVectorAxis() && !keyStateIt.second.bConsumed)
				{
					InputKeyBinding subKeyBinding(keyBinding);
					subKeyBinding.mChord.mKey = keyStateIt.first;
					getChordsForKey(subKeyBinding, bGamePaused, foundChords, keysToConsume);
				}
			}
		}
		else if (!isKeyConsumed(keyBinding.mChord.mKey))
		{
			BOOST_ASSERT(!mEventIndices.size());
			if ((keyBinding.mChord.bAlt == false || isAltPressed())
				&& (keyBinding.mChord.bCtrl == false || isCtrlPressed())
				&& (keyBinding.mChord.bShift == false || isShiftPressed())
				&& (keyBinding.mChord.bCmd == false || isCmdPressed())
				&& keyEventOccurred(keyBinding.mChord.mKey, keyBinding.mKeyEvent, mEventIndices))
			{
				bool bAddDelegate = true;
				for (int32 chordIndex = foundChords.size() - 1; chordIndex >= 0; --chordIndex)
				{
					InputChord::ERelationshipType chordRelationship = keyBinding.mChord.getRelationship(foundChords[chordIndex].mChord);
					if (chordRelationship == InputChord::Masks)
					{
						foundChords.removeAtSwap(chordIndex);
					}
					else if (chordRelationship == InputChord::Masked)
					{
						bAddDelegate = false;
						break;
					}
				}
				if (bAddDelegate)
				{
					BOOST_ASSERT(mEventIndices.size() > 0);
					DelegateDispatchDetails foundChord(mEventIndices[0], foundChords.size(), keyBinding.mChord, ((!bGamePaused || keyBinding.bExecuteWhenPaused) ? keyBinding.mKeyDelegate : InputActionUnifiedDelegate()), keyBinding.mKeyEvent);

					foundChords.add(foundChord);
					for (int32 eventsIndex = 1; eventsIndex < mEventIndices.size(); ++eventsIndex)
					{
						foundChord.mEventIndex = mEventIndices[eventsIndex];
						foundChords.add(foundChord);
					}
					bConsumeInput = true;
				}
				mEventIndices.reset();
			}
		}

		if (keyBinding.bConsumeInput && (bConsumeInput || !(keyBinding.mChord.bAlt || keyBinding.mChord.bCtrl || keyBinding.mChord.bShift || keyBinding.mChord.bCmd || keyBinding.mKeyEvent == EInputEvent::IE_DoubleClick)))
		{
			keysToConsume.addUnique(keyBinding.mChord.mKey);
		}
	}

	float PlayerInput::determineAxisValue(const InputAxisBinding& axisBinding, const bool bGamePaused, TArray<Key>& keysToConsume)
	{
		conditionalBuildKeyMappings();
		float axisValue = 0.f;
		auto keyDetailsIt = mAxisKeyMap.find(axisBinding.mAxisName);
		if (keyDetailsIt != mAxisKeyMap.end())
		{
			for (int32 axisIndex = 0; axisIndex < keyDetailsIt->second.mKeyMappings.size(); ++axisIndex)
			{
				const InputAxisKeyMapping& keyMapping = (keyDetailsIt->second.mKeyMappings)[axisIndex];
				if (!isKeyConsumed(keyMapping.mKey))
				{
					if (!bGamePaused || axisBinding.bExecuteWhenPaused)
					{
						axisValue += getKeyValue(keyMapping.mKey) * keyMapping.mScale;
					}
					if (axisBinding.bConsumeInput)
					{
						keysToConsume.addUnique(keyMapping.mKey);
					}
				}
			}
			if (keyDetailsIt->second.bInverted)
			{
				axisValue *= -1.f;
			}
		}
		return axisValue;
	}

	void PlayerInput::processNonAxesKeys(Key inKey, KeyState* keyState)
	{
		keyState->mValue.x = massageAxisInput(inKey, keyState->mRawValue.x);
		int32 const pressDelta = keyState->mEventCount[IE_Pressed].size() - keyState->mEventCount[IE_Released].size();
		if (pressDelta < 0)
		{
			keyState->bDown = false;
		}
		else if (pressDelta > 0)
		{
			keyState->bDown = true;
		}
		else
		{
			keyState->bDown = keyState->bDownPrevious;
		}
	}

	bool PlayerInput::isKeyConsumed(Key key) const
	{
		if (key == EKeys::AnyKey)
		{
			for (const auto& it : mKeyStateMap)
			{
				if (it.second.bConsumed)
				{
					return true;
				}
			}
		}
		else
		{
			auto it = mKeyStateMap.find(key);
			if (it != mKeyStateMap.end())
			{
				return it->second.bConsumed;
			}
		}
		return false;
	}

	float PlayerInput::getKeyValue(Key inKey)const
	{
		auto it = mKeyStateMap.find(inKey);
		return it != mKeyStateMap.end() ? it->second.mValue.x : 0.f;
	}

	APlayerController* PlayerInput::getOuterAPlayerController()
	{
		return getTypedOuter<APlayerController>();
	}

	float3 PlayerInput::getVectorKeyValue(Key inKey) const
	{
		auto it = mKeyStateMap.find(inKey);
		return it != mKeyStateMap.end() ? it->second.mRawValue : float3(0.0f);
	}

	void PlayerInput::consumeKey(Key key)
	{
		auto it = mKeyStateMap.find(key);
		if (it != mKeyStateMap.end())
		{
			it->second.bConsumed = true;
		}
	}

	void PlayerInput::conditionalBuildKeyMappings_Internal()
	{
		if (mActionKeyMap.size() == 0)
		{
			struct
			{
				void build(const TArray<InputActionKeyMapping>& mappings, TMap<wstring, ActionKeyDetails>& keyMap)
				{
					for (const InputActionKeyMapping& actionMapping : mappings)
					{
						TArray<InputActionKeyMapping>& keyMappings = keyMap.findOrAdd(actionMapping.mActionName).mActions;
						keyMappings.addUnique(actionMapping);
					}
				}
			} ActionMappingsUtility;

			ActionMappingsUtility.build(mActionMappings, mActionKeyMap);
			ActionMappingsUtility.build(mEngineDefinedActionMappings, mActionKeyMap);
		}
		if (mAxisKeyMap.size() == 0)
		{
			auto AxisMappingsUtility = [](const TArray<InputAxisKeyMapping>& mappings, TMap<wstring, AxisKeyDetails>& axisMap)
			{
				for (const InputAxisKeyMapping& axisMapping : mappings)
				{
					bool bAdd = true;
					AxisKeyDetails& keydetails = axisMap.findOrAdd(axisMapping.mAxisName);
					for (const InputAxisKeyMapping& keyMapping : keydetails.mKeyMappings)
					{
						if (keyMapping.mKey == axisMapping.mKey)
						{
							bAdd = false;
							break;
						}
					}
					if (bAdd)
					{
						keydetails.mKeyMappings.add(axisMapping);
					}
				}
			};
			AxisMappingsUtility(mAxisMappings, mAxisKeyMap);
			AxisMappingsUtility(mEngineDefinedAxisMappings, mAxisKeyMap);

			for (int32 invertedAxisIndex = 0; invertedAxisIndex < mInvertedAxis.size(); ++invertedAxisIndex)
			{
				auto& it = mAxisKeyMap.find(mInvertedAxis[invertedAxisIndex]);
				if (it != mAxisKeyMap.end())
				{
					it->second.bInverted = true;
				}
			}
		}
		bKeyMapsBuilt = true;
	}


	void PlayerInput::finishProcessingPlayerInput()
	{
		for (auto& it : mKeyStateMap)
		{
			it.second.bDownPrevious = it.second.bDown;
			it.second.bConsumed = false;
		}
	}

	bool PlayerInput::keyEventOccurred(Key key, EInputEvent e, TArray<uint32>& inEventIndices) const
	{
		auto& const it = mKeyStateMap.find(key);
		if (it != mKeyStateMap.end())
		{
			if (it->second.mEventCount[e].size() > 0)
			{
				inEventIndices = it->second.mEventCount[e];
				return true;
			}
		}
		return false;
	}

	float PlayerInput::massageAxisInput(Key key, float rawValue)
	{
		float newVal = rawValue;
		conditionalInitAxisProperties();
		auto& const it = mAxisProperties.find(key);
		if (it != mAxisProperties.end())
		{
			InputAxisProperties const* const axisProps = &it->second;
			if (axisProps->mDeadZone > 0.f)
			{
				if (newVal > 0)
				{
					newVal = Math::max(0.f, newVal - axisProps->mDeadZone) / (1.f - axisProps->mDeadZone);
				}
				else
				{
					newVal = -Math::max(0.f, -newVal - axisProps->mDeadZone) / (1.0f - axisProps->mDeadZone);
				}
			}
			if (axisProps->mExponent != 1.0f)
			{
				newVal = Math::sign(newVal) * Math::pow(Math::abs(newVal), axisProps->mExponent);
			}

			newVal *= axisProps->mSensitivity;
			if (axisProps->bInvert)
			{
				newVal *= -1.0f;
			}
		}
		if ((key == EKeys::MouseX) || (key == EKeys::MouseY))
		{
			const InputSettings* defaultInputSettings = getDefault<InputSettings>();

			/*APlayerController const * const playerController = getOuterAPlayerController();*/

			float const FOVScale = 1.0f;
			newVal *= FOVScale;
		}
		return newVal;
	}

	void PlayerInput::getChordsForKeyMapping(const InputActionKeyMapping& keyMapping, const InputActionBinding& actionBinding, const bool bGamePaused, TArray<DelegateDispatchDetails>& foundChords, TArray<Key>& keysToConsume)
	{
		bool bConsumeInput = false;
		BOOST_ASSERT(!mEventIndices.size());
		if ((keyMapping.bAlt == false || isAltPressed())
			&& (keyMapping.bCtrl == false || isCtrlPressed())
			&& (keyMapping.bShift == false || isShiftPressed())
			&& (keyMapping.bCmd == false || isCmdPressed())
			&& keyEventOccurred(keyMapping.mKey, actionBinding.mKeyEvent, mEventIndices))
		{
			bool bAddDelegate = true;
			const InputChord chord(keyMapping.mKey, keyMapping.bShift, keyMapping.bCtrl, keyMapping.bAlt, keyMapping.bCmd);
			for (int32 chordIndex = foundChords.size() - 1; chordIndex >= 0; --chordIndex)
			{
				InputChord::ERelationshipType chordRelationship = chord.getRelationship(foundChords[chordIndex].mChord);
				if (chordRelationship == InputChord::Masks)
				{
					foundChords.removeAtSwap(chordIndex);
				}
				else if (chordRelationship == InputChord::Masked)
				{
					bAddDelegate = false;
					break;
				}
			}
			if (bAddDelegate)
			{
				BOOST_ASSERT(mEventIndices.size() > 0);
				DelegateDispatchDetails foundChord(mEventIndices[0], foundChords.size(), chord, ((!bGamePaused || actionBinding.bExecuteWhenPaused) ? actionBinding.mActionDelegate : InputActionUnifiedDelegate()), actionBinding.mKeyEvent, &actionBinding);
				foundChords.add(foundChord);

				for (int32 eventsIndex = 0; eventsIndex < mEventIndices.size(); ++eventsIndex)
				{
					foundChord.mEventIndex = mEventIndices[eventsIndex];
					foundChords.add(foundChord);

				}
				bConsumeInput = true;
			}
		}
		if (actionBinding.bConsumeInput && (bConsumeInput || !(keyMapping.bAlt || keyMapping.bCtrl || keyMapping.bShift || keyMapping.bCmd || actionBinding.mKeyEvent == EInputEvent::IE_DoubleClick)))
		{
			keysToConsume.addUnique(keyMapping.mKey);
		}
		mEventIndices.reset();
	}

	DECLARE_SIMPLER_REFLECTION(PlayerInput);
}