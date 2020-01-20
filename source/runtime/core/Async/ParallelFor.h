#pragma once
#include "Async/TaskGraph.h"
#include "Misc/App.h"
#include "Math/Math.h"
#include "HAL/Event.h"
#include "HAL/PlatformProcess.h"
namespace Air
{
	class ParallelForTask;
	struct ParallelForData;
	

	class ParallelForTask
	{
		std::shared_ptr<ParallelForData> mData;
		int32 mTaskToSpawn;
	public:
		ParallelForTask(std::shared_ptr<ParallelForData>& inData, int32 inTaskToSpawn = 0)
			:mData(inData),
			mTaskToSpawn(inTaskToSpawn)
		{}

		static FORCEINLINE ENamedThreads::Type getDesiredThread()
		{
			return ENamedThreads::AnyHiPriThreadHiPriTask;
		}

		static FORCEINLINE ESubsequentsMode::Type getSubsequentsMode()
		{
			return ESubsequentsMode::FireAndForget;
		}
		void doTask(ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent);
	};

	struct ParallelForData
	{
		int32 mNum;
		int32 mBlockSize;
		int32 mLastBlockExtraNum;
		std::function<void(int32)> mBody;
		Event* mEvent;
		ThreadSafeCounter mIndexToDo;
		ThreadSafeCounter mNumCompleted;
		bool bExited;
		bool bTriggered;
		bool bSaveLastBlockForMaster;
		ParallelForData(int32 inTotalNum, int32 inNumThreads, bool bInSaveLastBlockForMaster, std::function<void(int32)> inBody)
			:mBody(inBody),
			mEvent(PlatformProcess::getSynchEventFromPool(false))
			, bExited(false)
			, bTriggered(false),
			bSaveLastBlockForMaster(bInSaveLastBlockForMaster)
		{
			BOOST_ASSERT(inTotalNum >= inNumThreads);
			mBlockSize = 0;
			mNum = 0;
			for (int32 div = 3; div; div++)
			{
				mBlockSize = inTotalNum / (inNumThreads * div);
				if (mBlockSize)
				{
					mNum = inTotalNum / mBlockSize;
					if (mNum >= inNumThreads + !!bSaveLastBlockForMaster)
					{
						break;
					}
				}
			}
			BOOST_ASSERT(mBlockSize && mNum);
			mLastBlockExtraNum = inTotalNum - mNum * mBlockSize;
			BOOST_ASSERT(mLastBlockExtraNum >= 0);
		}
		~ParallelForData()
		{
			BOOST_ASSERT(mIndexToDo.getValue() >= mNum);
			BOOST_ASSERT(mNumCompleted.getValue() == mNum);
			BOOST_ASSERT(bExited);
			PlatformProcess::returnSynchEventToPool(mEvent);
		}

		bool process(int32 taskToSpawn, std::shared_ptr<ParallelForData>& data, bool bMaster)
		{
			int32 maybeTasksLeft = mNum - mIndexToDo.getValue();
			if (taskToSpawn && maybeTasksLeft > 0)
			{
				taskToSpawn = Math::min<int32>(taskToSpawn, maybeTasksLeft);
				TGraphTask<ParallelForTask>::createTask().constructAndDispatchWhenReady(data, taskToSpawn - 1);
			}
			int32 localBlockSize = mBlockSize;
			int32 lockNum = mNum;
			bool bLocalSaveLastBlockForMaster = bSaveLastBlockForMaster;
			std::function<void(int32)>localBody(mBody);
			while (true)
			{
				int32 myIndex = mIndexToDo.increment() - 1;
				if (bLocalSaveLastBlockForMaster)
				{
					if (!bMaster && myIndex >= lockNum - 1)
					{
						break;
					}
					else if (bMaster && myIndex > lockNum - 1)
					{
						myIndex = lockNum - 1;
					}
				}
				if (myIndex < lockNum)
				{
					int32 thisBlockSize = localBlockSize;
					if (myIndex == lockNum - 1)
					{
						thisBlockSize += mLastBlockExtraNum;
					}
					for (int32 localIndex = 0; localIndex < thisBlockSize; localIndex++)
					{
						localBody(myIndex * localBlockSize + localIndex);
					}
					BOOST_ASSERT(!bExited);
					int32 localNumCoompleted = mNumCompleted.increment();
					if (localNumCoompleted == lockNum)
					{
						return true;
					}
					BOOST_ASSERT(localNumCoompleted < lockNum);
				}
				if (myIndex >= lockNum - 1)
				{
					break;
				}
			}
			return false;
		}
	};
	void ParallelForTask::doTask(ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent)
	{
		if (mData->process(mTaskToSpawn, mData, false))
		{
			BOOST_ASSERT(!mData->bTriggered);
			mData->bTriggered = true;
			mData->mEvent->trigger();
		}
	}
	

	inline void parallelFor(int32 num, std::function<void(int32)> body, bool bForceSingleThread = false)
	{
		BOOST_ASSERT(num >= 0);
		int32 anyThreadTasks = 0;
		if (num > 1 && !bForceSingleThread && App::shouldUseThreadingForPerformance())
		{
			anyThreadTasks = Math::min<int32>(TaskGraphInterface::get().getNumWorkerThreads(), num - 1);
		}
		if (!anyThreadTasks)
		{
			for (int32 index = 0; index < num; index++)
			{
				body(index);
			}
			return;
		}

		std::shared_ptr<ParallelForData> data = MakeSharedPtr<ParallelForData>(num, anyThreadTasks + 1, num > anyThreadTasks + 1, body);
		TGraphTask<ParallelForTask>::createTask().constructAndDispatchWhenReady(data, anyThreadTasks - 1);
		if (!data->process(0, data, true))
		{
			data->mEvent->wait();
			BOOST_ASSERT(data->bTriggered);
		}
		else
		{
			BOOST_ASSERT(!data->bTriggered);
		}
		BOOST_ASSERT(data->mNumCompleted.getValue() == data->mNum);
		data->bExited = true;


	}


	
}