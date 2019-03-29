#pragma once
#include "CoreType.h"
namespace Air
{
#define BYTESWAP_ORDER16_unsigned(x) ((((x) >> 8) & 0xff) + (((x) << 8) & 0xff00))

#define BYTESWAP_ORDER32_unsigned(x) (((x) >> 24) + (((x) >>8) & 0xff00) + (((x) << 8) & 0xff0000) + ((x) << 24))

	static FORCEINLINE uint16 BYTESWAP_ORDER16(uint16 val)
	{
		return (BYTESWAP_ORDER16_unsigned(val));
	}

	static FORCEINLINE int16 BYTESWAP_ORDER16(int16 val)
	{
		uint16 uval = *((uint16*)&val);
		uval = BYTESWAP_ORDER16_unsigned(uval);
		return *((int16*)&uval);
	}

	static FORCEINLINE uint32 BYTESWAP_ORDER32(uint32 val)
	{
		return (BYTESWAP_ORDER32_unsigned(val));
	}

	static FORCEINLINE int32 BYTESWAP_ORDER32(int32 val)
	{
		uint32 uval = *((uint32*)&val);
		uval = BYTESWAP_ORDER32_unsigned(uval);
		return *((int32*)&uval);
	}

	static FORCEINLINE float BYTESWAP_ORDERF(float val)
	{
		uint32 uval = *((uint32*)&val);
		uval = BYTESWAP_ORDER32_unsigned(uval);
		return *((float*)&val);
	}

	static FORCEINLINE uint64 BYTESWAP_OREDER64(uint64 val)
	{
		uint64 swapped = 0;
		uint8* forward = (uint8*)&val;
		uint8* reverse = (uint8*)&swapped + 7;
		for (int32 i = 0; i < 8; i++)
		{
			*(reverse--) = *(forward++);
		}
		return swapped;
	}

	static FORCEINLINE int64 BYTESWAP_ORDER64(int64 value)
	{
		int64 swapped = 0;
		uint8* forward = (uint8*)&value;
		uint8* reverse = (uint8*)&swapped + 7;
		for (int32 i = 0; i < 8; i++)
		{
			*(reverse--) = *(forward++);
		}
		return swapped;
	}

	static FORCEINLINE void BYTESWAP_ORDER_TCHARARRAY(TCHAR* str)
	{
		for (TCHAR* c = str; *c; ++c)
		{
			*c = BYTESWAP_ORDER16_unsigned(*c);
		}
	}

#if PLATFORM_LITTLE_ENDIAN
#define INTEL_ORDER16(x)   (x)
#define INTEL_ORDER32(x)   (x)
#define INTEL_ORDERF(x)    (x)
#define INTEL_ORDER64(x)   (x)
#define INTEL_ORDER_TCHARARRAY(x)
#else
#define INTEL_ORDER16(x)			BYTESWAP_ORDER16(x)
#define INTEL_ORDER32(x)			BYTESWAP_ORDER32(x)
#define INTEL_ORDERF(x)				BYTESWAP_ORDERF(x)
#define INTEL_ORDER64(x)			BYTESWAP_ORDER64(x)
#define INTEL_ORDER_TCHARARRAY(x)	BYTESWAP_ORDER_TCHARARRAY(x)
#endif

}