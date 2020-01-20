#pragma once
#include "CoreType.h"
#include "Containers/String.h"
#include "boost/assert.hpp"
#include "Containers/Array.h"
#include "Delegates/IDelegateInstance.h"
#include "Delegates/Delegate.h"
#include "Misc/OutputDevice.h"
#include "Delegates/DelegateCombinations.h"
namespace Air
{
	class World;

	template<class T> class TConsoleVariableData;

	enum EConsoleVariableFlags
	{
		ECVF_FlagMask = 0x0000ffff,
		ECVF_Default = 0x0,
		ECVF_Cheat = 0x1,
		ECVF_ReadOnly = 0x4,
		ECVF_Unregistered = 0x8,
		ECVF_CreatedFromIni = 0x10,
		ECVF_RenderThreadSafe = 0x20,
		ECVF_Scalability = 0x40,
		ECVF_SetByMask = 0xff000000,

		ECVF_SetByCode = 0x08000000,
	};



	class IConsoleObject
	{
	public:
		IConsoleObject()
#if 0
#endif
		{
		}

		virtual ~IConsoleObject() {}

		virtual const TCHAR* getHelp() const = 0;

		virtual void setHelp(const TCHAR* value) = 0;

		virtual EConsoleVariableFlags getFlags() const = 0;

		virtual void setFlags(const EConsoleVariableFlags value) = 0;

		void clearFlags(const EConsoleVariableFlags value)
		{
			uint32 n = (uint32)getFlags() & ~(uint32)value;
			setFlags((EConsoleVariableFlags)n);
		}

		bool testFlags(const EConsoleVariableFlags value) const
		{
			return ((uint32)getFlags() & (uint32)value) != 0;
		}

		virtual class IConsoleVariable* asVariable()
		{
			return 0;
		}

		virtual bool isVariableInt() const { return false; }

		virtual bool isVariableFloat() const { return false; }

		virtual bool isVariableString() const { return false; }

		virtual class TConsoleVariableData<int32>* asVariableInt()
		{
			BOOST_ASSERT(false);
			return 0;
		}

		virtual class TConsoleVariableData<float>* asVariableFloat()
		{
			BOOST_ASSERT(false);
			return 0;
		}
		virtual class TConsoleVariableData<wstring>* asVariableString()
		{
			BOOST_ASSERT(false);
			return 0;
		}

		virtual struct IConsoleCommand* asCommand()
		{
			return 0;
		}

	private:
		virtual void release() = 0;

		friend class ConsoleManager;
	};

	class IConsoleVariable : public IConsoleObject
	{
	public:
		virtual void set(const TCHAR* inValue, EConsoleVariableFlags setBy = ECVF_SetByCode) = 0;

		virtual int32 getInt() const = 0;

		virtual float getFloat() const = 0;

		virtual wstring getString() const = 0;

		//virtual void setOnChangeCallback()

		void set(int32 inValue, EConsoleVariableFlags setBy = ECVF_SetByCode)
		{
			set(wprintf(TEXT("%d"), inValue), setBy);
		}

		void set(float inValue, EConsoleVariableFlags setBy = ECVF_SetByCode)
		{
			set(wprintf(TEXT("%g"), inValue), setBy);
		}

		void setWithCurrentPriority(int32 inValue)
		{
			EConsoleVariableFlags curFlags = (EConsoleVariableFlags)(getFlags() & ECVF_SetByMask);
			set(inValue, curFlags);
		}

		void setWithCurrentPriority(float inValue)
		{
			EConsoleVariableFlags curFlags = (EConsoleVariableFlags)(getFlags() & ECVF_SetByMask);
			set(inValue, curFlags);
		}

		void setWithCurrentPriority(const TCHAR* inValue)
		{
			EConsoleVariableFlags curFlags = (EConsoleVariableFlags)(getFlags() & ECVF_SetByMask);
			set(inValue, curFlags);
		}




	};

	struct IConsoleCommand : public IConsoleObject
	{
		virtual bool execute(const TArray<wstring>& args, World* inWorld, class OutputDevice& outputDevice) = 0;
	};
	typedef std::function<void()> ConsoleCommandDelegate;

