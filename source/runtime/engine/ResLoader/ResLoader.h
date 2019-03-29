#pragma once
#include "EngineMininal.h"
#include "HAL/Runnable.h"
#include "boost/lockfree/spsc_queue.hpp"
#include "HAL/PlatformMisc.h"
#include "HAL/Event.h"
#include "HAL/PlatformProcess.h"
#include "HAL/ThreadHeartBeat.h"
namespace Air
{
	class RClass;
	class Object;

	class ENGINE_API ResLoadingDesc : boost::noncopyable
	{
	public:

		virtual ~ResLoadingDesc() {}

		virtual uint64 type() const = 0;

		virtual bool stateLess() const = 0;

		virtual bool match(ResLoadingDesc const & rhs) const = 0;

		virtual wstring name()const = 0;

		virtual std::shared_ptr<Object> createResource();
		virtual void subThreadStage() = 0;

		virtual void mainThreadStage() = 0;

		virtual bool hasSubThreadStage() const = 0;

		virtual void copyDataFrom(ResLoadingDesc const & rhs) = 0;

		virtual std::shared_ptr<void> cloneResourceFrom(std::shared_ptr<void> const & resource) = 0;


		virtual std::shared_ptr<Object>	resource() const = 0;
	protected:
		virtual void setResource(std::shared_ptr<Object> ptr) = 0;
	public:
		RClass* mResourceClass;
	};

	typedef std::shared_ptr<ResLoadingDesc> ResLoadingDescPtr;

	class ENGINE_API ResLoader : public boost::noncopyable, public Runnable
	{
	private:


	public:
		ResLoader();


		~ResLoader();

		static ResLoader& instance();

		virtual uint32 run();

		virtual bool init();

		virtual void stop();

		void update();

	public:

		std::shared_ptr<void> syncQuery(ResLoadingDescPtr const & resDesc);
		std::shared_ptr<void> asyncQuery(ResLoadingDescPtr const & resDesc);


		template<typename T>
		std::shared_ptr<T> syncQueryT(ResLoadingDescPtr const & resDesc)
		{
			return std::static_pointer_cast<T>(this->syncQuery(resDesc));
		}


		template<typename T>
		std::shared_ptr<T> asyncQueryT(ResLoadingDescPtr const & resDesc)
		{
			return std::static_pointer_cast<T>(this->asyncQuery(resDesc));
		}

		static FORCEINLINE bool isMultithreaded()
		{
			return true;
		}

		static FORCEINLINE bool isInAsyncLoadThread()
		{
			bool result = false;
			if (isMultithreaded())
			{
				result = PlatformTLS::getCurrentThreadId() == mAsyncLoadingThreadId;
			}
			return result;
		}

		FORCEINLINE bool isAsyncLoading()
		{
			PlatformMisc::memoryBarrier();
			return mLoadingResCounter.getValue() != 0;
		}

	private:
		std::shared_ptr<void> findMatchLoadedResource(ResLoadingDescPtr const & resDesc);


		void addLoadedResource(ResLoadingDescPtr const & res_desc, std::shared_ptr<void> const & res);

		void removeUnrefResources();
	private:
		enum LoadingStatus
		{
			LS_Loading,
			LS_Complete,
			LS_CanBeRemoved,
		};

		static std::unique_ptr<ResLoader> mInstance;

		RunnableThread* mThread;

		static uint32 mAsyncLoadingThreadId;

		ThreadSafeCounter mStopTaskCounter;

		ThreadSafeCounter isLoadingSuspended;

		Event* mThreadResumedEvent;

		Event* mThreadSuspendedEvent;

		std::vector<std::pair<ResLoadingDescPtr, std::weak_ptr<void>>> mLoadedRes;

		std::vector<std::pair<ResLoadingDescPtr, std::shared_ptr<volatile LoadingStatus>>> mLoadingRes;
		ThreadSafeCounter mLoadingResCounter;

		boost::lockfree::spsc_queue<std::pair<ResLoadingDescPtr, std::shared_ptr<volatile LoadingStatus>>, boost::lockfree::capacity<1024>> mLoadingResQueue;


		std::mutex mLoadingMutex;
		std::mutex mLoadedMutex;
	};

	ENGINE_API void processAsyncLoading(bool bUseTimeLimit, bool bUseFullTimeLimit, float timeLimit);

	ENGINE_API void flushAsyncLoading();
}