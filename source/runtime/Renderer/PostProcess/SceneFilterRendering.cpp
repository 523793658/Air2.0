#include "PostProcess/SceneFilterRendering.h"
#include "Shader.h"
#include "ShaderParameterUtils.h"
#include "Containers/DynamicRHIResourceArray.h"
namespace Air
{
	TGlobalResource<FilterVertexDeclaration> GFilterVertexDeclaration;

	class ScreenRectangleVertexBuffer : public VertexBuffer
	{
	public:
		void initRHI() override
		{
			TResourceArray<FilterVertex, VERTEXBUFFER_ALIGNMENT> vertices;
			vertices.setNumUninitialized(6);
			vertices[0].mPosition = float4(1, 1, 1, 1);
			vertices[0].mUV = float2(1, 1);

			vertices[1].mPosition = float4(0, 1, 1, 1);
			vertices[1].mUV = float2(0, 1);

			vertices[2].mPosition = float4(1, 0, 1, 1);
			vertices[2].mUV = float2(1, 0);

			vertices[3].mPosition = float4(0, 0, 1, 1);
			vertices[3].mUV = float2(0, 0);

			vertices[4].mPosition = float4(-1, 1, 1, 1);
			vertices[4].mUV = float2(-1, 1);

			vertices[5].mPosition = float4(1, -1, 1, 1);
			vertices[5].mUV = float2(1, -1);

			RHIResourceCreateInfo createInfo(&vertices);
			mVertexBufferRHI = RHICreateVertexBuffer(vertices.getResourceDataSize(), BUF_Static, createInfo);
		}
	};

	class ScreenRectangleIndexBuffer : public IndexBuffer
	{
	public:
		void initRHI() override
		{
			const uint16 indices[] = { 0, 1, 2, 2, 1, 3, 0, 4, 5 };
			TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> indexBuffer;
			uint32 numIndices = ARRAY_COUNT(indices);
			indexBuffer.addUninitialized(numIndices);
			Memory::memcpy(indexBuffer.getData(), indices, numIndices * sizeof(uint16));

			RHIResourceCreateInfo createInf(&indexBuffer);
			mIndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), indexBuffer.getResourceDataSize(), BUF_Static, createInf);
		}
	};

	void TesselatedScreenRectangleIndexBuffer::initRHI()
	{
		TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> indexBuffer;
		uint32 numIndices = numPrimitives() * 3;
		indexBuffer.addUninitialized(numIndices);
		uint16* out = (uint16*)indexBuffer.getData();
		for (uint32 y = 0; y < mHeight; ++y)
		{
			for (uint32 x = 0; x < mWidth; ++x)
			{
				uint16 index00 = x + y * (mWidth + 1);
				uint16 index10 = index00 + 1;
				uint16 index01 = index00 + (mWidth + 1);
				uint16 index11 = index01 + 1;

				*out++ = index00; *out++ = index01; *out++ = index10;
				*out++ = index11; *out++ = index10; *out++ = index01;
			}
		}
		RHIResourceCreateInfo createInfo(&indexBuffer);
		mIndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), indexBuffer.getResourceDataSize(), BUF_Static, createInfo);
	}

	uint32 TesselatedScreenRectangleIndexBuffer::numVertices() const
	{
		return (mWidth + 1) * (mHeight + 1);
	}

	uint32 TesselatedScreenRectangleIndexBuffer::numPrimitives() const
	{
		return 2 * mWidth * mHeight;
	}


	static TGlobalResource<ScreenRectangleVertexBuffer> GScreenRectangleVertexBuffer;
	static TGlobalResource<ScreenRectangleIndexBuffer> GScreenRectangleIndexBuffer;



	static TGlobalResource<TesselatedScreenRectangleIndexBuffer> GTesselatedScreenRectangleIndexBuffer;

	static void doDrawRectangleFlagOverride(EDrawRectangleFlags& flags)
	{

	}



	IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(DrawRectangleParameters, "DrawRectangleParameters");

	template<typename TRHICommandList>
	static inline void internalDrawRectangle(TRHICommandList& RHICmdList,
		float x,
		float y,
		float sizeX,
		float sizeY,
		float u,
		float v,
		float sizeU,
		float sizeV,
		int2 targetSize,
		int2 textureSize,
		Shader* vertexShader,
		EDrawRectangleFlags flags,
		int32 instanceCount)
	{
		float clipSpaceQuadZ = 0.0f;
		doDrawRectangleFlagOverride(flags);
		if (x > 0.0f || y > 0.0f)
		{
			flags = EDRF_Default;
		}
		DrawRectangleParameters parameters;
		parameters.PosScaleBias = float4(sizeX, sizeY, x, y);
		parameters.UVScaleBias = float4(sizeU, sizeV, u, v);
		parameters.InvTargetSizeAndTextureSize = float4(1.0f / targetSize.x, 1.0f / targetSize.y, 1.0f / textureSize.x, 1.0f / textureSize.y);
		setConstantBufferParameterImmediate(RHICmdList, vertexShader->getVertexShader(), vertexShader->getConstantBufferParameter<DrawRectangleParameters>(), parameters);
		if (flags == EDRF_UseTesselatedIndexBuffer)
		{
			RHICmdList.setStreamSource(0, nullptr, 0);
			RHICmdList.drawIndexedPrimitive(GTesselatedScreenRectangleIndexBuffer.mIndexBufferRHI, PT_TriangleList, 0, 0, GTesselatedScreenRectangleIndexBuffer.numVertices(), 0, GTesselatedScreenRectangleIndexBuffer.numPrimitives(), instanceCount);
		}
		else
		{
			RHICmdList.setStreamSource(0, GScreenRectangleVertexBuffer.mVertexBufferRHI, 0);
			if (flags == EDRF_UseTriangleOptimization)
			{
				RHICmdList.drawIndexedPrimitive(GScreenRectangleIndexBuffer.mIndexBufferRHI, PT_TriangleList, 0, 0, 3, 6, 1, instanceCount);
			}
			else
			{
				RHICmdList.setStreamSource(0, GScreenRectangleVertexBuffer.mVertexBufferRHI, 0);
				RHICmdList.drawIndexedPrimitive(GScreenRectangleIndexBuffer.mIndexBufferRHI, PT_TriangleList, 0, 0, 4, 0, 2, instanceCount);
			}
		}
	}

	void drawRectangle(
		RHICommandList& RHICmdList,
		float x,
		float y,
		float sizeX,
		float sizeY,
		float U,
		float V,
		float sizeU,
		float sizeV,
		int2 targetSize,
		int2 textureSize,
		class Shader* vertexShader,
		EDrawRectangleFlags flags,
		uint32 instanceCount
	)
	{
		internalDrawRectangle(RHICmdList, x, y, sizeX, sizeY, U, V, sizeU, sizeV, targetSize, textureSize, vertexShader, flags, instanceCount);
	}

	

	void drawPostProcessPass(
		RHICommandList& RHICmdList,
		float x,
		float y,
		float width,
		float height,
		float u,
		float v,
		float sizeU,
		float sizeV,
		int2 targetSize,
		int2 textureSize,
		Shader* vertexShader,
		EStereoscopicPass stereoView,
		bool bHasCustomMesh,
		EDrawRectangleFlags flags)
	{
		if (bHasCustomMesh && stereoView != eSSP_FULL)
		{
		}
		else
		{
			drawRectangle(RHICmdList, x, y, width, height, u, v, sizeU, sizeV, targetSize, textureSize, vertexShader, flags);
		}
	}
}