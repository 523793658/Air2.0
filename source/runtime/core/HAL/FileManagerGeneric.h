#pragma once
#include "HAL/FileManager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/Archive.h"
#include "Containers/String.h"
namespace Air
{

	


	class CORE_API ArchiveFileReaderGeneric : public Archive
	{
	public:
		ArchiveFileReaderGeneric(IFileHandle* inHandle, const TCHAR* inFilename, int64 inSize);

		~ArchiveFileReaderGeneric();

		virtual void seek(int64 inPos) final;
		virtual int64 tell() final
		{
			return mPos;
		}

		virtual int64 totalSize() final
		{
			return mSize;
		}

		virtual bool close() final;
		virtual void serialize(void* v, int64 length) final;


		virtual wstring getArchiveName() const override
		{
			return mFilename;
		}

	private:

		bool internalPrecache(int64 precacheOffset, int64 precacheSize);

		virtual bool seekLowLevel(int64 inPos);

		virtual void closeLowLevel();

		virtual void readLowLevel(uint8* dest, int64 countToRead, int64& outBytesRead);




	private:
		wstring mFilename;
		int64 mSize;
		int64 mPos;
		int64 mBufferBase;
		int64 mBufferCount;
		std::unique_ptr<IFileHandle> mHandle;
		uint8 mBuffer[1024];
	};

	class CORE_API FileManagerGeneric
		: public IFileManager
	{
		FORCEINLINE IPlatformFile& getLowLevel() const
		{
			return PlatformFileManager::get().getPlatformFile();
		}


		virtual int64 fileSize(const TCHAR* filename) override;



		virtual void findFilesRecursive(TArray<wstring>& foundFiles, const TCHAR* startDirectory, const TCHAR* filename, bool files, bool directories, bool bClearFileNames = true) override;


		virtual void findFiles(TArray<wstring>& fileNames, const TCHAR* filename, bool files, bool directories) override;

		virtual time_t getTimeStamp(const TCHAR* fileName);

		virtual Archive* createFileReader(const TCHAR* filename, uint32 readFlags = 0) override;

		virtual Archive* createFileWriter(const TCHAR* filename, uint32 writeFlags = 0) override;

		virtual bool makeDirectory(const TCHAR* path, bool tree = 0) override;

		virtual bool move(const TCHAR* dest, const TCHAR* src, bool replace = 1, bool eventIfReadOnly = 0, bool attributes = 0, bool bDoNotRetryOrError = 0) override;

		virtual bool del(const TCHAR* filename, bool requireExists = 0, bool eventReadOnly = 0, bool quiet = 0) override;

		virtual wstring convertToAbsolutePathForExternalApplForWrite(const TCHAR* filename) override;

		virtual bool deleteDirectory(const TCHAR* path, bool requireExists = 0, bool tree = 0) override;

		virtual bool directoryExist(const TCHAR* path) override;

		virtual wstring convertToRelativePath(const TCHAR* filename) override;

	private:
		wstring defaultConvertToRelativePath(const TCHAR* filename);

		void findFileRecursiveInternal(TArray<wstring>& fileNames, const TCHAR* startDirectory, const TCHAR* fileName, bool files, bool directories);
	};

	class ArchiveFileWriterGeneric : public Archive
	{
	public:
		ArchiveFileWriterGeneric(IFileHandle* inHandle, const TCHAR* inFileName, int64 inPos);

		~ArchiveFileWriterGeneric();

		virtual void seek(int64 inPos) final;

		virtual int64 tell() final
		{
			return mPos;
		}

		virtual int64 totalSize() override;

		virtual bool close() final;

		virtual void serialize(void* v, int64 length) final;

		virtual void flush() final;

		virtual wstring getArchiveName() const override
		{
			return mFilename;
		}
	protected:
		virtual bool seekLowLevel(int64 inPos);

		virtual bool closeLowLevel();

		virtual bool writeLowLevel(const uint8* src, int64 countToWrite);

		void logWriteError(const TCHAR* message);


		wstring mFilename;
		int64 mPos;
		int64 mBufferCount;
		std::unique_ptr<IFileHandle> mHandle;
		uint8 mBuffer[4096];
		long bLoggingError;
	};

}