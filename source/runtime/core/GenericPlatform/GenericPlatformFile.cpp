#include "GenericPlatform/GenericPlatformFile.h"
#include "Misc/Paths.h"
#include "Math/Math.h"
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

	bool IPlatformFile::copyFile(const TCHAR* to, const TCHAR* from, EPlatformFileRead readFlags /* = EPlatformFileRead::None */, EPlatformFileWrite writeFlags /* = EPlatformFileWrite::None */)
	{
		const int64 maxBufferSize = 1024 * 1024;

		std::unique_ptr<IFileHandle> fromFile(openRead(from, (readFlags & EPlatformFileRead::AllowWrite) != EPlatformFileRead::None));
		if (!fromFile)
		{
			return false;
		}

		std::unique_ptr<IFileHandle> toFile(openWrite(to, false, (writeFlags & EPlatformFileWrite::AllowRead) != EPlatformFileWrite::None));

		if (!toFile)
		{
			return false;
		}

		int64 size = fromFile->size();
		if (size < 1)
		{
			BOOST_ASSERT(size == 0);
			return true;
		}
		int64 allocSize = Math::min<int64>(maxBufferSize, size);
		BOOST_ASSERT(allocSize);
		uint8* buffer = (uint8*)Memory::malloc(int32(allocSize));
		BOOST_ASSERT(buffer);
		while (size)
		{
			int64 thisSize = Math::min<int64>(allocSize, size);
			fromFile->read(buffer, thisSize);
			toFile->write(buffer, thisSize);
			size -= thisSize;
			BOOST_ASSERT(size >= 0);
		}
		Memory::free(buffer);
		return true;
	}
}