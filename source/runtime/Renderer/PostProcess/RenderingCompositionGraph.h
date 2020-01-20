#pragma once
#include "CoreMinimal.h"
#include "sceneRendering.h"
#include "ScenePrivate.h"
#include "PostProcess/PostProcessParameters.h"
namespace Air
{
	struct RenderingCompositePass;
	struct RenderingCompositeOutputRef;
	struct RenderingCompositeOutput;
	struct PooledRenderTargetDesc;
	struct IPooledRenderTarget;
	class SceneRenderTargetItem;
	class RenderingCompositionGraph
	{
	public:
		RenderingCompositionGraph();
		~RenderingCompositionGraph();

		template<class T>
		T* registerPass(T* inPass)
		{
			BOOST_ASSERT(inPass);
			mNodes.add(inPass);
			return inPass;
		}

		friend struct RenderingCompositePassContext;

	private:
		TArray<RenderingCompositePass*> mNodes;

		void free();

		void processGatherDependency(const RenderingCompositeOutputRef* outputRefIt);

		static void recursivelyGatherDependencies(RenderingCompositePass* pass);

		void recursivelyProcess(const RenderingCompositeOutputRef& inOutputRef, RenderingCompositePassContext& context) const;

		int32 computeUniquePassId(RenderingCompositePass* pass) const;

		int32 computeUniqueOutputId(RenderingCompositePass* pass, EPassOutputId outputId) const;
	};

	struct RenderingCompositePass
	{
		virtual ~RenderingCompositePass() {}

		virtual bool frameBufferBlendingWithInput0() const { return false; }

		virtual RenderingCompositeOutputRef* getInput(EPassInputId inPassInputId) = 0;

		virtual const RenderingCompositeOutputRef* getInput(EPassInputId inPassInputId) const = 0;

		const PooledRenderTargetDesc* getInputDesc(EPassInputId inPassInputId) const;

		bool wasComputeOutputDescCalled()const { return bComputeOutputDescWasCalled; }

		virtual void release() = 0;

		virtual RenderingCompositeOutputRef* getDependency(uint32 index) = 0;

		virtual RenderingCompositeOutput* getOutput(EPassOutputId inPassOutputId) = 0;

		virtual PooledRenderTargetDesc computeOutputDesc(EPassOutputId inPassOutputId) const = 0;

		virtual void process(RenderingCompositePassContext& context) = 0;

		virtual RenderingCompositeOutputRef* getAdditionalDependency(uint32 index) = 0;

		virtual void setInput(EPassInputId inPassInputId, const RenderingCompositeOutputRef& inOutputRef) = 0;

		virtual RHIComputeFence* getComputePassEndFence() const { return nullptr; }

	protected:

		bool bComputeOutputDescWasCalled{ false };

		bool bProcessWasCalled{ false };

		bool bIsComputePass{ false };

		bool bPreferAsyncCompute{ false };

		friend class RenderingCompositionGraph;
	};

	struct RenderingCompositeOutput
	{
		RenderingCompositeOutput()
			:mDependencies(0)
		{}

		void resetDependency()
		{
			mDependencies = 0;
		}

		void addDependency()
		{
			++mDependencies;
		}

		uint32 getDependencyCount() const
		{
			return mDependencies;
		}

		void resolveDependencies()
		{
			if (mDependencies > 0)
			{
				--mDependencies;
				if (!mDependencies)
				{
					mPooledRenderTarget.safeRelease();
				}
			}
		}


		TRefCountPtr<IPooledRenderTarget> requestInput()
		{
			BOOST_ASSERT(mDependencies > 0);
			return mPooledRenderTarget;
		}

		const SceneRenderTargetItem& requestSurface(const struct RenderingCompositePassContext& context);


		PooledRenderTargetDesc mRenderTargetDesc;
		TRefCountPtr<IPooledRenderTarget> mPooledRenderTarget;

	private:
		uint32 mDependencies;

	};


	struct RenderingCompositeOutputRef
	{
		RenderingCompositeOutputRef(RenderingCompositePass* inSource = 0, EPassOutputId inPassOutputId = ePId_Output0)
			:mSource(inSource)
			, mPassOutputId(inPassOutputId)
		{}

		RenderingCompositePass* getPass() const;

		RenderingCompositeOutput* getOutput() const;

		EPassOutputId getOutputId() const { return mPassOutputId; }

		bool isValid() const
		{
			return mSource != 0;
		}
	private:
		RenderingCompositePass * mSource;
		EPassOutputId mPassOutputId;
		friend class RenderingCompositionGraph;
	};

