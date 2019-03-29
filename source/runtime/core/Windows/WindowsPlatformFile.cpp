#include "Windows/WindowsPlatformFile.h"
#include "CoreType.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Containers/String.h"
#include "Misc/Paths.h"
#include "HAL/AirMemory.h"
#include "Windows/WindowsHWrapper.h"
#include "Math/Math.h"
#include "boost/algorithm/string.hpp"

namespace FileConstants
{
	uint32 WIN_INVALID_SET_FILE_POINTER = INVALID_SET_FILE_POINTER;
}

namespace Air
{



	class CORE_API AsyncBufferedFileReaderWindows : public IFileHandle
	{
	protected:
		enum { DEFAULT_BUFFER_SIZE = 64 * 1024 };
		HANDLE mHandle;

		int64 mFileSize;
		int64 mFilePos;

		int64 mOverlappedFilePos;
		int8* mBuffers[2];
		const int32 mBufferSize;

		int32 mSerializeBuffer;

		int32 mStreamBuffer;
		int32 mSerializePos;

		int32 mCurrentAsyncReadBuffer;

		OVERLAPPED mOverlappedIO;

		bool bIsAtEOF;

		bool bHasReadOutstanding;

		bool close(void)
		{
			if (mHandle != nullptr)
			{
				CloseHandle(mHandle);
				mHandle = nullptr;
			}
			return true;
		}

		FORCEINLINE void swapBuffers()
		{
			mStreamBuffer ^= 1;
			mSerializeBuffer ^= 1;
			mSerializePos = 0;
		}

		FORCEINLINE void copyOverlappedPosition()
		{
			ULARGE_INTEGER LI;
			LI.QuadPart = mOverlappedFilePos;
			mOverlappedIO.Offset = LI.LowPart;
			mOverlappedIO.OffsetHigh = LI.HighPart;
		}


		virtual bool seek(int64 newPosition) override
		{
			BOOST_ASSERT(isValid());
			BOOST_ASSERT(newPosition >= 0);
			BOOST_ASSERT(newPosition <= mFileSize);

			int64 posDelta = newPosition - mFilePos;
			if (posDelta == 0)
			{
				return true;
			}

			if (!waitForAsyncRead())
			{
				return false;
			}
			mFilePos = newPosition;

			bool bWithinSerializeBuffer = (posDelta < 0 && (mSerializePos - std::abs(posDelta) >= 0)) || (posDelta > 0 && ((posDelta + mSerializePos) < mBufferSize));
			if (bWithinSerializeBuffer)
			{
				mSerializePos += posDelta;
			}
			else
			{
				bIsAtEOF = false;
				mOverlappedFilePos = newPosition;
				copyOverlappedPosition();
				mCurrentAsyncReadBuffer = mSerializeBuffer;
				mSerializePos = 0;
				startSerializeBufferRead();
			}
			return true;
		}

		virtual int64 tell(void) override
		{
			BOOST_ASSERT(isValid());
			return mFilePos;
		}


		virtual bool read(uint8* destination, int64 bytesToRead)
		{
			BOOST_ASSERT(isValid());
			if (bytesToRead <= 0)
			{
				return false;
			}
			if (mCurrentAsyncReadBuffer == mSerializeBuffer)
			{
				if (!waitForAsyncRead())
				{
					return false;
				}
				startStreamBufferRead();
			}

			BOOST_ASSERT(destination != nullptr);
			while (bytesToRead > 0)
			{
				int64 numToCopy = std::min<int64>(bytesToRead, mBufferSize - mSerializePos);
				if (mFilePos + numToCopy > mFileSize)
				{
					return false;
				}
				if (numToCopy > 0)
				{
					Memory::memcpy(destination, &mBuffers[mSerializeBuffer][mSerializePos], numToCopy);

					mSerializePos += numToCopy;
					BOOST_ASSERT(mSerializePos <= mBufferSize);
					mFilePos += numToCopy;
					BOOST_ASSERT(mFilePos <= mFileSize);

					bytesToRead -= numToCopy;
					destination = (uint8*)destination + numToCopy;
				}
				else
				{
					if (!waitForAsyncRead())
					{
						return false;
					}
					swapBuffers();
					startStreamBufferRead();
				}
			}
			return true;
		}

		FORCEINLINE void startStreamBufferRead()
		{
			startAsyncRead(mStreamBuffer);
		}

