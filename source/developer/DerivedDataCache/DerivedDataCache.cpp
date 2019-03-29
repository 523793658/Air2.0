#include "DerivedDataCache/DerivedDataCacheInterface.h"
#include "Async/AsyncWork.h"
namespace Air
{



	class DerivedDataCache : public DerivedDataCacheInterface
	{
		friend class BuildAsyncWorker;
		class BuildAsyncWorker : public NonAbandonableTask
		{
		public:
			BuildAsyncWorker(DerivedDataCacheInterface* inDataDeriver, const TCHAR* inCacheKey, bool bInSynchronousForStats)
				:bSuccess(false)
				,bSynchronousForStats(bInSynchronousForStats)
				,bDataWasBuilt(false)
				,mDataDeriver(inDataDeriver)
				,mCacheKey(inCacheKey)
			{

			}

			void doWork();

			bool	bSuccess;
			bool	bSynchronousForStats;
			bool	bDataWasBuilt;
			DerivedDataCacheInterface* mDataDeriver;
			wstring	mCacheKey;
			TArray<uint8>	mData;
		};

	public:
		virtual bool getSynchronous(class DerivedDataCacheInterface* dataDeriver, TArray<uint8>& outData, bool* bDataWasBuilt = nullptr)
		{
			BOOST_ASSERT(dataDeriver);
			wstring cacheKey = DerivedDataCache::buildCacheKey(dataDeriver);
			
		}

	private:
		static wstring buildCacheKey(DerivedDataCacheInterface* dataDeriver)
		{
			//wstring result = DerivedDataCacheInterface::buil
			return TEXT("");
		}
	};
}