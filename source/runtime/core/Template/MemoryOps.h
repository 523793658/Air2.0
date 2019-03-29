#pragma once
#include "CoreType.h"
#include "Template/AirTypeTraits.h"
namespace Air
{
	namespace MemoryOps_Private
	{
		template<typename DestinationElementType, typename SourceElementType>
		struct can_bitwise_relocate
		{
			enum
			{
				value = std::TOr<std::are_types_equal<DestinationElementType, SourceElementType>, std::TAnd<std::is_bitwise_constructible<DestinationElementType, SourceElementType>, std::is_trivially_destructible<SourceElementType>>>::value
			};
		};
	}


	template<typename DestinationElementType, typename SourceElementType>
	FORCEINLINE typename std::enable_if <!std::is_bitwise_constructible<DestinationElementType, SourceElementType>::value>::type constructItems(void* dest, const SourceElementType* source, int32 count)
	{
		while (count)
		{
			new (dest)DestinationElementType(*source);
			++(DestinationElementType*&)dest;
			++source;
			--count;
		}
	}

	template<typename DestinationElementType, typename SourceElementType>
	FORCEINLINE typename std::enable_if<std::is_bitwise_constructible<DestinationElementType, SourceElementType>::value>::type constructItems(void* dest, const SourceElementType* source, int32 count)
	{
		Memory::memcpy(dest, source, sizeof(SourceElementType) * count);
	}

	template<typename ElementType>
	FORCEINLINE typename std::enable_if<!std::is_trivially_copy_assignable<ElementType>::value>::type moveAssignItems(ElementType* dest, const ElementType* source, int32 count)
	{
		while (count)
		{
			*dest = (ElementType&&)*source;
			++dest;
			++source;
			--count;
		}
	}

	template<typename ElementType>
	FORCEINLINE typename std::enable_if<std::is_trivially_copy_assignable<ElementType>::value>::type moveAssignItems(ElementType* dest, const ElementType* source, int32 count)
	{
		Memory::memmove(dest, source, sizeof(ElementType) * count);
	}

	template<typename ElementType>
	FORCEINLINE typename std::enable_if<!std::is_trivially_copy_assignable<ElementType>::value>::type copyAssignItems(ElementType* dest, const ElementType* source, uint32 count)
	{
		while (count)
		{
			*dest = *source;
			++dest;
			++source;
			--count;
		}
	}

	template <typename ElementType>
	FORCEINLINE typename std::enable_if<std::is_trivially_copy_assignable<ElementType>::value>::type copyAssignItems(ElementType* dest, const ElementType* source, uint32 count)
	{
		Memory::memcpy(dest, source, sizeof(ElementType) * count);
	}




	template <typename ElementType>
	FORCEINLINE typename std::enable_if<!std::is_trivially_constructible<ElementType>::value>::type defaultConstructItems(void* address, int32 count)
	{
		ElementType* element = (ElementType*)address;
		while (count)
		{
			new (element)ElementType;
			++element;
			--count;
		}
	}

	template <typename ElementType>
	FORCEINLINE typename std::enable_if<std::is_trivially_constructible<ElementType>::value>::type defaultConstructItems(void* address, int32 count)
	{
		Memory::Memset(address, 0, sizeof(ElementType) * count);
	}


	template <typename ElementType>
	FORCEINLINE typename std::enable_if<!std::is_trivially_destructible<ElementType>::value>::type destructItems(ElementType* element, int32 count)
	{
		while (count)
		{
			typedef ElementType DestructItemsElementTypeTypedef;
			element->DestructItemsElementTypeTypedef::~DestructItemsElementTypeTypedef();
			++element;
			--count;
		}
	}

	template <typename ElementType>
	FORCEINLINE typename std::enable_if<std::is_trivially_destructible<ElementType>::value>::type destructItems(ElementType* element, int32 count)
	{

	}

	template<typename ElementType>
	FORCEINLINE typename std::enable_if<!std::is_trivially_copy_constructible<ElementType>::value>::type moveConstructItems(void* dest, const ElementType* source, int32 count)
	{
		while (count)
		{
			new (dest)ElementType((ElementType&&)*source);
			++(ElementType*&)dest;
			++source;
			--count;
		}
	}

	template<typename ElementType>
	FORCEINLINE typename std::enable_if<std::is_trivially_copy_constructible<ElementType>::value>::type moveConstructItems(void* dest, const ElementType* source, int32 count)
	{
		Memory::memmove(dest, source, sizeof(ElementType) * count);
	}

	template<typename ElementType>
	FORCEINLINE typename std::enable_if<!std::is_move_constructible<ElementType>::value>::type tryMoveConstructItems(void* dest, const ElementType* source, int32 count)
	{
		constructItems<ElementType, ElementType>(dest, source, count);
	}
	template<typename ElementType>
	FORCEINLINE typename std::enable_if<std::is_move_constructible<ElementType>::value>::type tryMoveConstructItems(void* dest, const ElementType* source, int32 count)
	{
		moveConstructItems<ElementType>(dest, source, count);
	}

	template<typename DestinationElementType, typename SourceElementType>
	FORCEINLINE typename std::enable_if<!MemoryOps_Private::can_bitwise_relocate<DestinationElementType, SourceElementType>::value>::type relocateConstructItems(void* dest, const SourceElementType* source, int32 count)
	{
		while (count)
		{
			typedef SourceElementType RelocateConstructItemsElementTypeTypedef;
			new (dest)DestinationElementType(*source);
			++(DestinationElementType*&)dest;
			(source++)->RelocateConstructItemsElementTypeTypedef::~RelocateConstructItemsElementTypeTypedef();
			--count;
		}
	}

	template<typename DestinationElementType, typename SourceElementType>
	FORCEINLINE typename std::enable_if<MemoryOps_Private::can_bitwise_relocate<DestinationElementType, SourceElementType>::value>::type relocateConstructItems(void* dest, const SourceElementType* source, int32 count)
	{
		Memory::memmove(dest, source, sizeof(SourceElementType)* count);
	}


	template<typename ElementType>
	FORCEINLINE typename std::enable_if<std::is_bytewise_comparable<ElementType>::value, bool>::type compareItems(const ElementType* A, const ElementType* B, int32 count)
	{
		return !Memory::memcmp(A, B, sizeof(ElementType) * count);
	}

	template<typename ElementType>
	FORCEINLINE typename std::enable_if<!std::is_bytewise_comparable<ElementType>::value, bool>::type compareItems(const ElementType *A, const ElementType * B, int32 count)
	{
		while (count)
		{
			if (!(*A == *B))
			{
				return false;
			}
			++A;
			++B;
			--count;
		}
		return true;
	}

}