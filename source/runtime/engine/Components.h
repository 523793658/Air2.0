#pragma once
#include "EngineMininal.h"


namespace Air
{
	enum { MAX_TEXCOORDS = 4, MAX_STATIC_TEXCOORDS = 8 };

	struct StaticMeshBuildvertex
	{
		float3 mPosition;
		float3 TangentX;
		float3 TangentY;
		float3 TangentZ;

		float2 UVs[MAX_STATIC_TEXCOORDS];
		Color mColor;
	};

	struct MeshUVChannelInfo
	{
		FORCEINLINE MeshUVChannelInfo() { Memory::memzero(*this); }

		MeshUVChannelInfo(ENoInit) {}


		bool bInitialized;

		bool bOverrideDensities;

		float mLocalDensities[MAX_TEXCOORDS];

		friend Archive & operator <<(Archive& ar, MeshUVChannelInfo& info);
	};
}