		bool waitForAsyncRead()
		{
			if (bIsAtEOF || !bHasReadOutstanding)
			{
				return true;
			}
			uint32 numRead = 0;
			if (GetOverlappedResult(mHandle, &mOverlappedIO, (::DWORD*)&numRead, true) != false)
			{
				updateFileOffsetAfterRead(numRead);
				return true;
			}
			else if (GetLastError() == ERROR_HANDLE_EOF)
			{
				bIsAtEOF = true;
				return true;
			}
			return false;
		}

		FORCEINLINE bool isValid()
		{
			return mHandle != nullptr && mHandle != INVALID_HANDLE_VALUE;
		}
	public:

		AsyncBufferedFileReaderWindows(HANDLE inHandle, int32 inBufferSize = DEFAULT_BUFFER_SIZE)
			:mHandle(inHandle)
			,mFilePos(0)
			,mOverlappedFilePos(0)
			,mBufferSize(inBufferSize)
			,mSerializeBuffer(0)
			,mStreamBuffer(1)
			,mSerializePos(0)
			,mCurrentAsyncReadBuffer(0)
			,bIsAtEOF(false)
			,bHasReadOutstanding(false)
		{
			LARGE_INTEGER LI;
			GetFileSizeEx(mHandle, &LI);
			mFileSize = LI.QuadPart;
			mBuffers[0] = (int8*)Memory::malloc(mBufferSize);
			mBuffers[1] = (int8*)Memory::malloc(mBufferSize);

			Memory::memzero(&mOverlappedIO, sizeof(OVERLAPPED));
			startSerializeBufferRead();
		}


		virtual ~AsyncBufferedFileReaderWindows()
		{
			waitForAsyncRead();
			close();
			Memory::free(mBuffers[0]);
			Memory::free(mBuffers[1]);
		}

		void startAsyncRead(int32 bufferToReadInto)
		{
			if (!bIsAtEOF)
			{
				bHasReadOutstanding = true;
				mCurrentAsyncReadBuffer = bufferToReadInto;
				uint32 numRead = 0;
				if (!ReadFile(mHandle, mBuffers[bufferToReadInto], mBufferSize, (::DWORD*)&numRead, &mOverlappedIO))
				{
					uint32 errorCode = GetLastError();
					if (errorCode != ERROR_IO_PENDING)
					{
						bIsAtEOF = true;
						bHasReadOutstanding = false;
					}
				}
				else
				{
					updateFileOffsetAfterRead(numRead);
				}
			}
		}
		FORCEINLINE void updateFileOffsetAfterRead(uint32 amountRead)
		{
			bHasReadOutstanding = false;
			mOverlappedFilePos += amountRead;
			copyOverlappedPosition();
			if (mOverlappedFilePos >= uint64(mFileSize))
			{
				bIsAtEOF = true;
			}
		}

		FORCEINLINE void startSerializeBufferRead()
		{
			startAsyncRead(mSerializeBuffer);
		}

		virtual bool write(const uint8* source, int64 bytesToWrite)
		{
			BOOST_ASSERT(false, "This is an async reader only and doesn't support writing");
			return false;
		}

		virtual bool seekFromEnd(int64 newPositionRelativeToEnd = 0i64)
		{
			BOOST_ASSERT(isValid());
			BOOST_ASSERT(newPositionRelativeToEnd <= 0);
			return seek(mFileSize + newPositionRelativeToEnd);
		}

	};



	class CORE_API FileHandleWindows : public IFileHandle
	{
		enum { READWRITE_SIZE = 1024 * 1024 };
		HANDLE mFileHandle;

