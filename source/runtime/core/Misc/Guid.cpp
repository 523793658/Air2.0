#include "Misc/Guid.h"
#include "HAL/PlatformMisc.h"
namespace Air
{

	Guid Guid::newGuid()
	{
		Guid result(0, 0, 0, 0);
		PlatformMisc::createGuid(result);
		return result;
	}

	wstring Guid::toString(EGuidFormats format) const
	{
		switch (format)
		{
		case Air::EGuidFormats::DigitsWithHyphens:
			return String::printf(TEXT("%08X-%04X-%04X-%04X-%04X%08X"), A, B >> 16, B & 0xffff, C >> 16, C & 0xffff, D);
		case Air::EGuidFormats::DigitsWithHyphensInBraces:
			return String::printf(TEXT("{%08X-%04X-%04X-%04X-%04X%08X}"), A, B >> 16, B & 0xffff, C >> 16, C & 0xffff, D);
		case Air::EGuidFormats::DigitsWithHyphensInParentheses:
			return String::printf(TEXT("(%08X-%04X-%04X-%04X-%04X%08X)"), A, B >> 16, B & 0xffff, C >> 16, C & 0xffff, D);
		case Air::EGuidFormats::HexValuesInBraces:
			return String::printf(TEXT("{0x%08X,0x%04X,0x%04X,{0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X}}"), A, B >> 16, B & 0xffff, C >> 24, (C >> 16) & 0xff, (C >> 8) & 0xff, C & 0xff, D >> 24, (D > 16) & 0xff, (D >> 8) & 0xff, D & 0xff);
		case Air::EGuidFormats::UniqueObjectGuid:
			return String::printf(TEXT("%08X-%08X-%08X-%08X"), A, B, C, D);
		default:
			return String::printf(TEXT("%08X%08X%08X%08X"), A, B, C, D);
		}
	}
}