#include "PostProcessCombineLUTs.h"
namespace Air
{
	bool pipelineVolumeTextureLUTSupportGuaranteedAtRuntime(EShaderPlatform platform)
	{
		return RHIVolumeTextureRenderingSupportGuaranteed(platform) && (RHISupportsGeometryShaders(platform) || RHISupportsVertexShaderLayer(platform));
	}
}