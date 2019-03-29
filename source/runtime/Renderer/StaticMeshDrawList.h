#pragma once
#include "EngineMininal.h"
#include "RendererMininal.h"
#include "ScenePrivateBase.h"
#include "Containers/BitArray.h"
#include "RHICommandList.h"
#include "SceneCore.h"
#include "sceneRendering.h"
#include "DrawingPolicy.h"
#include "Containers/Set.h"
namespace Air
{
	extern ENGINE_API bool GDrawListsLocked;

	enum class InstancedStereoPolicy
	{
		Enabled,
		MobileMultiView,
		Disabled
	};


	class StaticMeshDrawListBase
	{
	public:
		static SIZE_T mTotalBytesUsed;
	};

	template<typename DrawingPolicyType>
	class TStaticMeshDrawList : public StaticMeshDrawListBase, public RenderResource
	{
	public:
		typedef typename DrawingPolicyType::ElementDataType ElementPolicyDataType;



	private:
		class ElementHandle : public StaticMesh::DrawListElementLink
		{
		public:
			ElementHandle(TStaticMeshDrawList* inStaticMeshDrawList, SetElementId inSetId, int32 inElementIndex)
				:mStaticMeshDrawList(inStaticMeshDrawList),
				mSetId(inSetId),
				mElementIndex(inElementIndex)
			{}
			virtual bool isInDrawList(const StaticMeshDrawListBase* drawList) const
			{
				return drawList == mStaticMeshDrawList;
			}

			virtual void remove(const bool bUnlinkMesh = true);

		private:
			TStaticMeshDrawList* mStaticMeshDrawList;
			SetElementId mSetId;
			int32 mElementIndex;
		};

		struct ElementCompact
		{
			int32 mMeshId;
			ElementCompact() {}
			ElementCompact(int32 inMeshId) :mMeshId(inMeshId)
			{}
		};
		struct Element
		{
			ElementPolicyDataType mPolicyData;
			StaticMesh* mMesh;
			BoxSphereBounds mBounds;
			bool bBackground;
			TRefCountPtr <ElementHandle> mHandle;

			Element():
				mMesh(nullptr)
			{}

			Element(StaticMesh* inMesh, const ElementPolicyDataType& inPolicyData, TStaticMeshDrawList* staticMeshDrawList, SetElementId setId, int32 elemeintIndex)
				:mPolicyData(inPolicyData),
				mMesh(inMesh),
				mHandle(new ElementHandle(staticMeshDrawList, setId, elemeintIndex))
			{
				//mBounds = mMesh->mPrimitiveSceneInfo->mProxy->get
			}
			~Element()
			{
				if (mMesh)
				{
					mMesh->unlinkDrawList(mHandle);
				}
			}
		};

		struct DrawingPolicyLink
		{ 
			TArray<ElementCompact>	mCompactElements;
			TArray<Element>			mElements;
			DrawingPolicyType		mDrawingPolicy;
			BoundShaderStateRHIRef	mBoundShaderState;
			ERHIFeatureLevel::Type	mFeatureLevel;
			TStaticMeshDrawList*	mDrawList;
			uint32					mVisibleCount;
			SetElementId			mSetId;
			//static uint32 GlobalLinkId;
			DrawingPolicyLink(TStaticMeshDrawList* inDrawList, const DrawingPolicyType& inDrawingPolicy, ERHIFeatureLevel::Type inFeatureLevel)
				:mDrawingPolicy(inDrawingPolicy),
				mFeatureLevel(inFeatureLevel),
				mDrawList(inDrawList),
				mVisibleCount(0)
			{
				createBoundShaderState();
			}

			SIZE_T getSizeBytes() const
			{
				return sizeof(*this) + mCompactElements.size() * mElements.getTypeSize() + mElements.size() * mElements.getTypeSize();
			}

			void releaseBoundShaderState()
			{
				mBoundShaderState.safeRelease();
			}

			void createBoundShaderState()
			{
				BOOST_ASSERT(isInRenderingThread());
				BoundShaderStateInput boundShaderStateInput = mDrawingPolicy.getBoundShaderStateInput(mFeatureLevel);
				mBoundShaderState = RHICreateBoundShaderState(boundShaderStateInput.mVertexDeclarationRHI,
					boundShaderStateInput.mVertexShaderRHI,
					boundShaderStateInput.mHullShaderRHI,
					boundShaderStateInput.mDomainShaderRHI,
					boundShaderStateInput.mGeometryShaderRHI,
					boundShaderStateInput.mPixelShaderRHI);
			}
		};

		struct DrawingPolicyKeyFuncs : BaseKeyFuncs<DrawingPolicyLink, DrawingPolicyType>
		{
			static const DrawingPolicyType& getSetKey(const DrawingPolicyLink& link)
			{
				return link.mDrawingPolicy;
			}
			static bool matches(const DrawingPolicyType& A, const DrawingPolicyType & B)
			{
				return A.matches(B).result();
			}
			static uint32 getKeyHash(const DrawingPolicyType& drawingPolicy)
			{
				return drawingPolicy.getTypeHash();
			}
		};

		class DrawingPolicyLineHash
		{

		};

		template<InstancedStereoPolicy instancedStereo>
		int32 drawElement(RHICommandList& RHICmdList, const ViewInfo& view, const typename DrawingPolicyType::ContextDataType policyContext, DrawingPolicyRenderState& drawRenderState, const Element& element, uint64 batchElementMask, DrawingPolicyLink* drawingPolicyLink, bool bDrawnShared);
		public:

			inline bool drawVisible(RHICommandList& RHICmdList, const ViewInfo& view, const typename DrawingPolicyType::ContextDataType policyContext,
				const DrawingPolicyRenderState& drawRenderState, const TBitArray<SceneRenderingBitArrayAllocator>& staticMeshVisibilityMap, const TArray<uint64>& batchVisibilityArray);

			inline bool drawVisible(RHICommandList& RHICmdList, const ViewInfo& view, const DrawingPolicyRenderState& drawRenderState, const TBitArray<SceneRenderingBitArrayAllocator>& staticMeshVisibilityMap, const TArray<uint64>& batchVisibilityArray)
			{
				return drawVisible(RHICmdList, view, typename DrawingPolicyType::ContextDataType(), drawRenderState, staticMeshVisibilityMap, batchVisibilityArray);
			}

			void addMesh(StaticMesh* mesh, const ElementPolicyDataType& policyData, const DrawingPolicyType& inDrawingPolicy, ERHIFeatureLevel::Type inFeatureLevel);

			template<InstancedStereoPolicy instancedStereo>
			bool drawVisibleInner(RHICommandList& rhiCmdList, const ViewInfo& view,
				const typename DrawingPolicyType::ContextDataType policyContext, DrawingPolicyRenderState& drawRenderState, const TBitArray<SceneRenderingBitArrayAllocator>* const staticMeshVisibilityMap, const TArray<uint64>* const batchVisibilityArray, int32 firstPolicy, int32 lastPolicy, bool bUpdateCounts);
	private:

		typedef TSet<DrawingPolicyLink, DrawingPolicyKeyFuncs> TDrawingPolicySet;

		TArray<SetElementId> mOrderedDrawingPolicies;

		TDrawingPolicySet mDrawingPolicySet;

		uint32 mFrameNumberForVisibleCount;
		uint32 mViewStateUniqueId;
	};


}
#include "StaticMeshDrawList.inl"	

