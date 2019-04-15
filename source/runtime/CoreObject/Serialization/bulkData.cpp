#include "bulkData.h"
namespace Air
{
	int32 UntypedBulkData::getElementCount() const
	{
		return mElementCount;
	}

	int32 UntypedBulkData::getBulkDataSize() const
	{
		return getElementCount() * getElementSize();
	}

	int32 ByteBulkData::getElementSize() const
	{
		return sizeof(uint8);
	}

	void* UntypedBulkData::lock(uint32 lockFlags)
	{
		BOOST_ASSERT(mLockStatus == LOCKSTATUS_Unlocked);
		makeSureBulkDataIsLoaded();

		if (lockFlags & LOCK_READ_WRITE)
		{
			mLockStatus = LOCKSTATUS_ReadWriteLock;

#if WITH_EDITOR
			if (mAttachedAr)
			{
				mAttachedAr->detachBulkData(this, false);
				BOOST_ASSERT(mAttachedAr == nullptr);
			}
#endif
		}
		else if (lockFlags & LOCK_READ_ONLY)
		{
			mLockStatus = LOCKSTATUS_ReadOnlyLock;
		}
		else
		{
			AIR_LOG(LogSerialization, Fatal, TEXT("Unknown lock flag &i"), lockFlags);
		}
		return mBulkData.get();
	}

	void UntypedBulkData::loadDataIntoMemory(void* dest)
	{
#if WITH_EDITOR
		BOOST_ASSERT(mAttachedAr);
		int64 pushedPos = mAttachedAr->tell();

		mAttachedAr->seek(mBulkDataOffsetInFile);
		serializeBulkData(*mAttachedAr, dest);
		mAttachedAr->seek(pushedPos);
#endif
	}

	void UntypedBulkData::makeSureBulkDataIsLoaded()
	{
		if (!mBulkData)
		{
			const int32 bytesNeeded = getBulkDataSize();
			mBulkData.reallocate(bytesNeeded, mBulkDataAlignment);
			if (bytesNeeded > 0)
			{
				loadDataIntoMemory(mBulkData.get());
			}
		}
	}

	void UntypedBulkData::serializeBulkData(Archive& ar, void* data)
	{
		const int32 bulkDataSize = getBulkDataSize();
		if (bulkDataSize == 0)
		{
			return;
		}
		bool bSerializeInBulk = true;
		if (requiresSingleElementSerialization(ar) || (ar.isSaving() && (getElementSize() > 1)))
		{
			bSerializeInBulk = false;
		}

		if (bSerializeInBulk)
		{
			ar.serialize(data, getBulkDataSize());
		}
		else
		{
			serializeElements(ar, data);
		}
	}

	void UntypedBulkData::serializeElements(Archive& ar, void* data)
	{
		for (int32 elementIndex = 0; elementIndex < mElementCount; elementIndex++)
		{
			serializeElement(ar, data, elementIndex);
		}
	}

	void ByteBulkData::serializeElement(Archive& ar, void* data, int32 elementIndex)
	{
		uint8& byteData = *((uint8*)data + elementIndex);
		ar << byteData;
	}


	const void* UntypedBulkData::lockReadOnly() const
	{
		BOOST_ASSERT(mLockStatus == LOCKSTATUS_Unlocked);
		UntypedBulkData* mutable_this = const_cast<UntypedBulkData*>(this);
		mutable_this->makeSureBulkDataIsLoaded();
		mutable_this->mLockStatus = LOCKSTATUS_ReadOnlyLock;
		BOOST_ASSERT(mBulkData);
		return mBulkData.get();
	}

	void* UntypedBulkData::realloc(int32 inElementCount)
	{
		BOOST_ASSERT(mLockStatus == LOCKSTATUS_ReadWriteLock);
		mElementCount = inElementCount;
		mBulkData.reallocate(getBulkDataSize(), mBulkDataAlignment);
		return mBulkData.get();
	}

	void UntypedBulkData::unlock() const
	{
		BOOST_ASSERT(mLockStatus != LOCKSTATUS_Unlocked);
		UntypedBulkData* mutable_this = const_cast<UntypedBulkData*>(this);
		mutable_this->mLockStatus = LOCKSTATUS_Unlocked;
		if (mBulkDataFlags & BULKDATA_SingleUse)
		{
			mutable_this->mBulkData.deallocate();
		}
	}

	bool UntypedBulkData::requiresSingleElementSerialization(Archive& ar)
	{
		return false;
	}
}