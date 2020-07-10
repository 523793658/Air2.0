#ifndef _Physx_Renderer_H_
#define _Physx_Renderer_H_
#include <Apex.h>

using namespace nvidia::apex;

class PiApexRenderer : public UserRenderer
{
public:
	PiApexRenderer();
	~PiApexRenderer();

	/**
	\brief Render a resource
	Renderable::dispatchRenderResouces() will call this
	function as many times as possible to render all of the actor's
	sub-meshes.
	*/
	virtual void renderResource(const RenderContext& context);
};

#endif