#pragma once
#include "CoreMinimal.h"
#include "EngineMininal.h"
namespace Air
{

#define FOREACH_OCTREE_CHILD_NODE(childRef) \
	for(OctreeChildNodeRef childRef(0); !childRef.isNull(); childRef.advance())

	class BoxCenterAndExtent
	{
	public:
		float4 mCenter;
		float4 mExtent;

		BoxCenterAndExtent() {}

		BoxCenterAndExtent(const float3& inCenter, const float3& inExtent)
			:mCenter(inCenter, 0)
			,mExtent(inExtent, 0)
		{}

		BoxCenterAndExtent(const Box& box)
		{
			box.getCenterAndExtents((float3&)mCenter, (float3&)mExtent);
			mCenter.w = mExtent.w = 0;
		}

		explicit BoxCenterAndExtent(const BoxSphereBounds& boxSphere)
		{
			mCenter = boxSphere.mOrigin;
			mExtent = boxSphere.mBoxExtent;
			mCenter.w = mExtent.w = 0;
		}

		explicit BoxCenterAndExtent(const float positionRadius[4])
		{
			mCenter = float3(positionRadius[0], positionRadius[1], positionRadius[2]);
			mExtent = float3(positionRadius[3]);
			mCenter.w = mExtent.w = 0;
		}

		Box getBox() const
		{
			return Box(mCenter - mExtent, mCenter + mExtent);
		}

		friend FORCEINLINE bool intersect(const BoxCenterAndExtent& A, const BoxCenterAndExtent& B)
		{
			const VectorRegister centerDifference = VectorAbs(VectorSubtract(VectorLoadAligned(&A.mCenter), VectorLoadAligned(&B.mCenter)));

			const VectorRegister compositeExtent = VectorAdd(VectorLoadAligned(&A.mExtent), VectorLoadAligned(&B.mExtent));

			return VectorAnyGreaterThan(centerDifference, compositeExtent) == false;
		}

		friend FORCEINLINE bool intersect(const BoxSphereBounds& A, const BoxCenterAndExtent& B)
		{
			const VectorRegister centerDifference = VectorAbs(VectorSubtract(VectorLoadFloat3_W0(&A.mOrigin), VectorLoadAligned(&B.mCenter)));

			const VectorRegister compositeExtend = VectorAdd(VectorLoadFloat3_W0(&A.mBoxExtent), VectorLoadAligned(&B.mExtent));

			return VectorAnyGreaterThan(centerDifference, compositeExtend) == false;
		}

		friend FORCEINLINE bool intersect(const float A[4], const BoxCenterAndExtent& B)
		{
			const VectorRegister centerDifference = VectorAbs(VectorSubtract(VectorLoadFloat3_W0(A), VectorLoadAligned(&B.mCenter)));

			const VectorRegister compositeExtent = VectorAdd(VectorSet_W0(VectorLoadFloat1(A + 3)), VectorLoadAligned(&B.mExtent));

			return VectorAnyGreaterThan(centerDifference, compositeExtent) == false;
		}


	};

	class OctreeChildNodeRef
	{
	public:
		union
		{
			struct  
			{
				uint32 x : 1;
				uint32 y : 1;
				uint32 z : 1;
				uint32 bNull : 1;
			};
			uint32 mIndex : 3;
		};

		OctreeChildNodeRef(int32 inx, int32 inY, int32 inZ)
			:x(inx)
			, y(inY)
			, z(inZ)
			, bNull(false)
		{

		}

		OctreeChildNodeRef(int32 inIndex = 0)
			:mIndex(inIndex)
		{
			bNull = false;
		}

		FORCEINLINE void advance()
		{
			if (mIndex < 7)
			{
				++mIndex;
			}
			else
			{
				bNull = true;
			}
		}

		FORCEINLINE bool isNull() const
		{
			return bNull;
		}
	};

	class OctreeChildNodeSubset
	{
	public:
		union 
		{
			struct 
			{
				uint32 bPositiveX : 1;
				uint32 bPositiveY : 1;
				uint32 bPositiveZ : 1;
				uint32 bNegativeX : 1;
				uint32 bNegativeY : 1;
				uint32 bNegativeZ : 1;
			};

			struct 
			{
				uint32 mPositiveChildBits : 3;
				uint32 mNegativeChildBits : 3;
			};

			uint32 mChildBits : 6;
			uint32 mAllBits;
		};

		OctreeChildNodeSubset()
			:mAllBits(0)
		{}

