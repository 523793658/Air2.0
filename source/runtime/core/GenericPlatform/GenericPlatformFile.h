#pragma once
#include "CoreType.h"
#include "Containers/String.h"
#include "Misc/EnumClassFlags.h"
namespace Air
{

	enum class EPlatformFileRead : uint8
	{
		None = 0x0,
		AllowWrite = 0x01
	};

	ENUM_CLASS_FLAGS(EPlatformFileRead);

	enum class EPlatformFileWrite : uint8
	{
		None = 0x0,
		AllowRead = 0x01,
	};

	ENUM_CLASS_FLAGS(EPlatformFileWrite);

	class CORE_API IFileHandle
	{
	public:
		virtual ~IFileHandle()
		{}
		virtual int64 tell() = 0;

		virtual bool seek(int64 newPosition) = 0;

		virtual bool seekFromEnd(int64 newPositionRelativeToEnd = 0i64) = 0;

		virtual bool read(uint8* destination, int64 bytesToRead) = 0;

		virtual bool write(const uint8* source, int64 bytesToWrite) = 0;

		

	public:
		virtual int64 size();
	};

	class CORE_API IPlatformFile
	{
	public:
		static IPlatformFile& getPlatformPhysical();


		virtual IFileHandle* openRead(const TCHAR* filename, bool bAllowWrite = false) = 0;

		virtual IFileHandle* openWrite(const TCHAR* filename, bool bAppend = false, bool bAllowRead = false) = 0;

		virtual int64 fileSize(const TCHAR* filename);

		virtual bool setReadOnly(const TCHAR* filename, bool bNewReadOnlyValue) = 0;

		virtual bool isReadOnly(const TCHAR* filename) = 0;

		virtual bool fileExists(const TCHAR* filename);

		virtual bool deleteFile(const TCHAR* filename);

		virtual bool moveFile(const TCHAR* dest, const TCHAR* src) = 0;

		virtual wstring convertToAbsolutePathForExternalAppForWrite(const TCHAR* filename);

		virtual bool deleteDirectoryRecursively(const TCHAR* directory);

		virtual bool directoryExists(const TCHAR* directory);

		virtual bool deleteDirectory(const TCHAR* directory);

		virtual bool copyFile(const TCHAR* to, const TCHAR* from, EPlatformFileRead readFlags = EPlatformFileRead::None, EPlatformFileWrite writeFlags = EPlatformFileWrite::None);


	};

	class CORE_API IPhysicalPlatformFile : public IPlatformFile
	{

	};
}