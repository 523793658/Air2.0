#include "physx_render_resources.h"
#include "physx_convert.h"
#include "renderlayout.h"
#include "texture.h"
#include "material.h"
#include "UserRenderResourceDesc.h"

PiApexRenderVertexBuffer::PiApexRenderVertexBuffer(const apex::UserRenderVertexBufferDesc& desc)
{
	mNumVertex = desc.maxVerts;
	for (uint32_t i = 0; i < apex::RenderVertexSemantic::NUM_SEMANTICS; i++)
	{
		apex::RenderDataFormat::Enum apexFormat = desc.buffersRequest[i];
		apex::RenderVertexSemantic::Enum apexSemantic = (apex::RenderVertexSemantic::Enum)(i);
		VertexSemantic piSemantic = apex_semantic_to_pi_semantic(apexSemantic);
		if (apexFormat != apex::RenderDataFormat::Enum::UNSPECIFIED)
		{
			EVertexType type;
			uint32 count;
			apex_vertex_format_to_pi_format(apexFormat, &type, &count);
			this->mElements[piSemantic] = pi_vertex_element_new(piSemantic, EVU_DYNAMIC_DRAW, type, count, desc.maxVerts);
			pi_vertex_element_init(this->mElements[piSemantic]);
		}
		else
		{
			mElements[piSemantic] = NULL;
		}
	}
}

PiApexRenderVertexBuffer::~PiApexRenderVertexBuffer()
{
	for (uint32_t i = 0; i < apex::RenderVertexSemantic::NUM_SEMANTICS; i++)
	{
		if (mElements[i] != NULL)
		{
			pi_vertex_element_free(mElements[i]);
		}
	}
}

void PiApexRenderVertexBuffer::writeBuffer(const apex::RenderVertexBufferData& data, uint32_t firstVertex, uint32_t numVertexs)
{
	for (uint32_t i = 0; i < apex::RenderVertexSemantic::NUM_SEMANTICS; i++)
	{
		apex::RenderVertexSemantic::Enum apexSemamtic = (apex::RenderVertexSemantic::Enum)i;
		const apex::RenderSemanticData& semanticeData = data.getSemanticData(apexSemamtic);
		if (semanticeData.data)
		{
			const void* srcData = semanticeData.data;
			const uint32_t srcStride = semanticeData.stride;
			VertexSemantic piSemantic = apex_semantic_to_pi_semantic(apexSemamtic);
			pi_vertex_element_update(mElements[piSemantic], firstVertex, numVertexs, srcData, srcStride);
		}
	}
}


PiApexRenderIndexBuffer::PiApexRenderIndexBuffer(const apex::UserRenderIndexBufferDesc& desc)
{
	mIndicesCount = desc.maxIndices;
	mType = RenderDataFormat::getFormatDataSize(desc.format) == 2 ? EINDEX_16BIT : EINDEX_32BIT;
	mIndexData = pi_index_data_new(mIndicesCount, mType, EVU_DYNAMIC_DRAW);
	pi_index_data_init(mIndexData);
}

PiApexRenderIndexBuffer::~PiApexRenderIndexBuffer()
{
	if (mIndexData)
	{
		pi_index_data_free(mIndexData);
	}
}

void PiApexRenderIndexBuffer::writeBuffer(const void* srcData, uint32_t srcStride, uint32_t firstDestElement, uint32_t numElements)
{

	pi_index_data_update(mIndexData, firstDestElement, numElements, srcData);
}

PiApexRenderBoneBuffer::PiApexRenderBoneBuffer(const UserRenderBoneBufferDesc &desc)
{
	mMaxBones = desc.maxBones;
	mBoneTexture = pi_texture_2d_create(RF_ABGR32F, TU_NORMAL, 1, 1, 4, mMaxBones, TRUE);
	pi_renderstate_set_default_sampler(&mSS);
	pi_sampler_set_texture(&mSS, mBoneTexture);
	pi_sampler_set_filter(&mSS, TFO_CMP_MIN_MAG_MIP_POINT);
	mBoneNumInv = 1.0f / mMaxBones;
}

