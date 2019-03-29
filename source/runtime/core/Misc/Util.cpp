#include "Util.h"
#include "CoreType.h"

template<>
CORE_API void EndianSwitch<2>(void* p) noexcept
{
	uint8* bytes = static_cast<uint8*>(p);
	std::swap(bytes[0], bytes[1]);
}
template<>
CORE_API void EndianSwitch<4>(void* p) noexcept
{
	uint8* bytes = static_cast<uint8*>(p);
	std::swap(bytes[0], bytes[3]);
	std::swap(bytes[1], bytes[2]);
}

template<>
CORE_API void EndianSwitch<8>(void*p) noexcept
{
	uint8* bytes = static_cast<uint8*>(p);
	std::swap(bytes[0], bytes[7]);
	std::swap(bytes[1], bytes[6]);
	std::swap(bytes[2], bytes[5]);
	std::swap(bytes[3], bytes[4]);

}