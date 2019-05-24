#pragma once
#include "EngineMininal.h"
#include "RenderResource.h"
#include "Components.h"
namespace Air
{
	class ColorVertexBuffer : public VertexBuffer
	{
	public:
		ENGINE_API ColorVertexBuffer();
		ENGINE_API ~ColorVertexBuffer();
		FORCEINLINE uint8 getStride() const
		{
			return mStride;
		}

		void cleanUp();

		ENGINE_API void init(const TArray<StaticMeshBuildVertex>& vertices);

		void serialize(Archive& ar, bool bNeedsCPUAccess);

		FORCEINLINE int32 getNumVertices() const
		{
			return mNumVertex;
		}

		FORCEINLINE Color& vertexColor(uint32 vertexIndex)
		{
			BOOST_ASSERT(vertexIndex < getNumVertices());
			return *(Color*)(mData + vertexIndex * mStride);
		}

		virtual void initRHI() override;

	private:
		class ColorVertexData*	mVertexData{ nullptr };
		uint8* mData{ nullptr };

		uint32 mStride{ 0 };

		uint32 mNumVertex{ 0 };

		void allocateData(bool bNeedsCPUAccess = true);

		ENGINE_API ColorVertexBuffer(const ColorVertexBuffer& rhs);

		friend class StaticLODModel;
	};
}