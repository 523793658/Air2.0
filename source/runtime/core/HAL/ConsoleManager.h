#include "HAL/IConsoleManager.h"
#include "Containers/Map.h"
#include "HAL/CriticalSection.h"
#include <mutex>
namespace Air
{
	class CORE_API ConsoleManager : public IConsoleManager
	{
	public:
		ConsoleManager()
			:bHistoryWasLoaded(false)
		{

		}

		~ConsoleManager()
		{
			for (auto pairIt : mConsoleObjects)
			{
				IConsoleObject* obj = pairIt.second;
				delete obj;
			}
		}

		virtual void unregisterConsoleObject(IConsoleObject* consoleObject, bool bKeepState = true) override;

		virtual void unregisterConsoleObject(const TCHAR* name, bool bKeepState = true) override;

		virtual IConsoleCommand* registerConsoleCommand(const TCHAR* name, const TCHAR* help, const ConsoleCommandDelegate& command, uint32 flags) override;

		virtual IConsoleCommand* registerConsoleCommand(const TCHAR* name, const TCHAR* help, const ConsoleCommandWithArgsDelegate& command, uint32 flags) override;

		virtual IConsoleCommand* registerConsoleCommand(const TCHAR* name, const TCHAR* help, const ConsoleCommandWithWorldArgsAndOutputDeviceDelegate& command, uint32 flags) override;

		wstring findConsoleObjectName(const IConsoleObject* obj) const;

		virtual IConsoleObject* findConsoleObject(const TCHAR* name, bool bTrackFrequentCalls = true) const override;

		IConsoleObject* findConsoleObjectUnfiltered(const TCHAR* name) const;

		IConsoleObject* addConsoleObject(const TCHAR* name, IConsoleObject* obj);
	private:
		TMap<wstring, IConsoleObject*> mConsoleObjects;
		bool bHistoryWasLoaded;
		TMap<wstring, TArray<wstring>> mHistoryEntryMap;

		mutable std::mutex mConsoleObjectsSynchronizationObject;
	};
}