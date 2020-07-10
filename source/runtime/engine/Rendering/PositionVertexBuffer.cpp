#include "StaticMeshResources.h"
namespace Air
{

	void PositionVertexBuffer::bindPositionVertexBuffer(const class VertexFactory* vertexFactory, struct StaticMeshDataType& data) const
	{
		data.mPositionComponent = VertexStreamComponent(
			this,
			STRUCT_OFFSET(PositionVertex, mPosition),
			getStride(),
			VET_Float3
		);
		data.mPositionComponentSRV = mPositionComponentSRV;
	}
}