#pragma once
#include "CoreType.h"
#include "Serialization/Archive.h"
namespace Air
{
	class MemoryArchive : public Archive
	{
	public:
		virtual wstring getArchiveName() const { return TEXT("MemoryArchive"); }
		void seek(int64 inPos) final
		{
			mOffset = inPos;
		}
		int64 tell() final
		{
			return mOffset;
		}

	protected:

		MemoryArchive()
			:Archive(), mOffset(0)
		{}
		int64 mOffset;
	};
}