void PiApexRenderBoneBuffer::writeBuffer(const nvidia::RenderBoneBufferData& data, uint32_t firstBone, uint32_t numBones)
{
	const apex::RenderSemanticData& semanticData = data.getSemanticData(apex::RenderBoneSemantic::POSE);
	if (!semanticData.data)
		return;

	pi_texture_2d_update(mBoneTexture, 0, 0, 0, firstBone, 4, numBones, 16 * 4 * numBones, (byte*)semanticData.data);
}



PiApexRenderResource::PiApexRenderResource(const apex::UserRenderResourceDesc& desc)
{



	mVertexBuffers.resize(desc.numVertexBuffers);
	if (desc.numVertexBuffers > 0)
	{
		for (uint32_t i = 0; i < desc.numVertexBuffers; ++i)
		{
			mVertexBuffers[i] = static_cast<PiApexRenderVertexBuffer*>(desc.vertexBuffers[i]);
		}
	}
	if (desc.indexBuffer)
	{
		mIndexbuffer = static_cast<PiApexRenderIndexBuffer*>(desc.indexBuffer);
	}

	if (desc.boneBuffer)
	{
		mBoneBuffer = static_cast<PiApexRenderBoneBuffer*>(desc.boneBuffer);
	}
	if (desc.spriteBuffer)
	{
		mSpriteBuffer = static_cast<PiApexRenderSpriteBuffer*>(desc.spriteBuffer);
	}
	if (desc.instanceBuffer)
	{
		mInstanceBuffer = static_cast<PiApexRenderInstanceBuffer*>(desc.instanceBuffer);
	}
	mCullMode = desc.cullMode;
	mPrimitiveTopology = desc.primitives;

	setVertexBufferRange(desc.firstVertex, desc.numVerts);
	setIndexBufferRange(desc.firstIndex, desc.numIndices);
	setBoneBufferRange(desc.firstBone, desc.numBones);
	setSpriteBufferRange(desc.firstSprite, desc.numSprites);
	setInstanceBufferRange(desc.firstInstance, desc.numInstances);

	mEntity = pi_entity_new();

	if (desc.material)
	{
		setMaterial(desc.material);

		if (mBoneBuffer)
		{
			pi_material_set_uniform((PiMaterial*)desc.material, "u_BoneTex", UT_SAMPLER_2D, 1, mBoneBuffer->getSampler(), FALSE);
			pi_material_set_uniform((PiMaterial*)desc.material, "u_BoneNumInv", UT_FLOAT, 1, mBoneBuffer->getBoneNumInv(), TRUE);
		}

	}

	VertexElement* elements[apex::RenderVertexSemantic::NUM_SEMANTICS];
	uint32_t element_num = 0;

	for (uint32_t i = 0; i < mVertexBuffers.size(); i++)
	{
		PiApexRenderVertexBuffer* vertex_buffer = mVertexBuffers[i];
		VertexElement** eles = vertex_buffer->getImpls();
		for (uint j = 0; j < apex::RenderVertexSemantic::NUM_SEMANTICS; j++)
		{
			if (eles[j] != NULL)
			{
				elements[element_num] = eles[j];
				element_num++;
			}
		}
	}

	mRenderMesh = pi_rendermesh_new(NULL, FALSE);
	pi_rendermesh_update_by_buffers(mRenderMesh, apex_geometery_to_pi_geometery(mPrimitiveTopology), mIndexbuffer->getImpl(), desc.numVerts, element_num, elements);
	pi_entity_set_mesh(mEntity, mRenderMesh);
}


void PiApexRenderResource::render(const apex::RenderContext& context)
{
	pi_mat4_copy(pi_entity_get_world_matrix(mEntity), (PiMatrix4*)context.local2world.front());
	pi_entity_draw(mEntity);
}

