#pragma once
#include "EngineMininal.h"
#include "Containers/DynamicRHIResourceArray.h"
#include "StaticMeshResources.h"
namespace Air
{
	template<typename VertexDataType>
	class TStaticMeshVertexData : public StaticMeshVertexDataInterface
	{
		TResourceArray<VertexDataType, VERTEXBUFFER_ALIGNMENT> mData;

	public:



		typedef TResourceArray<VertexDataType, VERTEXBUFFER_ALIGNMENT> ArrayType;
		TStaticMeshVertexData(bool inNeedsCPUAccess = false)
		{
		}

		virtual void resizeBuffer(uint32 numVertices)
		{
			if (mData.size() < numVertices)
			{
				mData.addUninitialized(numVertices - mData.size());
			}
			else if ((uint32)mData.size() > numVertices)
			{
				mData.removeAt(numVertices, mData.size() - numVertices);
			}
		}

		virtual uint32 getStride() const
		{
			return sizeof(VertexDataType);
		}

		virtual uint8* getDataPointer()
		{
			return (uint8*)mData.getData();
		}

		virtual ResourceArrayInterface* getResourceArray()
		{
			return &mData;
		}

		virtual void serialize(Archive& ar)
		{
			mData.bulkSerialize(ar);
		}
		
		virtual bool getAllowCPUAccess() const override
		{
			return mData.getAllowCPUAccess();
		}

		virtual uint32 size() const override
		{
			return mData.size();
		}
	};
}