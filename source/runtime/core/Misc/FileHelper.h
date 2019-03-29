#pragma once
#include "CoreType.h"
#include "Containers/Array.h"
#include "Containers/String.h"
#include "HAL/FileManager.h"
namespace Air
{
	struct CORE_API FileHelper
	{
		struct EHashOptions 
		{
			enum type
			{
				EnableVerify = 1 << 0,
				ErrorMissingHash = 1 << 1
			};
		};
		struct EEncodingOptions 
		{
			enum Type
			{
				AutoDetect,
				ForceAnsi,
				ForceUnicode,
				ForceUTF8,
				ForceUTF8WithoutBOM
			};
		};

		static bool loadFileToArray(TArray<uint8>& result, const TCHAR* filename, uint32 flags = 0);

		static bool loadFileToString(wstring& result, const TCHAR* filename, uint32 verifyFlags = 0);

		static void bufferToString(wstring& result, const uint8* buffer, int32 size);

		static bool saveStringToFile(const wstring& string, const TCHAR* filename, EEncodingOptions::Type encodingOptions = EEncodingOptions::AutoDetect, IFileManager* fileManager = &IFileManager::get(), uint8 writeflags = 0);
	};
}