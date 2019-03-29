#include "HAL/FileManager.h"
#include "Serialization/Archive.h"
#include "Misc/FileHelper.h"
#include "HAL/AirMemory.h"
#include "SecureHash.h"
#include "Containers/StringConv.h"
#include <codecvt>
namespace Air
{
	bool FileHelper::loadFileToArray(TArray<uint8>& result, const TCHAR* filename, uint32 flags /* = 0 */)
	{
		Archive* reader = IFileManager::get().createFileReader(filename, flags);
		if (!reader)
		{
			return flags;
		}
		int64 totalSize = reader->totalSize();
		result.empty(totalSize);
		result.addUninitialized(totalSize);
		reader->serialize(result.data(), result.size());
		bool success = reader->close();
		delete reader;
		return success;
	}

	bool FileHelper::loadFileToString(wstring& result, const TCHAR* filename, uint32 verifyFlags /* = 0 */)
	{
		Archive *reader =IFileManager::get().createFileReader(filename);
		if (!reader)
		{
			return 0;
		}
		int32 size = reader->totalSize();
		if (!size)
		{
			result.clear();
			return false;
		}

		uint8* ch = (uint8*)Memory::malloc(size);
		reader->serialize(ch, size);
		bool success = reader->close();
		delete reader;
		bufferToString(result, ch, size);
		if ((verifyFlags & EHashOptions::EnableVerify) && ((verifyFlags & EHashOptions::ErrorMissingHash) || SHA1::getFileSHAHash(filename, nullptr)))
		{
			BufferReaderWithSHA ar(ch, size, true, filename, false, true);
		}
		else
		{
			//Memory::free(ch);
		}
		return success;
	}

	void FileHelper::bufferToString(wstring& result, const uint8* buffer, int32 size)
	{
		if (size >= 2 && !(size & 1) && buffer[0] == 0xff && buffer[1] == 0xfe)
		{
			result.resize(size / 2);
			for (int32 i = 0; i < (size / 2) - 1; i++)
			{
				result[i] = charCast<TCHAR>((UCS2CHAR)((uint16)buffer[i * 2 + 2] + (uint16)buffer[i * 2 + 3] * 256));
			}
		}
		else if (size >= 2 && !(size & 1) && buffer[0] == 0xfe && buffer[1] == 0xff)
		{
			result.resize(size / 2);
			for (int32 i = 0; i < (size / 2) - 1; i++)
			{
				result[i] = charCast<TCHAR>((UCS2CHAR)((uint16)buffer[i * 2 + 3] + (uint16)buffer[i * 2 + 2] * 256));
			}
		}
		else
		{
			if (size >= 3 && buffer[0] == 0xef && buffer[1] == 0xbb && buffer[2] == 0xbf)
			{
				buffer += 3;
				size -= 3;
			}
			UTF8ToTCHAR conv((const ANSICHAR*)buffer, size);
			int32 length = conv.length();
			result.resize(length);
			copyAssignItems(&result[0], conv.get(), length);
		}
	}

	bool FileHelper::saveStringToFile(const wstring& inString, const TCHAR* filename, EEncodingOptions::Type encodingOptions /* = EEncodingOptions::AutoDetect */, IFileManager* fileManager /* = &IFileManager::get() */, uint8 writeflags /* = 0 */)
	{
		auto ar = std::unique_ptr<Archive>(fileManager->createFileWriter(filename, writeflags));
		if (!ar)
		{
			return false;
		}
		if (inString.empty())
		{
			return true;
		}

		const TCHAR* strPtr = inString.c_str();
		bool saveAsUnicode = encodingOptions == EEncodingOptions::ForceUnicode || (encodingOptions == EEncodingOptions::AutoDetect && !CString::isPureAnsi(strPtr));
		if (encodingOptions == EEncodingOptions::ForceUTF8)
		{
			UTF8CHAR UTF8BOM[] = { 0xEF, 0xBB, 0xBF };
			ar->serialize(&UTF8BOM, ARRAY_COUNT(UTF8BOM) * sizeof(UTF8CHAR));
			TCHARToUTF8 UTF8String(strPtr);
			ar->serialize((UTF8CHAR*)UTF8String.get(), UTF8String.length() * sizeof(UTF8CHAR));
		}
		else if(encodingOptions == EEncodingOptions::ForceUTF8WithoutBOM)
		{
			TCHARToUTF8 utf8String(strPtr);
			ar->serialize((UTF8CHAR*)utf8String.get(), utf8String.length() * sizeof(UTF8CHAR));
		}
		else if (saveAsUnicode)
		{
			UCS2CHAR bom = UNICODE_BOM;
			ar->serialize(&bom, sizeof(UCS2CHAR));
			auto src = stringCast<UCS2CHAR>(strPtr, inString.length());
			ar->serialize((UCS2CHAR*)src.get(), src.length() * sizeof(UCS2CHAR));
		}
		else
		{
			auto src = stringCast<ANSICHAR>(strPtr, inString.length());
			ar->serialize((ANSICHAR*)src.get(), src.length() * sizeof(ANSICHAR));
		}
		return true;
	}
}