	typedef std::function<void(const TArray<wstring>&)> ConsoleCommandWithArgsDelegate;

	typedef std::function<void(const TArray<wstring>&, World*, OutputDevice&)> ConsoleCommandWithWorldArgsAndOutputDeviceDelegate;

	struct CORE_API IConsoleManager
	{
	public:
		FORCEINLINE static IConsoleManager& get()
		{
			if (!mSingleton)
			{
				setupSingleton();
				BOOST_ASSERT(mSingleton != nullptr);
			}
			return *mSingleton;
		}

		virtual void unregisterConsoleObject(IConsoleObject* consoleObject, bool bKeepState = true) = 0;

		virtual void unregisterConsoleObject(const TCHAR* name, bool bKeepState = true) = 0;

		virtual IConsoleCommand* registerConsoleCommand(const TCHAR* name, const TCHAR* help, const ConsoleCommandDelegate& command, uint32 flags) = 0;

		virtual IConsoleCommand* registerConsoleCommand(const TCHAR* name, const TCHAR* help, const ConsoleCommandWithArgsDelegate& command, uint32 flags) = 0;

		virtual IConsoleCommand* registerConsoleCommand(const TCHAR* name, const TCHAR* help, const ConsoleCommandWithWorldArgsAndOutputDeviceDelegate& command, uint32 flags) = 0;

		virtual IConsoleObject* findConsoleObject(const TCHAR* name, bool bTrackFrequentCalls = true) const = 0;

	private:
		static IConsoleManager* mSingleton;

		static void setupSingleton();
	};


	class CORE_API AutoConsoleObject
	{
	protected:
		AutoConsoleObject(IConsoleObject* inTarget)
			:mTarget(inTarget)
		{
			BOOST_ASSERT(inTarget);
		}

		virtual ~AutoConsoleObject()
		{
			IConsoleManager::get().unregisterConsoleObject(mTarget);
		}

	private:
		IConsoleObject* mTarget;
	};

	template<class T>
	class AutoConsoleVariable : public AutoConsoleObject
	{

	};


	

	class CORE_API AutoConsoleCommand : private AutoConsoleObject
	{
	public:
		AutoConsoleCommand(const TCHAR* name, const TCHAR* help, const ConsoleCommandDelegate& command, uint32 flags = ECVF_Default)
			: AutoConsoleObject(IConsoleManager::get().registerConsoleCommand(name, help, command, flags))
		{

		}
		AutoConsoleCommand(const TCHAR* name, const TCHAR* help, const ConsoleCommandWithArgsDelegate& command, uint32 flags = ECVF_Default)
			:AutoConsoleObject(IConsoleManager::get().registerConsoleCommand(name, help, command, flags))
		{

		}

		AutoConsoleCommand(const TCHAR* name, const TCHAR* help, const ConsoleCommandWithWorldArgsAndOutputDeviceDelegate& command, uint32 flags = ECVF_Default)
			:AutoConsoleObject(IConsoleManager::get().registerConsoleCommand(name, help, command, flags))
		{

		}
	};


	template<class T>
	class TConsoleVariableData
	{
	public:

		TConsoleVariableData(const T defaultValue)
		{
			for (uint32 i = 0; i < ARRAY_COUNT(mShadowedValue); ++i)
			{
				mShadowedValue[i] = defaultValue;
			}
		}

		T getValueOnGameThread() const
		{
			return mShadowedValue[0];
		}

		T getValueOnRenderThread() const
		{
			return mShadowedValue[1];
		}

		T getValueOnAnyThread(bool bForceGameThread = false) const
		{
			return mShadowedValue[getShadowIndex(bForceGameThread)];
		}

	private:
		static uint32 getShadowIndex(bool bForceGameThread = false)
		{
			if (bForceGameThread)
			{
				return 0;
			}
			return isInGameThread() ? 0 : 1;
		}

		T mShadowedValue[2];

		T& getReferenceOnAnyThread(bool bForceGameThread = false)
		{
			return mShadowedValue[getShadowIndex(bForceGameThread)];
		}

		template<class T2> friend class ConsoleVariable;
	};

}