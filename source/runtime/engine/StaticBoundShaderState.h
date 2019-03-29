#pragma once
#include "CoreMinimal.h"
#include "RenderResource.h"
#include "EngineMininal.h"
#include "Containers/LinkList.h"
#include "RHICommandList.h"
namespace Air
{
	class Shader;
	class GlobalBoundShaderStateResource : public RenderResource
	{
	public:
		static TLinkedList<GlobalBoundShaderStateResource*>*& getGlobalBoundShaderStateList();
		ENGINE_API GlobalBoundShaderStateResource();

		ENGINE_API virtual ~GlobalBoundShaderStateResource();

		BoundShaderStateRHIParamRef getInitializedRHI(VertexDeclarationRHIParamRef vertexDeclaration, VertexShaderRHIParamRef vertexShader, PixelShaderRHIParamRef pixelShader, GeometryShaderRHIParamRef geometryShader);

		BoundShaderStateRHIParamRef getPreinitializedRHI();

	private:
		BoundShaderStateRHIRef mBoundShaderState;

		TLinkedList<GlobalBoundShaderStateResource*> mGlobalListLink;

		ENGINE_API virtual void releaseRHI();

		VertexDeclarationRHIParamRef mBoundVertexDeclaration;
		VertexShaderRHIParamRef mBoundVertexShader;
		PixelShaderRHIParamRef mBoundPixelShader;
		GeometryShaderRHIParamRef mBoundGeometryShader;
	};
	typedef TGlobalResource<GlobalBoundShaderStateResource> GlobalBoundShaderState_Internal;

	struct GlobalBoundShaderStateArgs
	{
		VertexDeclarationRHIParamRef mVertexDeclarationRHI;
		Shader* mVertexShader;
		Shader* mPixelShader;
		Shader* mGeometryShader;

	};

	struct GlobalBoundShaderStateWorkArea
	{
		GlobalBoundShaderStateArgs mArgs;
		GlobalBoundShaderState_Internal * BSS;
		GlobalBoundShaderStateWorkArea()
			:BSS(nullptr)
		{}
	};

	struct GlobalBoundShaderState
	{
	public:
		GlobalBoundShaderStateWorkArea* get(ERHIFeatureLevel::Type inFeatureLevel) { return mWorkAreas[inFeatureLevel]; }

		GlobalBoundShaderStateWorkArea** getPtr(ERHIFeatureLevel::Type inFeatureLevel) { return &mWorkAreas[inFeatureLevel]; }

	private:
		GlobalBoundShaderStateWorkArea* mWorkAreas[ERHIFeatureLevel::Num];
	};

	ENGINE_API void setGlobalBoundShaderState(RHICommandList& RHICmdList, ERHIFeatureLevel::Type featureLevel, GlobalBoundShaderState& boundShaderState,
		VertexDeclarationRHIParamRef vertexDeclaration,
		Shader* vertexShader,
		Shader* pixelShader,
		Shader* geometryShader = nullptr);
}