#pragma once
#include "CoreMinimal.h"
#include "RenderResource.h"
#include "sceneRendering.h"
#include "GenericOctree.h"
#include "PrimitiveSceneInfo.h"
#include "Classes/Components/LightComponent.h"
namespace Air
{



	class LightSceneProxy;
	class LightSceneInfo;
	class LightSceneInfoCompact
	{
	public:
		VectorRegister mBoundingSphereVector;
		LinearColor mColor;
		LightSceneInfo* mLightSceneInfo;
		uint32 mLightType : LightType_NumBits;
		uint32 bCastDynamicShadow : 1;
		uint32 bCastStaticShadow : 1;
		uint32 bStaticLighting : 1;
		void init(LightSceneInfo* inLightSceneInfo);

		LightSceneInfoCompact():
			mLightSceneInfo(nullptr)
		{}

		LightSceneInfoCompact(LightSceneInfo* inLightSceneInfo)
		{
			init(inLightSceneInfo);
		}


	};

	
	
	
	


	typedef TOctree<LightSceneInfoCompact, struct LightOctreeSemantics>
		SceneLightOctree;


	class LightSceneInfo : public RenderResource
	{
	public:
		LightSceneInfo(LightSceneProxy* inProxy, bool inbVisible);

		virtual ~LightSceneInfo();

		bool shouldRenderLightViewIndependent() const
		{
			return !mProxy->getColor().isAlmostBlack() && (!mProxy->hasStaticLighting());
		}

		bool shouldRenderLight(const ViewInfo& view) const;

		void addToScene();

		FORCEINLINE BoxCenterAndExtent getBoundingBox() const
		{
			const float extent = mProxy->getRadius();
			return BoxCenterAndExtent(mProxy->getOrigin(), float3(extent, extent, extent));
		}

		void createLightPrimitiveInteraction(const LightSceneInfoCompact& lightSceneInfoCompact, const PrimitiveSceneInfoCompact& primitiveSceneInfoCompact);

	public:

		LightSceneProxy * mProxy;

		uint32 bVisible : 1;

		Scene* mScene;

		int32 mId;

		OctreeElementId mOctreeId;

		mutable const MaterialShaderMap* mTranslucentInjectCachedShaderMaps[LightType_Max][2][2][2];
	};

	struct LightOctreeSemantics
	{
		enum { MaxElementsPerLeaf = 16 };
		enum { MinInclusiveElementsPerNode = 7 };
		enum { MaxNodeDepth = 12 };

		typedef TInlineAllocator<MaxElementsPerLeaf> ElementAllocator;

		FORCEINLINE static BoxCenterAndExtent getBoundingBox(const LightSceneInfoCompact& element)
		{
			return element.mLightSceneInfo->getBoundingBox();
		}

		FORCEINLINE static bool areElementEqual(const LightSceneInfoCompact& A, LightSceneInfoCompact& B)
		{
			return A.mLightSceneInfo == B.mLightSceneInfo;
		}

		FORCEINLINE static void setElementId(const LightSceneInfoCompact& element, OctreeElementId id)
		{
			element.mLightSceneInfo->mOctreeId = id;
		}

		FORCEINLINE static void applyOffset(LightSceneInfoCompact& element, float3 offset)
		{
			VectorRegister offsetReg = VectorLoadFloat3_W0(&offset);
			element.mBoundingSphereVector = VectorAdd(element.mBoundingSphereVector, offsetReg);
		}
	};


	struct SortedLightSceneInfo
	{
		union
		{
			struct  
			{
				uint32 mLightType : LightType_NumBits;
				uint32 bTextureProfile : 1;
				uint32 bLightFunction : 1;
				uint32 bShadowed : 1;
			} mFields;
			int32 mPacked;
		}mSortKey;
		LightSceneInfoCompact mSceneInfo;
		explicit SortedLightSceneInfo(const LightSceneInfoCompact& inSceneInfo)
			:mSceneInfo(inSceneInfo)
		{
			mSortKey.mPacked = 0;
		}

	};
}