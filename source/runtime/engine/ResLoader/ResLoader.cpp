#include "ResLoader/ResLoader.h"
#include "HAL/RunnableThread.h"
#include "HAL/PlatformProcess.h"
#include "Class.h"

namespace
{
	std::mutex singletonMutex;
}

namespace Air
{
	uint32 ResLoader::mAsyncLoadingThreadId = 0;
	std::unique_ptr<ResLoader> ResLoader::mInstance;

	ResLoader& ResLoader::instance()
	{
		if (!mInstance)
		{
			std::lock_guard<std::mutex> lock(singletonMutex);
			if (!mInstance)
			{
				mInstance = makeUniquePtr<ResLoader>();
			}
		}
		return *mInstance;
	}


	ResLoader::ResLoader()
	{

		mThreadSuspendedEvent = PlatformProcess::getSynchEventFromPool();
		mThreadResumedEvent = PlatformProcess::getSynchEventFromPool();

		if (ResLoader::isMultithreaded())
		{
			mThread = RunnableThread::create(this, TEXT("AsyncLoadingThread"), 0, TPri_Normal);
		}
		else
		{
			mThread = nullptr;
		}
	}

	bool ResLoader::init()
	{
		return true;
	}

	void ResLoader::stop()
	{
		mStopTaskCounter.increment();
	}

	uint32 ResLoader::run()
	{
		mAsyncLoadingThreadId = PlatformTLS::getCurrentThreadId();
		bool bWasSuspendedLastFrame = false;
		while (mStopTaskCounter.getValue() == 0)
		{
			if (isLoadingSuspended.getValue() == 0)
			{
				if (bWasSuspendedLastFrame)
				{
					bWasSuspendedLastFrame = false;
					mThreadResumedEvent->trigger();
				}
				bool bDidSomething = false;
				std::pair<ResLoadingDescPtr, std::shared_ptr<volatile LoadingStatus>> resPair;
				while (mLoadingResQueue.pop(resPair))
				{
					if (LS_Loading == *resPair.second)
					{
						resPair.first->subThreadStage();
						*resPair.second = LS_Complete;
					}
				}
				PlatformProcess::sleep(0.01f);
			}
			else if (!bWasSuspendedLastFrame)
			{
				bWasSuspendedLastFrame = true;
				mThreadSuspendedEvent->trigger();
			}
			else
			{
				PlatformProcess::sleepNoStats(0.001f);
			}
		}
		return 0;
	}

	void ResLoader::update()
	{
		std::vector<std::pair<ResLoadingDescPtr, std::shared_ptr<volatile LoadingStatus>>> tmpLoadingRes;
		{
			std::lock_guard<std::mutex> lock(mLoadingMutex);
			tmpLoadingRes = mLoadingRes;
		}
		for (auto & lrq : tmpLoadingRes)
		{
			if (LS_Complete == *lrq.second)
			{
				ResLoadingDescPtr const & resDesc = lrq.first;
				std::shared_ptr<void> res;
				std::shared_ptr<void> loadedRes = this->findMatchLoadedResource(resDesc);
				if (loadedRes)
				{
					if (!resDesc->stateLess())
					{
						res = resDesc->cloneResourceFrom(loadedRes);
						if (res != loadedRes)
						{
							this->addLoadedResource(resDesc, res);
						}
					}
				}
				else
				{
					resDesc->mainThreadStage();
					res = resDesc->resource();
					this->addLoadedResource(resDesc, res);
				}
				*lrq.second = LS_CanBeRemoved;
			}
		}
		{
			std::lock_guard<std::mutex> lock(mLoadingMutex);
			for (auto iter = mLoadingRes.begin(); iter != mLoadingRes.end();)
			{
				if (LS_CanBeRemoved == *(iter->second))
				{
					iter = mLoadingRes.erase(iter);
					mLoadingResCounter.decrement();
				}
				else
				{
					++iter;
				}
			}
		}
	}

	std::shared_ptr<void> ResLoader::findMatchLoadedResource(ResLoadingDescPtr const & resDesc)
	{
		std::lock_guard<std::mutex> lock(mLoadedMutex);

		std::shared_ptr<void> loadedRes;
		for (auto const & lr : mLoadedRes)
		{
			if (lr.first->match(*resDesc))
			{
				loadedRes = lr.second.lock();
				break;
			}
		}
		return loadedRes;
	}

	ResLoader::~ResLoader()
	{
		mStopTaskCounter.increment();
		PlatformProcess::returnSynchEventToPool(mThreadResumedEvent);
		PlatformProcess::returnSynchEventToPool(mThreadSuspendedEvent);
		delete mThread;
		mThread = nullptr;
	}