		OctreeChildNodeSubset(OctreeChildNodeRef childRef)
			:mAllBits(0)
		{
			mPositiveChildBits = childRef.mIndex;
			mNegativeChildBits = ~childRef.mIndex;
		}

		bool contains(OctreeChildNodeRef childRef) const;
	};

	class OctreeElementId
	{
	public:
		template<typename, typename>
		friend class TOctree;

		OctreeElementId()
			:mNode(nullptr)
			,mElementIndex(INDEX_NONE)
		{}

		bool isValidId() const
		{
			return mNode != nullptr;
		}

	private:
		const void * mNode;
		int32 mElementIndex;
		OctreeElementId(const void* inNode, int32 inElementIndex)
			:mNode(inNode)
			,mElementIndex(inElementIndex)
		{}

		operator int32() const
		{
			return mElementIndex;
		}
	};


	extern ENGINE_API float GNegativeOneOneTable[2];

	class OctreeNodeContext
	{
	public:
		enum { LoosenessDenominator = 16 };

		BoxCenterAndExtent mBounds;

		float mChildExtent;

		float mChildCenterOffset;

		uint32 mInCullBits;

		uint32 mOutCullBits;

		OctreeNodeContext()
		{

		}

		OctreeNodeContext(uint32 inInCullBits, uint32 inOutCullBits)
			:mInCullBits(inInCullBits)
			,mOutCullBits(inOutCullBits)
		{}

		OctreeNodeContext(const BoxCenterAndExtent& inBounds)
			:mBounds(inBounds)
		{
			const float tightChildExtent = inBounds.mExtent.x * 0.5f;
			const float looseChildExtent = tightChildExtent * (1.0f + 1.0f / (float)LoosenessDenominator);
			mChildExtent = looseChildExtent;
			mChildCenterOffset = mBounds.mExtent.x - looseChildExtent;
		}

		OctreeNodeContext(const BoxCenterAndExtent& inBounds, uint32 inInCullBits, uint32 inOutCullBits)
			:mBounds(inBounds)
			,mInCullBits(inInCullBits)
			,mOutCullBits(inOutCullBits)
		{
			const float tightChildExtent = inBounds.mExtent.x * 0.5f;
			const float looseChildExtent = tightChildExtent * (1.0f + 1.0f / (float)LoosenessDenominator);
			mChildExtent = looseChildExtent;
			mChildCenterOffset = mBounds.mExtent.x - looseChildExtent;
		}

		FORCEINLINE OctreeNodeContext getChildContext(OctreeChildNodeRef childRef) const
		{
			return OctreeNodeContext(BoxCenterAndExtent(
				float3(
					mBounds.mCenter.x + mChildCenterOffset * GNegativeOneOneTable[childRef.x],
					mBounds.mCenter.y + mChildCenterOffset * GNegativeOneOneTable[childRef.y],
					mBounds.mCenter.z + mChildCenterOffset * GNegativeOneOneTable[childRef.z]
				),
				float3(mChildExtent, mChildExtent, mChildExtent)
			));
		}

		FORCEINLINE void getChildContext(OctreeChildNodeRef childRef, OctreeNodeContext* RESTRICT childContext) const
		{
			const OctreeNodeContext* RESTRICT parentContext = this;
			childContext->mBounds.mCenter.x = parentContext->mBounds.mCenter.x + parentContext->mChildCenterOffset * GNegativeOneOneTable[childRef.x];
			childContext->mBounds.mCenter.y = parentContext->mBounds.mCenter.y + parentContext->mChildCenterOffset * GNegativeOneOneTable[childRef.y];
			childContext->mBounds.mCenter.z = parentContext->mBounds.mCenter.z + parentContext->mChildCenterOffset * GNegativeOneOneTable[childRef.z];

			childContext->mBounds.mExtent.x = parentContext->mChildExtent;
			childContext->mBounds.mExtent.y = parentContext->mChildExtent;
			childContext->mBounds.mExtent.z = parentContext->mChildExtent;
			childContext->mBounds.mExtent.w = 0;

			const float tightChildExtent = parentContext->mChildExtent * 0.5f;
			const float looseChildExtent = tightChildExtent * (1.0f + 1.0f / (float)LoosenessDenominator);
			childContext->mChildExtent = looseChildExtent;
			childContext->mChildCenterOffset = parentContext->mChildExtent - looseChildExtent;
		}

