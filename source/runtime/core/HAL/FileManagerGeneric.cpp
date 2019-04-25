#include "FileManagerGeneric.h"
#include "Serialization/Archive.h"
#include "Misc/Paths.h"
#include "Containers/StringUtil.h"
#include "HAL/AirMemory.h"
#include "Template/AirTemplate.h"
#include "Misc/SecureHash.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "Logging/LogMacros.h"
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

#include "boost/algorithm/string.hpp"


namespace Air
{
	using namespace std::experimental;

	void FileManagerGeneric::findFileRecursiveInternal(TArray<wstring>& fileNames, const TCHAR* startDirectory, const TCHAR* fileName, bool files, bool directories)
	{
		const wstring cleanFilename = Paths::getFilename(fileName);
		const bool findAllFiles = cleanFilename == TEXT("*") || cleanFilename == TEXT("*.*");
		wstring wildCard = findAllFiles ? TEXT("*") : cleanFilename;

		for (auto & item : filesystem::recursive_directory_iterator(startDirectory))
		{
			if (filesystem::is_directory(item.status()))
			{
				if (directories)
				{
					if (StringUtil::matchesWildcard(item.path().filename(), cleanFilename, ESearchCase::IgnoreCase))
					{
						fileNames.push_back(item.path().wstring());
					}
				}
			}
			else
			{
				if (files)
				{
					if (StringUtil::matchesWildcard(item.path().filename(), cleanFilename, ESearchCase::IgnoreCase))
					{
						fileNames.push_back(item.path().wstring());
					}
				}
			}
		}
	}

	void FileManagerGeneric::findFilesRecursive(TArray<wstring>& fileNames, const TCHAR* startDirectory, const TCHAR* filename, bool files, bool directories, bool bClearFileNames /* = true */)
	{

		if (bClearFileNames)
		{
			fileNames.clear();
		}

		findFileRecursiveInternal(fileNames, startDirectory, filename, files, directories);
	}

	void FileManagerGeneric::findFiles(TArray<wstring>& fileNames, const TCHAR* filename, bool files, bool directories)
	{
		//filesystem::directory_iterator
	}
	time_t FileManagerGeneric::getTimeStamp(const TCHAR* fileName)
	{
		filesystem::file_time_type t = filesystem::last_write_time(fileName);
		return decltype(t)::clock::to_time_t(t);
	}

	IFileManager& IFileManager::get()
	{
		static std::unique_ptr<FileManagerGeneric> autoDesctroySingleton;
		if (!autoDesctroySingleton)
		{
			autoDesctroySingleton = std::make_unique<FileManagerGeneric>();
		}

		return *autoDesctroySingleton;
	}

	bool FileManagerGeneric::makeDirectory(const TCHAR* path, bool tree /* = 0 */)
	{
		if (tree)
		{
			return filesystem::create_directories(path);
		}
		else
		{
			return filesystem::create_directory(path);
		}
	}


	Archive* FileManagerGeneric::createFileReader(const TCHAR* filename, uint32 readFlags /* = 0 */)
	{
		TCHAR finalPath[512];
		if (!Paths::isAbsolute(filename) && !Paths::fileExists(filename))
		{
			String::printf_b(finalPath, TEXT("%s/../../%s"), PlatformProcess::baseDir(), filename);
			if (!Paths::fileExists(finalPath))
			{
				return nullptr;
			}
			filename = finalPath;
		}
		IFileHandle* handle = getLowLevel().openRead(filename, !!(readFlags & FILEREAD_AllowWrite));
		if (!handle)
		{
			return nullptr;
		}
		return new ArchiveFileReaderGeneric(handle, filename, handle->size());
	}

	bool FileManagerGeneric::deleteDirectory(const TCHAR* path, bool requireExists /* = 0 */, bool tree /* = 0 */)
	{
		if (tree)
		{
			return getLowLevel().deleteDirectoryRecursively(path) || (!requireExists && !getLowLevel().directoryExists(path));
		}
		return getLowLevel().deleteDirectory(path) || (!requireExists && !getLowLevel().directoryExists(path));
	}

