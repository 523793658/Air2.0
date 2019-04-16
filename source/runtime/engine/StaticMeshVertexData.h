#pragma once
#include "EngineMininal.h"
#include "Containers/DynamicRHIResourceArray.h"
#include "StaticMeshResources.h"
namespace Air
{
	template<typename VertexDataType>
	class TStaticMeshVertexData : public StaticMeshVertexDataInterface,
		public TResourceArray<VertexDataType, VERTEXBUFFER_ALIGNMENT>
	{
	public:
		typedef TResourceArray<VertexDataType, VERTEXBUFFER_ALIGNMENT> ArrayType;
		TStaticMeshVertexData(bool inNeedsCPUAccess = false)
			:TResourceArray<VertexDataType, VERTEXBUFFER_ALIGNMENT>(inNeedsCPUAccess)
		{
		}

		virtual void resizeBuffer(uint32 numVertices)
		{
			if ((uint32)ArrayType::size() < numVertices)
			{
				ArrayType::addUninitialized(numVertices - ArrayType::size());
			}
			else if ((uint32)ArrayType::size() > numVertices)
			{
				ArrayType::removeAt(numVertices, ArrayType::size() - numVertices);
			}
		}

		virtual uint32 getStride() const
		{
			return sizeof(VertexDataType);
		}

		virtual uint8* getDataPointer()
		{
			return (uint8*)&(*this)[0];
		}

		virtual ResourceArrayInterface* getResourceArray()
		{
			return this;
		}

		virtual void serialize(Archive& ar)
		{
			TResourceArray<VertexDataType, VERTEXBUFFER_ALIGNMENT>::bulkSerialize(ar);
		}
		TStaticMeshVertexData<VertexDataType>& operator = (const TArray<VertexDataType>&other)
		{
			TResourceArray<VertexDataType, VERTEXBUFFER_ALIGNMENT>::operator =(TArray<VertexDataType, TAlignedHeapAllocator<VERTEXBUFFER_ALIGNMENT>>(other));
			return *this;
		}

	};
}