		FORCEINLINE OctreeNodeContext getChildContext(OctreeChildNodeRef childRef, uint32 inInCullBits, uint32 inOutCullBits) const
		{
			return OctreeNodeContext(BoxCenterAndExtent(
				float3(
					mBounds.mCenter.x + mChildCenterOffset * GNegativeOneOneTable[childRef.x],
					mBounds.mCenter.y + mChildCenterOffset * GNegativeOneOneTable[childRef.y],
					mBounds.mCenter.z + mChildCenterOffset * GNegativeOneOneTable[childRef.z]
				),
				float3(mChildExtent, mChildExtent, mChildExtent)
			), inInCullBits, inOutCullBits);
		}

		OctreeChildNodeSubset getIntersectingChildren(const BoxCenterAndExtent& boundingBox) const;

		OctreeChildNodeRef getContainingChild(const BoxCenterAndExtent& boundingBox) const;

	};

	template<typename ElementType, typename OctreeSemantics>
	class TOctree
	{
	public:
		typedef TArray<ElementType, typename OctreeSemantics::ElementAllocator> ElementArrayType;
		typedef typename ElementArrayType::TConstIterator ElementConstIt;

		class Node
		{
		public:	 
			friend class TOctree;

			explicit Node(const Node* inParent)
				:mParent(inParent)
				, mInclusiveNumElements(0)
				, bIsLeaf(true)
			{
				FOREACH_OCTREE_CHILD_NODE(childRef)
				{
					mChildren[childRef.mIndex] = nullptr;
				}
			}

			~Node()
			{
				FOREACH_OCTREE_CHILD_NODE(childRef)
				{
					delete mChildren[childRef.mIndex];
				}
			}

			FORCEINLINE ElementConstIt getElementIt() const { return ElementConstIt(mElements); }

			FORCEINLINE bool isLeaf() const { return bIsLeaf; }

			FORCEINLINE bool hasChild(OctreeChildNodeRef childRef) const
			{
				return mChildren[childRef.mIndex] != nullptr && mChildren[childRef.mIndex]->mInclusiveNumElements > 0;
			}

			FORCEINLINE Node* getChild(OctreeChildNodeRef childRef) const
			{
				return mChildren[childRef.mIndex];
			}

			FORCEINLINE int32 getElementCount()
			{
				return mElements.size();
			}

			FORCEINLINE int32 getInclusiveElementCount() const
			{
				return mInclusiveNumElements;
			}

			FORCEINLINE ElementArrayType& getElements() const
			{
				return mElements;
			}

			void shrinkElements()
			{
				mElements.shrink();
				FOREACH_OCTREE_CHILD_NODE(childRef)
				{
					if (mChildren[childRef.mIndex])
					{
						mChildren[childRef.mIndex]->shrinkElements();
					}
				}
			}

			void applyOffset(const float3& inOffset)
			{
				for (auto& element : mElements)
				{
					OctreeSemantics::applyOffset(element, inOffset);
				}
				FOREACH_OCTREE_CHILD_NODE(childRef)
				{
					if (mChildren[childRef.mIndex])
					{
						mChildren[childRef.mIndex]->applyOffset(inOffset);
					}
				}
			}

		private:
			mutable ElementArrayType mElements;
			const Node* mParent;
			mutable Node* mChildren[8];
			mutable uint32 mInclusiveNumElements : 31;

			mutable uint32 bIsLeaf : 1;
		};

		class NodeReference
		{
		public:	
			const Node* mNode;
			OctreeNodeContext mContext;
			NodeReference()
				:mNode(nullptr)
				,mContext()
			{}

			NodeReference(const Node* inNode, const OctreeNodeContext& inContext)
				:mNode(inNode)
				,mContext(inContext)
			{}
		};

		typedef TInlineAllocator<7 * (14 - 1) + 8> DefaultStackAllocator;

		template<typename StackAllocator = DefaultStackAllocator>
		class TConstIterator
		{
		public:
			void pushChild(OctreeChildNodeRef childRef)
			{
				NodeReference* newNode = new(mNodeStack) NodeReference;
				newNode->mNode = mCurrentNode.mNode->getChild(childRef);
				mCurrentNode.mContext.getChildContext(childRef, &newNode->mContext);
			}

			void pushChild(OctreeChildNodeRef childRef, uint32 fullyInsideView, uint32 fullyOutsideView)
			{
				NodeReference* newNode = new (mNodeStack)NodeReference;
				newNode->mNode = mCurrentNode.mNode->getChild(childRef);
				mCurrentNode.mContext.getChildContext(childRef, &newNode->mContext);
				newNode->mContext.mInCullBits = fullyInsideView;
				newNode->mContext.mOutCullBits = fullyOutsideView;
			}

