#pragma once
#include "CoreMinimal.h"
#include "RenderCore.h"
#include "Math/Math.h"
namespace Air
{
	struct PackedNormal
	{
		union
		{

			struct
			{
#if PLATFORM_LITTLE_ENDIAN
				uint8 x, y, z, w;
#else
				uint8 w, z, y, x;
#endif
			};
			uint32 mPacked;
		}	Vector;

		PackedNormal() { Vector.mPacked = 0; }
		PackedNormal(uint32 inPacked) { Vector.mPacked = inPacked; }
		PackedNormal(const float3& inVector) { *this = inVector; }
		PackedNormal(uint8 inX, uint8 inY, uint8 inZ, uint8 inW)
		{
			Vector.x = inX;
			Vector.y = inY;
			Vector.z = inZ;
			Vector.w = inW;
		}

		void operator = (const float3& inVector);
		void operator = (const float4& inVector);

		operator float3() const;

		operator float4() const;
		VectorRegister getVectorRegister() const;

		void set(const float3& inVector) { *this = inVector; }

		bool operator ==(const PackedNormal& B) const;

		bool operator !=(const PackedNormal& B) const;

		friend RENDER_CORE_API Archive& operator <<(Archive& ar, PackedNormal& n);

		wstring toString() const
		{
			return printf(TEXT("X=%d Y=%d Z=%d W=%d"), Vector.x, Vector.y, Vector.z, Vector.w);
		}

		static RENDER_CORE_API PackedNormal ZeroNormal;
	};

	extern RENDER_CORE_API const VectorRegister GVectorPackingConstants;

	FORCEINLINE void PackedNormal::operator=(const float3& inVector)
	{
		Vector.x = Math::clamp(Math::truncToInt(inVector.x * 127.5f + 127.5f), 0, 255);
		Vector.y = Math::clamp(Math::truncToInt(inVector.y * 127.5f + 127.5f), 0, 255);
		Vector.z = Math::clamp(Math::truncToInt(inVector.z * 127.5f + 127.5f), 0, 255);
		Vector.w = 128;
	}

	FORCEINLINE void PackedNormal::operator=(const float4& inVector)
	{
		Vector.x = Math::clamp(Math::truncToInt(inVector.x * 127.5f + 127.5f), 0, 255);
		Vector.y = Math::clamp(Math::truncToInt(inVector.y * 127.5f + 127.5f), 0, 255);
		Vector.z = Math::clamp(Math::truncToInt(inVector.z * 127.5f + 127.5f), 0, 255);
		Vector.w = Math::clamp(Math::truncToInt(inVector.w * 127.5f + 127.5f), 0, 255);;
	}

	FORCEINLINE bool PackedNormal::operator==(const PackedNormal& B) const
	{
		float3 v1 = *this;
		float3 v2 = B;
		if (Math::abs(v1.x - v2.x) > THRESH_NORMALS_ARE_SAME * 4.0f)
			return 0;
		if (Math::abs(v1.y - v2.y) > THRESH_NORMALS_ARE_SAME * 4.0f)
			return 0;
		if (Math::abs(v1.z - v2.z) > THRESH_NORMALS_ARE_SAME * 4.0f)
			return 0;
		return 1;
	}

	FORCEINLINE bool PackedNormal::operator!=(const PackedNormal& B) const
	{
		return !(*this == B);
	}

	FORCEINLINE PackedNormal::operator float3() const
	{
		VectorRegister vectorToUnpack = getVectorRegister();
		float3 unpackedVector;
		VectorStoreFloat3(vectorToUnpack, &unpackedVector);
		return unpackedVector;
	}

	FORCEINLINE PackedNormal::operator Air::float4() const
	{
		VectorRegister vectorToUnpack = getVectorRegister();
		float4 unpackedVector;
		VectorStore(vectorToUnpack, &unpackedVector);
		return unpackedVector;
	}
	FORCEINLINE VectorRegister PackedNormal::getVectorRegister() const
	{
		VectorRegister vectorToUnpack = VectorLoadByte4(this);
		vectorToUnpack = VectorMultiplyAdd(vectorToUnpack, VectorReplicate(GVectorPackingConstants, 2), VectorReplicate(GVectorPackingConstants, 3));
		VectorResetFloatRegisters();
		return vectorToUnpack;
	}