	wstring FileManagerGeneric::convertToRelativePath(const TCHAR* filename)
	{
		return defaultConvertToRelativePath(filename);
	}

	wstring FileManagerGeneric::defaultConvertToRelativePath(const TCHAR* filename)
	{
		filesystem::path f = filename;
		wstring relativePath(filename);
		Paths::normalizeFilename(relativePath);
		wstring rootDirectory(PlatformMisc::rootDir());
		Paths::normalizeFilename(rootDirectory);

		int32 numberOfDirectoriesToGoUp = 3;
		int32 currentSlashPosition;

		while ((currentSlashPosition = StringUtil::find(rootDirectory, TEXT("/"), ESearchCase::CaseSensitive, ESearchDir::FromEnd)) != INDEX_NONE)
		{
			if (boost::starts_with(relativePath, rootDirectory))
			{
				wstring binariesDir = wstring(PlatformProcess::baseDir());
				Paths::makePathRelativeTo(relativePath, binariesDir.c_str());
				break;
			}
			int32 positionOfNextSlash = StringUtil::find(rootDirectory, TEXT("/"), ESearchCase::CaseSensitive, ESearchDir::FromEnd, currentSlashPosition);
			if (positionOfNextSlash != INDEX_NONE)
			{
				numberOfDirectoriesToGoUp++;
				rootDirectory = rootDirectory.substr(0, positionOfNextSlash + 1);
			}
			else
			{
				rootDirectory.clear();
			}
		}
		return relativePath;
	}

	class ArchiveFileWriterDummy : public Archive
	{
	public:
		ArchiveFileWriterDummy()
		{
			mArIsSaving = mArIsPersistent = true;
		}

		virtual ~ArchiveFileWriterDummy()
		{
			close();
		}

		virtual void seek(int64 inPos) override
		{
			mPos = inPos;
		}

		virtual int64 tell() override
		{
			return mPos;
		}

		virtual void serialize(void* v, int64 length) override
		{
			mPos += length;
		}

		virtual wstring getArchiveName() const override { return TEXT("ArchiveFileWriterDummy"); }



	protected:
		int64 mPos{ 0 };
	};

	Archive* FileManagerGeneric::createFileWriter(const TCHAR* filename, uint32 writeFlags /* = 0 */)
	{
		if (SHA1::getFileSHAHash(filename, nullptr) && fileSize(filename) != -1)
		{
			return new ArchiveFileWriterDummy();
		}

		makeDirectory(Paths::getPath(filename).c_str(), true);

		if (writeFlags  & FILEWRITE_EvenIfReadOnly)
		{
			getLowLevel().setReadOnly(filename, false);
		}
		IFileHandle* handle = getLowLevel().openWrite(filename, !!(writeFlags & FILEWRITE_Append), !!(writeFlags & FILEWRITE_AllowRead));
		if (!handle)
		{
			return nullptr;
		}
		return new ArchiveFileWriterGeneric(handle, filename, handle->tell());
	}

	bool FileManagerGeneric::del(const TCHAR* filename, bool requireExists /* = 0 */, bool eventReadOnly /* = 0 */, bool quiet /* = 0 */)
	{
		if (SHA1::getFileSHAHash(filename, nullptr))
		{
			if (!quiet)
			{

			}
			return false;
		}
		bool bExists = getLowLevel().fileExists(filename);
		if (requireExists && !bExists)
		{
			if (!quiet)
			{

			}
			return false;
		}
		if (bExists)
		{
			if (eventReadOnly)
			{
				getLowLevel().setReadOnly(filename, false);
			}
			if (!getLowLevel().deleteFile(filename))
			{
				if (!quiet)
				{

				}
				return false;
			}
		}
		return true;
	}

