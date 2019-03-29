#pragma once
#include "CoreMinimal.h"
#include "shader.h"
#include "RendererInterface.h"
namespace Air
{
	extern TGlobalResource<FilterVertexDeclaration> GFilterVertexDeclaration;

	enum EDrawRectangleFlags
	{
		EDRF_Default,
		EDRF_UseTriangleOptimization,
		EDRF_UseTesselatedIndexBuffer,
	};

	extern void drawRectangle(
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
		EDrawRectangleFlags flags = EDRF_Default,
		uint32 instanceCount = 1
	);

	class TesselatedScreenRectangleIndexBuffer : public IndexBuffer
	{
	public:
		static const uint32 mWidth = 32;
		static const uint32 mHeight = 20;

		void initRHI() override;

		uint32 numVertices() const;
		uint32 numPrimitives() const;
	};

	extern void drawPostProcessPass(
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
		EDrawRectangleFlags flags = EDRF_Default);
}