	struct PackagedRGBA16N
	{
		struct
		{
			uint16 mX;
			uint16 mY;
			uint16 mZ;
			uint16 mW;
		};

		PackagedRGBA16N() { mX = 0; mY = 0; mZ = 0; mW = 0; }
		PackagedRGBA16N(const float3& inVector)
		{
			*this = inVector;
		}

		PackagedRGBA16N(const float4& inVector)
		{
			*this = inVector;
		}

		PackagedRGBA16N(uint16 inX, uint16 inY, uint16 inZ, uint16 inW)
		{
			mX = inX;
			mY = inY;
			mZ = inZ;
			mW = inW;
		}

		void operator = (const float3& inVector);
		void operator = (const float4& inVector);

		operator float3() const;
		operator float4() const;

		VectorRegister getVectorRegister() const;

		void set(const float3& inVector) { *this = inVector; }
		void set(const float4& inVector) { *this = inVector; }

		bool operator == (const PackagedRGBA16N& B) const;
		bool operator != (const PackagedRGBA16N& B) const;
		friend RENDER_CORE_API Archive& operator << (Archive& ar, PackagedRGBA16N& n);

		wstring toString() const
		{
			return printf(TEXT("X=%d Y=%d Z=%d W=%d"), mX, mY, mZ, mW);
		}
		
		static RENDER_CORE_API PackagedRGBA16N ZeroVector;
	};

	FORCEINLINE void PackagedRGBA16N::operator =(const float3& inVector)
	{
		mX = Math::clamp(Math::truncToInt(inVector.x * 32767.5f + 32767.5f), 0, 65535);
		mY = Math::clamp(Math::truncToInt(inVector.y * 32767.5f + 32767.5f), 0, 65535);
		mZ = Math::clamp(Math::truncToInt(inVector.z * 32767.5f + 32767.5f), 0, 65535);
		mW = 65535;
	}

	FORCEINLINE void PackagedRGBA16N::operator =(const float4& inVector)
	{
		mX = Math::clamp(Math::truncToInt(inVector.x * 32767.5f + 32767.5f), 0, 65535);
		mY = Math::clamp(Math::truncToInt(inVector.y * 32767.5f + 32767.5f), 0, 65535);
		mZ = Math::clamp(Math::truncToInt(inVector.z * 32767.5f + 32767.5f), 0, 65535);
		mW = Math::clamp(Math::truncToInt(inVector.w * 32767.5f + 32767.5f), 0, 65535);
	}

	FORCEINLINE VectorRegister PackagedRGBA16N::getVectorRegister() const
	{
		VectorRegister vectorToUnpack = vectorLoadURGBA16N((void*)this);
		vectorToUnpack = VectorMultiplyAdd(vectorToUnpack, MakeVectorRegister(2.0f, 2.0f, 2.0f, 2.0f), MakeVectorRegister(-1.0f, -1.0f, -1.0f, -1.0f));
		VectorResetFloatRegisters();
		return vectorToUnpack;
	}

	FORCEINLINE bool PackagedRGBA16N::operator ==(const PackagedRGBA16N& b) const
	{
		float3 v1 = *this;
		float3 v2 = b;
		if (Math::abs(v1.x - v2.x) > THRESH_NORMALS_ARE_SAME * 4.0f)
			return false;
		if (Math::abs(v1.y - v2.y) > THRESH_NORMALS_ARE_SAME * 4.0f)
			return false;
		if (Math::abs(v1.z - v2.z) > THRESH_NORMALS_ARE_SAME * 4.0f)
			return false;
		return true;
	}

	FORCEINLINE bool PackagedRGBA16N::operator !=(const PackagedRGBA16N& b) const
	{
		return !(*this == b);
	}

}