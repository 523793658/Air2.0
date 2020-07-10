#pragma once
#include "EngineMininal.h"
#include "VertexFactory.h"

namespace Air
{
	enum { MAX_TEXCOORDS = 4, MAX_STATIC_TEXCOORDS = 8 };

	struct StaticMeshBuildVertex
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

	struct StaticMeshDataType
	{
		VertexStreamComponent mPositionComponent;
		VertexStreamComponent mTangentBasisComponents[2];
		TArray<VertexStreamComponent, TFixedAllocator<MAX_STATIC_TEXCOORDS / 2> > mTextureCoordinates;
		VertexStreamComponent mLightMapCoordinateComponent;
		VertexStreamComponent mColorComponent;
		RHIShaderResourceView* mPositionComponentSRV = nullptr;
		RHIShaderResourceView* mTangetsSRV = nullptr;
		RHIShaderResourceView* mColorComponentsSRV = nullptr;
		RHIShaderResourceView* mTextureCoordinateSRV = nullptr;

		int mLightMapCoordinateIndex = -1;
		int mNumTexCoords = -1;
		uint32 mColorIndexMask = ~0u;
		uint32 mLODLightmapDataIndex = 0;
	};
}