#ifndef _PiApexRenderVertexBuffer_H_
#define _PiApexRenderVertexBuffer_H_
#include "vector"
#include "UserRenderVertexBuffer.h"
#include "UserRenderIndexBufferDesc.h"
#include "UserRenderIndexBuffer.h"
#include "UserRenderBoneBuffer.h"
#include "UserRenderBoneBufferDesc.h"
#include "UserRenderInstanceBuffer.h"
#include "UserRenderSpriteBuffer.h"
#include "UserRenderResource.h"
#include "RenderContext.h"

#include "pi_renderdata.h"
#include "renderstate.h"
#include "pi_rendermesh.h"
#include "entity.h"
using namespace nvidia;





class PiApexRenderVertexBuffer : public UserRenderVertexBuffer
{
public:
	PiApexRenderVertexBuffer(const apex::UserRenderVertexBufferDesc& desc);
	~PiApexRenderVertexBuffer();

	VertexElement** getImpls()
	{
		return mElements;
	}

	virtual void writeBuffer(const apex::RenderVertexBufferData& data, uint32_t firstVertex, uint32_t numVertexs);

private:
	VertexElement* mElements[apex::RenderVertexSemantic::NUM_SEMANTICS];
	uint32_t mNumVertex;
};

class PiApexRenderIndexBuffer : public UserRenderIndexBuffer
{
public:
	PiApexRenderIndexBuffer(const apex::UserRenderIndexBufferDesc& desc);
	~PiApexRenderIndexBuffer();
	virtual void writeBuffer(const void* srcData, uint32_t srcStride, uint32_t firstDestElement, uint32_t numElements);
	const IndexData* getImpl() const
	{
		return mIndexData;
	}
private:
	uint32_t mIndicesCount;
	uint32_t mStride;
	IndexData* mIndexData{ NULL };
	EIndexType mType;
};

class PiApexRenderBoneBuffer : public UserRenderBoneBuffer
{
public:
	PiApexRenderBoneBuffer(const UserRenderBoneBufferDesc &desc);
	~PiApexRenderBoneBuffer()
	{
	}
	void writeBuffer(const nvidia::RenderBoneBufferData& data, uint32_t firstBone, uint32_t numBones);
	SamplerState* getSampler()
	{
		return &mSS;
	}
	float* getBoneNumInv()
	{
		return &mBoneNumInv;
	}
private:
	uint32_t mMaxBones{ 0 };
	float mBoneNumInv;
	PiTexture* mBoneTexture;
	SamplerState mSS;
};

class PiApexRenderInstanceBuffer : public UserRenderInstanceBuffer
{

};

class PiApexRenderSpriteBuffer : public UserRenderSpriteBuffer
{
public:
	virtual		~PiApexRenderSpriteBuffer() {}
};

class PiApexRenderResource : public UserRenderResource
{
public:
	PiApexRenderResource(const apex::UserRenderResourceDesc& desc);
	virtual ~PiApexRenderResource() {}
	virtual void setVertexBufferRange(uint32_t firstVertex, uint32_t numVerts)
	{
		mFirstVert = firstVertex;
		mNumVertices = numVerts;
	}
	/** \brief Set index buffer range */
	virtual void setIndexBufferRange(uint32_t firstIndex, uint32_t numIndices)
	{
		mFirstIndex = firstIndex;
		mNumIndices = numIndices;
	}
	/** \brief Set bone buffer range */
	virtual void setBoneBufferRange(uint32_t firstBone, uint32_t numBones)
	{
		mFirstBone = firstBone;
		mNumBone = numBones;
	}
	/** \brief Set instance buffer range */
	virtual void setInstanceBufferRange(uint32_t firstInstance, uint32_t numInstances)
	{
		mFirstInstance = firstInstance;
		mNumInstance = numInstances;
	}
	/** \brief Set sprite buffer range */
	virtual void setSpriteBufferRange(uint32_t firstSprite, uint32_t numSprites)
	{
		mFirstSprite = firstSprite;
		mNumSprite = numSprites;
	}
	/** \brief Set sprite visible count */
	virtual void setSpriteVisibleCount(uint32_t visibleCount) { PX_UNUSED(visibleCount); }
	/** \brief Set material */
	virtual void setMaterial(void* material)
	{
		pi_entity_set_material(mEntity, (PiMaterial*)material);
	}

	/** \brief Get number of vertex buffers */
	virtual uint32_t getNbVertexBuffers() const
	{
		return mVertexBuffers.size();
	}
	/** \brief Get vertex buffer */
	virtual UserRenderVertexBuffer*	getVertexBuffer(uint32_t index) const
	{
		return mVertexBuffers[index];
	}
	/** \brief Get index buffer */
	virtual UserRenderIndexBuffer* getIndexBuffer() const
	{
		return mIndexbuffer;
	}
	/** \brief Get bone buffer */
	virtual UserRenderBoneBuffer* getBoneBuffer() const
	{
		return mBoneBuffer;
	}
	/** \brief Get instance buffer */
	virtual UserRenderInstanceBuffer* getInstanceBuffer() const
	{
		return mInstanceBuffer;
	}
	/** \brief Get sprite buffer */
	virtual UserRenderSpriteBuffer*	getSpriteBuffer() const
	{
		return mSpriteBuffer;
	}

public:
	void render(const apex::RenderContext& context);
private:
	std::vector<PiApexRenderVertexBuffer*> mVertexBuffers;
	uint32_t mFirstVert{ 0 };
	uint32_t mNumVertices{ 0 };

	PiApexRenderIndexBuffer* mIndexbuffer{ NULL };
	uint32_t mFirstIndex{ 0 };
	uint32_t mNumIndices{ 0 };

	PiApexRenderBoneBuffer* mBoneBuffer{ NULL };
	uint32_t mFirstBone{ 0 };
	uint32_t mNumBone{ 0 };

	PiApexRenderInstanceBuffer* mInstanceBuffer{ NULL };
	uint32_t mFirstInstance{ 0 };
	uint32_t mNumInstance{ 0 };

	PiApexRenderSpriteBuffer* mSpriteBuffer{ NULL };
	uint32_t mFirstSprite{ 0 };
	uint32_t mNumSprite{ 0 };

	nvidia::apex::RenderPrimitiveType::Enum mPrimitiveTopology;

	nvidia::apex::RenderCullMode::Enum mCullMode{ apex::RenderCullMode::Enum::NONE };
	PiRenderMesh* mRenderMesh{ NULL };
	PiEntity* mEntity;;
};
#endif