	bool FileManagerGeneric::move(const TCHAR* dest, const TCHAR* src, bool replace /* = 1 */, bool eventIfReadOnly /* = 0 */, bool attributes /* = 0 */, bool bDoNotRetryOrError /* = 0 */)
	{
		makeDirectory(Paths::getPath(dest).c_str(), true);

		if (getLowLevel().fileExists(dest) && !getLowLevel().deleteFile(dest) && !bDoNotRetryOrError)
		{
			AIR_LOG(LogFileManager, Warning, TEXT("Delete File was unable to delete '%s', retrying in .5s..."), dest);
			PlatformProcess::sleep(0.5f);
			if (!getLowLevel().deleteFile(dest))
			{
				AIR_LOG(LogFileManager, ERROR, TEXT("Error deleting file '%s'."), dest);
				return false;
			}
			else
			{
				AIR_LOG(LogFileManager, Warning, TEXT("DeleteFile recovered during retry"));
			}
		}
		if (!getLowLevel().moveFile(dest, src))
		{
			if (bDoNotRetryOrError)
			{
				return false;
			}

			int32 retryCount = 10;
			bool bSuccess = false;
			while (retryCount--)
			{
				AIR_LOG(logFileManager, Warning, TEXT("MoveFile was unable to move '%s' to '%s', retrying in .5s..."), src, dest);
				PlatformProcess::sleep(0.5f);
				bSuccess = getLowLevel().moveFile(dest, src);
				if (bSuccess)
				{
					break;
				}
			}
			if (!bSuccess)
			{
				return false;
			}
		}
		return true;
	}

	int64 FileManagerGeneric::fileSize(const TCHAR* filename)
	{
		return getLowLevel().fileSize(filename);
	}

	bool FileManagerGeneric::directoryExist(const TCHAR* path)
	{
		return getLowLevel().directoryExists(path);
	}

	wstring FileManagerGeneric::convertToAbsolutePathForExternalApplForWrite(const TCHAR* filename)
	{
		return getLowLevel().convertToAbsolutePathForExternalAppForWrite(filename);
	}

	ArchiveFileReaderGeneric::ArchiveFileReaderGeneric(IFileHandle* inHandle, const TCHAR* inFilename, int64 inSize)
		:mFilename(inFilename)
		, mSize(inSize)
		, mPos(0)
		, mBufferBase(0)
		, mBufferCount(0)
		, mHandle(inHandle)
	{
		mArIsLoading = mArIsPersistent = true;
	}

	ArchiveFileReaderGeneric::~ArchiveFileReaderGeneric()
	{
		close();
	}

	void ArchiveFileReaderGeneric::seek(int64 inPos)
	{
		if (!seekLowLevel(inPos))
		{
			TCHAR errorBuffer[1024];
			mArIsError = true;
		}
		mPos = inPos;
		mBufferBase = mPos;
		mBufferCount = 0;
	}

	bool ArchiveFileReaderGeneric::close()
	{
		closeLowLevel();
		return !mArIsError;
	}

	bool ArchiveFileReaderGeneric
		::internalPrecache(int64 precacheOffset, int64 precacheSize)
	{
		if (mPos == precacheOffset && (!mBufferBase || !mBufferCount || mBufferBase != mPos))
		{
			mBufferBase = mPos;
			mBufferCount = std::min<int64>(std::min<int64>(precacheSize, (int64)(ARRAY_COUNT(mBuffer) - (mPos & (ARRAY_COUNT(mBuffer) - 1)))), mSize - mPos);
			mBufferCount = std::max(mBufferCount, 0LL);
			int64 count = 0;
			{
				if (mBufferCount > ARRAY_COUNT(mBuffer) || mBufferCount <= 0)
				{
					return false;
				}
				readLowLevel(mBuffer, mBufferCount, count);
			}
			if (count != mBufferCount)
			{
				mArIsError = true;
			}
		}
		return true;
	}

	void ArchiveFileReaderGeneric::serialize(void* v, int64 length)
	{
		while (length > 0)
		{
			int64 copy = std::min<int64>(length, mBufferBase + mBufferCount - mPos);
			if (copy <= 0)
			{
				if (length >= ARRAY_COUNT(mBuffer))
				{
					int64 count = 0;
					{
						readLowLevel((uint8*)v, length, count);
					}
					if (count != length)
					{
						mArIsError = true;
					}
					mPos += length;
					return;
				}
				if (!internalPrecache(mPos, std::numeric_limits<int32>::max()))
				{
					mArIsError = true;
					return;
				}
				copy = std::min<int64>(length, mBufferBase + mBufferCount + mPos);
				if (copy <= 0)
				{
					mArIsError = true;
				}
				if (mArIsError)
				{
					return;
				}
			}
			Memory::memcpy(v, mBuffer + mPos - mBufferBase, copy);
			mPos += copy;
			length -= copy;
			v = (uint8*)v + copy;
		}
	}

