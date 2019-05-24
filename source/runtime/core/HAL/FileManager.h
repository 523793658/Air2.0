#pragma once

#include "CoreType.h"
#include "Containers/Array.h"
#include "Containers/String.h"
namespace Air
{
	class Archive;

	enum EFileRead
	{
		FILEREAD_None				= 0x00,
		FILEREAD_NoFail				= 0x01,
		FILEREAD_Silent				= 0x02,
		FILEREAD_AllowWrite			= 0x04,
	};

	enum EFileWrite
	{
		FILEWRITE_None				=0x00,
		FILEWRITE_NoFail			=0x01,
		FILEWRITE_NoReplaceExisting = 0x02,
		FILEWRITE_EvenIfReadOnly	 = 0x04,
		FILEWRITE_Append			=0x08,
		FILEWRITE_AllowRead			=0x10,
	};

	enum ECopyResult
	{
		COPY_OK						=0x00,
		COPY_Fail					= 0x01,
		COPY_Canceled				= 0x02,
	};

	struct CopyProgress
	{
		virtual bool poll(float fraction) = 0;
	};

	class CORE_API IFileManager
	{
	protected:
		IFileManager() {}
	public:
		static IFileManager& get();

		virtual void findFilesRecursive(TArray<wstring>& foundFiles, const TCHAR* startDirectory, const TCHAR* filename, bool files, bool directories, bool bClearFileNames = true) = 0;

		virtual void findFiles(TArray<wstring>& fileNames, const TCHAR* filename, bool files, bool directories) = 0;

		virtual time_t getTimeStamp(const TCHAR* fileName) = 0;

		virtual Archive* createFileReader(const TCHAR* filename, uint32 readFlags = 0) = 0;

		virtual wstring convertToRelativePath(const TCHAR* filename) = 0;

		virtual Archive* createFileWriter(const TCHAR* filename, uint32 writeFlags = 0) = 0;

		virtual bool makeDirectory(const TCHAR* path, bool tree = 0) = 0;

		virtual bool move(const TCHAR* dest, const TCHAR* src, bool replace = 1, bool eventIfReadOnly = 0, bool attributes = 0, bool bDoNotRetryOrError = 0) = 0;

		virtual int64 fileSize(const TCHAR* filename) = 0;

		virtual bool del(const TCHAR* filename, bool requireExists = 0, bool eventReadOnly = 0, bool quiet = 0) = 0;

		virtual wstring convertToAbsolutePathForExternalApplForWrite(const TCHAR* filename) = 0;

		virtual bool deleteDirectory(const TCHAR* path, bool requireExists = 0, bool tree = 0) = 0;

		virtual bool directoryExist(const TCHAR* path) = 0;

		virtual uint32 copy(const TCHAR* dest, const TCHAR* src, bool replace = true, bool evenIfReadOnly = false, bool attributes = false, CopyProgress* progress = nullptr, EFileRead readflags = FILEREAD_None, EFileWrite writeFlags = FILEWRITE_None) = 0;
	};
}