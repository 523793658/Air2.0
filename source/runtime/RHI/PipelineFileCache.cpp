#include "PipelineFileCache.h"
#include "RHIResource.h"
#include "PipelineStateCache.h"
namespace Air
{
	void RHIComputeShader::updateStats()
	{
		PipelineStateStats::updateStats(mStats);
	}
}