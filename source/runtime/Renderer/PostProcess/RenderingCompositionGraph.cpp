#include "PostProcess/RenderingCompositionGraph.h"
#include "PostProcess/RenderTargetPool.h"
namespace Air
{
	void RenderingCompositePassContext::process(RenderingCompositePass* root, TCHAR* graphDebugName)
	{
		BOOST_ASSERT(!bWasProcessed);

		bWasProcessed = true;

		bHasHmdMesh = false;

		if (root)
		{
			bool bNewOrder = false;

			mGraph.recursivelyGatherDependencies(root);

			if (bNewOrder)
			{
				for (RenderingCompositePass* node : mGraph.mNodes)
				{
					if (node->wasComputeOutputDescCalled())
					{
						mGraph.recursivelyProcess(node, *this);
					}
				}
			}
			else
			{
				mGraph.recursivelyProcess(root, *this);
			}
		}
	}

	RenderingCompositionGraph::RenderingCompositionGraph()
	{}

	RenderingCompositionGraph::~RenderingCompositionGraph()
	{
		free();
	}

	void RenderingCompositionGraph::free()
	{
		for (uint32 i = 0; i < (uint32)mNodes.size(); ++i)
		{
			RenderingCompositePass* element = mNodes[i];
			if (MemStack::get().containsPointer(element))
			{
				element->~RenderingCompositePass();
			}
			else
			{
				element->release();
			}
		}
		mNodes.empty();
	}

	void RenderingCompositionGraph::recursivelyGatherDependencies(RenderingCompositePass* pass)
	{
		BOOST_ASSERT(pass);
		if (pass->bComputeOutputDescWasCalled)
		{
			return;
		}
		pass->bComputeOutputDescWasCalled = true;

		uint32 index = 0; 
		while (const RenderingCompositeOutputRef* outputRefIt = pass->getDependency(index++))
		{
			RenderingCompositeOutput* inputOutput = outputRefIt->getOutput();
			if (inputOutput)
			{
				inputOutput->addDependency();
			}

			if (RenderingCompositePass* outputrefItPass = outputRefIt->getPass())
			{
				recursivelyGatherDependencies(outputrefItPass);
			}
		}
		for (uint32 outputId = 0; ; ++outputId)
		{
			EPassOutputId passOutputId = (EPassOutputId)(outputId);
			RenderingCompositeOutput* output = pass->getOutput(passOutputId);
			if (!output)
			{
				break;
			}

			output->mRenderTargetDesc = pass->computeOutputDesc(passOutputId);

		}
	}

	void RenderingCompositionGraph::recursivelyProcess(const RenderingCompositeOutputRef& inOutputRef, RenderingCompositePassContext& context) const
	{
		RenderingCompositePass* pass = inOutputRef.getPass();
		RenderingCompositeOutput* output = inOutputRef.getOutput();

		BOOST_ASSERT(pass);
		BOOST_ASSERT(output);

		if (pass->bProcessWasCalled)
		{
			return;
		}
		pass->bProcessWasCalled = true;

		{
			uint32 index = 0; 
			while (const RenderingCompositeOutputRef* outputRefIt = pass->getDependency(index++))
			{
				if (outputRefIt->getPass())
				{
					if (!outputRefIt)
					{
						break;
					}

					RenderingCompositeOutput* input = outputRefIt->getOutput();
					BOOST_ASSERT(outputRefIt->getPass());
					context.mPass = pass;
					recursivelyProcess(*outputRefIt, context);
				}
			}
		}

		context.mPass = pass;
		context.setViewportInValid();

		pass->process(context);

		{
			uint32 inputId = 0;
			while (const RenderingCompositeOutputRef* outputRefIt = pass->getDependency(inputId++))
			{
				RenderingCompositeOutput* input = outputRefIt->getOutput();
				if (input)
				{
					input->resolveDependencies();
				}
			}
		}
	}

	const SceneRenderTargetItem& RenderingCompositeOutput::requestSurface(const struct RenderingCompositePassContext& context)
	{
		if (mPooledRenderTarget)
		{
			context.mRHICmdList.transitionResource(EResourceTransitionAccess::EWritable, mPooledRenderTarget->getRenderTargetItem().mTargetableTexture);
			return mPooledRenderTarget->getRenderTargetItem();
		}

		if (!mRenderTargetDesc.isValid())
		{
			static SceneRenderTargetItem Null;
			return Null;
		}

		if (!mPooledRenderTarget)
		{
			GRenderTargetPool.findFreeElement(context.mRHICmdList, mRenderTargetDesc, mPooledRenderTarget, mRenderTargetDesc.mDebugName);
		}
		BOOST_ASSERT(!mPooledRenderTarget->isFree());
		SceneRenderTargetItem& renderTargetItem = mPooledRenderTarget->getRenderTargetItem();
		return renderTargetItem;
	}

	RenderingCompositePass* RenderingCompositeOutputRef::getPass() const
	{
		return mSource;
	}

	RenderingCompositeOutput* RenderingCompositeOutputRef::getOutput() const
	{
		if (mSource == 0)
		{
			return 0;
		}
		return mSource->getOutput(mPassOutputId);
	}

	RenderingCompositePassContext::RenderingCompositePassContext(RHICommandListImmediate& RHICmdList, const ViewInfo& inView)
		:mView(inView)
		,mViewState((SceneViewState*)inView.mState)
		,mPass(0)
		,mRHICmdList(RHICmdList)
		,mViewportRect(0, 0, 0, 0)
		,mFeatureLevel(inView.getFeatureLevel())
		,mShaderMap(inView.mShaderMap)
		,bWasProcessed(false)
		,bHasHmdMesh(false)
	{
		BOOST_ASSERT(!isViewportValid());
	}

	RenderingCompositePassContext::~RenderingCompositePassContext()
	{
		mGraph.free();
	}

	int32 RenderingCompositionGraph::computeUniquePassId(RenderingCompositePass* pass) const
	{
		for (uint32 i = 0; i < (uint32)mNodes.size(); ++i)
		{
			RenderingCompositePass* element = mNodes[i];
			if (element == pass)
			{
				return i;
			}
		}
		return -1;
	}

	int32 RenderingCompositionGraph::computeUniqueOutputId(RenderingCompositePass* pass, EPassOutputId outputId) const
	{
		uint32 ret = mNodes.size();

		for (uint32 i = 0; i < (uint32)mNodes.size(); ++i)
		{
			RenderingCompositePass* element = mNodes[i];
			if (element == pass)
			{
				return (int32)(ret + (uint32)outputId);
			}
			uint32 outputCount = 0;
			while (pass->getOutput((EPassOutputId)outputCount))
			{
				++outputCount;
			}
			ret += outputCount;
		}
		return -1;
	}

	const PooledRenderTargetDesc* RenderingCompositePass::getInputDesc(EPassInputId inPassInputId) const
	{
		RenderingCompositePass* This = (RenderingCompositePass*)this;
		const RenderingCompositeOutputRef* outputRef = This->getInput(inPassInputId);
		if (!outputRef)
		{
			return nullptr;
		}
		RenderingCompositeOutput* input = outputRef->getOutput();
		if (!input)
		{
			return nullptr;
		}
		return &input->mRenderTargetDesc;
	}
}