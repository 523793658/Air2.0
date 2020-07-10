#include "MeshBatch.h"
#include "PrimitiveSceneProxy.h"
#include "RenderUtils.h"
namespace Air
{
	void MeshBatch::preparePrimitiveConstantBuffer(const PrimitiveSceneProxy* primitiveSceneProxy, ERHIFeatureLevel::Type featureLevel)
	{
		const int bVFSupportsPrimtiveIdStream = mVertexFactory->getType()->supportsPrimitiveIdStream();
		BOOST_ASSERT(primitiveSceneProxy->doesVFRequirePrimitiveConstantBuffer() || bVFSupportsPrimtiveIdStream);
		const bool bPrimitiveShaderDataComesFromSceneBuffer = mVertexFactory->getPrimitiveIdStreamIndex(EVertexInputStreamType::Default) >= 0;

		for (int32 elementIndex = 0; elementIndex < mElements.size(); elementIndex++)
		{
			MeshBatchElement& meshElement = mElements[elementIndex];
			if (bPrimitiveShaderDataComesFromSceneBuffer)
			{
				BOOST_ASSERT(!mElements[elementIndex].mPrimitiveConstantBuffer);
			}

			if (!meshElement.mPrimitiveConstantBufferResource && !useGPUScene(GMaxRHIShaderPlatform, featureLevel) && bVFSupportsPrimtiveIdStream)
			{
				meshElement.mPrimitiveConstantBuffer = primitiveSceneProxy->getConstantBuffer();
			}

			BOOST_ASSERT(bPrimitiveShaderDataComesFromSceneBuffer || mElements[elementIndex].mPrimitiveConstantBuffer || mElements[elementIndex].mPrimitiveConstantBufferResource != nullptr);
		}
	}
}