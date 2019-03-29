#pragma once
#include "CoreType.h"
#include <ctype.h>
#include <wctype.h>
namespace Air
{
	template<typename T, const unsigned int Size>
	struct TCharBase
	{
		typedef T CharType;
		static const CharType LineFeed = L'\x000A';
		static const CharType VerticalTab = L'\x000B';
		static const CharType FormFeed = L'\x000C';
		static const CharType CarriageReturn = L'\x000D';
		static const CharType NextLine = L'\x0085';
		static const CharType LineSeparator = L'\x2028';
		static const CharType ParagraphSeparator = L'\x2029';
	};


	template <typename T>
	struct TCharBase<T, 1>
	{
		typedef T CharType;

		static const CharType LineFeed = '\x000A';
		static const CharType VerticalTab = '\x000B';
		static const CharType FormFeed = '\x000C';
		static const CharType CarriageReturn = '\x000D';
		static const CharType NextLine = '\x0085';
	};

	template <typename T> struct TLiteral
	{
		static const ANSICHAR select(const ANSICHAR ansi, const WIDECHAR) { return ansi; }
		static const ANSICHAR* select(const ANSICHAR * ansi, const WIDECHAR*) { return ansi; }
	};

	template<> struct TLiteral<WIDECHAR>
	{
		static const WIDECHAR select(const ANSICHAR ansi, const WIDECHAR wide) { return wide; }
		static const WIDECHAR* select(const ANSICHAR *, const WIDECHAR* wide) { return wide; }
	};

#define LITERAL(CharType, StringLiteral) TLiteral<CharType>::select(StringLiteral, L##StringLiteral)

	template<typename T>
	struct TChar : public TCharBase < T, sizeof(T)>
	{
		typedef T CharType;
	public:
		static inline CharType toUpper(CharType c);
		static inline CharType toLower(CharType c);

		static inline bool isDigit(CharType c);

		static inline bool isWhitespace(CharType c);

		static inline bool isAlnum(CharType c);

		static inline bool isUnderScore(CharType c)
		{
			return c == LITERAL(CharType, '_');
		}
	};

	typedef TChar<TCHAR>	Char;
	typedef TChar<WIDECHAR> CharWide;
	typedef TChar<ANSICHAR>	CharAnsi;

	template<> inline TChar<ANSICHAR>::CharType TChar<ANSICHAR>::toUpper(CharType c)
	{
		return ::toupper(c);
	}

	template<> inline TChar<ANSICHAR>::CharType TChar<ANSICHAR>::toLower(CharType c)
	{
		return ::tolower(c);
	}

	template<> inline bool TChar<ANSICHAR>::isWhitespace(CharType c)
	{
		return ::isspace((unsigned char)c) != 0;
	}

	template<> inline bool TChar<ANSICHAR>::isAlnum(CharType c)
	{
		return ::isalnum((unsigned char)c) != 0;
	}

	template<> inline bool TChar<ANSICHAR>::isDigit(CharType c)
	{
		return ::isdigit((unsigned char)c) != 0;
	}




	template<> inline TChar<WIDECHAR>::CharType TChar<WIDECHAR>::toUpper(CharType c)
	{
		return ::towupper(c);
	}

	template<> inline TChar<WIDECHAR>::CharType TChar<WIDECHAR>::toLower(CharType c)
	{
		return ::towlower(c);
	}
	template<> inline bool TChar<WIDECHAR>::isWhitespace(CharType c)
	{
		return ::iswspace(c) != 0;
	}

	template<> inline bool TChar<WIDECHAR>::isAlnum(CharType c)
	{
		return ::iswalnum(c) != 0;
	}

	template<> inline bool TChar<WIDECHAR>::isDigit(CharType c)
	{
		return ::iswdigit(c) != 0;
	}

}