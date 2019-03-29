#pragma once
#include "CoreType.h"
#include "Serialization/MemoryArchive.h"
#include "Containers/Array.h"
#include "Math/Math.h"
#include <limits>
namespace Air
{
	class MemoryReader final : public MemoryArchive
	{
	public:
		MemoryReader(const TArray<uint8>& inBytes, bool bIsPersistent = false)
			:MemoryArchive()
			,mBytes(inBytes)
			,mLimitSize(std::numeric_limits<int64>::max())
		{
			mArIsLoading = true;
			mArIsPersistent = bIsPersistent;
		}

		virtual wstring getArchiveName() const { return TEXT("MemoryReader"); }
		int64 totalSize()
		{
			return Math::min((int64)mBytes.size(), mLimitSize);
		}

		void serialize(void* v, int64 length)
		{
			if (length && !mArIsError)
			{
				if (mOffset + length <= totalSize())
				{
					Memory::memcpy(v, &mBytes[mOffset], length);
					mOffset += length;
				}
				else
				{
					mArIsError = true;
				}
			}
		}

		void setLimitSize(int64 newLimitSize0)
		{
			mLimitSize = newLimitSize0;
		}
	protected:
		const TArray<uint8>& mBytes;
		int64 mLimitSize;
	};
}