#pragma once
#include "CoreObject.h"
namespace Air
{
	enum EBulkDataLockStatus
	{
		LOCKSTATUS_Unlocked					=0,
		LOCKSTATUS_ReadOnlyLock				=1,
		LOCKSTATUS_ReadWriteLock			=2,
	};

	enum EBulkDataFlags
	{
		BULKDATA_None	= 0,
		BULKDATA_SingleUse = 1 << 3,
	};

	enum EBulkDataLockFlags
	{
		LOCK_READ_ONLY						=1,
		LOCK_READ_WRITE						=2,
	};


	struct COREOBJECT_API UntypedBulkData
	{
	private:
		struct AllocatedPtr
		{
			AllocatedPtr()
				:mPtr(nullptr)
				,bAllocated(false)
			{}

			AllocatedPtr(AllocatedPtr& other)
				:mPtr(other.mPtr)
				, bAllocated(other.bAllocated)
			{
				other.mPtr = nullptr;
				other.bAllocated = false;
			}

			AllocatedPtr& operator = (AllocatedPtr&& other)
			{
				Swap(*this, other);
				other.deallocate();
				return *this;
			}

			~AllocatedPtr()
			{
				Memory::free(mPtr);
			}

			void* get() const
			{
				return mPtr;
			}

			FORCEINLINE explicit operator bool() const
			{
				return bAllocated;
			}

			void reallocate(int32 count, int32 alignment = DEFAULT_ALIGNMENT)
			{
				if (count)
				{
					mPtr = Memory::realloc(mPtr, count, alignment);
				}
				else
				{
					Memory::free(mPtr);
					mPtr = nullptr;
				}

				bAllocated = true;
			}

			void* releaseWithoutDeallocating()
			{
				void* result = mPtr;
				mPtr = nullptr;
				bAllocated = false;
				return result;
			}


			void deallocate()
			{
				Memory::free(mPtr);
				mPtr = nullptr;
				bAllocated = false;
			}

		private:
			AllocatedPtr(const AllocatedPtr&);

			AllocatedPtr& operator= (const AllocatedPtr&);

			void* mPtr;
			bool bAllocated;

		
		};
	public:
		friend class LinkerLoad;


		int32 getElementCount() const;

		virtual int32 getElementSize() const = 0;

		int32 getBulkDataSize() const;

		void* lock(uint32 lockFlags);

		const void* lockReadOnly() const;

		void* realloc(int32 inElementCount);

		void unlock() const;

		bool isLocked() const { return mLockStatus != LOCKSTATUS_Unlocked; }

		void loadDataIntoMemory(void* dest);

		void serializeBulkData(Archive& ar, void* data);

		void makeSureBulkDataIsLoaded();

		virtual bool requiresSingleElementSerialization(Archive& ar);
	protected:
		virtual void serializeElements(Archive& ar, void* data);

		virtual void serializeElement(Archive& ar, void* data, int32 elementIndex) = 0;
	public:

		uint32 mBulkDataFlags;

		uint32 mElementCount;

		int64 mBulkDataOffsetInFile;

		int32 mBulkDataSizeOnDisk;

		int32 mBulkDataAlignment;

		AllocatedPtr mBulkData;

		AllocatedPtr mBulkDataAsync;

		uint32 mLockStatus;

	protected:
		wstring mFilename;
#if WITH_EDITOR
		Archive* mAttachedAr;
#endif
	};


	struct COREOBJECT_API ByteBulkData : public UntypedBulkData
	{
		virtual int32 getElementSize() const override;

		void serializeElement(Archive& ar, void* data, int32 elementIndex) override;
	};


}