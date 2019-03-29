#include "ShaderCompilerCommon.h"
#include "RHIDefinitions.h"
namespace Air
{
	void buildResourceTableTokenStream(const TArray<uint32>& inResourceMap, int32 maxBoundResourceTable, TArray<uint32>& outTokenStream)
	{
		TArray<uint32> sortedResourceMap = inResourceMap;
		sortedResourceMap.sort();
		outTokenStream.addZeroed(maxBoundResourceTable + 1);
		auto lastBufferIndex = RHIResourceTableEntry::getEndOffStreamToken();
		for (int32 i = 0; i < sortedResourceMap.size(); ++i)
		{
			auto bufferIndex = RHIResourceTableEntry::getConstantBufferIndex(sortedResourceMap[i]);
			if (bufferIndex != lastBufferIndex)
			{
				outTokenStream[bufferIndex] = outTokenStream.size();
				lastBufferIndex = bufferIndex;
			}
			outTokenStream.add(sortedResourceMap[i]);
		}
		if (outTokenStream.size())
		{
			outTokenStream.add(RHIResourceTableEntry::getEndOffStreamToken());
		}
	}
}