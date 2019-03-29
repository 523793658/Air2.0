#pragma once
#include "CoreMinimal.h"
namespace Air
{
#ifdef _SHADER_COMPILER_COMMON_
#define SHADER_COMPILER_COMMON_API DLLEXPORT
#else
#define SHADER_COMPILER_COMMON_API DLLIMPORT
#endif

	extern SHADER_COMPILER_COMMON_API void buildResourceTableTokenStream(
		const TArray<uint32>& inResourceMap,
		int32 maxBoundResourceTable,
		TArray<uint32>& outTokenStream
	);
}