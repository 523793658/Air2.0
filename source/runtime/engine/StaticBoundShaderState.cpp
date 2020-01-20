#include "StaticBoundShaderState.h"
#include "shader.h"
#include "RenderingThread.h"
namespace Air
{
	TLinkedList<GlobalBoundShaderStateResource*>*& GlobalBoundShaderStateResource::getGlobalBoundShaderStateList()
	{
		static TLinkedList<GlobalBoundShaderStateResource*>* list = nullptr;
		return list;
	}

	GlobalBoundShaderStateResource::GlobalBoundShaderStateResource()
		:mGlobalListLink(this)
		, mBoundVertexDeclaration(nullptr)
		, mBoundVertexShader(nullptr)
		, mBoundPixelShader(nullptr)
		, mBoundGeometryShader(nullptr)
	{
		if (isInRenderingThread())
		{
			mGlobalListLink.linkHead(getGlobalBoundShaderStateList());
		}
		else
		{
			ENQUEUE_RENDER_COMMAND(
				LinkGlobalBoundShaderStateResource)([this](RHICommandListImmediate&)
				{
					this->mGlobalListLink.linkHead(getGlobalBoundShaderStateList());
				}
			);
		}
	}

	GlobalBoundShaderStateResource::~GlobalBoundShaderStateResource()
	{
		mGlobalListLink.unLink();
	}

	RHIBoundShaderState* GlobalBoundShaderStateResource::getInitializedRHI(RHIVertexDeclaration* vertexDeclaration, RHIVertexShader* vertexShader, RHIPixelShader* pixelShader, RHIGeometryShader* geometryShader)
	{
		BOOST_ASSERT(isInitialized());
		BOOST_ASSERT(GIsRHIInitialized);
		BOOST_ASSERT(isInRenderingThread());
		if (!isValidRef(mBoundShaderState))
		{
			mBoundVertexDeclaration = vertexDeclaration;
			mBoundVertexShader = vertexShader;
			mBoundPixelShader = pixelShader;
			mBoundGeometryShader = geometryShader;

			mBoundShaderState = RHICreateBoundShaderState(vertexDeclaration, vertexShader, HullShaderRHIRef(), DomainShaderRHIRef(), geometryShader, pixelShader);
		}
		BOOST_ASSERT(mBoundVertexDeclaration == vertexDeclaration && mBoundVertexShader == vertexShader && mBoundPixelShader == pixelShader && mBoundGeometryShader == geometryShader);
		return mBoundShaderState;
	}

	RHIBoundShaderState* GlobalBoundShaderStateResource::getPreinitializedRHI()
	{
		return mBoundShaderState;
	}

	void GlobalBoundShaderStateResource::releaseRHI()
	{
		mBoundShaderState.safeRelease();
	}

	

	

	/*void setGlobalBoundShaderState(RHICommandList& RHICmdList, ERHIFeatureLevel::Type featureLevel, GlobalBoundShaderState& boundShaderState, RHIVertexDeclaration* vertexDeclaration, Shader* vertexShader, Shader* pixelShader, Shader* geometryShader)
	{
		GlobalBoundShaderStateWorkArea* existingGlobalBoundShaderState = boundShaderState.get(featureLevel);
		if (!existingGlobalBoundShaderState)
		{
			GlobalBoundShaderStateWorkArea* newGlobalBoundShaderState = new GlobalBoundShaderStateWorkArea();
			newGlobalBoundShaderState->mArgs.mVertexDeclarationRHI = vertexDeclaration;
			newGlobalBoundShaderState->mArgs.mVertexShader = vertexShader;
			newGlobalBoundShaderState->mArgs.mPixelShader = pixelShader;
			newGlobalBoundShaderState->mArgs.mGeometryShader = geometryShader;
			PlatformMisc::memoryBarrier();
			GlobalBoundShaderStateWorkArea* oldGlobalBoundShaderState = (GlobalBoundShaderStateWorkArea*)PlatformAtomics::interLockedCompareExchangePointer((void**)boundShaderState.getPtr(featureLevel), newGlobalBoundShaderState, nullptr);
			if (oldGlobalBoundShaderState != nullptr)
			{
				delete newGlobalBoundShaderState;
				BOOST_ASSERT(oldGlobalBoundShaderState == boundShaderState.get(featureLevel));
				existingGlobalBoundShaderState = oldGlobalBoundShaderState;
			}
			else
			{
				BOOST_ASSERT(newGlobalBoundShaderState == boundShaderState.get(featureLevel));
				existingGlobalBoundShaderState = newGlobalBoundShaderState;
			}
		}
		else if (!(vertexDeclaration == existingGlobalBoundShaderState->mArgs.mVertexDeclarationRHI && vertexShader == existingGlobalBoundShaderState->mArgs.mVertexShader && pixelShader == existingGlobalBoundShaderState->mArgs.mPixelShader && geometryShader == existingGlobalBoundShaderState->mArgs.mGeometryShader))
		{
			existingGlobalBoundShaderState->mArgs.mVertexDeclarationRHI = vertexDeclaration;
			existingGlobalBoundShaderState->mArgs.mVertexShader = vertexShader;
			existingGlobalBoundShaderState->mArgs.mPixelShader = pixelShader;
			existingGlobalBoundShaderState->mArgs.mGeometryShader = geometryShader;
		}
		if (RHICmdList.bypass() || isInRenderingThread())
		{
			RHICmdList.setBoundShaderState(getGlobalBoundShaderState_Internal(boundShaderState, featureLevel));
			return;
		}
		if (existingGlobalBoundShaderState->BSS)
		{
			RHIBoundShaderState* boundShaderState = existingGlobalBoundShaderState->BSS->getPreinitializedRHI();
			if (boundShaderState)
			{
				RHICmdList.setBoundShaderState(boundShaderState);
				return;
			}
		}
		RHICommandList* cmdList = new RHICommandList;
		cmdList->copyRenderThreadContexts(RHICmdList);
		GraphEventRef renderThreadCompletionEvent = GraphTask<SetGlobalBoundShaderStateRenderThreadTask>::createTask().constructAndDispatchWhenReady(*cmdList, boundShaderState, featureLevel);
		RHICmdList.queueRenderThreadCommandListSubmit(renderThreadCompletionEvent, cmdList);
	}*/
}