	bool ArchiveFileReaderGeneric::seekLowLevel(int64 inPos)
	{
		return mHandle->seek(inPos);
	}

	void ArchiveFileReaderGeneric::readLowLevel(uint8* dest, int64 countToRead, int64& outBytesRead)
	{
		if (mHandle->read(dest, countToRead))
		{
			outBytesRead = countToRead;
		}
		else
		{
			outBytesRead = 0;
		}
	}

	void ArchiveFileReaderGeneric::closeLowLevel()
	{
		mHandle.reset();
	}

	ArchiveFileWriterGeneric::ArchiveFileWriterGeneric(IFileHandle* inHandle, const TCHAR* inFileName, int64 inPos)
		:mFilename(inFileName),
		mPos(inPos),
		mBufferCount(0),
		mHandle(inHandle),
		bLoggingError(false)
	{
		mArIsSaving = mArIsPersistent = true;
	}

	ArchiveFileWriterGeneric::~ArchiveFileWriterGeneric()
	{
		close();
	}

	bool ArchiveFileWriterGeneric::closeLowLevel()
	{
		mHandle.reset();
		return true;
	}

	bool ArchiveFileWriterGeneric::seekLowLevel(int64 inPos)
	{
		return mHandle->seek(inPos);
	}

	int64 ArchiveFileWriterGeneric::totalSize()
	{
		flush();
		return mHandle->size();
	}

	bool ArchiveFileWriterGeneric::writeLowLevel(const uint8* src, int64 countToWrite)
	{
		return mHandle->write(src, countToWrite);
	}

	void ArchiveFileWriterGeneric::seek(int64 inPos)
	{
		flush();
		if (!seekLowLevel(inPos))
		{
			mArIsError = true;
			logWriteError(TEXT("Error seeking file"));
		}
		mPos = inPos;
	}

	bool ArchiveFileWriterGeneric::close()
	{
		flush();
		if (!closeLowLevel())
		{
			mArIsError = true;
			logWriteError(TEXT("Error closing file"));
		}
		return !mArIsError;
	}

	void ArchiveFileWriterGeneric::serialize(void* v, int64 length)
	{
		mPos += length;
		if (length >= ARRAY_COUNT(mBuffer))
		{
			flush();
			if (!writeLowLevel((uint8*)v, length))
			{
				mArIsError = true;
				logWriteError(TEXT("error writing to file"));
			}
		}
		else
		{
			int64 copy;
			while (length > (copy = ARRAY_COUNT(mBuffer) - mBufferCount))
			{
				Memory::memcpy(mBuffer + mBufferCount, v, copy);
				mBufferCount += copy;
				BOOST_ASSERT(mBufferCount <= ARRAY_COUNT(mBuffer) && mBufferCount >= 0);
				length -= copy;
				v = (uint8*)v + copy;
				flush();
			}
			if (length)
			{
				Memory::memcpy(mBuffer + mBufferCount, v, length);
				mBufferCount += length;
				BOOST_ASSERT(mBufferCount <= ARRAY_COUNT(mBuffer) && mBufferCount >= 0);
			}
		}
	}
	void ArchiveFileWriterGeneric::flush()
	{
		if (mBufferCount)
		{
			BOOST_ASSERT(mBufferCount <= ARRAY_COUNT(mBuffer) && mBufferCount > 0);
			if (!writeLowLevel(mBuffer, mBufferCount))
			{
				mArIsError = true;
				logWriteError(TEXT("Error flushing file"));
			}
			mBufferCount = 0;
		}
	}
	void ArchiveFileWriterGeneric::logWriteError(const TCHAR* message)
	{
		if (!bLoggingError)
		{
			bLoggingError = true;
			TCHAR erroBuffer[1024];
			bLoggingError = false;
		}
	}
}
