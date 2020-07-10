#include "physx_render_resource_manager.h"

PiRenderResourceManager::~PiRenderResourceManager() {}


UserRenderVertexBuffer*	PiRenderResourceManager::createVertexBuffer(const UserRenderVertexBufferDesc& desc)
{
	return new PiApexRenderVertexBuffer(desc);
}
void PiRenderResourceManager::releaseVertexBuffer(UserRenderVertexBuffer& buffer)
{
	delete &buffer;
}

UserRenderIndexBuffer* PiRenderResourceManager::createIndexBuffer(const UserRenderIndexBufferDesc& desc)
{
	return new PiApexRenderIndexBuffer(desc);
}
void PiRenderResourceManager::releaseIndexBuffer(UserRenderIndexBuffer& buffer)
{
	delete &buffer;
}

UserRenderBoneBuffer* PiRenderResourceManager::createBoneBuffer(const UserRenderBoneBufferDesc& desc)
{
	return new PiApexRenderBoneBuffer(desc);
}
void PiRenderResourceManager::releaseBoneBuffer(UserRenderBoneBuffer& buffer)
{
	delete &buffer;
}

UserRenderInstanceBuffer* PiRenderResourceManager::createInstanceBuffer(const UserRenderInstanceBufferDesc& desc)
{
	return NULL;

}
void PiRenderResourceManager::releaseInstanceBuffer(UserRenderInstanceBuffer& buffer)
{

}

UserRenderSpriteBuffer* PiRenderResourceManager::createSpriteBuffer(const UserRenderSpriteBufferDesc& desc)
{
	return NULL;
}
	/** \brief Release sprite buffer */
void PiRenderResourceManager::releaseSpriteBuffer(UserRenderSpriteBuffer& buffer)
{

}

	/** \brief Create surface buffer */
UserRenderSurfaceBuffer*  	PiRenderResourceManager::createSurfaceBuffer(const UserRenderSurfaceBufferDesc& desc)
{
	return NULL;
}
void PiRenderResourceManager::releaseSurfaceBuffer(UserRenderSurfaceBuffer& buffer)
{

}

UserRenderResource* PiRenderResourceManager::createResource(const UserRenderResourceDesc& desc)
{
	return new PiApexRenderResource(desc);
}

void PiRenderResourceManager::releaseResource(UserRenderResource& resource)
{

}

uint32_t PiRenderResourceManager::getMaxBonesForMaterial(void* material)
{
	return 0;
}


bool PiRenderResourceManager::getSpriteLayoutData(uint32_t spriteCount, uint32_t spriteSemanticsBitmap, UserRenderSpriteBufferDesc* textureDescArray)
{
	return false;
}

bool PiRenderResourceManager::getInstanceLayoutData(uint32_t spriteCount, uint32_t spriteSemanticsBitmap, UserRenderInstanceBufferDesc* instanceDescArray)
{
	return false;
}