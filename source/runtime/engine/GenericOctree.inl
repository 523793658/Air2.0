#pragma once
#include "CoreType.h"

#include "CoreFwd.h"

namespace Air
{
	template<typename ElementType, typename OctreeSemantics>
	void TOctree<ElementType, OctreeSemantics>::addElement(typename std::type_traits<ElementType>::const_init_type element)
	{
		addElementToNode(element, mRootNode, mRootNodeContext);
	}

	template<typename ElementType, typename OctreeSemantics>
	void TOctree<ElementType, OctreeSemantics>::addElementToNode(typename std::type_traits<ElementType>::const_init_type element, const Node& inNode, const OctreeNodeContext& inContext)
	{
		const BoxCenterAndExtent elementBounds(OctreeSemantics::getBoundingBox(element));
		for (TConstIterator<TInlineAllocator<1>> nodeIt(inNode, inContext); nodeIt.hasPendingNodes(); nodeIt.advance())
		{
			const Node& node = nodeIt.getCurrentNode();
			const OctreeNodeContext& context = nodeIt.getCurrentContext();

			const bool bIsLeaf = node.isLeaf();

			bool bAddElementToThisNode = false;

			node.mInclusiveNumElements++;
			if (bIsLeaf)
			{
				if (node.mElements.size() + 1 > OctreeSemantics::MaxElementsPerLeaf && context.mBounds.mExtent.x > mMinLeafExtent)
				{
					ElementArrayType childElements;
					exchange(childElements, node.mElements);
					setOctreeMemoryUsage(this, mTotalSizeBytes - childElements.size() * sizeof(ElementType));
					node.mInclusiveNumElements = 0;
					node.bIsLeaf = false;

					for (ElementConstIt elementIt(childElements); elementIt; ++elementIt)
					{
						addElementToNode(*elementIt, node, context);
					}

					addElementToNode(element, node, context);
					return;
				}
				else
				{
					bAddElementToThisNode = true;
				}
			}
			else
			{
				const OctreeChildNodeRef childRef = context.getContainingChild(elementBounds);
				if (childRef.isNull())
				{
					bAddElementToThisNode = true;
				}
				else
				{
					if (!node.mChildren[childRef.mIndex])
					{
						node.mChildren[childRef.mIndex] = new typename TOctree<ElementType, OctreeSemantics>::Node(&node);
						setOctreeMemoryUsage(this, mTotalSizeBytes + sizeof(*node.mChildren[childRef.mIndex]));
					}
					nodeIt.pushChild(childRef);
				}
			}
			if (bAddElementToThisNode)
			{
				new(node.mElements)ElementType(element);
				setOctreeMemoryUsage(this, mTotalSizeBytes + sizeof(ElementType));

				OctreeSemantics::setElementId(element, OctreeElementId(&node, node.mElements.size() - 1));
				return;
			}
		}
	}

	template<typename TElement, typename TSemantics>
	FORCEINLINE void setOctreeMemoryUsage(TOctree<TElement, TSemantics>* octree, int32 newSize)
	{
		octree->mTotalSizeBytes = newSize;
	}

	template<typename ElementType, typename OctreeSemantics>
	TOctree<ElementType, OctreeSemantics>::TOctree(const float3& inOrigin, float inExtent)
		:mRootNode(nullptr)
		,mRootNodeContext(BoxCenterAndExtent(inOrigin, float3(inExtent)), 0, 0)
		, mMinLeafExtent(inExtent * Math::pow((1.0f + 1.0f / (float)OctreeNodeContext::LoosenessDenominator) / 2.0f, OctreeSemantics::MaxNodeDepth))
		,mTotalSizeBytes(0)
	{

	}

	FORCEINLINE bool OctreeChildNodeSubset::contains(OctreeChildNodeRef childRef) const
	{
		const OctreeChildNodeSubset childSubSet(childRef);
		return (mChildBits & childSubSet.mChildBits) == childSubSet.mChildBits;
	}

