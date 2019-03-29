#pragma once
#include "CoreType.h"
#include "Containers/String.h"
#include <map>
#include <mutex>
namespace Air
{

	class CORE_API ThreadManager
	{
		std::map<uint32, class RunnableThread*> mThreads;

		std::mutex mThreadManagerSingleton;
	private:
		ThreadManager() {};


	public:
		void addThread(uint32 threadId, class RunnableThread* thread);

		void removeThread(class RunnableThread* thread);

		void tick();

		const wstring& getThreadName(uint32 threadId);

		static ThreadManager& get();
	};
}