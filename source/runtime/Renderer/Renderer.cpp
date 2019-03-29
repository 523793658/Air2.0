#include "RendererModule.h"
#include "Modules/ModuleManager.h"
#include "PostProcess/RenderTargetPool.h"
namespace Air
{
	void RendererModule::tickRenderTargetPool()
	{
		GRenderTargetPool.tickPoolElements();
	}


	IMPLEMENT_MODULE(RendererModule, Renderer);

	ICustomCulling* GCustomCullingImpl = nullptr;
}