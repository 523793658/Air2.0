#pragma once
#include "CoreType.h"
#include "Serialization/Archive.h"
#include "Containers/String.h"
namespace Air
{
	class BufferReaderBase : public Archive
	{
	public:
		BufferReaderBase(void* data, int64 size, bool bInFreeOnClose, bool bIsPersistent = false)
			:mReaderData(data),
			mReaderPos(0),
			mReaderSize(size),
			bFreeOnClose(bInFreeOnClose)
		{
			mArIsLoading = true;
			mArIsPersistent = bIsPersistent;
		}

		void serialize(void* v, int64 length) final
		{
			BOOST_ASSERT(mReaderPos >= 0);
			BOOST_ASSERT(mReaderPos + length <= mReaderSize);
			Memory::memcpy(v, (uint8*)mReaderData + mReaderPos, length);
			mReaderPos += length;
		}

		~BufferReaderBase()
		{
			close();
		}

		bool close()
		{
			if (bFreeOnClose)
			{
				Memory::free(mReaderData);
				mReaderData = nullptr;
			}
			return !mArIsError;
		}

		int64 tell() final
		{
			return mReaderPos;
		}
		int64 totalSize() final
		{
			return mReaderSize;
		}

		void seek(int64 inPos) final
		{
			BOOST_ASSERT(inPos >= 0);
			BOOST_ASSERT(inPos <= mReaderSize);
			mReaderPos = inPos;
		}

		bool atEnd() 
		{
			return mReaderPos >= mReaderSize;
		}

		virtual wstring getArchiveName() const
		{
			return TEXT("BufferReaderBase");
		}
	protected:
		void* mReaderData;
		int64 mReaderPos;
		int64 mReaderSize;
		bool bFreeOnClose;
	};

	class BufferReader final : public BufferReaderBase
	{
	public:
		BufferReader(void* data, int64 size, bool bInFreeOnClose, bool bIsPersistent = false)
			:BufferReaderBase(data, size, bInFreeOnClose, bIsPersistent)
		{}

		virtual wstring getArchiveName() const { return TEXT("BufferReader"); }
	};
}