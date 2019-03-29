#include "GenericPlatform/GenericPlatformFile.h"
#include "Misc/Paths.h"

#if AIR_TS_LIBRARY_FILESYSTEM_V3_SUPPORT
#include <experimental/filesystem>
#elif AIR_TS_LIBRARY_FILESYSTEM_V2_SUPPORT
#include <filesystem>
namespace std
{
	namespace experimental
	{
		namespace filesystem = std::tr2::sys;
	}
}

#endif
namespace Air
{
	int64 IFileHandle::size()
	{
		int64 current = tell();
		seekFromEnd();
		int64 result = tell();
		seek(current);
		return result;
	}

	wstring IPlatformFile::convertToAbsolutePathForExternalAppForWrite(const TCHAR* filename)
	{
		return Paths::convertRelativePathToFull(filename);
	}

	int64 IPlatformFile::fileSize(const TCHAR* filename)
	{
		if (filesystem::exists(filename))
		{
			return filesystem::file_size(filename);
		}
		return INDEX_NONE;
	}

	bool IPlatformFile::fileExists(const TCHAR* filename)
	{
		return filesystem::exists(filename);
	}

	bool IPlatformFile::deleteFile(const TCHAR* filename)
	{
		return filesystem::remove(filename);
	}

	bool IPlatformFile::deleteDirectoryRecursively(const TCHAR* directory)
	{
		return filesystem::remove_all(directory);
	}

	bool IPlatformFile::deleteDirectory(const TCHAR* directory)
	{
		return filesystem::remove(directory);
	}

	bool IPlatformFile::directoryExists(const TCHAR* directory)
	{
		return filesystem::exists(directory);
	}
}