	template<uint32 InputCount, uint32 OutputCount>
	struct TRenderingCompositePassBase : public RenderingCompositePass
	{
		virtual const RenderingCompositeOutputRef* getInput(EPassInputId inPassInputId) const override
		{
			if ((int32)inPassInputId < InputCount)
			{
				return &mPassInputs[inPassInputId];
			}

			return 0;
		}

		virtual RenderingCompositeOutputRef* getInput(EPassInputId inPassInputId) override
		{
			if ((int32)inPassInputId < InputCount)
			{
				return &mPassInputs[inPassInputId];
			}

			return 0;
		}


		virtual RenderingCompositeOutput* getOutput(EPassOutputId inPassOutputId)
		{
			if ((int32)inPassOutputId < OutputCount)
			{
				return &mPassOutputs[inPassOutputId];
			}
			return nullptr;
		}

		virtual void setInput(EPassInputId inPassInputId, const RenderingCompositeOutputRef& inOutputRef) override
		{
			if ((int32)inPassInputId < InputCount)
			{
				mPassInputs[inPassInputId] = inOutputRef;
			}
			else
			{
				BOOST_ASSERT(false);
			}
		}

		virtual RenderingCompositeOutputRef* getDependency(uint32 index)
		{
			RenderingCompositeOutputRef* ret = getInput((EPassInputId)index);
			if (!ret)
			{
				ret = getAdditionalDependency(index - InputCount);
			}
			return ret;
		}

		virtual RenderingCompositeOutputRef* getAdditionalDependency(uint32 index)
		{
			uint32 additionalDependenciesCount = mAdditionalDependencies.size();
			if (index < additionalDependenciesCount)
			{
				return &mAdditionalDependencies[index];
			}
			return nullptr;
		}
	private:
		RenderingCompositeOutputRef mPassInputs[InputCount == 0 ? 1 : InputCount];

	protected:
		RenderingCompositeOutput mPassOutputs[OutputCount];

		wstring mPassOutputDumpFilenames[OutputCount];

		TArray<Color>*	mPassOutputColorArrays[OutputCount];

		TArray<RenderingCompositeOutputRef> mAdditionalDependencies;
	};

	


	struct RenderingCompositePassContext
	{
		RenderingCompositePassContext(RHICommandListImmediate& RHICmdList, const ViewInfo& inView);

		~RenderingCompositePassContext();

		void process(RenderingCompositePass* root, TCHAR* graphDebugName);

		void setViewportAndCallRHI(IntRect inViewPortRect, float inMinZ = 0.0f, float inMaxZ = 1.0f)
		{
			mViewportRect = inViewPortRect;
			mRHICmdList.setViewport(mViewportRect.min.x, mViewportRect.min.y, inMinZ, mViewportRect.max.x, mViewportRect.max.y, inMaxZ);
		}

		void setViewportAndCallRHI(uint32 inMinx, uint32 inMinY, float inMinZ, uint32 inMaxX, uint32 inMaxY, float inMaxZ)
		{
			setViewportAndCallRHI(IntRect(inMinx, inMinY, inMaxX, inMaxY), inMinZ, inMaxZ);
			BOOST_ASSERT(isViewportValid());
		}

		void setViewportInValid()
		{
			mViewportRect = IntRect(0, 0, 0, 0);
			BOOST_ASSERT(!isViewportValid());
		}

		IntRect getViewport() const
		{
			BOOST_ASSERT(isViewportValid());
			return mViewportRect;
		}

		bool hasHmdMesh() const
		{
			return bHasHmdMesh;
		}

		ERHIFeatureLevel::Type getFeatureLevel() const { return mFeatureLevel; }

		EShaderPlatform getShaderPlatform() const { return GShaderPlatformForFeatureLevel[mFeatureLevel]; }

		TShaderMap<GlobalShaderType>* getShaderMap() const { BOOST_ASSERT(mShaderMap); return mShaderMap; }

		bool isViewportValid() const
		{
			return mViewportRect.min != mViewportRect.max;
		}

		const ViewInfo& mView;
		SceneViewState* mViewState;
		RenderingCompositePass* mPass;
		RenderingCompositionGraph mGraph;
		RHICommandListImmediate& mRHICmdList;
		IntRect mSceneColorViewRect;

	private:
		IntRect mViewportRect;

		ERHIFeatureLevel::Type mFeatureLevel;

		TShaderMap<GlobalShaderType>* mShaderMap;

		bool bWasProcessed;

		bool bHasHmdMesh;
	};

}