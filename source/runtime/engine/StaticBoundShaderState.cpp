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
			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
				LinkGlobalBoundShaderStateResource, GlobalBoundShaderStateResource*, resource, this,
				{
					resource->mGlobalListLink.linkHead(getGlobalBoundShaderStateList());
				}
			);
		}
	}

	GlobalBoundShaderStateResource::~GlobalBoundShaderStateResource()
	{
		mGlobalListLink.unLink();
	}

	BoundShaderStateRHIParamRef GlobalBoundShaderStateResource::getInitializedRHI(VertexDeclarationRHIParamRef vertexDeclaration, VertexShaderRHIParamRef vertexShader, PixelShaderRHIParamRef pixelShader, GeometryShaderRHIParamRef geometryShader)
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

	BoundShaderStateRHIParamRef GlobalBoundShaderStateResource::getPreinitializedRHI()
	{
		return mBoundShaderState;
	}

	void GlobalBoundShaderStateResource::releaseRHI()
	{
		mBoundShaderState.safeRelease();
	}

	static BoundShaderStateRHIParamRef getGlobalBoundShaderState_Internal(GlobalBoundShaderState& globalBoundShaderState, ERHIFeatureLevel::Type inFeatureLevel)
	{
		auto workArea = globalBoundShaderState.get(inFeatureLevel);
		GlobalBoundShaderState_Internal* BSS = workArea->BSS;
		bool bNewBSS = false;
		if (!BSS)
		{
			BSS = new GlobalBoundShaderState_Internal();
			bNewBSS = true;
		}
		BoundShaderStateRHIParamRef result = BSS->getInitializedRHI(workArea->mArgs.mVertexDeclarationRHI, GETSAFERHISHADER_VERTEX(workArea->mArgs.mVertexShader),
			GETSAFERHISHADER_PIXELSHADER(workArea->mArgs.mPixelShader),
			GETSAFERHISHADER_GEOMETRY(workArea->mArgs.mGeometryShader));
		if (bNewBSS)
		{
			PlatformMisc::memoryBarrier();
			GlobalBoundShaderState_Internal *oldBSS = (GlobalBoundShaderState_Internal*)PlatformAtomics::interLockedCompareExchangePointer((void**)&workArea->BSS, BSS, nullptr);
			BOOST_ASSERT(!oldBSS);
		}
		return result;
	}

	class SetGlobalBoundShaderStateRenderThreadTask
	{
		RHICommandList& RHICmdList;
		GlobalBoundShaderState& mGlobalBoundShaderState;
		ERHIFeatureLevel::Type mFeatureLevel;
	public:
		SetGlobalBoundShaderStateRenderThreadTask(RHICommandList& inRHICmdList, GlobalBoundShaderState& inGlobalBoundShaderState, ERHIFeatureLevel::Type inFeatureLevel)
			:RHICmdList(inRHICmdList)
			, mGlobalBoundShaderState(inGlobalBoundShaderState)
			, mFeatureLevel(inFeatureLevel)
		{

		}

		ENamedThreads::Type getDesiredThread()
		{
			return ENamedThreads::RenderThread_Local;
		}

		static ESubsequentsMode::Type getSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }

		void doTask(ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent)
		{
			RHICmdList.setBoundShaderState(getGlobalBoundShaderState_Internal(mGlobalBoundShaderState, mFeatureLevel));
		}
	};

	void setGlobalBoundShaderState(RHICommandList& RHICmdList, ERHIFeatureLevel::Type featureLevel, GlobalBoundShaderState& boundShaderState, VertexDeclarationRHIParamRef vertexDeclaration, Shader* vertexShader, Shader* pixelShader, Shader* geometryShader)
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
			BoundShaderStateRHIParamRef boundShaderState = existingGlobalBoundShaderState->BSS->getPreinitializedRHI();
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
	}
}