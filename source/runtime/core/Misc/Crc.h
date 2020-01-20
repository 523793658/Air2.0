#pragma once
#include "CoreType.h"
#include "Misc/ByteSwap.h"
#include "Template/AlignmentTemplates.h"
namespace Air
{
	struct CORE_API Crc
	{
		template<typename T> 
		static uint32 typeCrc32(const T& data, uint32 crc = 0)
		{
			return memCrc32(&data, sizeof(T), crc);
		}

		static uint32 memCrc32(const void* data, int32 length, uint32 crc = 0);

		static uint32 memCrc_DEPRECATED(const void* inData, int32 length, uint32 crc = 0u)
		{
			crc = ~BYTESWAP_ORDER32(crc);
			const uint8* __restrict data = (uint8*)inData;
			int32 initBytes = align(data, 4) - data;
			if (length > initBytes)
			{
				length -= initBytes;
				for (; initBytes; --initBytes)
				{
					crc = (crc >> 8) ^ CRCTablesSB8_DEPRECATED[0][(crc & 0xff) ^ *data++];
				}
				auto data4 = (const uint32*)data;
				for (uint32 repeat = length / 8; repeat; --repeat)
				{
					uint32 V1 = *data4++ ^crc;
					uint32 V2 = *data4++;
					crc = 
						CRCTablesSB8_DEPRECATED[7][V1 & 0xFF] ^
						CRCTablesSB8_DEPRECATED[6][(V1 >> 8) & 0xFF] ^
						CRCTablesSB8_DEPRECATED[5][(V1 >> 16) & 0xFF] ^
						CRCTablesSB8_DEPRECATED[4][V1 >> 24] ^
						CRCTablesSB8_DEPRECATED[3][V2 & 0xFF] ^
						CRCTablesSB8_DEPRECATED[2][(V2 >> 8) & 0xFF] ^
						CRCTablesSB8_DEPRECATED[1][(V2 >> 16) & 0xFF] ^
						CRCTablesSB8_DEPRECATED[0][V2 >> 24];
				}
				data = (const uint8*)data4;
				length %= 8;
			}
			for (; length; --length)
			{
				crc = (crc >> 8) ^ CRCTablesSB8_DEPRECATED[0][(crc & 0xff) ^ *data++];
			}

			return BYTESWAP_ORDER32(~crc);

		}
		static uint32 CRCTable_DEPRECATED[256];
		static uint32 CRCTablesSB8_DEPRECATED[8][256];
		static uint32 CRCTablesSB8[8][256];
	};
}