			void pushChild(OctreeChildNodeRef childRef, const OctreeNodeContext& context)
			{
				new(mNodeStack) NodeReference(mCurrentNode.mNode->getChild(childRef), context);
			}

			void advance()
			{
				if (mNodeStack.size())
				{
					mCurrentNode = mNodeStack[mNodeStack.size() - 1];
					mNodeStack.removeAt(mNodeStack.size() - 1);
				}
				else
				{
					mCurrentNode = NodeReference();
				}
			}


			bool hasPendingNodes() const
			{
				return mCurrentNode.mNode != nullptr;
			}

			TConstIterator(const TOctree& tree)
				:mCurrentNode(NodeReference(&tree.mRootNode, tree.mRootNodeContext))
			{

			}

			TConstIterator(const Node& node, const OctreeNodeContext& context)
				:mCurrentNode(NodeReference(&node, context))
			{}

			const Node& getCurrentNode() const
			{
				return *mCurrentNode.mNode;
			}

			const OctreeNodeContext& getCurrentContext() const
			{
				return mCurrentNode.mContext;
			}


		private:
			NodeReference mCurrentNode;
			TArray<NodeReference, StackAllocator> mNodeStack;
		};

		template<typename StackAllocator = DefaultStackAllocator>
		class TConstElementBoxIterator
		{
		public:
			void advance()
			{
				++mElementIt;
				advanceToNextIntersectingElement();
			}

			bool hasPendingElements() const
			{
				return mNodeIt.hasPendingNodes();
			}

			TConstElementBoxIterator(const TOctree& tree, const BoxCenterAndExtent& inBoundingBox)
				:mIteratorBounds(inBoundingBox)
				,mNodeIt(tree)
				,mElementIt(tree.mRootNode.getElementIt())
			{
				processChildren();
				advanceToNextIntersectingElement();
			}

			const ElementType& getCurrentElement() const
			{
				return *mElementIt;
			}


		private:
			void advanceToNextIntersectingElement()
			{
				BOOST_ASSERT(mNodeIt.hasPendingNodes());
				while (1)
				{
					ElementConstIt localElementIt(mElementIt);
					if (localElementIt)
					{
						PlatformMisc::prefetch(&(*localElementIt));
						PlatformMisc::prefetch(&(*localElementIt), PLATFORM_CACHE_LINE_SIZE);

						if (intersect(OctreeSemantics::getBoundingBox(*localElementIt), mIteratorBounds))
						{
							move(mElementIt, localElementIt);
							return;
						}
						while (++localElementIt)
						{
							PlatformMisc::prefetch(&(*localElementIt), PLATFORM_CACHE_LINE_SIZE);
							if (intersect(OctreeSemantics::getBoundingBox(*localElementIt), mIteratorBounds))
							{
								move(mElementIt, localElementIt);
								return;
							}
						}
					}
					mNodeIt.advance();
					if (!mNodeIt.hasPendingNodes())
					{
						move(mElementIt, localElementIt);
						return;
					}
					processChildren();
					move(mElementIt, mNodeIt.getCurrentNode().getElementIt());
				}
			}

			void processChildren()
			{
				const Node& currentNode = mNodeIt.getCurrentNode();
				const OctreeNodeContext& context = mNodeIt.getCurrentContext();
				const OctreeChildNodeSubset intersectingChildSubset = context.getIntersectingChildren(mIteratorBounds);
				FOREACH_OCTREE_CHILD_NODE(childRef)
				{
					if (intersectingChildSubset.contains(childRef) && currentNode.hasChild(childRef))
					{
						mNodeIt.pushChild(childRef);
					}
				}
			}
		private:
			BoxCenterAndExtent mIteratorBounds;
			TConstIterator<StackAllocator> mNodeIt;

			ElementConstIt mElementIt;

			
		};


		void addElement(typename std::type_traits<ElementType>::const_init_type element);

		void removeElement(OctreeElementId elementId);


		TOctree(const float3& inOrigin, float inExtent);

		TOctree();
	private:
		void addElementToNode(typename std::type_traits<ElementType>::const_init_type element, const Node& inNode, const OctreeNodeContext& inContext);


		template<typename TElement, typename TSemantics>
		friend void setOctreeMemoryUsage(class TOctree<TElement, TSemantics>* octree, int32 newSize);

		size_t mTotalSizeBytes;

	private:
		Node mRootNode;

		OctreeNodeContext mRootNodeContext;

		float mMinLeafExtent;
	};
}

#include "GenericOctree.inl"