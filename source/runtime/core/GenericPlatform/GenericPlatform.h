#pragma once

template<typename T32BITS, typename T64BITS, int PointerSize>
struct SelectIntPointerType
{

};
template<typename T32BITS, typename T64BITS>
struct SelectIntPointerType<T32BITS, T64BITS, 8>
{
	typedef T64BITS TIntPointer; // select the 64 bit type
};

template<typename T32BITS, typename T64BITS>
struct SelectIntPointerType<T32BITS, T64BITS, 4>
{
	typedef T32BITS TIntPointer; // select the 64 bit type
};

struct GenericPlatformTypes
{
	typedef char			ANSICHAR;
	typedef wchar_t			WIDECHAR;





};
