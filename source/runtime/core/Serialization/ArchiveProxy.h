#pragma once
#include "Serialization/Archive.h"
namespace Air
{
	class ArchiveProxy : public Archive
	{
	public:
		CORE_API ArchiveProxy(Archive& inInnerArchive);

		virtual void serialize(void* v, int64 length) override
		{
			mInnerArchive.serialize(v, length);
		}

		virtual void serializeBits(void* bits, int64 lengthBits) override
		{
			mInnerArchive.serializeBits(bits, lengthBits);
		}

		CORE_API virtual wstring getArchiveName() const override;

		virtual int64 tell() override
		{
			return mInnerArchive.tell();
		}

		virtual int64 totalSize() override
		{
			return mInnerArchive.totalSize();
		}

		virtual void seek(int64 inPos) override
		{
			mInnerArchive.seek(inPos);
		}




	protected:
		Archive& mInnerArchive;
	};
}