#ifndef _Physx_Render_Resource_Mananger_H_
#define _Physx_Render_Resource_Mananger_H_
#include "UserRenderResourceManager.h"
#include "physx_render_resources.h"
using namespace nvidia;



class PiRenderResourceManager : public UserRenderResourceManager
{
public:
	virtual								~PiRenderResourceManager();

	/**
	The create methods in this class will only be called from the context of an Renderable::updateRenderResources()
	call, but the release methods can be triggered by any APEX API call that deletes an Actor.  It is up to
	the end-user to make the release methods thread safe.
	*/

	virtual UserRenderVertexBuffer*		createVertexBuffer(const UserRenderVertexBufferDesc& desc);
	/** \brief Release vertex buffer */
	virtual void                      	releaseVertexBuffer(UserRenderVertexBuffer& buffer);

	/** \brief Create index buffer */
	virtual UserRenderIndexBuffer*    	createIndexBuffer(const UserRenderIndexBufferDesc& desc);
	/** \brief Release index buffer */
	virtual void                      	releaseIndexBuffer(UserRenderIndexBuffer& buffer);

	/** \brief Create bone buffer */
	virtual UserRenderBoneBuffer*     	createBoneBuffer(const UserRenderBoneBufferDesc& desc);
	/** \brief Release bone buffer */
	virtual void                      	releaseBoneBuffer(UserRenderBoneBuffer& buffer);

	/** \brief Create instance buffer */
	virtual UserRenderInstanceBuffer* 	createInstanceBuffer(const UserRenderInstanceBufferDesc& desc);
	/** \brief Release instance buffer */
	virtual void                        releaseInstanceBuffer(UserRenderInstanceBuffer& buffer);

	/** \brief Create sprite buffer */
	virtual UserRenderSpriteBuffer*   	createSpriteBuffer(const UserRenderSpriteBufferDesc& desc);
	/** \brief Release sprite buffer */
	virtual void                        releaseSpriteBuffer(UserRenderSpriteBuffer& buffer);

	/** \brief Create surface buffer */
	virtual UserRenderSurfaceBuffer*  	createSurfaceBuffer(const UserRenderSurfaceBufferDesc& desc);
	/** \brief Release surface buffer */
	virtual void                        releaseSurfaceBuffer(UserRenderSurfaceBuffer& buffer);

	/** \brief Create resource */
	virtual UserRenderResource*       	createResource(const UserRenderResourceDesc& desc);

	/**
	releaseResource() should not release any of the included buffer pointers.  Those free methods will be
	called separately by the APEX SDK before (or sometimes after) releasing the UserRenderResource.
	*/
	virtual void                        releaseResource(UserRenderResource& resource);

	/**
	Get the maximum number of bones supported by a given material. Return 0 for infinite.
	For optimal rendering, do not limit the bone count (return 0 from this function).
	*/
	virtual uint32_t                    getMaxBonesForMaterial(void* material);


	/** \brief Get the sprite layout data
	Returns true in case textureDescArray is set.
	In case user is not interested in setting specific layout for sprite PS,
	this function should return false.
	*/
	virtual bool 						getSpriteLayoutData(uint32_t spriteCount, uint32_t spriteSemanticsBitmap, UserRenderSpriteBufferDesc* textureDescArray);

	/** \brief Get the instance layout data
	Returns true in case textureDescArray is set.
	In case user is not interested in setting specific layout for sprite PS,
	this function should return false.
	*/
	virtual bool 						getInstanceLayoutData(uint32_t spriteCount, uint32_t spriteSemanticsBitmap, UserRenderInstanceBufferDesc* instanceDescArray);
};

#endif
