#pragma once
#include "CoreMinimal.h"
#include "Containers/BitArray.h"
#include "RendererInterface.h"
namespace Air
{
	class SceneRenderingBitArrayAllocator
		:public TInlineAllocator<4, SceneRenderingAllocator>
	{

	};

	typedef TBitArray<SceneRenderingBitArrayAllocator> SceneBitArray;
	typedef TConstSetBitIterator<SceneRenderingBitArrayAllocator> SceneSetBitIterator;
}