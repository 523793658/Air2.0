#pragma once
#include "EngineMininal.h"
#include "RenderResource.h"
#include "RHICommandList.h"
namespace Air
{
	struct ScreenVertex
	{
		float2 mPosition;
		float2 mUV;
	};

	class ScreenVertexDeclaration : public RenderResource
	{
	public:
		VertexDeclarationRHIRef mVertexDeclarationRHI;
		virtual ~ScreenVertexDeclaration() {}

		virtual void initRHI() override
		{
			VertexDeclarationElementList elements;
			uint32 stride = sizeof(ScreenVertex);
			elements.add(VertexElement(0, STRUCT_OFFSET(ScreenVertex, mPosition), VET_Float2, 0, stride));
			elements.add(VertexElement(0, STRUCT_OFFSET(ScreenVertex, mUV), VET_Float2, 1, stride));
			mVertexDeclarationRHI = RHICreateVertexDeclaration(elements);
		}

		virtual void releaseRHI() override
		{
			mVertexDeclarationRHI.safeRelease();
		}
	};

	extern ENGINE_API TGlobalResource<ScreenVertexDeclaration> GScreenVertexDeclaration;
}