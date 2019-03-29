#include "ArchiveProxy.h"
namespace Air
{
	ArchiveProxy::ArchiveProxy(Archive& inInnerArchive)
		:Archive(inInnerArchive)
		,mInnerArchive(inInnerArchive)
	{

	}

	wstring ArchiveProxy::getArchiveName() const
	{
		return mInnerArchive.getArchiveName();
	}
}