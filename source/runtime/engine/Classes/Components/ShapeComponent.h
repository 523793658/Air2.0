#pragma once
#include "Classes/Components/PrimitiveComponent.h"
#include "Classes/Materials/Material.h"
#include "DynamicMeshBuild.h"
#include "LocalVertexFactory.h"
namespace Air
{
	class CustomMeshVertexBuffer : public VertexBuffer
	{
	public:
		TArray<DynamicMeshVertex> mVertices;

		virtual void initRHI() override
		{
			RHIResourceCreateInfo createInfo;
			void* vertexBufferData = nullptr;
			mVertexBufferRHI = RHICreateAndLockVertexBuffer(mVertices.size() * sizeof(DynamicMeshVertex), BUF_Static, createInfo, vertexBufferData);
			Memory::memcpy(vertexBufferData, mVertices.getData(), mVertices.size() * sizeof(DynamicMeshVertex));
			RHIUnlockVertexBuffer(mVertexBufferRHI);
		}
	};

	class CustomMeshIndexBuffer : public IndexBuffer
	{
	public:
		TArray<int32> mIndices;
		virtual void initRHI() override
		{
			RHIResourceCreateInfo createInfo;
			void* buffer = nullptr;
			mIndexBufferRHI = RHICreateAndLockIndexBuffer(sizeof(int32), mIndices.size() * sizeof(int32), BUF_Static, createInfo, buffer);
			Memory::memcpy(buffer, mIndices.getData(), mIndices.size() * sizeof(int32));
			RHIUnlockIndexBuffer(mIndexBufferRHI);
		}
	};

	class CustomVertexFactory : public LocalVertexFactory
	{
	public:
		CustomVertexFactory() 
			:LocalVertexFactory(ERHIFeatureLevel::SM5, "CustomVertexFactory")
		{}

		void init_RenderThread(const CustomMeshVertexBuffer* vertexBuffer);

		void init(const CustomMeshVertexBuffer* vertexBuffer)
		{
			if (isInRenderingThread())
			{
				init_RenderThread(vertexBuffer);
			}
			else
			{
				ENQUEUE_RENDER_COMMAND(
					InitShapeMeshVertexFactory)([this, vertexBuffer](RHICommandListImmediate& RHICmdList)
						{
							this->init_RenderThread(vertexBuffer);
						});
			}
		}
	};




	struct CustomMesh : std::enable_shared_from_this<CustomMesh>
	{
		CustomMeshVertexBuffer mVertexBuffer;
		CustomMeshIndexBuffer mIndexBuffer;
		CustomVertexFactory mVertexFactory;
		bool	bInited{ false };
		void init();
	};

	class ENGINE_API ShapeComponent : public PrimitiveComponent
	{
		GENERATED_RCLASS_BODY(ShapeComponent, PrimitiveComponent)
	public:
		

		virtual PrimitiveSceneProxy* ShapeComponent::createSceneProxy() override;
	protected:
		friend class CustomMeshSceneProxy;
		CustomMesh*	mMesh;
	protected:
		virtual void _genericGeometryData() { BOOST_ASSERT(false); };
	};
}