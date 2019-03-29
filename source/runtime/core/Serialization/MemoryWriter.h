#pragma once
#include "Serialization/MemoryArchive.h"
#include "Containers/Array.h"
#include "HAL/AirMemory.h"
namespace Air
{
	class MemoryWriter : public MemoryArchive
	{
	public:
		MemoryWriter(TArray<uint8>& inBytes, bool bIsPersistent = false, bool bSetOffset = false, const wstring inArchiveName = TEXT(""))
			:MemoryArchive()
			, mBytes(inBytes)
			, mArchiveName(inArchiveName)
		{
			mArIsPersistent = bIsPersistent;
			mArIsSaving = true;
			if (bSetOffset)
			{
				mOffset = inBytes.size();
			}
		}

		virtual void serialize(void* data, int64 num) override
		{
			const int64 numBytesToAdd = mOffset + num - mBytes.size();
			if (numBytesToAdd > 0)
			{
				const int64 newArrayCount = mBytes.size() + numBytesToAdd;
				if (newArrayCount >= std::numeric_limits<int32>::max())
				{

				}
				mBytes.addUninitialized((int32)numBytesToAdd);
			}
			BOOST_ASSERT((mOffset + num) <= mBytes.size());
			if (num)
			{
				Memory::memcpy(&mBytes[mOffset], data, num);
				mOffset += num;
			}
		}

		virtual wstring getArchiveName() const override
		{
			return TEXT("MemoryWriter");
		}

		int64 totalSize() override
		{
			return mBytes.size();
		}

	protected:
		TArray<uint8>& mBytes;
		const wstring mArchiveName;
	};
}