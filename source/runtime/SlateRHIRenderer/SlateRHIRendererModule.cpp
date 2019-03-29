#include "Interfaces/ISlateRHIRendererModule.h"
#include "SlateRHIRenderer.h"
#include "Modules/ModuleManager.h"
namespace Air
{
	class SlateRHIRendererModule
		: public ISlateRHIRendererModule
	{
	public:
		SlateRHIRendererModule()
		{
			int x = 0;
			int y = x;
			y = y;
		}

		virtual std::shared_ptr<SlateRenderer> createSlateRHIRenderer() override
		{
			conditionalCreateResource();
			return MakeSharedPtr<SlateRHIRenderer>();
		}

		virtual std::shared_ptr<ISlate3DRenderer> createSlate3DRenderer(bool useGammaCorrect)
		{
			conditionalCreateResource();
			return MakeSharedPtr<ISlate3DRenderer>();
		}

	private:
		void conditionalCreateResource()
		{
		}
	};

	SlateRHIRendererModule m;

	IMPLEMENT_MODULE(SlateRHIRendererModule, SlateRHIRenderer)
}