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

		RHIBoundShaderState* getInitializedRHI(RHIVertexDeclaration* vertexDeclaration, RHIVertexShader* vertexShader, RHIPixelShader* pixelShader, RHIGeometryShader* geometryShader);

		RHIBoundShaderState* getPreinitializedRHI();

	private:
		BoundShaderStateRHIRef mBoundShaderState;

		TLinkedList<GlobalBoundShaderStateResource*> mGlobalListLink;

		ENGINE_API virtual void releaseRHI();

		RHIVertexDeclaration* mBoundVertexDeclaration;
		RHIVertexShader* mBoundVertexShader;
		RHIPixelShader* mBoundPixelShader;
		RHIGeometryShader* mBoundGeometryShader;
	};
	typedef TGlobalResource<GlobalBoundShaderStateResource> GlobalBoundShaderState_Internal;

	struct GlobalBoundShaderStateArgs
	{
		RHIVertexDeclaration* mVertexDeclarationRHI;
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
		RHIVertexDeclaration* vertexDeclaration,
		Shader* vertexShader,
		Shader* pixelShader,
		Shader* geometryShader = nullptr);
}