#pragma once
#include "CoreType.h"

namespace Air
{
	template<int32 Order>
	class MS_ALIGN(16) TSHVector
	{
	public:
		enum { MaxSHOrder = Order };
		enum { MaxSHBasis = MaxSHOrder * MaxSHOrder };
		enum { NumComponentsPerSIMDVector = 4 };
		enum { NumSIMDVectors = (MaxSHBasis + NumComponentsPerSIMDVector - 1) / NumComponentsPerSIMDVector };
		enum { NumTotalFloats = NumSIMDVectors * NumComponentsPerSIMDVector };
		float V[NumTotalFloats];

		CORE_API static const float mConstantBisisIntegral;

		TSHVector()
		{
			Memory::memzero(V, sizeof(V));
		}

		TSHVector(float v0, float v1, float v2, float v3)
		{
			Memory::memzero(V, sizeof(V));
			V[0] = v0;
			V[1] = v1;
			V[2] = v2;
			V[3] = v3;
		}

		explicit TSHVector(const float4 & vector)
		{
			Memory::memzero(V, sizeof(V));
			V[0] = vector.x;
			V[1] = vector.y;
			V[2] = vector.z;
			V[3] = vector.w;

		}

		template<int32 OtherOrder>
		explicit TSHVector(const TSHVector<OtherOrder> & other)
		{
			if (order <= OtherOrder)
			{
				Memory::memcpy(V, other.V, sizeof(V));
			}
			else
			{
				Memory::memzero(V);
				Memory::memcpy(V, other.V, sizeof(V));
			}
		}

		friend FORCEINLINE TSHVector operator* (const TSHVector & A, const float& B)
		{
			const VectorRegister replicatedRegister = VectorLoadFloat1(&B);
			TSHVector result;
			for (int32 basisIndex = 0; basisIndex < NumSIMDVectors; basisIndex++)
			{
				VectorRegister mulResult = VectorMultiply(
					VectorLoadAligned(&A.V[basisIndex * NumComponentsPerSIMDVector]),
					replicatedRegister);
				VectorStoreAligned(mulResult, &result.V[basisIndex * NumComponentsPerSIMDVector]);
			}
			return result;
		}

		friend FORCEINLINE TSHVector operator / (const TSHVector & a, const float& scalar)
		{
			const float b = (1.0f / scalar);
			const VectorRegister replicatedScalar = VectorLoadFloat1(&b);
			TSHVector result;
			for (int32 basisIndex = 0; basisIndex < NumSIMDVectors; basisIndex++)
			{
				VectorRegister mulResult = VectorMultiply(
					VectorLoadAligned(&a.V[basisIndex * NumComponentsPerSIMDVector]),
					replicatedScalar);
				VectorStoreAligned(mulResult, &result.V[basisIndex * NumComponentsPerSIMDVector]);
			}
			return result;
		}

		friend FORCEINLINE TSHVector operator + (const TSHVector & a, const TSHVector & b)
		{
			TSHVector result;
			for (int32 basisIndex = 0; basisIndex < NumSIMDVectors; basisIndex++)
			{
				VectorRegister addResult = VectorAdd(
					VectorLoadAligned(&a.V[basisIndex * NumComponentsPerSIMDVector]),
					VectorLoadAligned(&b.V[basisIndex * NumComponentsPerSIMDVector]));
				VectorStoreAligned(addResult, &result.V[basisIndex * NumComponentsPerSIMDVector]);
			}
			return result;
		}


	};

	template<int32 MaxSHOrder>
	class TSHVectorRGB
	{
	public:
		TSHVector<MaxSHOrder> r;
		TSHVector<MaxSHOrder> g;
		TSHVector<MaxSHOrder> b;

		TSHVectorRGB() {}


		friend FORCEINLINE TSHVectorRGB operator* (const TSHVectorRGB& a, const float& scalar)
		{
			TSHVectorRGB result;
			result.r = a.r * scalar;
			result.g = a.g * scalar;
			result.b = a.b * scalar;
			return result;
		}

		friend FORCEINLINE TSHVectorRGB operator + (const TSHVectorRGB& a, const TSHVectorRGB& inB)
		{
			TSHVectorRGB result;
			result.r = a.r + inB.r;
			result.g = a.g + inB.g;
			result.b = a.b + inB.b;
			return result;
		}
	};

	typedef TSHVector<3> SHVector3;
	typedef TSHVector<2> SHVector2;

	typedef TSHVectorRGB<3> SHVectorRGB3;
	typedef TSHVectorRGB<2> SHVectorRGB2;
}