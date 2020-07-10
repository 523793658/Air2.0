#pragma once
#include "CoreType.h"
#define ENUM_CLASS_FLAGS(Enum)	\
	inline			Enum& operator |= (Enum& lhs, Enum rhs){return lhs = (Enum)((__underlying_type(Enum))lhs | (__underlying_type(Enum))rhs);}\
	inline			Enum& operator &= (Enum& lhs, Enum rhs){return lhs = (Enum)((__underlying_type(Enum))lhs & (__underlying_type(Enum))rhs);}	  \
	inline           Enum& operator^=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs ^ (__underlying_type(Enum))Rhs); } \
	inline CONSTEXPR Enum  operator| (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs | (__underlying_type(Enum))Rhs); } \
	inline CONSTEXPR Enum  operator& (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs & (__underlying_type(Enum))Rhs); } \
	inline CONSTEXPR Enum  operator^ (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs ^ (__underlying_type(Enum))Rhs); } \
	inline CONSTEXPR bool  operator! (Enum  E)             { return !(__underlying_type(Enum))E; } \
	inline CONSTEXPR Enum  operator~ (Enum  E)             { return (Enum)~(__underlying_type(Enum))E; }

template<typename Enum>
inline bool enumHasAllFlags(Enum flags, enum contains)
{
	return (((__underlying_type(Enum))flags) & (__underlying_type(Enum))contains) == ((__underlying_type(Enum))contains);
}


template<typename Enum>
inline bool enumHasAnyFlags(Enum flags, Enum contains)
{
	return (((__underlying_type(Enum))flags) & (__underlying_type(Enum))contains) != 0;
}