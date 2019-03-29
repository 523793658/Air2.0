#include "PlatformFileManager.h"
#include "GenericPlatform/GenericPlatformFile.h"
namespace Air
{
	IPlatformFile& PlatformFileManager::getPlatformFile()
	{
		if (mTopmostPlatformFile == nullptr)
		{
			mTopmostPlatformFile = &IPlatformFile::getPlatformPhysical();
		}
		return *mTopmostPlatformFile;
	}

	PlatformFileManager& PlatformFileManager::get()
	{
		static PlatformFileManager singleton;
		return singleton;
	}

}