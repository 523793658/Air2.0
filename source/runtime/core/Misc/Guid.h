#pragma once
#include "CoreType.h"
#include "Serialization/Archive.h"
#include "boost/assert.hpp"
#include "Containers/Map.h"
#include "Misc/Crc.h"
namespace Air
{

	enum class EGuidFormats
	{
		Digits,

		DigitsWithHyphens,

		DigitsWithHyphensInBraces,

		DigitsWithHyphensInParentheses,

		HexValuesInBraces,

		UniqueObjectGuid

	};

	struct Guid 
	{
	public:
		Guid()
			:A(0)
			,B(0)
			,C(0)
			,D(0)
		{}
		Guid(uint32 inA, uint32 inB, uint32 inC, uint32 inD)
			:A(inA)
			,B(inB)
			,C(inC)
			,D(inD)
		{}
	public:
		static CORE_API Guid newGuid();

		friend bool operator == (const Guid& X, const Guid& Y)
		{
			return ((X.A^ Y.A) | (X.B ^ Y.B) | (X.C ^ Y.C) | (X.D ^ Y.D)) == 0;
		}

		friend bool operator != (const Guid& X, const Guid& Y)
		{
			return ((X.A^ Y.A) | (X.B ^ Y.B) | (X.C ^ Y.C) | (X.D ^ Y.D)) != 0;
		}

		friend bool operator < (const Guid& X, const Guid& Y)
		{
			return	((X.A < Y.A) ? true : ((X.A > Y.A) ? false :
					((X.B < Y.B) ? true : ((X.B > Y.B) ? false :
					((X.C < Y.C) ? true : ((X.C > Y.C) ? false :
					((X.D < Y.D) ? true : ((X.D > Y.D) ? false :
						false))))))));
		}

		uint32& operator[](int32 index)
		{
			BOOST_ASSERT(index >= 0);
			BOOST_ASSERT(index < 4);
			switch (index)
			{
			case 0:return A;
			case 1:return B;
			case 2:return C;
			case 3:return D;
			}
			return A;
		}

		const uint32& operator[](int32 index) const
		{
			BOOST_ASSERT(index >= 0);
			BOOST_ASSERT(index < 4);
			switch (index)
			{
			case 0:return A;
			case 1:return B;
			case 2:return C;
			case 3:return D;
			}
			return A;
		}

		friend Archive& operator << (Archive& ar, Guid& g)
		{
			return ar << g.A << g.B << g.C << g.D;
		}
		bool serialize(Archive& ar)
		{
			ar << *this;
			return true;
		}

		bool isValid() const
		{
			return ((A | B | C | D) != 0);
		}

		wstring toString() const
		{
			return toString(EGuidFormats::Digits);
		}

		CORE_API wstring toString(EGuidFormats format) const;

		void invalidate()
		{
			A = B = C = D = 0;
		}

	public:	 
		friend uint32 getTypeHash(const Guid& guid)
		{
			return Crc::memCrc_DEPRECATED(&guid, sizeof(Guid));
		}

	public:
		uint32 A;
		uint32 B;
		uint32 C;
		uint32 D;
	};
}

namespace std
{
	template<>
	struct hash<Air::Guid>
	{
		size_t operator()(const Air::Guid& id) const
		{
			return getTypeHash(id);
		}
	};
}