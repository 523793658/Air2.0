#include "VolumeRendering.h"
namespace Air
{
	TGlobalResource<VolumeRasterizeVertexBuffer> GVolumeRasterizeVertexBuffer;

	RENDERER_API void rasterizeToVolumeTexture(RHICommandList& RHICmdList, VolumeBounds volumeBounds)
	{
		RHICmdList.setViewport(volumeBounds.MinX, volumeBounds.MinY, 0, volumeBounds.MaxX, volumeBounds.MaxY, 0);
		RHICmdList.setStreamSource(0, GVolumeRasterizeVertexBuffer.mVertexBufferRHI, 0);
		const int32 numInstances = volumeBounds.MaxZ - volumeBounds.MinZ;
		RHICmdList.drawPrimitive(0, 2, numInstances);
	}
}