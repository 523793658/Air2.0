#pragma once
#include "Classes/Components/ActorComponent.h"
#include <functional>
#include "Classes/Engine/EngineBaseTypes.h"
#include "Framework/Commands/InputChord.h"
namespace Air
{

	struct InputBinding
	{
		uint32 bConsumeInput : 1;
		uint32 bExecuteWhenPaused : 1;
		InputBinding()
			:bConsumeInput(true)
			,bExecuteWhenPaused(false)
		{}

	};

	typedef std::function<void(void)> InputActionHandlerSignature;

	typedef std::function<void(Key)> InputActionHandlerWithKeySignature;

	typedef std::function<void(float)> InputAxisUnifiedDelegate;

	typedef std::function<void(float3)> InputVectorAxisUnifiedDelegate;

	typedef std::function<void(const Key key)> InputActionUnifiedDelegate;

	struct InputActionBinding : public InputBinding
	{
		wstring mActionName;
		TEnumAsByte<EInputEvent> mKeyEvent;
		uint32 bPaired : 1;
		InputActionUnifiedDelegate mActionDelegate;

		InputActionBinding()
			:InputBinding()
			,mActionName(Name_None)
			,mKeyEvent(EInputEvent::IE_Pressed)
			,bPaired(false)
		{}

		InputActionBinding(const wstring &inActionName, const enum EInputEvent inKeyEvent)
			:InputBinding()
			,mActionName(inActionName)
			,mKeyEvent(inKeyEvent)
			,bPaired(false)
		{}
	};

	struct InputKeyBinding : public InputBinding
	{
		InputChord mChord;
		TEnumAsByte<EInputEvent> mKeyEvent;

		InputActionUnifiedDelegate mKeyDelegate;

		InputKeyBinding()
			:InputBinding()
			,mKeyEvent(EInputEvent::IE_Pressed)
		{}

		InputKeyBinding(const InputChord inChord, const enum EInputEvent inKeyEvent)
			:InputBinding()
			,mChord(inChord)
			,mKeyEvent(inKeyEvent)
		{}
	};

	struct InputAxisKeyBinding : public InputBinding
	{
		Key mAxisKey;

		InputAxisUnifiedDelegate mAxisDelegate;

		float mAxisValue;
		InputAxisKeyBinding()
			:InputBinding()
			,mAxisValue(0.f)
		{}
		InputAxisKeyBinding(const Key inAxisKey)
			:InputBinding()
			, mAxisKey(inAxisKey)
			, mAxisValue(0.f)
		{
			BOOST_ASSERT(mAxisKey.isFloatAxis());
		}

	};

	struct InputVectorAxisBinding : public InputBinding
	{
		Key mAxisKey;
		float3 mAxisValue;

		InputVectorAxisUnifiedDelegate mAxisDelegate;

		InputVectorAxisBinding()
			:InputBinding()
		{}
		InputVectorAxisBinding(const Key inAxisKey)
			:InputBinding()
			, mAxisKey(inAxisKey)
		{
			BOOST_ASSERT(mAxisKey.isVectorAxis());
		}
	};

	struct InputAxisBinding : public InputBinding
	{
		wstring mAxisName;
		InputAxisUnifiedDelegate mAxisDelegate;

		float mAxisValue;
		InputAxisBinding()
			:InputBinding()
			, mAxisName(Name_None)
			,mAxisValue(0.f)
		{}
		InputAxisBinding(const wstring inName)
			:InputBinding()
			,mAxisName(inName)
			,mAxisValue(0.f)
		{}

		InputAxisBinding(InputAxisBinding&& other)
			:InputBinding()
			, mAxisName(std::move(other.mAxisName))
			, mAxisValue(other.mAxisValue)
			, mAxisDelegate(std::move(other.mAxisDelegate))
		{

		}

		InputAxisBinding(const InputAxisBinding& other)
			:InputBinding()
			,mAxisName(other.mAxisName)
			,mAxisValue(other.mAxisValue)
			,mAxisDelegate(other.mAxisDelegate)
		{
		}
	};

	namespace EControllerAnalogStick
	{
		enum Type
		{
			CAS_LeftStick,
			CAS_RightStick,
			CAS_MAX
		};
	}

	class ENGINE_API InputComponent : public ActorComponent
	{
		GENERATED_RCLASS_BODY(InputComponent, ActorComponent)
	public:
		void clearBindingValues();

		InputAxisBinding& bindAxis(const wstring axisName, InputAxisUnifiedDelegate func)
		{
			InputAxisBinding ab(axisName);
			ab.mAxisDelegate = func;
			mAxisBindings.add(ab);
			return mAxisBindings.last();
		}

		int32 getNumActionBindings()const { return mActionBindings.size(); }

		InputActionBinding& getActionBinding(const int32 bindingIndex) { return mActionBindings[bindingIndex]; }
	public:
		TArray<struct InputKeyBinding> mKeyBindings;
		TArray<struct InputAxisBinding> mAxisBindings;

		TArray<struct InputAxisKeyBinding> mAxisKeyBindings;

		TArray<struct InputVectorAxisBinding> mVectorAxisBindings;

		TArray<struct InputActionBinding> mActionBindings;

		int32 mPriority;
		uint32 bBlockInput : 1;

		
	};
}