	void ResLoader::addLoadedResource(ResLoadingDescPtr const & res_desc, std::shared_ptr<void> const & res)
	{
		std::lock_guard<std::mutex> lock(mLoadedMutex);
		bool found = false;
		for (auto& cDesc : mLoadedRes)
		{
			if (cDesc.first == res_desc)
			{
				cDesc.second = std::weak_ptr<void>(res);
				found = true;
				break;
			}
		}
		if (!found)
		{
			mLoadedRes.emplace_back(res_desc, std::weak_ptr<void>(res));
		}
	}

	std::shared_ptr<void> ResLoader::syncQuery(ResLoadingDescPtr const & resDesc)
	{
		this->removeUnrefResources();
		std::shared_ptr<void> loadedRes = this->findMatchLoadedResource(resDesc);
		std::shared_ptr<void> res;
		if (loadedRes)
		{
			if (resDesc->stateLess())
			{
				res = loadedRes;
			}
			else
			{
				res = resDesc->cloneResourceFrom(loadedRes);
				if (res != loadedRes)
				{
					this->addLoadedResource(resDesc, res);
				}
			}
		}
		else
		{
			std::shared_ptr<volatile LoadingStatus> asyncIsDone;
			bool found = false;
			{
				std::lock_guard<std::mutex> lock(mLoadingMutex);
				for (auto const & lrq : mLoadingRes)
				{
					if (lrq.first->match(*resDesc))
					{
						resDesc->copyDataFrom(*lrq.first);
						res = lrq.first->resource();
						asyncIsDone = lrq.second;
						found = true;
						break;
					}
				}
			}
			if (found)
			{
				*asyncIsDone = LS_Complete;
			}
			else
			{
				res = resDesc->createResource();
			}
			if (resDesc->hasSubThreadStage())
			{
				resDesc->subThreadStage();
			}
			resDesc->mainThreadStage();
			res = resDesc->resource();
			this->addLoadedResource(resDesc, res);
		}
		return res;
	}

	std::shared_ptr<void> ResLoader::asyncQuery(ResLoadingDescPtr const & resDesc)
	{
		this->removeUnrefResources();
		std::shared_ptr<void> res;
		std::shared_ptr<void> loadedRes = this->findMatchLoadedResource(resDesc);
		if (loadedRes)
		{
			if (resDesc->stateLess())
			{
				res = loadedRes;
			}
			else
			{
				res = resDesc->cloneResourceFrom(loadedRes);
				if (res != loadedRes)
				{
					this->addLoadedResource(resDesc, res);
				}
			}
		}
		else
		{
			std::shared_ptr<volatile LoadingStatus> asyncIsDone;
			bool found = false;
			{
				std::lock_guard<std::mutex> lock(mLoadingMutex);
				for (auto const & lrq : mLoadingRes)
				{
					if (lrq.first->match(*resDesc))
					{
						resDesc->copyDataFrom(*lrq.first);
						res = lrq.first->resource();
						asyncIsDone = lrq.second;
						found = true;
						break;
					}
				}
			}
			if (found)
			{
				if (!resDesc->stateLess())
				{
					std::lock_guard<std::mutex> lock(mLoadingMutex);
					mLoadingResCounter.increment();
					mLoadingRes.emplace_back(resDesc, asyncIsDone);
				}
			}
			else
			{
				if (resDesc->hasSubThreadStage())
				{
					res = resDesc->createResource();
					asyncIsDone = MakeSharedPtr<LoadingStatus>(LS_Loading);
					{
						std::lock_guard<std::mutex> lock(mLoadingMutex);
						mLoadingResCounter.increment();
						mLoadingRes.emplace_back(resDesc, asyncIsDone);
					}
					mLoadingResQueue.push(std::make_pair(resDesc, asyncIsDone));
				}
				else
				{
					resDesc->mainThreadStage();
					res = resDesc->resource();
					this->addLoadedResource(resDesc, res);
				}
			}
		}
		return res;
	}

	void ResLoader::removeUnrefResources()
	{
		std::lock_guard<std::mutex> lock(mLoadedMutex);
		for (auto iter = mLoadedRes.begin(); iter != mLoadedRes.end();)
		{
			if (iter->second.lock())
			{
				++iter;
			}
			else
			{
				iter = mLoadedRes.erase(iter);
			}
		}
	}

	std::shared_ptr<Object> ResLoadingDesc::createResource()
	{
		Object* obj = newObject<Object>(nullptr, mResourceClass, name(), RF_NeedPostLoad);
		auto ptr = std::shared_ptr<Object>(obj);
		setResource(ptr);
		return ptr;
	}

	void processAsyncLoading(bool bUseTimeLimit, bool bUseFullTimeLimit, float timeLimit)
	{
		ResLoader::instance().update();
	}

	void flushAsyncLoading()
	{
		if (isAsyncLoading())
		{
			ResLoader& resLoader = ResLoader::instance();
			while (isAsyncLoading())
			{
				resLoader.update();
				if (resLoader.isMultithreaded())
				{
					ThreadHeartBeat::get().heartBeat();
					PlatformProcess::sleepNoStats(0.0001f);
				}
			}
		}
	}
}