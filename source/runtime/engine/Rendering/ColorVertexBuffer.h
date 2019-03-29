#pragma once
#include "EngineMininal.h"
#include "RenderResource.h"
namespace Air
{
	class ColorVertexBuffer : public VertexBuffer
	{
	private:
		class ColorVertexData*	mVertexData;
		uint8* mData;

		uint32 mStride;

		uint32 mNumVertex;

		void allocateData(bool bNeedsCPUAccess = true);

		ENGINE_API ColorVertexBuffer(const ColorVertexBuffer& rhs);

		friend class StaticLODModel;
	};
}