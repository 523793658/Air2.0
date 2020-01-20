#include "HAL/IConsoleManager.h"
#include "HAL/ConsoleManager.h"
#include "Misc/ScopeLock.h"
#include "Logging/LogMacros.h"
namespace Air
{

	class ConsoleCommandBase : public IConsoleCommand
	{
	public:
		ConsoleCommandBase(const TCHAR* inHelp, EConsoleVariableFlags inFlags)
			:mHelp(inHelp)
			, mFlags(inFlags)
		{
			BOOST_ASSERT(inHelp);
		}
		virtual const TCHAR* getHelp() const
		{
			return mHelp.c_str();
		}

		virtual void setHelp(const TCHAR* inValue)
		{
			BOOST_ASSERT(inValue);
			BOOST_ASSERT(*inValue != 0);
			mHelp = inValue;
		}

		virtual EConsoleVariableFlags getFlags() const
		{
			return mFlags;
		}

		virtual void setFlags(const EConsoleVariableFlags value)
		{
			mFlags = value;
		}

		virtual struct IConsoleCommand* asCommand()
		{
			return this;
		}

	private:
		wstring mHelp;
		EConsoleVariableFlags mFlags;
	};

	class ConsoleCommand : public ConsoleCommandBase
	{
	public:
		ConsoleCommand(const ConsoleCommandDelegate& initDelegate, const TCHAR* initHelp, const EConsoleVariableFlags initFlags)
			:ConsoleCommandBase(initHelp, initFlags)
			,mDelegate(initDelegate)
		{}

		virtual void release() override
		{
			delete this;
		}

		virtual bool execute(const TArray<wstring>& args, World* inWorld, class OutputDevice& outputDevice) override
		{
			if (mDelegate)
			{
				mDelegate();
				return true;
			}
			return false;
		}

	private:

		ConsoleCommandDelegate mDelegate;
	};

	class ConsoleCommandWithArgs : public ConsoleCommandBase
	{
	public:
		ConsoleCommandWithArgs(const ConsoleCommandWithArgsDelegate& initDelegate, const TCHAR* initHelp, const EConsoleVariableFlags initFlags)
			:ConsoleCommandBase(initHelp, initFlags)
			,mDelegate(initDelegate)
		{}

		virtual void release() override
		{
			delete this;
		}

		virtual bool execute(const TArray<wstring>& args, World* inWorld, class OutputDevice& outputDevice)
		{
			if (mDelegate)
			{
				mDelegate(args);
				return true;
			}
			return false;
		}
	private:
		ConsoleCommandWithArgsDelegate mDelegate;
	};


	class ConsoleCommandWithWorldArgsAndOutputDevice : public ConsoleCommandBase
	{
	public:
		ConsoleCommandWithWorldArgsAndOutputDevice(const ConsoleCommandWithWorldArgsAndOutputDeviceDelegate& initDelegate, const TCHAR* initHelp, const EConsoleVariableFlags initFlags)
			:ConsoleCommandBase(initHelp, initFlags)
			, mDelegate(initDelegate)
		{}

		virtual void release() override
		{
			delete this;
		}

		virtual bool execute(const TArray<wstring>& args, World* inWorld, class OutputDevice& outputDevice)
		{
			if (mDelegate)
			{
				mDelegate(args, inWorld, outputDevice);
				return true;
			}
			return false;
		}
	private:
		ConsoleCommandWithWorldArgsAndOutputDeviceDelegate mDelegate;
	};

	IConsoleManager* IConsoleManager::mSingleton;

	void createConsoleVariables()
	{

	}

	void IConsoleManager::setupSingleton()
	{
		BOOST_ASSERT(!mSingleton);
		if (!mSingleton)
		{
			mSingleton = new ConsoleManager();
			createConsoleVariables();
		}
		BOOST_ASSERT(mSingleton);
	}

	wstring ConsoleManager::findConsoleObjectName(const IConsoleObject* obj) const
	{
		BOOST_ASSERT(obj);
		std::scoped_lock lock(mConsoleObjectsSynchronizationObject);
		for (auto& pairIt : mConsoleObjects)
		{
			IConsoleObject* var = pairIt.second;
			if (var == obj)
			{
				const wstring& name = pairIt.first;
				return name;
			}
		}
		return TEXT("");
	}

	IConsoleObject* ConsoleManager::findConsoleObjectUnfiltered(const TCHAR* name) const
	{
		std::scoped_lock lock(mConsoleObjectsSynchronizationObject);
		IConsoleObject* var = mConsoleObjects.findRef(name);
		return var;
	}