		FORCEINLINE int64 fileSeek(int64 distance, uint32 moveMethod)
		{
			LARGE_INTEGER li;
			li.QuadPart = distance;
			li.LowPart = SetFilePointer(mFileHandle, li.LowPart, &li.HighPart, moveMethod);
			if (li.LowPart == FileConstants::WIN_INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
			{
				li.QuadPart = -1;
			}
			return li.QuadPart;
		}

		FORCEINLINE bool isValid()
		{
			return mFileHandle != NULL && mFileHandle != INVALID_HANDLE_VALUE;
		}

	public:
		FileHandleWindows(HANDLE inFileHandle = NULL)
			:mFileHandle(inFileHandle)
		{

		}

		virtual ~FileHandleWindows()
		{
			CloseHandle(mFileHandle);
		}

		virtual int64 tell() override
		{
			BOOST_ASSERT(isValid());
			return fileSeek(0, FILE_CURRENT);
		}

		virtual bool seek(int64 newPosition) override
		{
			BOOST_ASSERT(isValid());
			BOOST_ASSERT(newPosition >= 0);
			return fileSeek(newPosition, FILE_BEGIN) != -1;
		}

		virtual bool seekFromEnd(int64 newPositionRelativeToEnd = 0) override
		{
			BOOST_ASSERT(isValid());
			BOOST_ASSERT(newPositionRelativeToEnd <= 0);
			return fileSeek(newPositionRelativeToEnd, FILE_END) != 1;
		}


		virtual bool read(uint8* destination, int64 bytesToRead) override
		{
			BOOST_ASSERT(isValid());
			while (bytesToRead)
			{
				BOOST_ASSERT(bytesToRead >= 0);
				int64 thisSize = Math::min<int64>(READWRITE_SIZE, bytesToRead);
				BOOST_ASSERT(destination);
				uint32 result = 0;
				if (!ReadFile(mFileHandle, destination, uint32(thisSize), (::DWORD*)&result, NULL) || result != uint32(thisSize))
				{
					return false;
				}
				destination += thisSize;
				bytesToRead -= thisSize;
			}
			return true;
		}

		virtual bool write(const uint8* source, int64 bytesToWrite) override
		{
			BOOST_ASSERT(isValid());
			while (bytesToWrite)
			{
				BOOST_ASSERT(bytesToWrite >= 0);
				int64 thisSize = Math::min<int64>(READWRITE_SIZE, bytesToWrite);
				BOOST_ASSERT(source);

				uint32 result = 0;
				if (!WriteFile(mFileHandle, source, uint32(thisSize), (::DWORD*)&result, NULL) || result != uint32(thisSize))
				{
					return false;
				}
				source += thisSize;
				bytesToWrite -= thisSize;
			}
			return true;
		}


	};

	class CORE_API WindowsPlatformFile : public IPhysicalPlatformFile
	{
	protected:

		virtual wstring normalizeFilename(const TCHAR* filename)
		{
			wstring result(filename);
			Paths::normalizeFilename(result);
			if (boost::starts_with(result, TEXT("//")))
			{
				result = wstring(TEXT("\\\\")) + result.substr(2);
			}
			return Paths::convertRelativePathToFull(result);
		}

		virtual IFileHandle* openRead(const TCHAR* filename, bool bAllowWrite  = false ) override
		{
			uint32 access = GENERIC_READ;
			uint32 winFlags = FILE_SHARE_READ | (bAllowWrite ? FILE_SHARE_WRITE : 0);
			uint32 create = OPEN_EXISTING;
			HANDLE handle = CreateFileW(normalizeFilename(filename).c_str(), access, winFlags, NULL, create, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
			if (handle != INVALID_HANDLE_VALUE)
			{
				return new AsyncBufferedFileReaderWindows(handle);
			}
			return NULL;
		}

		virtual bool setReadOnly(const TCHAR* filename, bool bNewReadOnlyValue) override
		{
			return !!SetFileAttributesW(normalizeFilename(filename).c_str(), bNewReadOnlyValue ? FILE_ATTRIBUTE_READONLY : FILE_ATTRIBUTE_NORMAL);
		}

		virtual IFileHandle* openWrite(const TCHAR* filename, bool bAppend = false, bool bAllowRead = false)
		{
			uint32 access = GENERIC_WRITE;
			uint32 winFlags = bAllowRead ? FILE_SHARE_READ : 0;
			uint32 create = bAppend ? OPEN_ALWAYS : CREATE_ALWAYS;
			HANDLE handle = CreateFileW(normalizeFilename(filename).c_str(), access, winFlags, NULL, create, FILE_ATTRIBUTE_NORMAL, NULL);
			if (handle != INVALID_HANDLE_VALUE)
			{
				FileHandleWindows *platformFileHandle = new FileHandleWindows(handle);
				if (bAppend)
				{
					platformFileHandle->seekFromEnd(0);
				}
				return platformFileHandle;
			}
			return nullptr;
		}

		virtual bool moveFile(const TCHAR* dest, const TCHAR* src) override
		{
			return !!MoveFileW(normalizeFilename(src).c_str(), normalizeFilename(dest).c_str());
		}
	};


	IPlatformFile& IPlatformFile::getPlatformPhysical()
	{
		static WindowsPlatformFile singleton;
		return singleton;
	}
}