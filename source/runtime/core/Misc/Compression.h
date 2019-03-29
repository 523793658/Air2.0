#pragma once
#include "CoreType.h"
namespace Air
{
	enum ECompressionFlags
	{
		COMPRESS_None			= 0x00,
		COMPRESS_ZLIB			= 0x01,
		COMPRESS_GZIP			= 0x02,
		COMPRESS_BiasMemory		= 0x10,
		COMPRESS_BiasSpeed		= 0x20,
	};

#define DEFAULT_ZLIB_BIT_WINDOW 15

	struct Compression
	{
		const static uint32 MaxUncompressedSize = 256 * 1024;

		CORE_API static bool uncompressMemory(ECompressionFlags flags, void* unCompressedBuffer, int32 unCompressedSize, const void* compressedBuffer, int32 compressedSize, bool bIsSourcePadded = false, int32 bitWindow = DEFAULT_ZLIB_BIT_WINDOW);
	};
}