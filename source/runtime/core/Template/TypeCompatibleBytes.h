#pragma once
#include "CoreType.h"
#include "Template/AlignOf.h"
namespace Air
{

	template<int32 Size, uint32 alignment>
	struct TAlignedBytes;

	template<int32 Size>
	struct TAlignedBytes<Size, 1>
	{
		uint8 Pad[Size];
	};
#ifndef __cplusplus_cli
#define IMPLEMENT_ALIGNED_STORAGE(Align)	\
	template<int32 Size>	\
	struct TAlignedBytes<Size, Align>	\
	{	\
		struct MS_ALIGN(Align)	TPadding	\
	{	\
		uint8 Pad[Size];	\
	}GCC_ALIGN(Align);	\
		TPadding Padding;	\
	};
#endif

	IMPLEMENT_ALIGNED_STORAGE(16);
	IMPLEMENT_ALIGNED_STORAGE(8);
	IMPLEMENT_ALIGNED_STORAGE(4);
	IMPLEMENT_ALIGNED_STORAGE(2);

#undef IMPLEMENT_ALIGNED_STORAGE

	template<typename ElementType>
	struct TTypeCompatibleBytes : public TAlignedBytes<sizeof(ElementType), ALIGNOF(ElementType)>
	{};
}