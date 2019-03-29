#pragma once
#include "CoreMinimal.h"
#include "MeshBatch.h"
namespace Air
{
	class PrimitiveSceneInfo;

	class StaticMesh : public MeshBatch
	{
	public:
		StaticMesh(PrimitiveSceneInfo* inPrimitiveSceneInfo,
			const MeshBatch& inMesh,
			float inScreenSize,
			HitProxyId inHitProxyId)
			:MeshBatch(inMesh),
			mScreenSize(inScreenSize),
			mPrimitiveSceneInfo(inPrimitiveSceneInfo),
			mId(INDEX_NONE)
		{
			mBatchHitPrixyId = inHitProxyId;
		}

		void addToDrawList(RHICommandListImmediate& RHICmdList, Scene* scene);

		class DrawListElementLink : public RefCountedObject
		{
		public:
			virtual bool isInDrawList(const class StaticMeshDrawListBase* drawList) const = 0;
			virtual void remove(const bool bUnlinkMesh) = 0;
		};

		void linkDrawList(DrawListElementLink* link);
		void unlinkDrawList(DrawListElementLink* link);

		void removeFromDrawList();
	public:
		TArray<TRefCountPtr<DrawListElementLink>> mDrawListLinks;

		float mScreenSize;
		int32 mId;
		PrimitiveSceneInfo* mPrimitiveSceneInfo;
	};
}