	FORCEINLINE OctreeChildNodeSubset OctreeNodeContext::getIntersectingChildren(const BoxCenterAndExtent& boundingBox) const
	{
		OctreeChildNodeSubset result;
		const VectorRegister queryBoudnsCenter = VectorLoadAligned(&boundingBox.mCenter);
		const VectorRegister queryBoundsExtents = VectorLoadAligned(&boundingBox.mExtent);
		const VectorRegister queryBoundsMax = VectorAdd(queryBoudnsCenter, queryBoundsExtents);

		const VectorRegister queryBoundsMin = VectorSubtract(queryBoudnsCenter, queryBoundsExtents);

		const VectorRegister boundsCenter = VectorLoadAligned(&mBounds.mCenter);
		const VectorRegister boundsExtents = VectorLoadAligned(&mBounds.mExtent);

		const VectorRegister positiveChildBoundsMin = VectorSubtract(VectorAdd(boundsCenter, VectorLoadFloat1(&mChildCenterOffset)), VectorLoadFloat1(&mChildExtent));

		const VectorRegister negativeChildBoundMax = VectorAdd(VectorSubtract(boundsCenter, VectorLoadFloat1(&mChildCenterOffset)), VectorLoadFloat1(&mChildExtent));

		result.bPositiveX = VectorAnyGreaterThan(VectorReplicate(queryBoundsMax, 0), VectorReplicate(positiveChildBoundsMin, 0)) != false;
		result.bPositiveY = VectorAnyGreaterThan(VectorReplicate(queryBoundsMax, 1), VectorReplicate(positiveChildBoundsMin, 1)) != false;
		result.bPositiveZ = VectorAnyGreaterThan(VectorReplicate(queryBoundsMax, 2), VectorReplicate(positiveChildBoundsMin, 2)) != false;

		result.bNegativeX = VectorAnyGreaterThan(VectorReplicate(queryBoundsMin, 0), VectorReplicate(negativeChildBoundMax, 0)) == false;
		result.bNegativeY = VectorAnyGreaterThan(VectorReplicate(queryBoundsMin, 1), VectorReplicate(negativeChildBoundMax, 1)) == false;
		result.bNegativeZ = VectorAnyGreaterThan(VectorReplicate(queryBoundsMin, 2), VectorReplicate(negativeChildBoundMax, 2)) == false;
		return result;
	}

	FORCEINLINE OctreeChildNodeRef OctreeNodeContext::getContainingChild(const BoxCenterAndExtent& boundingBox) const
	{
		OctreeChildNodeRef result;
		const VectorRegister queryBoundsCenter = VectorLoadAligned(&boundingBox.mCenter);
		const VectorRegister queryBoudnsExtent = VectorLoadAligned(&boundingBox.mExtent);

		const VectorRegister boundsCenter = VectorLoadAligned(&mBounds.mCenter);
		const VectorRegister childCenterOffsetVector = VectorLoadFloat1(&mChildCenterOffset);
		const VectorRegister negativeCenterDifference = VectorSubtract(queryBoundsCenter, VectorSubtract(boundsCenter, childCenterOffsetVector));
		const VectorRegister positiveCenterDifference = VectorSubtract(VectorAdd(boundsCenter, childCenterOffsetVector), queryBoundsCenter);

		const VectorRegister minDifference = VectorMin(positiveCenterDifference, negativeCenterDifference);
		if (VectorAnyGreaterThan(VectorAdd(queryBoudnsExtent, minDifference), VectorLoadFloat1(&mChildExtent)))
		{
			result.bNull = true;
		}
		else
		{
			result.x = VectorAnyGreaterThan(VectorReplicate(queryBoundsCenter, 0), VectorReplicate(boundsCenter, 0)) != false;

			result.y = VectorAnyGreaterThan(VectorReplicate(queryBoundsCenter, 1), VectorReplicate(boundsCenter, 1)) != false;

			result.z = VectorAnyGreaterThan(VectorReplicate(queryBoundsCenter, 2), VectorReplicate(boundsCenter, 2)) != false;
		}
		return result;
	}
}