	IConsoleObject* ConsoleManager::findConsoleObject(const TCHAR* name, bool bTrackFrequentCalls /* = true */) const
	{
		IConsoleObject* cvar = findConsoleObjectUnfiltered(name);
		if (cvar && cvar->testFlags(ECVF_CreatedFromIni))
		{
			return 0;
		}
		return cvar;
	}


	void ConsoleManager::unregisterConsoleObject(IConsoleObject* consoleObject, bool bKeepState)
	{
		if (!consoleObject)
		{
			return;
		}
		std::scoped_lock scopeLock(mConsoleObjectsSynchronizationObject);
		const wstring objName = findConsoleObjectName(consoleObject);
		if (!objName.empty())
		{
			unregisterConsoleObject(objName.c_str(), bKeepState);
		}

	}

	void ConsoleManager::unregisterConsoleObject(const TCHAR* name, bool bKeepState)
	{
		std::scoped_lock lock(mConsoleObjectsSynchronizationObject);
		IConsoleObject* object = findConsoleObject(name);
		if (object)
		{
			IConsoleVariable* cvar = object->asVariable();

			if (cvar && bKeepState)
			{
				cvar->setFlags(ECVF_Unregistered);
			}
			else
			{
				mConsoleObjects.erase(name);
				object->release();
			}
		}
	}

	IConsoleCommand* ConsoleManager::registerConsoleCommand(const TCHAR* name, const TCHAR* help, const ConsoleCommandDelegate& command, uint32 flags)
	{
		return addConsoleObject(name, new ConsoleCommand(command, help, (EConsoleVariableFlags)flags))->asCommand();
	}

	IConsoleCommand* ConsoleManager::registerConsoleCommand(const TCHAR* name, const TCHAR* help, const ConsoleCommandWithArgsDelegate& command, uint32 flags)
	{
		return addConsoleObject(name, new ConsoleCommandWithArgs(command, help, (EConsoleVariableFlags)flags))->asCommand();
	}

	IConsoleCommand* ConsoleManager::registerConsoleCommand(const TCHAR* name, const TCHAR* help, const ConsoleCommandWithWorldArgsAndOutputDeviceDelegate& command, uint32 flags)
	{
		return addConsoleObject(name, new ConsoleCommandWithWorldArgsAndOutputDevice(command, help, (EConsoleVariableFlags)flags))->asCommand();
	}

	IConsoleObject* ConsoleManager::addConsoleObject(const TCHAR* name, IConsoleObject* obj)
	{
		BOOST_ASSERT(name);
		BOOST_ASSERT(*name != 0);
		BOOST_ASSERT(obj);

		std::scoped_lock lock(mConsoleObjectsSynchronizationObject);
		IConsoleObject* existingObj = mConsoleObjects.findRef(name);

		if (obj->getFlags() & ECVF_Scalability)
		{
			BOOST_ASSERT(!(obj->getFlags() & ECVF_Cheat));
			BOOST_ASSERT(!(obj->getFlags() & ECVF_ReadOnly));
		}

		if (obj->getFlags() & ECVF_RenderThreadSafe)
		{
			if (obj->asCommand())
			{
				BOOST_ASSERT(0);
			}
		}

		if (existingObj)
		{
#if 0
#else
			const bool bCanUpdateOrReplaceObj = existingObj->asVariable() && existingObj->testFlags(ECVF_Unregistered);
#endif
			if (!bCanUpdateOrReplaceObj)
			{
				AIR_LOG(logConsoleManager, Warning, TEXT("console object named"));
			}

			IConsoleVariable* existingVar = existingObj->asVariable();
			IConsoleCommand* existingCmd = existingObj->asCommand();
			const int existingType = existingVar ? existingCmd ? 3 : 2 : 1;

			IConsoleVariable* var = obj->asVariable();
			IConsoleCommand* cmd = obj->asCommand();
			const int newType = var ? cmd ? 3 : 2 : 1;

			if (existingType != newType)
			{
				AIR_LOG(LogConsoleManager, Fatal, TEXT("Console object named"));
			}

			if (existingVar && var)
			{
				if (existingVar->testFlags(ECVF_CreatedFromIni))
				{
					if (!var->testFlags(ECVF_Cheat))
					{
						var->set(existingVar->getString().c_str(), (EConsoleVariableFlags)((uint32)existingVar->getFlags() & ECVF_SetByMask));
					}

					existingVar->release();
					mConsoleObjects.emplace(name, var);
					return var;
				}
				else
				{
					existingVar->setFlags(var->getFlags());
					existingVar->setHelp(var->getHelp());
					var->release();
					return existingVar;
				}
			}
			else if (existingCmd)
			{
				mConsoleObjects.emplace(name, cmd);
				existingCmd->release();
				return cmd;
			}
			return nullptr;
		}
		else
		{
			mConsoleObjects.emplace(name, obj);
			return obj;
		}
	}
}